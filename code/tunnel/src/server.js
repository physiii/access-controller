import http from 'http';
import net from 'net';
import { randomUUID } from 'crypto';
import { config as loadEnv } from 'dotenv';
import { FrameParser, writeFrame } from './protocol.js';

loadEnv();

const CONFIG = {
  tunnelBind: process.env.TUNNEL_BIND ?? '0.0.0.0',
  tunnelPort: parseInt(process.env.TUNNEL_PORT ?? '9001', 10),
  httpBind: process.env.HTTP_BIND ?? '0.0.0.0',
  httpPort: parseInt(process.env.HTTP_PORT ?? '9000', 10),
  requestTimeoutMs: parseInt(process.env.REQUEST_TIMEOUT_MS ?? '15000', 10),
  handshakeTimeoutMs: parseInt(process.env.HANDSHAKE_TIMEOUT_MS ?? '5000', 10),
  heartbeatIntervalMs: parseInt(process.env.HEARTBEAT_INTERVAL_MS ?? '20000', 10),
  maxRequestBodyBytes: parseInt(process.env.MAX_REQUEST_BODY_BYTES ?? `${5 * 1024 * 1024}`, 10),
  requireIdentify: ['1', 'true', 'yes'].includes((process.env.REQUIRE_IDENTIFY ?? '').toLowerCase()),
  logLevel: process.env.LOG_LEVEL ?? 'info',
};

const VALID_LOG_LEVELS = new Set(['debug', 'info', 'warn', 'error']);
if (!VALID_LOG_LEVELS.has(CONFIG.logLevel)) {
  CONFIG.logLevel = 'info';
}

const LOG_LEVEL_ORDER = { debug: 10, info: 20, warn: 30, error: 40 };

function log(level, message, meta = {}) {
  if (LOG_LEVEL_ORDER[level] < LOG_LEVEL_ORDER[CONFIG.logLevel]) {
    return;
  }
  const output = { level, time: new Date().toISOString(), message, ...meta };
  // eslint-disable-next-line no-console
  console.log(JSON.stringify(output));
}

class PayloadTooLargeError extends Error {
  constructor(limit) {
    super(`Request body exceeds maximum size of ${limit} bytes`);
    this.name = 'PayloadTooLargeError';
  }
}

class DeviceConnection {
  constructor(manager, socket) {
    this.manager = manager;
    this.socket = socket;
    this.deviceId = generateShortId();
    this.registered = !this.manager.config.requireIdentify;
    this.createdAt = Date.now();
    this.lastSeen = Date.now();
    this.pendingRequests = new Map();
    this.parser = new FrameParser({
      onMessage: (header, body) => this.handleMessage(header, body),
      onError: (err) => this.handleParserError(err),
    });

    this.handshakeTimer = null;
    if (this.manager.config.requireIdentify) {
      this.handshakeTimer = setTimeout(() => {
        if (!this.registered) {
          log('warn', 'Closing tunnel due to missing identification', {
            remoteAddress: this.remoteAddress,
            defaultDeviceId: this.deviceId,
          });
          this.close('Handshake timeout');
        }
      }, this.manager.config.handshakeTimeoutMs);
    }

    socket.setKeepAlive(true, this.manager.config.heartbeatIntervalMs);
    socket.setNoDelay(true);

    socket.on('data', (chunk) => {
      this.lastSeen = Date.now();
      this.parser.push(chunk);
    });
    socket.on('close', () => this.handleClose());
    socket.on('error', (err) => this.handleSocketError(err));

    this.remoteAddress = `${socket.remoteAddress}:${socket.remotePort}`;
    try {
      writeFrame(this.socket, { type: 'assign', deviceId: this.deviceId });
    } catch (err) {
      log('error', 'Failed to send initial assignment frame', {
        remoteAddress: this.remoteAddress,
        error: err.message,
      });
      this.close('Failed to communicate with device');
      return;
    }
    log('info', 'Tunnel connection accepted', {
      deviceId: this.deviceId,
      remoteAddress: this.remoteAddress,
    });
  }

  get info() {
    return {
      deviceId: this.deviceId,
      remoteAddress: this.remoteAddress,
      registered: this.registered,
      createdAt: this.createdAt,
      lastSeen: this.lastSeen,
      pendingRequests: this.pendingRequests.size,
    };
  }

