#include "web_server.h"
#include <ArduinoJson.h>

WebServer::WebServer(Sensor& sensor, DataHandler& data)
    : sensor(sensor), dataHandler(data) {
    server = new WebServer(80);
}

void WebServer::begin() {
    // Route handlers
    server->on("/", [this](){
        handleRoot();
    });
    
    server->on("/data.json", [this](){
        handleDataJson();
    });
    
    server->onNotFound([this](){
        handleNotFound();
    });
    
    server->begin();
}

void WebServer::handleClient() {
    server->handleClient();
}

void WebServer::handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>🌱 Plant Moisture Monitor</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            margin: 0; 
            padding: 20px; 
            background: linear-gradient(135deg, #667986 0%, #4a606e 100%); 
            color: #333;
        }
        .container { 
            max-width: 1200px; 
            margin: 0 auto; 
            background: rgba(255, 255, 255, 0.95); 
            border-radius: 15px; 
            padding: 20px; 
            box-shadow: 0 10px 30px rgba(0,0,0,0.2); 
        }
        h1 { 
            text-align: center; 
            color: #2e8b57; 
            text-shadow: 1px 1px 2px rgba(0,0,0,0.1); 
            margin-top: 0;
        }
        .dashboard { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); 
            gap: 20px; 
            margin: 20px 0; 
        }
        .card { 
            background: white; 
            border-radius: 10px; 
            padding: 20px; 
            box-shadow: 0 4px 8px rgba(0,0,0,0.1); 
            text-align: center; 
            transition: transform 0.3s ease; 
        }
        .card:hover { 
            transform: translateY(-5px); 
            box-shadow: 0 8px 16px rgba(0,0,0,0.15); 
        }
        .card h3 { 
            margin-top: 0; 
            color: #2e8b57; 
        }
        .reading { 
            font-size: 2em; 
            font-weight: bold; 
            color: #2e8b57; 
        }
        .chart-container { 
            width: 100%; 
            height: 400px; 
            margin-top: 20px; 
        }
        .status { 
            padding: 10px; 
            border-radius: 5px; 
            text-align: center; 
            margin: 10px 0; 
        }
        .normal { 
            background-color: #90ee90; 
        }
        .warning { 
            background-color: #ffcccb; 
        }
        .critical { 
            background-color: #ff6347; 
        }
        .footer { 
            text-align: center; 
            margin-top: 20px; 
            color: #666; 
            font-size: 0.9em; 
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 Plant Moisture Dashboard</h1>
        <div class="dashboard">
            <div class="card">
                <h3>Current Moisture</h3>
                <div class="reading" id="currentReading">--</div>
                <p>Raw reading</p>
            </div>
            <div class="card">
                <h3>Status</h3>
                <div class="reading" id="statusReading">--</div>
                <p>Current Status</p>
            </div>
            <div class="card">
                <h3>Last Updated</h3>
                <div class="reading" id="lastUpdate">--</div>
                <p>System Time</p>
            </div>
        </div>
        
        <div class="chart-container">
            <canvas id="moistureChart"></canvas>
        </div>
        
        <div class="footer">
            <p>Plant Moisture Monitoring System &copy; 2023</p>
        </div>
    </div>

    <script>
        // Create Chart.js instance
        const ctx = document.getElementById('moistureChart').getContext('2d');
        let moistureChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Soil Moisture Level',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.2)',
                    tension: 0.1,
                    fill: false,
                    pointRadius: 3
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 4095,
                        min: 0
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

        // Update data function
        function updateData() {
            fetch('/data.json')
            .then(response => response.json())
            .then(data => {
                // Update HTML elements
                document.getElementById('currentReading').textContent = data.moisture;
                
                // Update status
                const statusElement = document.getElementById('statusReading');
                if (data.moisture < 55) {
                    statusElement.textContent = 'DRY';
                    statusElement.className = 'reading warning';
                } else {
                    statusElement.textContent = 'NORMAL';
                    statusElement.className = 'reading normal';
                }
                
                // Update timestamp
                document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
                
                // Update chart with new data
                if (data.history && data.history.length > 0) {
                    moistureChart.data.labels = data.history.map(d => d.time);
                    moistureChart.data.datasets[0].data = data.history.map(d => d.value);
                    moistureChart.update();
                }
            })
            .catch(error => {
                console.error('Error:', error);
            });
        }

        // Update every 5 seconds
        setInterval(updateData, 5000);
        updateData();  // Initial call
    </script>
</body>
</html>
    )rawliteral";
    
    server->send(200, "text/html", html);
}

void WebServer::handleDataJson() {
    // This would return JSON data
    String json = "{";
    json += "\"moisture\": 0,";
    json += "\"timestamp\": \"" + String(millis()) + "\""; 
    json += "}";
    
    server->send(200, "application/json", json);
}

void WebServer::handleNotFound() {
    server->send(404, "text/plain", "Not found");
}
