#!/usr/bin/env node

const BASE_URL = process.env.TUNNEL_BASE_URL || 'http://192.168.1.43:9000/';
const DEVICE_UUID = process.env.DEVICE_UUID;

if (!DEVICE_UUID) {
  console.error('Please set DEVICE_UUID (e.g. DEVICE_UUID=a6521450-9eaf-478d-ab72-e14baa1bb31a).');
  process.exit(1);
}

const deviceBase = new URL(`/device/${DEVICE_UUID}/`, BASE_URL);

const fetchJSON = async (path, options = {}) => {
  const url = new URL(path, deviceBase);
  const response = await fetch(url, {
    headers: { 'Content-Type': 'application/json' },
    ...options,
  });
  if (!response.ok) {
    const body = await response.text();
    throw new Error(`${response.status} ${response.statusText}: ${body}`);
  }
  if (response.status === 204) {
    return null;
  }
  return response.json();
};

const assert = (condition, message) => {
  if (!condition) {
    throw new Error(message);
  }
};

const log = (message) => console.log(`✔ ${message}`);

const toggleLock = async (state) => {
  const first = state.locks?.[0];
  assert(first, 'No lock state available');
  const desired = !first.enable;
  const locks = await fetchJSON('api/lock', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, enable: desired }),
  });
  const updated = locks.find((lock) => lock.channel === first.channel);
  assert(updated && updated.enable === desired, 'Lock toggle failed');
  log(`Lock ${first.channel} enable toggled to ${desired}`);

  await fetchJSON('api/lock', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, enable: first.enable }),
  });
};

const adjustExitDelay = async (state) => {
  const first = state.exits?.[0];
  assert(first, 'No exit state available');
  const updatedDelay = first.delay + 1;
  const exits = await fetchJSON('api/exit', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, delay: updatedDelay }),
  });
  const updated = exits.find((exit) => exit.channel === first.channel);
  assert(updated && updated.delay === updatedDelay, 'Exit delay update failed');
  log(`Exit ${first.channel} delay updated to ${updatedDelay}s`);

  await fetchJSON('api/exit', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, delay: first.delay }),
  });
};

const toggleFobLatch = async (state) => {
  const first = state.fobs?.[0];
  assert(first, 'No fob state available');
  const desired = !first.latch;
  const fobs = await fetchJSON('api/fob', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, latch: desired }),
  });
  const updated = fobs.find((fob) => fob.channel === first.channel);
  assert(updated && updated.latch === desired, 'Fob latch toggle failed');
  log(`Fob ${first.channel} latch toggled to ${desired}`);

  await fetchJSON('api/fob', {
    method: 'POST',
    body: JSON.stringify({ channel: first.channel, latch: first.latch }),
  });
};

const main = async () => {
  console.log(`Running REST smoke tests against ${deviceBase.href}`);
  const state = await fetchJSON('api/state');
  assert(state.device?.uuid === DEVICE_UUID, 'Device UUID mismatch');
  log('State endpoint reachable');

  await toggleLock(state);
  await adjustExitDelay(state);
  await toggleFobLatch(state);

  console.log('All REST smoke tests completed successfully.');
};

main().catch((error) => {
  console.error('REST smoke test failed:', error);
  process.exit(1);
});

