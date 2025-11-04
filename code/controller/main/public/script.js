const fetchJSON = async (path, options = {}) => {
  const url = new URL(path, window.location.href);
  const response = await fetch(url, {
    headers: { 'Content-Type': 'application/json' },
    ...options,
  });
  if (!response.ok) {
    const message = await response.text();
    throw new Error(message || `Request failed: ${response.status}`);
  }
  if (response.status === 204) {
    return null;
  }
  return response.json();
};

const applyLockState = (locks = []) => {
  locks.forEach((lock) => {
    const ch = lock.channel;
    document.getElementById(`enableLock_${ch}`).checked = !!lock.enable;
    document.getElementById(`arm_${ch}`).checked = !!lock.arm;
    document.getElementById(`enableContactAlert_${ch}`).checked = !!lock.enableContactAlert;
    document.getElementById(`polarity_${ch}`).checked = !!lock.polarity;
  });
};

const applyExitState = (exits = []) => {
  exits.forEach((exit) => {
    const ch = exit.channel;
    document.getElementById(`enableExit_${ch}`).checked = !!exit.enable;
    document.getElementById(`alertExit_${ch}`).checked = !!exit.alert;
    document.getElementById(`armDelay_${ch}`).value = exit.delay ?? 0;
  });
};

const applyFobState = (fobs = []) => {
  fobs.forEach((fob) => {
    const ch = fob.channel;
    document.getElementById(`enableFob_${ch}`).checked = !!fob.enable;
    document.getElementById(`alertFob_${ch}`).checked = !!fob.alert;
    document.getElementById(`latchFob_${ch}`).checked = !!fob.latch;
  });
};

const applyDeviceInfo = (device = {}) => {
  document.getElementById('uuid').textContent = device.uuid || '';
};

const applyState = (state = {}) => {
  applyDeviceInfo(state.device || {});
  applyLockState(state.locks || []);
  applyExitState(state.exits || []);
  applyFobState(state.fobs || []);
};

const loadState = async () => {
  try {
    const state = await fetchJSON('api/state');
    applyState(state);
  } catch (error) {
    console.error('Failed to load state', error);
  }
};

const updateLock = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const locks = await fetchJSON('api/lock', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    applyLockState(locks || []);
  } catch (error) {
    console.error('Failed to update lock', error);
  }
};

const updateExit = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const exits = await fetchJSON('api/exit', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    applyExitState(exits || []);
  } catch (error) {
    console.error('Failed to update exit', error);
  }
};

const updateFob = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const fobs = await fetchJSON('api/fob', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    applyFobState(fobs || []);
  } catch (error) {
    console.error('Failed to update fob', error);
  }
};

const setupLockHandlers = () => {
  [1, 2].forEach((ch) => {
    document.getElementById(`enableLock_${ch}`).addEventListener('change', (event) => {
      updateLock(ch, { enable: event.target.checked });
    });
    document.getElementById(`arm_${ch}`).addEventListener('change', (event) => {
      updateLock(ch, { arm: event.target.checked });
    });
    document.getElementById(`enableContactAlert_${ch}`).addEventListener('change', (event) => {
      updateLock(ch, { enableContactAlert: event.target.checked });
    });
    document.getElementById(`polarity_${ch}`).addEventListener('change', (event) => {
      updateLock(ch, { polarity: event.target.checked });
    });
  });
};

const setupExitHandlers = () => {
  [1, 2].forEach((ch) => {
    document.getElementById(`enableExit_${ch}`).addEventListener('change', (event) => {
      updateExit(ch, { enable: event.target.checked });
    });
    document.getElementById(`alertExit_${ch}`).addEventListener('change', (event) => {
      updateExit(ch, { alert: event.target.checked });
    });
  });

  document.getElementById('relock').addEventListener('click', () => {
    const value = parseInt(document.getElementById('armDelay_1').value, 10) || 0;
    updateExit(1, { delay: value });
  });

  document.getElementById('relock_2').addEventListener('click', () => {
    const value = parseInt(document.getElementById('armDelay_2').value, 10) || 0;
    updateExit(2, { delay: value });
  });
};

const setupFobHandlers = () => {
  [1, 2].forEach((ch) => {
    document.getElementById(`enableFob_${ch}`).addEventListener('change', (event) => {
      updateFob(ch, { enable: event.target.checked });
    });
    document.getElementById(`alertFob_${ch}`).addEventListener('change', (event) => {
      updateFob(ch, { alert: event.target.checked });
    });
    document.getElementById(`latchFob_${ch}`).addEventListener('change', (event) => {
      updateFob(ch, { latch: event.target.checked });
    });
  });
};

const setupForms = () => {
  document.getElementById('wifiForm').addEventListener('submit', async (event) => {
    event.preventDefault();
    const wifiName = document.getElementById('wifiName').value;
    const wifiPassword = document.getElementById('wifiPassword').value;
    try {
      await fetchJSON('api/wifi', {
        method: 'POST',
        body: JSON.stringify({ wifiName, wifiPassword }),
      });
      alert('WiFi credentials updated. Device will reboot to apply changes.');
    } catch (error) {
      console.error('Failed to update WiFi info', error);
      alert('Failed to update WiFi credentials.');
    }
  });

  document.getElementById('serverForm').addEventListener('submit', async (event) => {
    event.preventDefault();
    const serverIp = document.getElementById('ipAddress').value;
    const serverPort = document.getElementById('port').value;
    try {
      await fetchJSON('api/server', {
        method: 'POST',
        body: JSON.stringify({ serverIp, serverPort }),
      });
      alert('Server info updated. Device will reboot to apply changes.');
    } catch (error) {
      console.error('Failed to update server info', error);
      alert('Failed to update server info.');
    }
  });
};

const initialise = () => {
  setupLockHandlers();
  setupExitHandlers();
  setupFobHandlers();
  setupForms();
  loadState();
};

document.addEventListener('DOMContentLoaded', initialise);
