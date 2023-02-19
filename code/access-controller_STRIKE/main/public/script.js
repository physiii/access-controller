var url = "ws://" + location.host + '/ws';
// var url = "ws://" + location.host;
var webSocket = new WebSocket(url);

webSocket.onopen = function (event) {
	webSocket.send("{ \"eventType\":\"exit\", \"payload\": {\"getState\": true}}");
	setTimeout(() => {
		webSocket.send("{ \"eventType\":\"lock\", \"payload\": {\"getState\": true}}");
	}, 250);
	setTimeout(() => {
		webSocket.send("{ \"eventType\":\"fob\", \"payload\": {\"getState\": true}}");
	}, 500);

	setTimeout(() => {
		webSocket.send("{ \"eventType\":\"users\", \"payload\": {\"getState\": true}}");
	}, 750);
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


webSocket.onmessage = function (event) {
	let state = JSON.parse(event.data);
	let pl = state.payload;
	let ch = state.payload.channel;
	if (!event.data) return;


	function removeUser(user) {
			console.log(users[i]);
			let msg = "{ \"eventType\":\"users\", \"payload\": {\"removeUser\":\"" + user + "\"}}";
			webSocket.send(msg);
	}

	console.log("Incoming: ", event.data, pl);
	if (state.eventType == "exit") {
		if (pl.enable) document.getElementById('enableExit_' + ch).checked = pl.enable;
		if (pl.alert) document.getElementById('alertExit_' + ch).checked = pl.alert;
		if (pl.delay) document.getElementById('armDelay_' + ch).value = pl.delay;
	}

	if (state.eventType == "lock") {
		if (pl.enable) document.getElementById('enableLock_' + ch).checked = pl.enable;
		if (pl.arm) document.getElementById('arm_' + ch).checked = pl.arm;
		if (pl.enableContactAlert) document.getElementById('enableContactAlert_' + ch).checked = pl.enableContactAlert;
	}

	if (state.eventType == "fob") {
		if (pl.enable) document.getElementById('enableFob_' + ch).checked = pl.enable;
		if (pl.alert) document.getElementById('alertFob_' + ch).checked = pl.alert;
	}

	if (state.eventType == "users") {
		let users = pl;
		let rowsAsString = "<table>";
		rowsAsString += "<tr><th colspan=\"2\">Users</th></tr>";
		rowsAsString += "<tr><th>ID</th><th>Remove</th></tr>";

		for(var i = 0; i < users.length; i++) {
			rowsAsString += "<tr><td>" + users[i] + "</td>"
				+ "<td style=\"width:100px\"><button id=\"remove_" + users[i] + "\" onClick=\"" + users[i] + "\" type='button'\">remove</button></td></tr>";
		}

		rowsAsString += "<tr><th colspan=\"2\"><button id=\"addUserBtn\" style=\"width:100%\" type=\"button\">Add User</button></th></tr>";
		rowsAsString += "</table>";

		document.getElementById('users_list').innerHTML = rowsAsString;
		document.getElementById('addUserBtn').onclick = function(event) {
			console.log(event.target.textContent);
			if (event.target.textContent === "Done") {
				webSocket.send("{ \"eventType\":\"users\", \"payload\": {\"addUser\":false}}");
				document.getElementById('addUserBtn').innerHTML = "Add User";
			} else {
				webSocket.send("{ \"eventType\":\"users\", \"payload\": {\"addUser\":true}}");
				document.getElementById('addUserBtn').innerHTML = "Done";
			}
		};

		for(var i = 0; i < users.length; i++) {
			document.getElementById('remove_' + users[i]).onclick = function(event) {
						let user = event.target.attributes.id.value.replace("remove_", "");
						let msg = "{ \"eventType\":\"users\", \"payload\": {\"removeUser\":\"" + user + "\"}}";
						webSocket.send(msg);
			};
		}

	}
}
