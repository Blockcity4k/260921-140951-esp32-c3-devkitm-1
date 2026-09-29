#include "web_server.h"
#include "config.h"
#include <ArduinoJson.h>
#include <vector>
/*
CHANGE: graph scale lower like 0 to 100
*/
#define STRINGIFY_VALUE(value) #value
#define STRINGIFY(value) STRINGIFY_VALUE(value)

namespace {
int estimateMoisturePercent(int rawReading) {
    const int rawPoints[] = {
        MOISTURE_RAW_DRY,
        MOISTURE_RAW_25,
        MOISTURE_RAW_50,
        MOISTURE_RAW_75,
        MOISTURE_RAW_100
    };
    const int percentPoints[] = {0, 25, 50, 75, 100};

    const int dryRaw = rawPoints[0];
    const int wetRaw = rawPoints[4];
    if (dryRaw == wetRaw) {
        return 0;
    }
    if ((dryRaw > wetRaw && rawReading >= dryRaw) ||
        (dryRaw < wetRaw && rawReading <= dryRaw)) {
        return 0;
    }
    if ((dryRaw > wetRaw && rawReading <= wetRaw) ||
        (dryRaw < wetRaw && rawReading >= wetRaw)) {
        return 100;
    }

    for (int i = 0; i < 4; ++i) {
        const int first = rawPoints[i];
        const int second = rawPoints[i + 1];
        if ((rawReading >= min(first, second) && rawReading <= max(first, second)) && first != second) {
            return percentPoints[i] +
                (rawReading - first) * (percentPoints[i + 1] - percentPoints[i]) / (second - first);
        }
    }

    return 0;
}
}

MyWebServer::MyWebServer(Sensor& sensor, DataHandler& data)
    : sensor(sensor), dataHandler(data) {
    server = new AsyncWebServer(80);
}

void MyWebServer::begin() {
    server->on("/", [this](AsyncWebServerRequest *request){
        handleRoot(request);
    });
    
    server->on("/data.json", [this](AsyncWebServerRequest *request){
        handleDataJson(request);
    });

    server->on("/history.json", [this](AsyncWebServerRequest *request){
        handleHistoryJson(request);
    });
    
    server->on("/data.html", [this](AsyncWebServerRequest *request){
        handleDataHtml(request);
    });
    
    server->onNotFound([this](AsyncWebServerRequest *request){
        handleNotFound(request);
    });
    
    server->begin();
}

