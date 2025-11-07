# ESP32 HTTP Tunnel Server

This module provides a multiplexed reverse HTTP tunnel that lets many ESP32 controllers expose their local web UIs through a single Ubuntu gateway. Each device maintains an outbound TCP connection to the server; inbound HTTP requests are routed through that tunnel and relayed back as plain HTTP responses.

## Folder Layout

- `src/server.js` – main Node.js tunnel daemon
- `src/protocol.js` – framing helpers shared by the server and mock client
- `src/mock-client.js` – Node-based ESP32 stand-in for local testing
- `scripts/install-service.sh` – deploys the server into `/opt/esp32-tunnel` and registers the systemd unit
- `scripts/uninstall-service.sh` – removes the service
- `systemd/esp32-tunnel.service` – hardened systemd unit definition
- `env.example` – starter environment file; copy to `.env` when deploying

## How It Works

1. Each ESP32 (or the mock client) opens a TCP connection to the tunnel server (default port `9001`).
2. The server assigns a short device ID and keeps the socket alive.
3. External clients make HTTP requests to `http://<server>:9000/device/<deviceId>/<path>`.
4. The server serialises the HTTP request (headers, body) into a framed message and forwards it to the correct device.
5. The device responds with an HTTP payload, which the server streams back to the client.

The protocol supports running >100 concurrent devices with independent sockets. Optional `IDENTIFY` frames let your firmware claim a fixed device ID after connecting (recommended so URLs stay stable).

## Quick Start (local test on 192.168.1.43)

1. Install dependencies:

   ```bash
   cd /home/andy/devices/access-controller/code/tunnel
   npm install
   ```

2. Set up a local `.env` (bind the HTTP interface to your Ubuntu host at `192.168.1.43`):

   ```bash
   cp env.example .env
   cat <<'EOF' > .env
   TUNNEL_BIND=0.0.0.0
   TUNNEL_PORT=9001
   HTTP_BIND=192.168.1.43
   HTTP_PORT=9000
   REQUEST_TIMEOUT_MS=15000
   HEARTBEAT_INTERVAL_MS=20000
   MAX_REQUEST_BODY_BYTES=5242880
   LOG_LEVEL=info
   EOF
   ```

3. Run the tunnel server:

   ```bash
   npm run start
   ```

   The server logs will show the TCP and HTTP ports once it is ready.

4. In a second shell, start the mock ESP32 client. It will connect through the tunnel and respond to forwarded HTTP requests:

   ```bash
   npm run mock-client
   ```

   The mock client prints the assigned device ID (by default an 8-character hex string). You can override it with `DEVICE_ID=my-test-device npm run mock-client`.

5. Hit the tunneled device from your browser or with `curl`:

   ```bash
   curl http://192.168.1.43:9000/device/my-test-device/
   ```

   Replace `my-test-device` with the device ID shown in the mock-client logs. The response proves the HTTP tunnel is working end-to-end.

## Run with Docker Compose

1. Copy the example environment file and adjust it for your deployment:

   ```bash
   cp env.example .env
   ```

   Set `HTTP_BIND=0.0.0.0` so the proxy is reachable outside the container, and tweak the other ports/timeouts as needed.

2. Build and start the tunnel:

   ```bash
   docker compose up --build -d
   ```

   The compose service maps container ports `9000` (HTTP proxy) and `9001` (device tunnel) directly to the host. The service uses `restart: unless-stopped`, so it comes back automatically on Docker daemon restarts or host reboots.

3. View logs or stop the stack when required:

   ```bash
   docker compose logs -f
   docker compose down
   ```

   After the first build you can omit `--build` unless the source changes.

## Deploy as a systemd Service (Ubuntu)

> These steps target the local machine first (`192.168.1.43`). Once confirmed, repeat on the public host (`142.93.57.114`) and update DNS/firewall rules accordingly.

1. Ensure Node.js 18+ and npm exist (`sudo apt install -y nodejs npm`).
2. From the repository root, run the installer:

   ```bash
   cd /home/andy/devices/access-controller/code/tunnel
   ./scripts/install-service.sh
   ```

   The script:

   - creates a dedicated `esp32-tunnel` system user
   - syncs the current folder into `/opt/esp32-tunnel`
   - installs production npm dependencies
   - seeds `/opt/esp32-tunnel/.env` from `env.example` (edit it afterward)
   - installs and enables the `esp32-tunnel.service`

3. Tail logs with:

   ```bash
   sudo journalctl -u esp32-tunnel -f
   ```

4. To remove the service later:

   ```bash
   ./scripts/uninstall-service.sh
   ```

## ESP32 Firmware Integration

Until the firmware adopts the new framing protocol, use the Node mock client as a reference:

- Each frame is `[4-byte big-endian header length][JSON header][body bytes]`.
- Server sends `{ "type": "assign", "deviceId": "abcd1234" }` immediately after connect.
- Firmware can respond with `{ "type": "identify", "deviceId": "door-101" }` to claim a persistent ID.
- HTTP requests arrive as `{ "type": "httpRequest", "requestId": "...", "method": "GET", "target": "/" }` with the raw HTTP body attached.
- Reply with `{ "type": "httpResponse", "requestId": "...", "statusCode": 200, "headers": { "content-type": ["text/html"] } }` and include the full HTTP response body.
- Use `{ "type": "httpError", "requestId": "...", "message": "optional details" }` to signal failures.

The `src/mock-client.js` implementation shows the minimal logic required on the device side.

## Security & Scaling Notes

- Terminate TLS at the Ubuntu gateway (e.g., with Nginx or Caddy) and proxy to `http://127.0.0.1:9000`.
- Rotate device IDs and/or add an authentication token inside the JSON headers before granting access.
- Set `LOG_LEVEL=debug` while diagnosing individual device tunnels.
- Tune `REQUEST_TIMEOUT_MS`, `HEARTBEAT_INTERVAL_MS`, and `MAX_REQUEST_BODY_BYTES` as load grows beyond 100 devices.

## Next Steps

- Adapt the ESP32 firmware to speak the framed protocol (or embed a minimal TCP client using the mock as guidance).
- After local validation, deploy onto the public server (`142.93.57.114`) and expose HTTPS with a reverse proxy.
- Add device authentication tokens and per-device access control to harden the deployment.

