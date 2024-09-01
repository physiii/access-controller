var url = "ws://" + location.host + '/ws';
var webSocket = new WebSocket(url);

webSocket.onopen = function (event) {
    // Request current device info (including room name) on connection
    setTimeout(() => {
        webSocket.send("{ \"eventType\":\"getInfo\", \"payload\": {\"getInfo\": true}}");
    }, 1250);
};

// Event listener for setting the room name
document.getElementById('roomForm').addEventListener('submit', function(e) {
    e.preventDefault(); // Prevents the default form submission behavior
    const roomName = document.getElementById('roomNameInput').value;

    // Send the room name to your server using WebSocket
    webSocket.send(JSON.stringify({
        eventType: "setRoomName",
        payload: {
            roomName: roomName
        }
    }));
});

// Handling incoming WebSocket messages
webSocket.onmessage = function (event) {
    let state = JSON.parse(event.data);
    let pl = state.payload;

    if (!event.data) return;

    if (state.eventType == "getInfo") {
        // Update room name in the UI
        if (pl.room_name) {
            document.getElementById('roomName').textContent = pl.room_name;
        }
        // Update other device info
        if (pl.uuid) {
            document.getElementById('uuid').textContent = pl.uuid;
        }
    }

    // Other eventType handling...
};

document.getElementById('enableLock_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('polarity_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"polarity\": " + this.checked + "}}");
};

document.getElementById('arm_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"arm\": " + this.checked + "}}");
};

document.getElementById('enableContactAlert_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"enableContactAlert\": " + this.checked + "}}");
};

document.getElementById('enableExit_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertExit_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 1, \"alert\": " + this.checked + "}}");
};

document.getElementById('enableFob_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertFob_1').onclick = function() {
    webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 1, \"alert\": " + this.checked + "}}");
};

document.getElementById('enableLock_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"enable\": " + this.checked + "}}");
};

document.getElementById('polarity_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"polarity\": " + this.checked + "}}");
};

document.getElementById('arm_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"arm\": " + this.checked + "}}");
};

document.getElementById('enableContactAlert_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"enableContactAlert\": " + this.checked + "}}");
};

document.getElementById('enableExit_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 2, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertExit_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 2, \"alert\": " + this.checked + "}}");
};

document.getElementById('enableFob_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 2, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertFob_2').onclick = function() {
    webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 2, \"alert\": " + this.checked + "}}");
};

document.getElementById('wifiForm').addEventListener('submit', function(e) {
    e.preventDefault(); // Prevents the default form submission behavior
    const wifiName = document.getElementById('wifiName').value;
    const wifiPassword = document.getElementById('wifiPassword').value;

    // Send the WiFi credentials to your server using WebSocket
    webSocket.send(JSON.stringify({
        eventType: "setWifiCredentials",
        payload: {
            wifiName: wifiName,
            wifiPassword: wifiPassword
        }
    }));
});

document.getElementById('serverForm').addEventListener('submit', function(e) {
    e.preventDefault(); // Prevents the default form submission behavior
    const ipAddress = document.getElementById('ipAddress').value;
    const port = document.getElementById('port').value;

    // Send the server info to your server using WebSocket
    webSocket.send(JSON.stringify({
        eventType: "setServerInfo",
        payload: {
            serverIp: ipAddress,
            serverPort: port
        }
    }));
});
