const App = {
  data: null,
  stateTimer: null,
  wiegandPollTimer: null,
  toastTimer: null,
  elements: {},
};

const WIEGAND_STATUS_META = {
  0: { label: 'Pending', className: 'pending' },
  1: { label: 'Active', className: 'active' },
  2: { label: 'Disabled', className: '' },
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

const escapeHtml = (value = '') => value
  .replace(/&/g, '&amp;')
  .replace(/</g, '&lt;')
  .replace(/>/g, '&gt;')
  .replace(/"/g, '&quot;')
  .replace(/'/g, '&#39;');

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

    if (enableEl) enableEl.checked = !!lock.enable;
    if (armEl) armEl.checked = !!lock.arm;
    if (contactEl) contactEl.checked = !!lock.enableContactAlert;
    if (polarityEl) polarityEl.checked = !!lock.polarity;
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

const buildWiegandUserRow = (user) => {
  const meta = WIEGAND_STATUS_META[user.status] || WIEGAND_STATUS_META[2];
  const statusClass = meta.className ? `status-chip ${meta.className}` : 'status-chip';
  const code = escapeHtml(user.code || '');
  const name = escapeHtml(user.name || '');

  return `
    <div class="wiegand-user" data-id="${escapeHtml(user.id)}">
      <div class="user-meta">
        <span class="code-pill">${code || '—'}</span>
        <span class="${statusClass}">${meta.label}</span>
        <span class="muted">${formatChannelLabel(user.channel)}</span>
      </div>
      <div class="wiegand-actions">
        <input type="text" class="wiegand-name-input" value="${name}" placeholder="User name">
        <button type="button" class="secondary" data-action="rename" data-id="${escapeHtml(user.id)}">Save</button>
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

  if (statusEl) {
    statusEl.textContent = registrationActive
      ? `Registering (${formatChannelLabel(registrationChannel)})`
      : 'Idle';
  }

  if (pendingEl) {
    pendingEl.textContent = registrationPending;
  }

  if (duplicateEl) {
    if (lastDuplicateCode) {
      duplicateEl.textContent = lastDuplicateCode;
      duplicateEl.classList.remove('muted');
    } else {
      duplicateEl.textContent = 'None';
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
      listEl.innerHTML = '<p class="muted">No Wiegand users stored yet.</p>';
    } else {
      listEl.innerHTML = users
        .map((user) => buildWiegandUserRow(user))
        .join('');
    }
  }

  if (registrationActive) {
    startWiegandPolling();
  } else {
    stopWiegandPolling();
  }
};

const renderState = (state = {}) => {
  applyDeviceInfo(state.device || {});
  applyLockState(state.locks || []);
  applyExitState(state.exits || []);
  applyFobState(state.fobs || []);
  applyKeypadState(state.keypads || []);
  renderWiegand(state.wiegand || {});
  renderLogs(state.logs || []);
};

const loadState = async () => {
  try {
    const data = await fetchJSON('api/state');
    App.data = data;
    renderState(data);
  } catch (error) {
    handleError(error, 'Unable to load device state');
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

  if (wifiForm) {
    wifiForm.addEventListener('submit', async (event) => {
      event.preventDefault();
      const wifiName = document.getElementById('wifiName')?.value || '';
      const wifiPassword = document.getElementById('wifiPassword')?.value || '';

      try {
        await fetchJSON('api/wifi', {
          method: 'POST',
          body: JSON.stringify({ wifiName, wifiPassword }),
        });
        showToast('Wi-Fi credentials updated. Device will reboot to apply changes.');
      } catch (error) {
        handleError(error, 'Failed to update Wi-Fi credentials');
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
      const button = event.target.closest('button[data-action="rename"]');
      if (!button) return;

      const container = button.closest('.wiegand-user');
      if (!container) return;

      const input = container.querySelector('.wiegand-name-input');
      if (!input) return;

      const id = container.getAttribute('data-id');
      const name = input.value.trim();
      if (!id || !name) {
        showToast('Please provide a name before saving.');
        return;
      }

      button.disabled = true;
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
        button.disabled = false;
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

document.addEventListener('DOMContentLoaded', () => {
  App.elements = {
    navItems: Array.from(document.querySelectorAll('.nav-item')),
    pages: Array.from(document.querySelectorAll('.page')),
    toast: document.getElementById('toast'),
    wiegandStatus: document.getElementById('wiegandStatus'),
    wiegandPending: document.getElementById('wiegandPending'),
    wiegandDuplicate: document.getElementById('wiegandDuplicate'),
    wiegandUserList: document.getElementById('wiegandUserList'),
    wiegandRegisterBtn: document.getElementById('wiegandRegisterBtn'),
    wiegandStopBtn: document.getElementById('wiegandStopBtn'),
    wiegandChannelSelect: document.getElementById('wiegandChannelSelect'),
    logItems: document.getElementById('logItems'),
    logEmptyState: document.getElementById('logEmptyState'),
  };

  bindNavigation();
  setupLockHandlers();
  setupExitHandlers();
  setupFobHandlers();
  setupKeypadHandlers();
  setupForms();
  setupWiegandHandlers();

  loadState();
  App.stateTimer = setInterval(loadState, 7000);
});