  setDeviceId(newId) {
    if (!newId || typeof newId !== 'string') {
      throw new Error('Device ID must be a non-empty string');
    }
    if (newId === this.deviceId && this.registered) {
      return;
    }
    log('info', 'Registering tunnel device ID', {
      oldDeviceId: this.deviceId,
      newDeviceId: newId,
      remoteAddress: this.remoteAddress,
    });
    this.manager.reassignDeviceId(this, newId);
    this.deviceId = newId;
    this.registered = true;
    this.lastSeen = Date.now();
    if (this.handshakeTimer) {
      clearTimeout(this.handshakeTimer);
      this.handshakeTimer = null;
    }
    writeFrame(this.socket, { type: 'ready', deviceId: this.deviceId });
  }

  async forwardHttpRequest(req, res, requestTarget) {
    if (!this.socket || this.socket.destroyed) {
      throw new Error('Tunnel socket is not available');
    }

    const { maxRequestBodyBytes } = this.manager.config;
    const body = await collectRequestBody(req, maxRequestBodyBytes);
    const requestId = randomUUID();
    const headers = normaliseOutgoingHeaders(req, body.length);

    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.pendingRequests.delete(requestId);
        if (!res.headersSent) {
          res.writeHead(504, 'Gateway Timeout');
          res.end('Tunnel request timed out');
        } else {
          res.end();
        }
        reject(new Error('Request timed out waiting for device response'));
      }, this.manager.config.requestTimeoutMs);

      const pending = {
        res,
        resolve,
        reject,
        timeout,
        startedAt: Date.now(),
        method: req.method,
        requestTarget,
        streaming: false,
        responseStarted: false,
      };

      this.pendingRequests.set(requestId, pending);

      res.on('close', () => {
        if (!res.writableEnded && this.pendingRequests.has(requestId)) {
          log('warn', 'HTTP client closed connection before device response', {
            deviceId: this.deviceId,
            requestId,
          });
          clearTimeout(timeout);
          this.pendingRequests.delete(requestId);
          pending.reject(new Error('HTTP client aborted the request'));
          try {
            writeFrame(this.socket, { type: 'abort', requestId });
          } catch (err) {
            log('debug', 'Failed to notify device of aborted request', {
              deviceId: this.deviceId,
              requestId,
              error: err.message,
            });
          }
        }
      });

      try {
        writeFrame(this.socket, {
          type: 'httpRequest',
          requestId,
          method: req.method,
          target: requestTarget,
          httpVersion: req.httpVersion,
          headers,
        }, body);
      } catch (err) {
        clearTimeout(timeout);
        this.pendingRequests.delete(requestId);
        reject(err);
      }
    });
  }

  handleMessage(header, body) {
    switch (header?.type) {
      case 'hello':
      case 'identify': {
        const requestedId = header.deviceId || header.deviceName || header.id;
        if (!requestedId) {
          writeFrame(this.socket, {
            type: 'error',
            message: 'IDENTIFY message requires deviceId field',
          });
          return;
        }
        this.setDeviceId(requestedId);
        break;
      }
      case 'pong': {
        this.lastSeen = Date.now();
        break;
      }
      case 'log': {
        log('info', 'Device log', {
          deviceId: this.deviceId,
          message: header.message ?? body.toString('utf8'),
          level: header.level ?? 'info',
        });
        break;
      }
      case 'httpResponseStart': {
        this.handleHttpResponseStart(header);
        break;
      }
      case 'httpResponseChunk': {
        this.handleHttpResponseChunk(header, body);
        break;
      }
      case 'httpResponseEnd': {
        this.handleHttpResponseEnd(header);
        break;
      }
      case 'httpResponse': {
        this.handleHttpResponse(header, body);
        break;
      }
      case 'httpError': {
        this.handleHttpError(header);
        break;
      }
      default: {
        log('warn', 'Received unknown message type from device', {
          deviceId: this.deviceId,
          type: header?.type,
        });
      }
    }
  }

  setResponseHeaders(res, headers) {
    if (!headers || typeof headers !== 'object') {
      return;
    }

    for (const [key, values] of Object.entries(headers)) {
      if (!key) {
        continue;
      }
      if (Array.isArray(values)) {
        res.setHeader(key, values.length === 1 ? values[0] : values);
      } else if (typeof values !== 'undefined') {
        res.setHeader(key, values);
      }
    }
  }

  handleHttpResponseStart(header) {
    const requestId = header?.requestId;
    if (!requestId || !this.pendingRequests.has(requestId)) {
      log('warn', 'Received streamed response start for unknown request', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    const pending = this.pendingRequests.get(requestId);
    if (pending.streaming) {
      log('warn', 'Received duplicate streamed response start', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    pending.streaming = true;
    pending.responseStarted = true;

    const res = pending.res;
    const statusCode = Number.isInteger(header.statusCode) ? header.statusCode : 200;
    if (header.statusMessage) {
      res.statusMessage = header.statusMessage;
    }
    res.statusCode = statusCode;

    this.setResponseHeaders(res, header.headers);
  }

  handleHttpResponseChunk(header, body) {
    const requestId = header?.requestId;
    if (!requestId || !this.pendingRequests.has(requestId)) {
      log('warn', 'Received streamed chunk for unknown request', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    const pending = this.pendingRequests.get(requestId);
    if (!pending.streaming) {
      log('warn', 'Received streamed chunk before start frame', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    try {
      if (body && body.length) {
        pending.res.write(body);
      }
    } catch (err) {
      log('error', 'Failed to forward streamed response chunk', {
        deviceId: this.deviceId,
        requestId,
        error: err.message,
      });
      clearTimeout(pending.timeout);
      this.pendingRequests.delete(requestId);
      if (!pending.res.headersSent) {
        pending.res.writeHead(500, 'Tunnel Stream Error');
      }
      try {
        pending.res.end('Failed to forward device response');
      } catch (endErr) {
        log('warn', 'Failed to close HTTP response after stream error', {
          deviceId: this.deviceId,
          requestId,
          error: endErr.message,
        });
      }
      pending.reject(err);
    }
  }

  handleHttpResponseEnd(header) {
    const requestId = header?.requestId;
    if (!requestId || !this.pendingRequests.has(requestId)) {
      log('warn', 'Received streamed response end for unknown request', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    const pending = this.pendingRequests.get(requestId);
    this.pendingRequests.delete(requestId);
    clearTimeout(pending.timeout);

    try {
      if (!pending.res.writableEnded) {
        pending.res.end();
      }
      pending.resolve();
    } catch (err) {
      pending.reject(err);
      if (!pending.res.headersSent) {
        pending.res.writeHead(500, 'Tunnel Response Error');
      }
      pending.res.end('Failed to complete device response');
      log('error', 'Failed to finalise streamed response to HTTP client', {
        deviceId: this.deviceId,
        requestId,
        error: err.message,
      });
    }
  }

  handleHttpResponse(header, body) {
    const { requestId } = header;
    if (!requestId || !this.pendingRequests.has(requestId)) {
      log('warn', 'Received response for unknown request', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    const pending = this.pendingRequests.get(requestId);
    if (pending.streaming) {
      if (body && body.length) {
        try {
          pending.res.write(body);
        } catch (err) {
          log('error', 'Failed to forward buffered response body during streaming fallback', {
            deviceId: this.deviceId,
            requestId,
            error: err.message,
          });
        }
      }
      this.handleHttpResponseEnd(header);
      return;
    }
    this.pendingRequests.delete(requestId);
    clearTimeout(pending.timeout);

    const res = pending.res;
    try {
      const statusCode = Number.isInteger(header.statusCode) ? header.statusCode : 200;
      if (header.statusMessage) {
        res.statusMessage = header.statusMessage;
      }
      res.statusCode = statusCode;

      this.setResponseHeaders(res, header.headers);

      res.end(body);
      pending.resolve();
    } catch (err) {
      pending.reject(err);
      if (!res.headersSent) {
        res.writeHead(500, 'Tunnel Response Error');
        res.end('Failed to write response from device');
      }
      log('error', 'Failed to forward response to HTTP client', {
        deviceId: this.deviceId,
        requestId,
        error: err.message,
      });
    }
  }

  handleHttpError(header) {
    const { requestId, message } = header;
    if (!requestId || !this.pendingRequests.has(requestId)) {
      log('warn', 'Received error for unknown request', {
        deviceId: this.deviceId,
        requestId,
      });
      return;
    }

    const pending = this.pendingRequests.get(requestId);
    this.pendingRequests.delete(requestId);
    clearTimeout(pending.timeout);

    if (!pending.res.headersSent) {
      pending.res.writeHead(502, 'Bad Gateway');
    }
    pending.res.end(message ? String(message) : 'Device reported an error');
    pending.reject(new Error(`Device error: ${message}`));
  }

  handleParserError(err) {
    log('error', 'Protocol error on tunnel socket', {
      deviceId: this.deviceId,
      remoteAddress: this.remoteAddress,
      error: err.message,
    });
    this.close('Protocol error');
  }

  handleSocketError(err) {
    log('warn', 'Socket error on tunnel connection', {
      deviceId: this.deviceId,
      remoteAddress: this.remoteAddress,
      error: err.message,
    });
  }

  handleClose() {
    log('info', 'Tunnel connection closed', {
      deviceId: this.deviceId,
      remoteAddress: this.remoteAddress,
    });
    if (this.handshakeTimer) {
      clearTimeout(this.handshakeTimer);
      this.handshakeTimer = null;
    }
    this.flushPendingRequests();
    this.manager.removeDevice(this);
  }

  flushPendingRequests() {
    for (const [requestId, pending] of this.pendingRequests.entries()) {
      clearTimeout(pending.timeout);
      if (!pending.res.headersSent) {
        pending.res.writeHead(502, 'Bad Gateway');
      }
      pending.res.end('Tunnel disconnected while waiting for device response');
      pending.reject(new Error('Tunnel disconnected'));
      this.pendingRequests.delete(requestId);
    }
  }

  close(reason) {
    if (!this.socket.destroyed) {
      try {
        writeFrame(this.socket, {
          type: 'disconnect',
          reason: reason ?? 'Server closing connection',
        });
      } catch (err) {
        // ignore errors while closing
      }
      this.socket.destroy();
    }
  }
}

class TunnelManager {
  constructor(config) {
    this.config = config;
    this.devices = new Map();
    this.tunnelServer = null;
    this.httpServer = null;
    this.heartbeatTimer = null;
  }

  start() {
    return new Promise((resolve, reject) => {
      this.tunnelServer = net.createServer((socket) => this.handleTunnelConnection(socket));
      this.tunnelServer.on('error', (err) => {
        log('error', 'Tunnel server error', { error: err.message });
      });

      this.tunnelServer.listen(this.config.tunnelPort, this.config.tunnelBind, () => {
        log('info', 'Tunnel TCP server started', {
          bind: this.config.tunnelBind,
          port: this.config.tunnelPort,
        });

        this.httpServer = http.createServer((req, res) => this.handleHttpRequest(req, res));
        this.httpServer.on('error', (err) => {
          log('error', 'HTTP server error', { error: err.message });
        });

        this.httpServer.listen(this.config.httpPort, this.config.httpBind, () => {
          log('info', 'HTTP proxy server started', {
            bind: this.config.httpBind,
            port: this.config.httpPort,
          });

          this.startHeartbeat();
          resolve();
        });
      });
      this.tunnelServer.on('error', reject);
    });
  }

  async stop() {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = null;
    }

    for (const device of this.devices.values()) {
      device.close('Server shutting down');
    }
    this.devices.clear();

    await Promise.all([
      new Promise((resolve) => {
        if (!this.httpServer) {
          resolve();
          return;
        }
        this.httpServer.close(() => resolve());
      }),
      new Promise((resolve) => {
        if (!this.tunnelServer) {
          resolve();
          return;
        }
        this.tunnelServer.close(() => resolve());
      }),
    ]);
  }

  startHeartbeat() {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
    }

    this.heartbeatTimer = setInterval(() => {
      const cutoff = Date.now() - this.config.heartbeatIntervalMs * 2;
      for (const device of this.devices.values()) {
        try {
          writeFrame(device.socket, { type: 'ping' });
        } catch (err) {
          log('warn', 'Failed to send heartbeat ping', {
            deviceId: device.deviceId,
            error: err.message,
          });
        }
        if (device.lastSeen < cutoff) {
          log('warn', 'Device appears unresponsive, closing connection', {
            deviceId: device.deviceId,
          });
          device.close('Heartbeat missed');
        }
      }
    }, this.config.heartbeatIntervalMs).unref();
  }

  handleTunnelConnection(socket) {
    const connection = new DeviceConnection(this, socket);
    this.devices.set(connection.deviceId, connection);
  }

  reassignDeviceId(connection, newId) {
    if (this.devices.has(newId)) {
      const existing = this.devices.get(newId);
      if (existing !== connection) {
        log('warn', 'Duplicate device ID detected, closing previous connection', {
          deviceId: newId,
        });
        existing.close('Duplicate device ID detected');
      }
    }

    this.devices.delete(connection.deviceId);
    this.devices.set(newId, connection);
  }

  removeDevice(connection) {
    const current = this.devices.get(connection.deviceId);
    if (current === connection) {
      this.devices.delete(connection.deviceId);
    }
  }

  getDevice(deviceId) {
    return this.devices.get(deviceId);
  }

  async handleHttpRequest(req, res) {
    try {
      const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
      if (url.pathname === '/' && req.method === 'GET') {
        this.handleRootRequest(res);
        return;
      }

      if (url.pathname === '/devices' && req.method === 'GET') {
        this.handleDeviceList(req, res);
        return;
      }

      if (!url.pathname.startsWith('/device/')) {
        res.writeHead(404, 'Not Found');
        res.end('Unknown endpoint');
        return;
      }

      const segments = url.pathname.split('/').filter(Boolean);
      if (segments.length < 2) {
        res.writeHead(400, 'Bad Request');
        res.end('Missing device ID in path');
        return;
      }

      const needsTrailingSlash = segments.length === 2 && !url.pathname.endsWith('/')
        && (req.method === 'GET' || req.method === 'HEAD');
      if (needsTrailingSlash) {
        const location = `${url.pathname}/${url.search ?? ''}`;
        res.writeHead(302, { Location: location });
        res.end();
        return;
      }

      const deviceId = segments[1];
      const device = this.getDevice(deviceId);
      if (!device || device.socket.destroyed) {
        res.writeHead(404, 'Device Not Connected');
        res.end(`Device ${deviceId} is not connected`);
        return;
      }

      const pathSegments = segments.slice(2);
      const forwardPath = `/${pathSegments.join('/')}`.replace(/\/+/g, '/');
      const targetPath = forwardPath === '/' ? '/' : forwardPath;
      const requestTarget = `${targetPath}${url.search}`;

      await device.forwardHttpRequest(req, res, requestTarget);
    } catch (err) {
      if (err instanceof PayloadTooLargeError) {
        res.writeHead(413, 'Payload Too Large');
        res.end(err.message);
        return;
      }

      log('error', 'Error handling HTTP request', {
        error: err.message,
      });
      if (!res.headersSent) {
        res.writeHead(500, 'Internal Server Error');
      }
      res.end('Internal server error');
    }
  }

  handleRootRequest(res) {
    const payload = {
      message: 'ESP32 Tunnel Server',
      httpEndpoint: '/device/{deviceId}/...path',
      devicesEndpoint: '/devices',
      connectedDevices: this.devices.size,
    };
    res.setHeader('content-type', 'application/json; charset=utf-8');
    res.end(JSON.stringify(payload, null, 2));
  }

  handleDeviceList(req, res) {
    const protocol = req.socket.encrypted ? 'https' : 'http';
    const host = req.headers.host ?? `${this.config.httpBind}:${this.config.httpPort}`;
    const baseUrl = `${protocol}://${host}`;
    const devices = Array.from(this.devices.values()).map((device) => ({
      ...device.info,
      url: `${baseUrl}/device/${encodeURIComponent(device.deviceId)}/`,
    }));
    res.setHeader('content-type', 'application/json; charset=utf-8');
    res.end(JSON.stringify({ devices }, null, 2));
  }
}

function generateShortId() {
  return randomUUID().replace(/-/g, '').slice(0, 8);
}

async function collectRequestBody(req, maxBytes) {
  const chunks = [];
  let total = 0;
  for await (const chunk of req) {
    const buffer = Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk);
    total += buffer.length;
    if (total > maxBytes) {
      throw new PayloadTooLargeError(maxBytes);
    }
    chunks.push(buffer);
  }
  return chunks.length ? Buffer.concat(chunks) : Buffer.alloc(0);
}

function normaliseOutgoingHeaders(req, bodyLength) {
  const headers = {};
  for (const [key, value] of Object.entries(req.headers)) {
    if (typeof value === 'undefined') {
      continue;
    }
    if (Array.isArray(value)) {
      headers[key] = value.map((v) => String(v));
    } else {
      headers[key] = [String(value)];
    }
  }

  headers['content-length'] = [String(bodyLength)];
  headers.connection = ['close'];

  const forwardedFor = headers['x-forwarded-for']?.[0];
  const remoteAddress = req.socket.remoteAddress;
  const xffParts = forwardedFor ? forwardedFor.split(',').map((part) => part.trim()).filter(Boolean) : [];
  if (remoteAddress) {
    xffParts.push(remoteAddress);
  }
  if (xffParts.length) {
    headers['x-forwarded-for'] = [xffParts.join(', ')];
  }

  headers['x-forwarded-proto'] = [req.socket.encrypted ? 'https' : 'http'];
  if (req.headers.host) {
    headers['x-forwarded-host'] = [String(req.headers.host)];
  }

  if (!headers.host && req.headers.host) {
    headers.host = [String(req.headers.host)];
  }

  return headers;
}

async function main() {
  const manager = new TunnelManager(CONFIG);
  try {
    await manager.start();
  } catch (err) {
    log('error', 'Failed to start tunnel server', { error: err.message });
    process.exitCode = 1;
    return;
  }

  const shutdown = async (signal) => {
    log('info', 'Received shutdown signal', { signal });
    await manager.stop();
    process.exit(0);
  };

  process.once('SIGINT', () => shutdown('SIGINT'));
  process.once('SIGTERM', () => shutdown('SIGTERM'));
}

main();

