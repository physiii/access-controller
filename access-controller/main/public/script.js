var url = "ws://" + location.host + '/ws';
var webSocket = new WebSocket(url);

webSocket.onopen = function (event) {
	webSocket.send("Here's some text that the server is urgently awaiting!");
};

let armDelay = (channel, value) => {
	if (!value) value = 0;
	webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": " + channel + ", \"delay\": " + value + "}}");
	console.log("Set alarm delay to ", value);
}

document.getElementById('enableLock_1').onclick = function() {
			webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('arm_1').onclick = function() {
			webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"arm\": " + this.checked + "}}");
};

document.getElementById('enableExit_1').onclick = function() {
			webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertExit_1').onclick = function() {
			webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 1, \"alert\": " + this.checked + "}}");
};

document.getElementById('alertFob_1').onclick = function() {
			webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 1, \"alert\": " + this.checked + "}}");
};

document.getElementById('arm_2').onclick = function() {
			webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 2, \"arm\": " + this.checked + "}}");
};

document.getElementById('enableLock_2').onclick = function() {
			webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"channel\": 1, \"enable\": " + this.checked + "}}");
};

document.getElementById('enableExit_2').onclick = function() {
			webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 2, \"enable\": " + this.checked + "}}");
};

document.getElementById('alertExit_2').onclick = function() {
			webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"channel\": 2, \"alert\": " + this.checked + "}}");
};

document.getElementById('alertFob_2').onclick = function() {
			webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"channel\": 2, \"alert\": " + this.checked + "}}");
};


webSocket.onmessage = function (event) {
	console.log(event.data);
}