void MyWebServer::handleClient() {
    // No need to call server->handleClient() as it's handled by the framework
    // The actual handling is done by the ESPAsyncWebServer library
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
    // Send a simple HTML page with embedded CSS/JS for the graph
    const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Plant Monitor</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f0f8ff; margin: 0; padding: 20px; }
        .container { max-width: 1200px; margin: 0 auto; background: white; border-radius: 10px; padding: 20px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        h1 { color: #2c7744; text-align: center; }
        .dashboard { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 20px; margin-bottom: 20px; }
        .sensor-card { background: #e8f5e9; padding: 15px; border-radius: 8px; text-align: center; border-left: 5px solid #4caf50; }
        .sensor-value { font-size: 2em; font-weight: bold; color: #1b5e20; }
        .sensor-label { font-size: 0.9em; color: #666; }
        .graph-container { grid-column: 1 / -1; }
        .plot { width: 100%; height: 320px; display: block; background: #fff; }
        .controls { text-align: center; margin: 20px 0; }
        button { background-color: #4CAF50; color: white; border: none; padding: 12px 20px; cursor: pointer; border-radius: 4px; font-size: 16px; margin: 5px; }
        button:hover { background-color: #45a049; }
        .status { text-align: center; padding: 10px; border-radius: 4px; margin: 10px 0; }
        .connected { background-color: #d4edda; color: #155724; }
        .disconnected { background-color: #f8d7da; color: #721c24; }
        #last-update { text-align: center; color: #666; margin-top: 10px; }
        @media (max-width: 600px) {
            body { padding: 10px; }
            .dashboard { grid-template-columns: 1fr; }
            .sensor-value { font-size: 1.6em; }
            .plot { height: 260px; }
        }
    </style>
</head>
<body>
    <div class="container">

        <!-- visible, local title based on user input and stays even on refresh  -->

        <h1 id="dashboardTitle" contenteditable="true">Enter title here!</h1>
        <script>
            const title = document.getElementById("dashboardTitle");

            const savedTitle = localStorage.getItem("plantTitle");

            if (savedTitle) {
                title.textContent = savedTitle;
            }

            title.addEventListener("input", function() {
                localStorage.setItem("plantTitle", title.textContent);
            });
        </script>

        <!-- display sensor connection status -->

        <div id="status" class="status">Connecting to sensor...</div>

        <!-- dashboard -->

        <div class="dashboard">

            <!-- estimated moisture card -->

            <div class="sensor-card">
                <div class="sensor-label">Estimated Moisture</div>
                <div id="moisture-value" class="sensor-value">--</div>
                <div class="sensor-label">calibrated estimate</div>
            </div>

            <!-- raw ADC reading card -->

            <div class="sensor-card">
                <div class="sensor-label">Raw ADC Reading</div>
                <div id="raw-value" class="sensor-value">--</div>
                <div class="sensor-label">0 to 4095</div>
            </div>

            <!-- last reading card -->

            <div class="sensor-card">
                <div class="sensor-label">Last Reading</div>
                <div id="timestamp" class="sensor-value">--</div>
                <div class="sensor-label">seconds ago</div>
            </div>
        </div>

        <!-- graph -->

        <div class="graph-container">
            <h2>Sensor History</h2>
            <canvas id="rawChart" class="plot" aria-label="Raw sensor readings over time"></canvas>
            <div id="plot-range">Waiting for sensor readings</div>
        </div>
        <div class="controls">
            <button onclick="refreshData()">Reset Graph</button>
        </div>
        <div id="last-update">Last updated: --:--:--</div>
    </div>

    <script>
        const POLL_INTERVAL_MS = )rawliteral" STRINGIFY(UPDATE_INTERVAL) R"rawliteral(;
        var isConnected = false;
        var lastReadingAgeMs = null;
        var ageReceivedAt = 0;
        var readings = [];
        var lastPlottedTimestamp = null;
        var maxPlotReadings = 80;
        var totalReadings = 0;

        function updatePlotLabel() {
            document.getElementById('plot-range').textContent =
                'Showing latest ' + readings.length + ' plotted readings; ' +
                totalReadings + ' readings captured since boot';
        }

        // data graph config
        function drawPlot() {
            const canvas = document.getElementById('rawChart');
            const bounds = canvas.getBoundingClientRect();
            if (!bounds.width || !bounds.height) return;

            const pixelRatio = window.devicePixelRatio || 1;
            canvas.width = Math.round(bounds.width * pixelRatio);
            canvas.height = Math.round(bounds.height * pixelRatio);
            const context = canvas.getContext('2d');
            context.scale(pixelRatio, pixelRatio);

            const width = bounds.width;
            const height = bounds.height;
            const left = 52;
            const right = width - 12;
            const top = 12;
            const bottom = height - 28;
            const graphHeight = bottom - top;

            context.clearRect(0, 0, width, height);
            context.font = '12px sans-serif';
            context.textAlign = 'right';
            context.textBaseline = 'middle';
            context.strokeStyle = '#d8e2dc';
            context.fillStyle = '#52645a';

            for (let step = 0; step <= 4; step++) {
                const value = Math.round(4095 - step * 4095 / 4);
                const y = top + step * graphHeight / 4;
                context.beginPath();
                context.moveTo(left, y);
                context.lineTo(right, y);
                context.stroke();
                context.fillText(String(value), left - 8, y);
            }

            context.textAlign = 'left';
            context.textBaseline = 'alphabetic';
            context.fillText('Older', left, height - 5);
            context.textAlign = 'right';
            context.fillText('Newer', right, height - 5);

            if (readings.length === 0) {
                updatePlotLabel();
                return;
            }

            context.strokeStyle = '#21834a';
            context.lineWidth = 2;
            context.beginPath();
            readings.forEach(function(reading, index) {
                const x = readings.length === 1
                    ? (left + right) / 2
                    : left + index * (right - left) / (readings.length - 1);
                const y = bottom - Math.max(0, Math.min(4095, reading.raw)) * graphHeight / 4095;
                if (index === 0) context.moveTo(x, y);
                else context.lineTo(x, y);
            });
            context.stroke();

            updatePlotLabel();
        }

        function updateStatus() {
            const status = document.getElementById('status');
            status.className = isConnected ? 'status connected' : 'status disconnected';
            status.textContent = isConnected ? 'Connected to ESP32 Sensor' : 'No connection to ESP32';
        }

        function updateReadingAge() {
            const timestamp = document.getElementById('timestamp');
            if (lastReadingAgeMs === null) {
                timestamp.textContent = '--';
                return;
            }

            const elapsedMs = Date.now() - ageReceivedAt;
            timestamp.textContent = Math.floor((lastReadingAgeMs + elapsedMs) / 1000);
        }

        // Function to fetch data from ESP32 and update the dashboard
        async function fetchData() {
            try {
                const response = await fetch('/data.json', { cache: 'no-store' });
                if (!response.ok) {
                    throw new Error('HTTP ' + response.status);
                }
                const data = await response.json();
                totalReadings = Number(data.totalReadings) || totalReadings;
                updateDashboard(data);
                updatePlotLabel();
            } catch (error) {
                console.error('Error fetching data:', error);
                isConnected = false;
                updateStatus();
            }
        }

        // Update the dashboard with new data
        function updateDashboard(data) {
            isConnected = true;
            document.getElementById('moisture-value').textContent = data.hasReading
                ? data.moisturePercent + '%'
                : '--';
            document.getElementById('raw-value').textContent = data.hasReading
                ? data.raw
                : '--';

            if (data.hasReading && data.timestamp !== lastPlottedTimestamp) {
                readings.push({ timestamp: data.timestamp, raw: Number(data.raw) });
                if (readings.length > maxPlotReadings) readings.shift();
                lastPlottedTimestamp = data.timestamp;
                drawPlot();
            }

            if (data.hasReading && Number.isFinite(Number(data.ageMs))) {
                lastReadingAgeMs = Number(data.ageMs);
                ageReceivedAt = Date.now();
                updateReadingAge();
            } else {
                lastReadingAgeMs = null;
                updateReadingAge();
            }

            document.getElementById('last-update').textContent = 'Last updated: ' + new Date().toLocaleTimeString();
            updateStatus();
        }

        // Clear the plotted history and immediately fetch the current reading.
        function refreshData() {
            readings = [];
            lastPlottedTimestamp = null;
            drawPlot();
            fetchData();
        }

        async function loadHistory() {
            try {
                const response = await fetch('/history.json', { cache: 'no-store' });
                if (!response.ok) throw new Error('HTTP ' + response.status);
                const history = await response.json();
                totalReadings = Number(history.totalReadings) || 0;
                readings = (history.samples || []).map(function(sample) {
                    return { timestamp: sample[0], raw: Number(sample[1]) };
                }).slice(-maxPlotReadings);
                lastPlottedTimestamp = readings.length
                    ? readings[readings.length - 1].timestamp
                    : null;
                drawPlot();
            } catch (error) {
                console.error('Error loading sensor history:', error);
            }
        }

        // Monitor continuously at the same interval used by the sensor loop.
        window.addEventListener('load', async function() {
            await loadHistory();
            await fetchData();
            setInterval(fetchData, Math.max(1, POLL_INTERVAL_MS));
            setInterval(updateReadingAge, 1000);
            window.addEventListener('resize', drawPlot);
        });
    </script>
</body>
</html>
    )rawliteral";
    
    request->send(200, "text/html", html);
}

void MyWebServer::handleDataJson(AsyncWebServerRequest *request) {
    JsonDocument doc;
    int moisture = dataHandler.getLatestReading();
    unsigned long timestamp = dataHandler.getLatestTimestamp();

    bool hasReading = moisture != -1;
    doc["hasReading"] = hasReading;
    doc["totalReadings"] = dataHandler.getTotalReadingCount();
    doc["raw"] = hasReading ? moisture : 0;
    doc["moisturePercent"] = hasReading ? estimateMoisturePercent(moisture) : 0;
    if (hasReading) {
        doc["ageMs"] = millis() - timestamp;
    } else {
        doc["ageMs"] = -1;
    }
    doc["timestamp"] = timestamp;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void MyWebServer::handleHistoryJson(AsyncWebServerRequest *request) {
    const int availableReadings = dataHandler.getReadingCount();
    std::vector<int> rawReadings(availableReadings);
    std::vector<unsigned long> timestamps(availableReadings);
    const int readingCount = dataHandler.getRecentReadings(
        availableReadings, rawReadings.data(), timestamps.data());

    JsonDocument doc;
    doc["totalReadings"] = dataHandler.getTotalReadingCount();
    JsonArray samples = doc["samples"].to<JsonArray>();
    for (int index = 0; index < readingCount; ++index) {
        JsonArray sample = samples.add<JsonArray>();
        sample.add(timestamps[index]);
        sample.add(rawReadings[index]);
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void MyWebServer::handleDataHtml(AsyncWebServerRequest *request) {
    // This method is now handled by the HTML page itself through JavaScript calls
    const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Plant Monitor - Data</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f0f8ff; margin: 0; padding: 20px; }
        .container { max-width: 1200px; margin: 0 auto; background: white; border-radius: 10px; padding: 20px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        h1 { color: #2c7744; text-align: center; }
        .sensor-data { text-align: center; font-size: 1.2em; margin: 20px 0; }
        .sensor-value { font-size: 3em; font-weight: bold; color: #1b5e20; }
        .sensor-label { color: #666; }
        .chart-container { width: 100%; height: 400px; }
        .controls { text-align: center; margin: 20px 0; }
        button { background-color: #4CAF50; color: white; border: none; padding: 12px 20px; cursor: pointer; border-radius: 4px; font-size: 16px; margin: 5px; margin: 5px; }
        button:hover { background-color: #45a049; }
        .status { text-align: center; padding: 10px; border-radius: 4px; margin: 10px 0; }
        .connected { background-color: #d4edda; color: #155724; }
        .disconnected { background-color: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Plant Monitor</h1>
        <div id="status" class="status">Connecting to sensor...</div>
        <div class="sensor-data">
            <div class="sensor-label">Current Moisture Level</div>
            <div id="moisture-value" class="sensor-value">--</div>
            <div class="sensor-label">percentage</div>
        </div>
        <div class="chart-container">
            <canvas id="moistureChart"></canvas>
        </div>
        <div class="controls">
            <button onclick="startMonitoring()">Start Monitoring</button>
            <button onclick="stopMonitoring()">Stop Monitoring</button>
            <button onclick="refreshData()">Refresh Data</button>
        </div>
    </div>
</body>
</html>
    )rawliteral";
    
    request->send(200, "text/html", html);
}

void MyWebServer::handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

