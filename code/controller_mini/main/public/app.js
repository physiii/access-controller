window.appFunctions = {
    initializeApp: function() {
        console.log("Initializing Access Controller App");
        this.initializeControls();
        this.connectWebSocket();  // Establish WebSocket connection here
    },

    initializeControls: function() {
        // Control initialization for Channel 1
        document.getElementById('enableLock_1').onclick = function() {
            webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
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
        document.getElementById('latchFob_1').onclick = function() {
            webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 1, \"latch\": " + this.checked + "}}");
        };
        document.getElementById('polarity_1').onclick = function() {
            webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"polarity\": " + this.checked + "}}");
        };

        // Control initialization for Channel 2
        document.getElementById('enableLock_2').onclick = function() {
            webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"enable\": " + this.checked + "}}");
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
        document.getElementById('latchFob_2').onclick = function() {
            webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 2, \"latch\": " + this.checked + "}}");
        };
        document.getElementById('polarity_2').onclick = function() {
            webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"polarity\": " + this.checked + "}}");
        };
    },

    connectWebSocket: function() {
        const url = "ws://" + location.host + '/ws';
        window.webSocket = new WebSocket(url);

        window.webSocket.onopen = function() {
            console.log("WebSocket connected");
            // Wait until the WebSocket is fully open before sending any messages
            if (window.webSocket.readyState === WebSocket.OPEN) {
                window.webSocket.send(JSON.stringify({ "eventType": "getDeviceInfo", "payload": { "getState": true } }));
            } else {
                console.warn("WebSocket is not ready to send messages");
            }
        };

        window.webSocket.onclose = function() {
            console.log("WebSocket disconnected. Reconnecting in 5 seconds...");
            setTimeout(() => window.appFunctions.connectWebSocket(), 5000);
        };

        window.webSocket.onerror = function(error) {
            console.error("WebSocket error:", error);
        };

        window.webSocket.onmessage = function(event) {
            const data = JSON.parse(event.data);
            console.log("Received WebSocket message", data);
            window.appFunctions.handleAppWebSocketMessage(data);
        };
    },

    handleAppWebSocketMessage: function(data) {
        console.log("Handling WebSocket message", data);

        if (data.event_type === "radar") {
            console.log("Radar Event Data:", data.payload);

            if (data.payload && data.payload.radars && data.payload.radars[0]) {
                const eventDescription = data.payload.radars[0].event_description || "No recent events";
                document.getElementById('eventDescription').textContent = eventDescription;
            } else {
                console.error("Radar data is missing or malformed");
            }
        }

        // Handling other events (unchanged)
        if (data.event_type === "lock") {
            let ch = data.payload.channel;
            if (ch === 1) {
                document.getElementById('enableLock_1').checked = data.payload.enable;
                document.getElementById('arm_1').checked = data.payload.arm;
                document.getElementById('enableContactAlert_1').checked = data.payload.enableContactAlert;
                document.getElementById('polarity_1').checked = data.payload.polarity;
            } else if (ch === 2) {
                document.getElementById('enableLock_2').checked = data.payload.enable;
                document.getElementById('arm_2').checked = data.payload.arm;
                document.getElementById('enableContactAlert_2').checked = data.payload.enableContactAlert;
                document.getElementById('polarity_2').checked = data.payload.polarity;
            }
        }

        if (data.event_type === "exit") {
            let ch = data.payload.channel;
            document.getElementById('enableExit_' + ch).checked = data.payload.enable;
            document.getElementById('alertExit_' + ch).checked = data.payload.alert;
            document.getElementById('armDelay_' + ch).value = data.payload.delay;
        }

        if (data.event_type === "fob") {
            let ch = data.payload.channel;
            document.getElementById('enableFob_' + ch).checked = data.payload.enable;
            document.getElementById('alertFob_' + ch).checked = data.payload.alert;
            document.getElementById('latchFob_' + ch).checked = data.payload.latch;
        }

        if (data.event_type === "authorize") {
            document.getElementById('uuid').textContent = data.payload.uuid;
        }
    }
};

// Initialize the app when the script is loaded
window.appFunctions.initializeApp();
