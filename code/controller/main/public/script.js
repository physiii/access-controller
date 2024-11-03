document.addEventListener('DOMContentLoaded', function() {
    var url = "ws://" + location.host + '/ws';
    var webSocket;
    var deviceSettingsDiv = document.getElementById('device-settings');
    var appContentDiv = document.getElementById('app-content');

    function connectWebSocket() {
        webSocket = new WebSocket(url);

        webSocket.onopen = function (event) {
            console.log("WebSocket connected");
            webSocket.send(JSON.stringify({ "eventType": "getDeviceInfo", "payload": {"getState": true} }));
        };

        webSocket.onclose = function(event) {
            console.log("WebSocket disconnected. Reconnecting in 5 seconds...");
            setTimeout(connectWebSocket, 5000);
        };

        webSocket.onerror = function(error) {
            console.error("WebSocket error:", error);
        };

        webSocket.onmessage = function (event) {
            var data = JSON.parse(event.data);
            console.log("[WebSocket]", data);

            if (data.event_type === "load" && data.payload && data.payload.services && data.payload.services.length > 0) {
                // Extract the state object from the first service in the array
                var deviceState = data.payload.services[0].state;
                updateDeviceInfo(deviceState);
                
                if (window.appFunctions && window.appFunctions.handleAppWebSocketMessage) {
                    window.appFunctions.handleAppWebSocketMessage(data);
                }
            } else if (data.eventType === "updateDeviceInfo") {
                updateDeviceInfo(data.payload);
            }
        };

        window.webSocket = webSocket;
    }

    connectWebSocket();

    // Navigation
    document.querySelectorAll('.nav-link').forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            var target = this.getAttribute('data-target');
            loadContent(target);
        });
    });

    function loadContent(target) {
        localStorage.setItem('lastView', target);
        if (target === 'app') {
            deviceSettingsDiv.style.display = 'none';
            appContentDiv.style.display = 'block';
            fetch('/app.html')
                .then(response => response.text())
                .then(html => {
                    appContentDiv.innerHTML = html;
                    loadScript('/app.js', () => {
                        if (window.appFunctions && window.appFunctions.initializeApp) {
                            window.appFunctions.initializeApp();
                        }
                    });
                });
        } else if (target === 'device') {
            appContentDiv.style.display = 'none';
            deviceSettingsDiv.style.display = 'block';
            initializeDeviceSettings();
        }
    }

    function loadScript(url, callback) {
        var script = document.createElement('script');
        script.type = 'text/javascript';
        script.src = url;
        script.onload = callback;
        document.head.appendChild(script);
    }

    function initializeDeviceSettings() {
        var wifiForm = document.getElementById('wifiForm');
        var serverForm = document.getElementById('serverForm');

        if (wifiForm) {
            wifiForm.addEventListener('submit', function(e) {
                e.preventDefault();
                var wifiName = document.getElementById('wifiName').value;
                var wifiPassword = document.getElementById('wifiPassword').value;
                webSocket.send(JSON.stringify({
                    "eventType": "setWifiCredentials",
                    "payload": { "wifiName": wifiName, "wifiPassword": wifiPassword }
                }));
                console.log("WiFi settings submitted");
            });
        }

        if (serverForm) {
            serverForm.addEventListener('submit', function(e) {
                e.preventDefault();
                var ipAddress = document.getElementById('serverIpAddress').value;
                var port = document.getElementById('port').value;
                webSocket.send(JSON.stringify({
                    "eventType": "setServerInfo",
                    "payload": { "ipAddress": ipAddress, "port": port }
                }));
                console.log("Server info submitted");
            });
        }
    }

    window.editDeviceName = function() {
        document.getElementById('deviceName').style.display = 'none';
        document.getElementById('deviceNameInput').style.display = 'inline';
        document.getElementById('deviceNameInput').value = document.getElementById('deviceName').textContent;
        document.getElementById('editDeviceNameBtn').style.display = 'none';
        document.getElementById('saveDeviceNameBtn').style.display = 'inline';
    }

    window.saveDeviceName = function() {
        var newDeviceName = document.getElementById('deviceNameInput').value;
        webSocket.send(JSON.stringify({
            "eventType": "setDeviceName",
            "payload": { "deviceName": newDeviceName }
        }));
        console.log("Device name submitted: " + newDeviceName);
        document.getElementById('deviceName').textContent = newDeviceName;
        document.getElementById('deviceName').style.display = 'inline';
        document.getElementById('deviceNameInput').style.display = 'none';
        document.getElementById('saveDeviceNameBtn').style.display = 'none';
        document.getElementById('editDeviceNameBtn').style.display = 'inline';
    }

    window.editDeviceRoom = function() {
        document.getElementById('deviceRoom').style.display = 'none';
        document.getElementById('deviceRoomInput').style.display = 'inline';
        document.getElementById('deviceRoomInput').value = document.getElementById('deviceRoom').textContent;
        document.getElementById('editDeviceRoomBtn').style.display = 'none';
        document.getElementById('saveDeviceRoomBtn').style.display = 'inline';
    }

    window.saveDeviceRoom = function() {
        var newDeviceRoom = document.getElementById('deviceRoomInput').value;
        webSocket.send(JSON.stringify({
            "eventType": "setDeviceRoom",
            "payload": { "deviceRoom": newDeviceRoom }
        }));
        console.log("Device room submitted: " + newDeviceRoom);
        document.getElementById('deviceRoom').textContent = newDeviceRoom;
        document.getElementById('deviceRoom').style.display = 'inline';
        document.getElementById('deviceRoomInput').style.display = 'none';
        document.getElementById('saveDeviceRoomBtn').style.display = 'none';
        document.getElementById('editDeviceRoomBtn').style.display = 'inline';
    }

    function updateDeviceInfo(info) {
        console.log("Updating device info:", info);
        if (info.device_id) document.getElementById('deviceID').textContent = info.device_id;
        if (info.ip_address) document.getElementById('ipAddress').textContent = info.ip_address;
        if (info.device_name) document.getElementById('deviceName').textContent = info.device_name;
        if (info.room_name) document.getElementById('deviceRoom').textContent = info.room_name;
        if (info.mac_address) document.getElementById('macAddress').textContent = info.mac_address;
    }

    // Load last viewed content or default to app
    var lastView = localStorage.getItem('lastView') || 'app';
    loadContent(lastView);
});
