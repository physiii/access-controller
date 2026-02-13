const App = {
  data: null,
  stateTimer: null,
  wiegandPollTimer: null,
  rfPollTimer: null,
  toastTimer: null,
  elements: {},
};

const WIEGAND_STATUS_META = {
  0: { label: 'Pending', className: 'pending' },
  1: { label: 'Active', className: 'active' },
  2: { label: 'Disabled', className: '' },
};

// Convert binary string to hex
const binaryToHex = (binaryStr) => {
  if (!binaryStr || !/^[01]+$/.test(binaryStr)) {
    return binaryStr || '—';
  }
  // Pad to multiple of 4
  const padded = binaryStr.padStart(Math.ceil(binaryStr.length / 4) * 4, '0');
  let hex = '';
  for (let i = 0; i < padded.length; i += 4) {
    hex += parseInt(padded.substr(i, 4), 2).toString(16).toUpperCase();
  }
  return '0x' + hex;
};

const fetchJSON = async (path, options = {}) => {
  const href = window.location.href;
  const baseHref = href.endsWith('/') ? href : `${href}/`;
  const url = new URL(path, baseHref);
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

const escapeHtml = (value) => {
  // Handle null, undefined, or any non-string value
  if (value == null) return '';
  const str = String(value);
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
};

const showToast = (message) => {
  const toast = App.elements.toast;
  if (!toast) return;

  toast.textContent = message;
  toast.hidden = false;
  toast.classList.add('show');

  clearTimeout(App.toastTimer);
  App.toastTimer = setTimeout(() => {
    toast.classList.remove('show');
    toast.hidden = true;
  }, 3200);
};

const handleError = (error, fallbackMessage) => {
  console.error(error);
  showToast(fallbackMessage || error.message || 'Something went wrong');
};

const formatChannelLabel = (channel) => (channel ? `Channel ${channel}` : 'All channels');

const setActivePage = (targetId) => {
  App.elements.pages.forEach((section) => {
    section.classList.toggle('active', section.id === `page-${targetId}`);
  });

  App.elements.navItems.forEach((item) => {
    item.classList.toggle('active', item.dataset.target === targetId);
  });
};

const bindNavigation = () => {
  App.elements.navItems.forEach((button) => {
    button.addEventListener('click', () => {
      const target = button.dataset.target;
      setActivePage(target);
    });
  });
};

const applyDeviceInfo = (device = {}) => {
  const uuidEl = document.getElementById('uuid');
  if (uuidEl) {
    uuidEl.textContent = device.uuid || '—';
  }
};

const applyLockState = (locks = []) => {
  locks.forEach((lock) => {
    const ch = lock.channel;
    const enableEl = document.getElementById(`enableLock_${ch}`);
    const armEl = document.getElementById(`arm_${ch}`);
    const contactEl = document.getElementById(`enableContactAlert_${ch}`);
    const polarityEl = document.getElementById(`polarity_${ch}`);
    const contactStatusEl = document.getElementById(`lockContact_${ch}`);
    const senseStatusEl = document.getElementById(`lockSense_${ch}`);

    if (enableEl) enableEl.checked = !!lock.enable;
    if (armEl) armEl.checked = !!lock.arm;
    if (contactEl) contactEl.checked = !!lock.enableContactAlert;
    if (polarityEl) polarityEl.checked = !!lock.polarity;

    /* Ch1: contact state is in API "sense". Ch2: contact state is in API "contact". */
    const contactState = ch === 1 ? lock.sense : lock.contact;
    const signalState = ch === 1 ? lock.contact : lock.sense;

    if (contactStatusEl) {
      if (typeof contactState === 'boolean') {
        contactStatusEl.classList.remove('status-ok', 'status-alert');
        contactStatusEl.classList.add(contactState ? 'status-ok' : 'status-alert');
        contactStatusEl.title = contactState ? 'Contact closed' : 'Contact open';
      } else {
        contactStatusEl.classList.remove('status-ok', 'status-alert');
        contactStatusEl.title = 'Contact state unknown';
      }
    }
    if (senseStatusEl) {
      if (typeof signalState === 'boolean') {
        senseStatusEl.classList.remove('status-ok', 'status-alert');
        senseStatusEl.classList.add(signalState ? 'status-ok' : 'status-alert');
        senseStatusEl.title = signalState ? 'Signal active' : 'Signal inactive';
      } else {
        senseStatusEl.classList.remove('status-ok', 'status-alert');
        senseStatusEl.title = 'Signal state unknown';
      }
    }
  });
};

const applyExitState = (exits = []) => {
  exits.forEach((exit) => {
    const ch = exit.channel;
    const enableEl = document.getElementById(`enableExit_${ch}`);
    const alertEl = document.getElementById(`alertExit_${ch}`);
    const delayEl = document.getElementById(`armDelay_${ch}`);

    if (enableEl) enableEl.checked = !!exit.enable;
    if (alertEl) alertEl.checked = !!exit.alert;
    if (delayEl) delayEl.value = exit.delay ?? 0;
  });
};

const applyFobState = (fobs = []) => {
  fobs.forEach((fob) => {
    const ch = fob.channel;
    const enableEl = document.getElementById(`enableFob_${ch}`);
    const alertEl = document.getElementById(`alertFob_${ch}`);
    const latchEl = document.getElementById(`latchFob_${ch}`);

    if (enableEl) enableEl.checked = !!fob.enable;
    if (alertEl) alertEl.checked = !!fob.alert;
    if (latchEl) latchEl.checked = !!fob.latch;
  });
};

const applyKeypadState = (keypads = []) => {
  keypads.forEach((pad) => {
    const ch = pad.channel;
    const enableEl = document.getElementById(`enableKeypad_${ch}`);
    const alertEl = document.getElementById(`alertKeypad_${ch}`);
    const delayEl = document.getElementById(`keypadDelay_${ch}`);

    if (enableEl) enableEl.checked = !!pad.enable;
    if (alertEl) alertEl.checked = !!pad.alert;
    if (delayEl) delayEl.value = pad.delay ?? 0;
  });
};

const buildWiegandUserRow = (user, existingValue) => {
  if (!user) return '';
  const meta = WIEGAND_STATUS_META[user.status] || WIEGAND_STATUS_META[2];
  const statusClass = meta.className ? `status-chip ${meta.className}` : 'status-chip';
  const rawCode = user.code || '';
  const hexCode = binaryToHex(rawCode);
  // Use existing input value if user was editing, otherwise use stored name
  const name = escapeHtml(existingValue !== undefined ? existingValue : (user.name || ''));
  const channelNum = user.channel || 0;
  const userId = escapeHtml(user.id || '');

  return `
    <div class="user-row" data-id="${userId}">
      <span class="user-code">${escapeHtml(hexCode)}</span>
      <div class="user-info">
        <input type="text" class="user-name-input" value="${name}" placeholder="Enter name...">
        <span class="user-channel">Channel ${channelNum}</span>
      </div>
      <div class="user-actions">
        <span class="${statusClass}">${meta.label}</span>
        <button type="button" class="secondary" data-action="rename" data-id="${userId}">Save</button>
        <button type="button" class="secondary danger" data-action="delete-wiegand" data-id="${userId}">Delete</button>
      </div>
    </div>
  `;
};

const startWiegandPolling = () => {
  if (App.wiegandPollTimer) return;
  App.wiegandPollTimer = setInterval(async () => {
    try {
      const wiegand = await fetchJSON('api/wiegand');
      if (App.data) {
        App.data.wiegand = wiegand;
      }
      renderWiegand(wiegand);
    } catch (error) {
      handleError(error, 'Failed to refresh Wiegand state');
      stopWiegandPolling();
    }
  }, 2500);
};

const stopWiegandPolling = () => {
  if (App.wiegandPollTimer) {
    clearInterval(App.wiegandPollTimer);
    App.wiegandPollTimer = null;
  }
};

const renderWiegand = (wiegand = {}) => {
  const {
    registrationActive = false,
    registrationChannel = 0,
    registrationPending = 0,
    lastDuplicateCode = '',
    users = [],
  } = wiegand;

  if (App.data) {
    App.data.wiegand = wiegand;
  }

  const statusEl = App.elements.wiegandStatus;
  const pendingEl = App.elements.wiegandPending;
  const duplicateEl = App.elements.wiegandDuplicate;
  const listEl = App.elements.wiegandUserList;
  const registerBtn = App.elements.wiegandRegisterBtn;
  const stopBtn = App.elements.wiegandStopBtn;
  const channelSelect = App.elements.wiegandChannelSelect;
  const statusBar = App.elements.wiegandStatusBar;

  if (statusBar) {
    statusBar.classList.toggle('registering', registrationActive);
  }

  if (statusEl) {
    statusEl.textContent = registrationActive
      ? `Registering ${formatChannelLabel(registrationChannel)}`
      : 'Idle';
  }

  if (pendingEl) {
    pendingEl.textContent = registrationPending;
  }

  if (duplicateEl) {
    if (lastDuplicateCode) {
      duplicateEl.textContent = binaryToHex(lastDuplicateCode);
      duplicateEl.classList.remove('muted');
    } else {
      duplicateEl.textContent = '—';
      duplicateEl.classList.add('muted');
    }
  }

  if (registerBtn) {
    registerBtn.disabled = registrationActive;
  }
  if (channelSelect) {
    channelSelect.disabled = registrationActive;
  }
  if (stopBtn) {
    stopBtn.disabled = !registrationActive;
  }

  if (listEl) {
    if (!users || users.length === 0) {
      listEl.innerHTML = '<p class="empty-state muted">No RFID cards registered yet. Click "Register" to add cards.</p>';
    } else {
      // Preserve name input values that user may be editing
      const existingNames = {};
      listEl.querySelectorAll('.user-row').forEach((row) => {
        const id = row.getAttribute('data-id');
        if (!id) return;
        const nameInput = row.querySelector('.user-name-input');
        if (nameInput) {
          existingNames[id] = nameInput.value;
        }
      });

      listEl.innerHTML = users
        .map((user) => buildWiegandUserRow(user, existingNames[user.id]))
        .join('');

      // Restore focus if user was editing
      Object.keys(existingValues).forEach((id) => {
        const row = listEl.querySelector(`.user-row[data-id="${id}"]`);
        if (row) {
          const input = row.querySelector('.user-name-input');
          if (input) {
            input.focus();
            input.setSelectionRange(input.value.length, input.value.length);
          }
        }
      });
    }
  }

  if (registrationActive) {
    startWiegandPolling();
  } else {
    stopWiegandPolling();
  }
};

// Remote FOBs (433 MHz)
const buildRfUserRow = (user, existingValue) => {
  if (!user) return '';
  const name = escapeHtml(existingValue?.name !== undefined ? existingValue.name : (user.name || ''));
  const code = user.code ? `0x${escapeHtml(user.code)}` : '—';
  const mode = existingValue?.mode || user.mode || 'toggle';
  const channelMask = existingValue?.channel_mask || user.channel_mask || 1;
  const exitSeconds = existingValue?.exit_seconds ?? user.exit_seconds ?? 4;
  const alert = existingValue?.alert ?? (user.alert ?? true);

  return `
    <div class="user-row" data-id="${escapeHtml(user.id || '')}">
      <span class="user-code">${code}</span>
      <div class="user-info">
        <label class="stacked">
          <span>Name</span>
          <input type="text" class="user-name-input" value="${name}" placeholder="Enter name...">
        </label>
        <div class="user-config">
          <label class="stacked">
            <span>Mode</span>
            <select class="rf-mode-select">
              <option value="toggle" ${mode === 'toggle' ? 'selected' : ''}>Toggle</option>
              <option value="momentary" ${mode === 'momentary' ? 'selected' : ''}>Momentary (hold = off)</option>
              <option value="exit" ${mode === 'exit' ? 'selected' : ''}>Exit (pulse)</option>
              <option value="power_on" ${mode === 'power_on' ? 'selected' : ''}>Power ON</option>
              <option value="power_off" ${mode === 'power_off' ? 'selected' : ''}>Power OFF</option>
            </select>
          </label>
          <label class="stacked">
            <span>Channel</span>
            <select class="rf-channel-select">
              <option value="1" ${channelMask === 1 ? 'selected' : ''}>Channel 1</option>
              <option value="2" ${channelMask === 2 ? 'selected' : ''}>Channel 2</option>
              <option value="3" ${channelMask === 3 ? 'selected' : ''}>Both</option>
            </select>
          </label>
          <label class="stacked">
            <span>Exit duration (s)</span>
            <input type="number" class="rf-exit-seconds" min="1" step="1" value="${exitSeconds}">
          </label>
          <label class="form-switch">
            <input type="checkbox" class="rf-alert-checkbox" ${alert ? 'checked' : ''}>
            <span>Alert (beep)</span>
          </label>
        </div>
      </div>
      <div class="user-actions">
        <button type="button" class="secondary" data-action="save-rf" data-id="${escapeHtml(user.id || '')}">Save</button>
        <button type="button" class="secondary danger" data-action="delete-rf" data-id="${escapeHtml(user.id || '')}">Delete</button>
      </div>
    </div>
  `;
};

const renderRf = (rf = {}) => {
  const {
    registrationActive = false,
    registrationPending = 0,
    lastDuplicateCode = '',
    users = [],
  } = rf;

  const statusEl = App.elements.rfStatus;
  const pendingEl = App.elements.rfPending;
  const duplicateEl = App.elements.rfDuplicate;
  const listEl = App.elements.rfUserList;
  const registerBtn = App.elements.rfRegisterBtn;
  const stopBtn = App.elements.rfStopBtn;
  const statusBar = App.elements.rfStatusBar;

  if (statusBar) statusBar.classList.toggle('registering', registrationActive);
  if (statusEl) statusEl.textContent = registrationActive ? 'Registering remotes' : 'Idle';
  if (pendingEl) pendingEl.textContent = registrationPending;
  if (duplicateEl) {
    if (lastDuplicateCode) {
      duplicateEl.textContent = `0x${lastDuplicateCode}`;
      duplicateEl.classList.remove('muted');
    } else {
      duplicateEl.textContent = '—';
      duplicateEl.classList.add('muted');
    }
  }
  if (registerBtn) registerBtn.disabled = registrationActive;
  if (stopBtn) stopBtn.disabled = !registrationActive;

  if (listEl && !listEl.contains(document.activeElement)) {
    if (!users || users.length === 0) {
      listEl.innerHTML = '<p class="empty-state muted">No remote FOBs learned yet. Click "Register" to learn codes.</p>';
    } else {
      const existingValues = {};
      listEl.querySelectorAll('.user-row').forEach((row) => {
        const id = row.getAttribute('data-id');
        if (!id) return;
        const nameInput = row.querySelector('.user-name-input');
        const modeSel = row.querySelector('.rf-mode-select');
        const chSel = row.querySelector('.rf-channel-select');
        const exitInput = row.querySelector('.rf-exit-seconds');
        existingValues[id] = {
          name: nameInput ? nameInput.value : undefined,
          mode: modeSel ? modeSel.value : undefined,
          channel_mask: chSel ? Number(chSel.value) : undefined,
          exit_seconds: exitInput ? Number(exitInput.value) : undefined,
        };
      });

      listEl.innerHTML = users
        .map((u) => buildRfUserRow(u, existingValues[u.id]))
        .join('');
    }
  }

  if (registrationActive) {
    if (!App.rfPollTimer) {
      App.rfPollTimer = setInterval(loadState, 2000);
    }
  } else if (App.rfPollTimer) {
    clearInterval(App.rfPollTimer);
    App.rfPollTimer = null;
  }
};

const renderState = (state = {}) => {
  applyDeviceInfo(state.device || {});
  applyLockState(state.locks || []);
  applyExitState(state.exits || []);
  applyFobState(state.fobs || []);
  applyKeypadState(state.keypads || []);
  renderWiegand(state.wiegand || {});
  renderRf(state.rf || {});
  renderKeypadUsers(state.keypadUsers || []);
  renderLogs(state.logs || []);
  renderWifi(state.wifi || {});
};

const DEVICE_STATE_ERROR_THROTTLE_MS = 15000;
let lastDeviceStateErrorToast = 0;

const loadState = async () => {
  try {
    const data = await fetchJSON('api/state');
    App.data = data;
    renderState(data);
    if (App.elements.toast && !App.elements.toast.hidden) {
      App.elements.toast.hidden = true;
      App.elements.toast.classList.remove('show');
    }
  } catch (error) {
    const now = Date.now();
    if (now - lastDeviceStateErrorToast >= DEVICE_STATE_ERROR_THROTTLE_MS) {
      lastDeviceStateErrorToast = now;
      handleError(error, 'Unable to load device state');
    }
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
    handleError(error, 'Failed to update lock state');
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
    handleError(error, 'Failed to update exit state');
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
    handleError(error, 'Failed to update FOB state');
  }
};

const updateKeypad = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const keypads = await fetchJSON('api/keypad', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    applyKeypadState(keypads || []);
  } catch (error) {
    handleError(error, 'Failed to update keypad state');
  }
};

const setupLockHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableLock_${ch}`);
    const armEl = document.getElementById(`arm_${ch}`);
    const contactEl = document.getElementById(`enableContactAlert_${ch}`);
    const polarityEl = document.getElementById(`polarity_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateLock(ch, { enable: event.target.checked });
      });
    }
    if (armEl) {
      armEl.addEventListener('change', (event) => {
        updateLock(ch, { arm: event.target.checked });
      });
    }
    if (contactEl) {
      contactEl.addEventListener('change', (event) => {
        updateLock(ch, { enableContactAlert: event.target.checked });
      });
    }
    if (polarityEl) {
      polarityEl.addEventListener('change', (event) => {
        updateLock(ch, { polarity: event.target.checked });
      });
    }
  });
};

const setupExitHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableExit_${ch}`);
    const alertEl = document.getElementById(`alertExit_${ch}`);
    const saveBtn = document.getElementById(ch === 1 ? 'relock' : 'relock_2');
    const delayEl = document.getElementById(`armDelay_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateExit(ch, { enable: event.target.checked });
      });
    }
    if (alertEl) {
      alertEl.addEventListener('change', (event) => {
        updateExit(ch, { alert: event.target.checked });
      });
    }
    if (saveBtn && delayEl) {
      saveBtn.addEventListener('click', () => {
        const value = parseInt(delayEl.value, 10) || 0;
        updateExit(ch, { delay: value });
      });
    }
  });
};

const setupFobHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableFob_${ch}`);
    const alertEl = document.getElementById(`alertFob_${ch}`);
    const latchEl = document.getElementById(`latchFob_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateFob(ch, { enable: event.target.checked });
      });
    }
    if (alertEl) {
      alertEl.addEventListener('change', (event) => {
        updateFob(ch, { alert: event.target.checked });
      });
    }
    if (latchEl) {
      latchEl.addEventListener('change', (event) => {
        updateFob(ch, { latch: event.target.checked });
      });
    }
  });
};

const setupKeypadHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableKeypad_${ch}`);
    const alertEl = document.getElementById(`alertKeypad_${ch}`);
    const delayEl = document.getElementById(`keypadDelay_${ch}`);
    const saveBtn = document.getElementById(`keypadSave_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateKeypad(ch, { enable: event.target.checked });
      });
    }
    if (alertEl) {
      alertEl.addEventListener('change', (event) => {
        updateKeypad(ch, { alert: event.target.checked });
      });
    }
    if (delayEl && saveBtn) {
      saveBtn.addEventListener('click', () => {
        const value = parseInt(delayEl.value, 10) || 0;
        updateKeypad(ch, { delay: value });
      });
    }
  });
};

const setupForms = () => {
  const wifiForm = document.getElementById('wifiForm');
  const serverForm = document.getElementById('serverForm');
  const wifiList = document.getElementById('wifiNetworks');

  if (wifiForm) {
    wifiForm.addEventListener('submit', async (event) => {
      event.preventDefault();
      const wifiName = document.getElementById('wifiName')?.value || '';
      const wifiPassword = document.getElementById('wifiPassword')?.value || '';

      try {
        await fetchJSON('api/wifi/add', {
          method: 'POST',
          body: JSON.stringify({ ssid: wifiName, password: wifiPassword }),
        });
        showToast('Wi‑Fi saved. Device will reboot to connect.');
      } catch (error) {
        handleError(error, 'Failed to update Wi-Fi credentials');
      }
    });
  }

  if (wifiList) {
    wifiList.addEventListener('click', async (event) => {
      const connectBtn = event.target.closest('button[data-action="wifi-connect"]');
      const deleteBtn = event.target.closest('button[data-action="wifi-delete"]');
      if (connectBtn) {
        const ssid = connectBtn.getAttribute('data-ssid');
        connectBtn.disabled = true;
        try {
          await fetchJSON('api/wifi/connect', {
            method: 'POST',
            body: JSON.stringify({ ssid }),
          });
          showToast('Connecting... device will reboot.');
        } catch (error) {
          handleError(error, 'Failed to connect');
        } finally {
          connectBtn.disabled = false;
        }
      }
      if (deleteBtn) {
        const ssid = deleteBtn.getAttribute('data-ssid');
        deleteBtn.disabled = true;
        try {
          await fetchJSON('api/wifi/delete', {
            method: 'POST',
            body: JSON.stringify({ ssid }),
          });
          showToast('Wi‑Fi removed.');
          loadState();
        } catch (error) {
          handleError(error, 'Failed to delete Wi‑Fi');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });
  }

  if (serverForm) {
    serverForm.addEventListener('submit', async (event) => {
      event.preventDefault();
      const serverIp = document.getElementById('ipAddress')?.value || '';
      const serverPort = document.getElementById('port')?.value || '';

      try {
        await fetchJSON('api/server', {
          method: 'POST',
          body: JSON.stringify({ serverIp, serverPort }),
        });
        showToast('Server information updated. Device will reboot to apply changes.');
      } catch (error) {
        handleError(error, 'Failed to update server information');
      }
    });
  }
};

const setupWiegandHandlers = () => {
  const registerBtn = App.elements.wiegandRegisterBtn;
  const stopBtn = App.elements.wiegandStopBtn;
  const channelSelect = App.elements.wiegandChannelSelect;
  const listEl = App.elements.wiegandUserList;

  if (registerBtn && channelSelect) {
    registerBtn.addEventListener('click', async () => {
      const channel = parseInt(channelSelect.value, 10) || 0;
      try {
        const wiegand = await fetchJSON('api/wiegand/register', {
          method: 'POST',
          body: JSON.stringify({ channel }),
        });
        renderWiegand(wiegand);
        showToast('Registration started. Tap tags to enrol.');
      } catch (error) {
        handleError(error, error.message || 'Failed to start registration');
      }
    });
  }

  if (stopBtn) {
    stopBtn.addEventListener('click', async () => {
      try {
        const wiegand = await fetchJSON('api/wiegand/stop', {
          method: 'POST',
          body: JSON.stringify({ promote: true }),
        });
        renderWiegand(wiegand);
        showToast('Registration stopped. New tags activated.');
      } catch (error) {
        handleError(error, error.message || 'Failed to stop registration');
      }
    });
  }

  if (listEl) {
    listEl.addEventListener('click', async (event) => {
      const renameBtn = event.target.closest('button[data-action="rename"]');
      const deleteBtn = event.target.closest('button[data-action="delete-wiegand"]');

      if (renameBtn) {
        const container = renameBtn.closest('.user-row');
        if (!container) return;

        const input = container.querySelector('.user-name-input');
        if (!input) return;

        const id = container.getAttribute('data-id');
        const name = input.value.trim();
        if (!id || !name) {
          showToast('Please provide a name before saving.');
          return;
        }

        renameBtn.disabled = true;
        try {
          const wiegand = await fetchJSON('api/wiegand/rename', {
            method: 'POST',
            body: JSON.stringify({ id, name }),
          });
          renderWiegand(wiegand);
          showToast('User name updated.');
        } catch (error) {
          handleError(error, error.message || 'Failed to rename user');
        } finally {
          renameBtn.disabled = false;
        }
      }

      if (deleteBtn) {
        const id = deleteBtn.getAttribute('data-id');
        if (!id) return;

        if (!confirm('Delete this RFID card?')) return;

        deleteBtn.disabled = true;
        try {
          const wiegand = await fetchJSON('api/wiegand/delete', {
            method: 'POST',
            body: JSON.stringify({ id }),
          });
          renderWiegand(wiegand);
          showToast('RFID card deleted.');
        } catch (error) {
          handleError(error, error.message || 'Failed to delete card');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });
  }
};

const setupRfHandlers = () => {
  const registerBtn = App.elements.rfRegisterBtn;
  const stopBtn = App.elements.rfStopBtn;
  const listEl = App.elements.rfUserList;

  if (registerBtn) {
    registerBtn.addEventListener('click', async () => {
      try {
        const rf = await fetchJSON('api/rf/register', { method: 'POST', body: JSON.stringify({}) });
        renderRf(rf);
        showToast('RF registration started. Press remote buttons to learn.');
      } catch (error) {
        handleError(error, error.message || 'Failed to start RF registration');
      }
    });
  }

  if (stopBtn) {
    stopBtn.addEventListener('click', async () => {
      try {
        const rf = await fetchJSON('api/rf/stop', { method: 'POST', body: JSON.stringify({}) });
        renderRf(rf);
        showToast('RF registration stopped.');
      } catch (error) {
        handleError(error, error.message || 'Failed to stop RF registration');
      }
    });
  }

  if (listEl) {
    listEl.addEventListener('click', async (event) => {
      const saveBtn = event.target.closest('button[data-action="save-rf"]');
      const deleteBtn = event.target.closest('button[data-action="delete-rf"]');

      if (saveBtn) {
        const container = saveBtn.closest('.user-row');
        const nameInput = container?.querySelector('.user-name-input');
        const modeSelect = container?.querySelector('.rf-mode-select');
        const channelSelect = container?.querySelector('.rf-channel-select');
        const exitInput = container?.querySelector('.rf-exit-seconds');
        const alertCb = container?.querySelector('.rf-alert-checkbox');
        const id = saveBtn.getAttribute('data-id');
        const name = nameInput?.value.trim();
        const mode = modeSelect?.value;
        const channel_mask = channelSelect ? Number(channelSelect.value) : 0;
        const exit_seconds = exitInput ? Number(exitInput.value || 0) : 0;
        const alert = alertCb ? !!alertCb.checked : true;

        if (!id) {
          showToast('Missing id');
          return;
        }
        if (!name) {
          showToast('Please provide a name before saving.');
          return;
        }
        saveBtn.disabled = true;
        try {
          await fetchJSON('api/rf/rename', {
            method: 'POST',
            body: JSON.stringify({ id, name }),
          });
          const rf = await fetchJSON('api/rf/config', {
            method: 'POST',
            body: JSON.stringify({ id, mode, channel_mask, exit_seconds, alert }),
          });
          renderRf(rf);
          showToast('Remote updated.');
        } catch (error) {
          handleError(error, error.message || 'Failed to update remote');
        } finally {
          saveBtn.disabled = false;
        }
      }

      if (deleteBtn) {
        const id = deleteBtn.getAttribute('data-id');
        if (!id) return;
        if (!confirm('Delete this remote FOB?')) return;
        deleteBtn.disabled = true;
        try {
          const rf = await fetchJSON('api/rf/delete', {
            method: 'POST',
            body: JSON.stringify({ id }),
          });
          renderRf(rf);
          showToast('Remote deleted.');
        } catch (error) {
          handleError(error, error.message || 'Failed to delete remote');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });
  }
};

const renderLogs = (logs = []) => {
  const list = App.elements.logItems;
  const emptyState = App.elements.logEmptyState;
  if (!list || !emptyState) {
    return;
  }

  if (!logs || logs.length === 0) {
    list.hidden = true;
    list.innerHTML = '';
    emptyState.hidden = false;
    return;
  }

  emptyState.hidden = true;
  list.hidden = false;

  const entries = logs.slice().reverse().map((entry) => {
    const timestampMs = entry.timestamp ?? 0;
    const secondsSinceBoot = Math.round(timestampMs / 1000);
    const message = escapeHtml(entry.message || '');
    let timeLabel = `${secondsSinceBoot}s since boot`;
    if (entry.unixTime) {
      const date = new Date(entry.unixTime * 1000);
      timeLabel = `${date.toLocaleString()} (${secondsSinceBoot}s since boot)`;
    }
    return `
      <li class="log-item">
        <span class="meta">${timeLabel}</span>
        <span>${message}</span>
      </li>
    `;
  });

  list.innerHTML = entries.join('');
};

// Keypad PIN Management
const buildKeypadUserRow = (user, index, existingValue) => {
  // Use existing input value if user was editing, otherwise use stored name
  const name = escapeHtml(existingValue !== undefined ? existingValue : (user.name || `User ${index + 1}`));
  const pin = escapeHtml(user.pin || '****');
  
  return `
    <div class="user-row" data-uuid="${escapeHtml(user.uuid || '')}">
      <span class="user-code">${pin}</span>
      <div class="user-info">
        <input type="text" class="user-name-input" value="${name}" placeholder="Enter name...">
      </div>
      <div class="user-actions">
        <button type="button" class="secondary" data-action="save-pin" data-uuid="${escapeHtml(user.uuid || '')}">Save</button>
        <button type="button" class="secondary danger" data-action="delete-pin" data-uuid="${escapeHtml(user.uuid || '')}">Delete</button>
      </div>
    </div>
  `;
};

const renderKeypadUsers = (users = []) => {
  const listEl = App.elements.keypadUserList;
  if (!listEl) return;

  if (!users || users.length === 0) {
    listEl.innerHTML = '<p class="empty-state muted">No PIN codes configured yet.</p>';
  } else {
    // Preserve input values that user may be editing
    const existingValues = {};
    listEl.querySelectorAll('.user-row').forEach((row) => {
      const uuid = row.getAttribute('data-uuid');
      const input = row.querySelector('.user-name-input');
      if (uuid && input && document.activeElement === input) {
        existingValues[uuid] = input.value;
      }
    });

    listEl.innerHTML = users
      .map((user, idx) => buildKeypadUserRow(user, idx, existingValues[user.uuid]))
      .join('');

    // Restore focus if user was editing
    Object.keys(existingValues).forEach((uuid) => {
      const row = listEl.querySelector(`.user-row[data-uuid="${uuid}"]`);
      if (row) {
        const input = row.querySelector('.user-name-input');
        if (input) {
          input.focus();
          input.setSelectionRange(input.value.length, input.value.length);
        }
      }
    });
  }
};

const renderWifi = (wifi = {}) => {
  const list = document.getElementById('wifiNetworks');
  const activeEl = document.getElementById('wifiActive');
  if (!list) return;
  const networks = wifi.networks || [];
  const active = wifi.active_ssid || '';
  if (activeEl) {
    activeEl.textContent = active || '—';
  }
  if (!networks.length) {
    list.innerHTML = '<p class="empty-state muted">No saved Wi‑Fi networks.</p>';
    return;
  }
  list.innerHTML = networks
    .map(
      (n) => `
      <div class="user-row" data-ssid="${escapeHtml(n.ssid || '')}">
        <div class="user-info">
          <strong>${escapeHtml(n.ssid || '')}</strong>
          <div class="meta muted">${n.last_used_ms ? `Last used: ${Math.round(n.last_used_ms / 1000)}s since boot` : ''}</div>
        </div>
        <div class="user-actions">
          <button type="button" class="secondary" data-action="wifi-connect" data-ssid="${escapeHtml(n.ssid || '')}" ${active === n.ssid ? 'disabled' : ''}>Connect</button>
          <button type="button" class="secondary danger" data-action="wifi-delete" data-ssid="${escapeHtml(n.ssid || '')}">Delete</button>
        </div>
      </div>
    `
    )
    .join('');
};

const setupKeypadPinHandlers = () => {
  const addBtn = App.elements.keypadAddBtn;
  const addForm = App.elements.keypadAddForm;
  const cancelBtn = App.elements.keypadCancelBtn;
  const saveNewBtn = App.elements.keypadSaveNewBtn;
  const listEl = App.elements.keypadUserList;

  if (addBtn && addForm) {
    addBtn.addEventListener('click', () => {
      addForm.hidden = false;
      addBtn.disabled = true;
      const nameInput = document.getElementById('keypadNewName');
      if (nameInput) nameInput.focus();
    });
  }

  if (cancelBtn && addForm && addBtn) {
    cancelBtn.addEventListener('click', () => {
      addForm.hidden = true;
      addBtn.disabled = false;
      document.getElementById('keypadNewName').value = '';
      document.getElementById('keypadNewPin').value = '';
    });
  }

  if (saveNewBtn) {
    saveNewBtn.addEventListener('click', async () => {
      const nameInput = document.getElementById('keypadNewName');
      const pinInput = document.getElementById('keypadNewPin');
      const name = nameInput?.value.trim() || '';
      const pin = pinInput?.value.trim() || '';

      if (!name || !pin) {
        showToast('Please enter both name and PIN code.');
        return;
      }

      if (!/^\d{4,6}$/.test(pin)) {
        showToast('PIN must be 4-6 digits.');
        return;
      }

      saveNewBtn.disabled = true;
      try {
        await fetchJSON('api/keypad/user', {
          method: 'POST',
          body: JSON.stringify({ name, pin }),
        });
        showToast('PIN code added successfully.');
        addForm.hidden = true;
        addBtn.disabled = false;
        nameInput.value = '';
        pinInput.value = '';
        loadState();
      } catch (error) {
        handleError(error, 'Failed to add PIN code');
      } finally {
        saveNewBtn.disabled = false;
      }
    });
  }

  if (listEl) {
    listEl.addEventListener('click', async (event) => {
      const saveBtn = event.target.closest('button[data-action="save-pin"]');
      const deleteBtn = event.target.closest('button[data-action="delete-pin"]');

      if (saveBtn) {
        const container = saveBtn.closest('.user-row');
        const input = container?.querySelector('.user-name-input');
        const uuid = saveBtn.getAttribute('data-uuid');
        const name = input?.value.trim();

        if (!uuid || !name) {
          showToast('Please provide a name.');
          return;
        }

        saveBtn.disabled = true;
        try {
          await fetchJSON('api/keypad/user', {
            method: 'PUT',
            body: JSON.stringify({ uuid, name }),
          });
          showToast('PIN user updated.');
        } catch (error) {
          handleError(error, 'Failed to update user');
        } finally {
          saveBtn.disabled = false;
        }
      }

      if (deleteBtn) {
        const uuid = deleteBtn.getAttribute('data-uuid');
        if (!uuid) return;

        if (!confirm('Delete this PIN code?')) return;

        deleteBtn.disabled = true;
        try {
          await fetchJSON('api/keypad/user', {
            method: 'DELETE',
            body: JSON.stringify({ uuid }),
          });
          showToast('PIN code deleted.');
          loadState();
        } catch (error) {
          handleError(error, 'Failed to delete PIN code');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });
  }
};

document.addEventListener('DOMContentLoaded', () => {
  App.elements = {
    navItems: Array.from(document.querySelectorAll('.nav-item')),
    pages: Array.from(document.querySelectorAll('.page')),
    toast: document.getElementById('toast'),
    wiegandStatus: document.getElementById('wiegandStatus'),
    wiegandStatusBar: document.getElementById('wiegandStatusBar'),
    wiegandPending: document.getElementById('wiegandPending'),
    wiegandDuplicate: document.getElementById('wiegandDuplicate'),
    wiegandUserList: document.getElementById('wiegandUserList'),
    wiegandRegisterBtn: document.getElementById('wiegandRegisterBtn'),
    wiegandStopBtn: document.getElementById('wiegandStopBtn'),
    wiegandChannelSelect: document.getElementById('wiegandChannelSelect'),
    rfStatus: document.getElementById('rfStatus'),
    rfStatusBar: document.getElementById('rfStatusBar'),
    rfPending: document.getElementById('rfPending'),
    rfDuplicate: document.getElementById('rfDuplicate'),
    rfUserList: document.getElementById('rfUserList'),
    rfRegisterBtn: document.getElementById('rfRegisterBtn'),
    rfStopBtn: document.getElementById('rfStopBtn'),
    keypadUserList: document.getElementById('keypadUserList'),
    keypadAddBtn: document.getElementById('keypadAddBtn'),
    keypadAddForm: document.getElementById('keypadAddForm'),
    keypadCancelBtn: document.getElementById('keypadCancelBtn'),
    keypadSaveNewBtn: document.getElementById('keypadSaveNewBtn'),
    logItems: document.getElementById('logItems'),
    logEmptyState: document.getElementById('logEmptyState'),
    wifiNetworks: document.getElementById('wifiNetworks'),
    wifiActive: document.getElementById('wifiActive'),
  };

  bindNavigation();
  setupLockHandlers();
  setupExitHandlers();
  setupFobHandlers();
  setupKeypadHandlers();
  setupForms();
  setupWiegandHandlers();
  setupRfHandlers();
  setupKeypadPinHandlers();

  loadState();
  App.stateTimer = setInterval(loadState, 7000);
});
