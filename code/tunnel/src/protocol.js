import { EventEmitter } from 'events';

const DEFAULT_MAX_HEADER_BYTES = 64 * 1024; // 64 KiB
const DEFAULT_MAX_BODY_BYTES = 32 * 1024 * 1024; // 32 MiB

export class FrameParser extends EventEmitter {
  constructor({
    onMessage,
    onError,
    maxHeaderBytes = DEFAULT_MAX_HEADER_BYTES,
    maxBodyBytes = DEFAULT_MAX_BODY_BYTES,
  } = {}) {
    super();
    this.maxHeaderBytes = maxHeaderBytes;
    this.maxBodyBytes = maxBodyBytes;
    this.onMessageCallback = onMessage;
    this.onErrorCallback = onError;
    this.buffer = Buffer.alloc(0);
    this.pending = null;
  }

  push(chunk) {
    if (!chunk || chunk.length === 0) {
      return;
    }

    this.buffer = Buffer.concat([this.buffer, chunk]);

    try {
      while (true) {
        if (this.pending) {
          if (this.buffer.length < this.pending.remaining) {
            this.pending.bodyChunks.push(this.buffer);
            this.pending.remaining -= this.buffer.length;
            this.buffer = Buffer.alloc(0);
            return;
          }

          if (this.pending.remaining > 0) {
            const consumed = this.buffer.slice(0, this.pending.remaining);
            this.buffer = this.buffer.slice(this.pending.remaining);
            this.pending.bodyChunks.push(consumed);
          }

          const body = Buffer.concat(this.pending.bodyChunks);
          const header = this.pending.header;
          this.pending = null;
          this.#emitMessage(header, body);
          continue;
        }

        if (this.buffer.length < 4) {
          return;
        }

        const headerLength = this.buffer.readUInt32BE(0);
        if (headerLength <= 0 || headerLength > this.maxHeaderBytes) {
          throw new Error(`Invalid frame header length: ${headerLength}`);
        }

        if (this.buffer.length < 4 + headerLength) {
          return;
        }

        const headerBuffer = this.buffer.slice(4, 4 + headerLength);
        this.buffer = this.buffer.slice(4 + headerLength);

        const headerJson = headerBuffer.toString('utf8');
        let header;
        try {
          header = JSON.parse(headerJson);
        } catch (err) {
          throw new Error(`Failed to parse frame header JSON: ${(err && err.message) || err}`);
        }

        const bodyLength = Number.isFinite(header?.bodyLength) ? Number(header.bodyLength) : 0;
        if (bodyLength < 0 || bodyLength > this.maxBodyBytes) {
          throw new Error(`Invalid frame body length: ${bodyLength}`);
        }

        if (bodyLength === 0) {
          this.#emitMessage(header, Buffer.alloc(0));
          continue;
        }

        if (this.buffer.length < bodyLength) {
          const initialChunk = this.buffer.length ? [this.buffer] : [];
          this.pending = {
            header,
            remaining: bodyLength - this.buffer.length,
            bodyChunks: initialChunk,
          };
          this.buffer = Buffer.alloc(0);
          return;
        }

        const body = this.buffer.slice(0, bodyLength);
        this.buffer = this.buffer.slice(bodyLength);
        this.#emitMessage(header, body);
      }
    } catch (err) {
      if (this.onErrorCallback) {
        this.onErrorCallback(err);
      } else {
        this.emit('error', err);
      }
    }
  }

  #emitMessage(header, body) {
    if (this.onMessageCallback) {
      this.onMessageCallback(header, body);
    }
    this.emit('message', header, body);
  }
}

export function writeFrame(socket, header, body = Buffer.alloc(0)) {
  if (!socket || socket.destroyed) {
    throw new Error('Cannot write frame: socket is not writable');
  }

  const payload = { ...header, bodyLength: body.length };
  const headerBuffer = Buffer.from(JSON.stringify(payload), 'utf8');
  const lengthBuffer = Buffer.allocUnsafe(4);
  lengthBuffer.writeUInt32BE(headerBuffer.length, 0);

  socket.write(lengthBuffer);
  socket.write(headerBuffer);
  if (body.length) {
    socket.write(body);
  }
}

