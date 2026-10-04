const App = {
  data: null,
  stateTimer: null,
  logsTimer: null,
  enrollmentPollTimer: null,
  wiegandPollTimer: null,
  rfPollTimer: null,
  signalPollTimer: null,
  rfTimestampTimer: null,
  uptimeTimer: null,
  systemUptimeBaseSeconds: 0,
  systemUptimeBaseMs: 0,
  headerClockTimer: null,
  headerClockBaseUnixSeconds: 0,
  headerClockBaseMs: 0,
  headerClockUtcOffsetSeconds: 0,
  headerClockResolved: false,
  networkHandoffTimer: null,
  rfAutoSavedIds: new Set(),
  rfAutoSavingIds: new Set(),
  credentialAutoSaveTimers: new Map(),
  credentialUserCreatePromises: new Map(),
  credentialUserOpenKeys: new Set(),
  scheduleEditingId: null,
  toastTimer: null,
  elements: {},
  pageBoot: {
    device: false,
    system: false,
    settings: false,
  },
};

const WIEGAND_STATUS_META = {
  0: { label: 'Pending', className: 'pending' },
  1: { label: 'Active', className: 'active' },
  2: { label: 'Disabled', className: '' },
};

const ACTIVITY_HIGHLIGHT_MS = 3500;

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
  const { timeoutMs = 8000, ...fetchOptions } = options;
  const href = window.location.href;
  const baseHref = href.endsWith('/') ? href : `${href}/`;
  const url = new URL(path, baseHref);
  const controller = new AbortController();
  const timeout = timeoutMs > 0 ? setTimeout(() => controller.abort(), timeoutMs) : null;
  const signal = fetchOptions.signal || controller.signal;

  let response;
  try {
    response = await fetch(url, {
      headers: { 'Content-Type': 'application/json' },
      cache: 'no-store',
      ...fetchOptions,
      signal,
    });
  } finally {
    if (timeout) clearTimeout(timeout);
  }

  if (!response.ok) {
    const message = await response.text();
    throw new Error(message || `Request failed: ${response.status}`);
  }

  if (response.status === 204) {
    return null;
  }

  const text = await response.text();
  if (!text) {
    return null;
  }
  try {
    return JSON.parse(text);
  } catch (error) {
    return text;
  }
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
  console.warn(error);
  showToast(fallbackMessage || error.message || 'Something went wrong');
};

const formatChannelLabel = (channel) => (channel ? `Wiegand ${channel}` : 'Both Wiegand devices');

const formatUptime = (value) => {
  const totalSeconds = Math.max(0, Math.floor(Number(value) || 0));
  const days = Math.floor(totalSeconds / 86400);
  const hours = Math.floor((totalSeconds % 86400) / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;
  const parts = [];

  if (days) parts.push(`${days}d`);
  if (hours || parts.length) parts.push(`${hours}h`);
  if (minutes || parts.length) parts.push(`${minutes}m`);
  parts.push(`${seconds}s`);

  return parts.join(' ');
};

const formatBytes = (value) => {
  const bytes = Math.max(0, Number(value) || 0);
  if (bytes >= 1024 * 1024) return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
  if (bytes >= 1024) return `${Math.round(bytes / 1024)} KB`;
  return `${bytes} B`;
};

const formatPartition = (partition = {}) => {
  if (!partition || !partition.label) return '—';
  const address = typeof partition.address === 'number'
    ? `0x${partition.address.toString(16)}`
    : '';
  return [partition.label, address].filter(Boolean).join(' · ');
};

const formatRemoteCode = (value) => {
  const code = Number(value) || 0;
  if (!code) return '—';
  return `0x${code.toString(16).toUpperCase().padStart(6, '0')}`;
};

const formatPercent = (value, digits = 0) => {
  const num = Number(value);
  if (!Number.isFinite(num)) return '—';
  return `${num.toFixed(digits)}%`;
};

const formatRate = (value) => {
  const num = Number(value);
  if (!Number.isFinite(num)) return '—';
  return `${Math.round(num)}/s`;
};

const formatAge = (ms) => {
  const value = Number(ms);
  if (!Number.isFinite(value) || value <= 0) return '—';
  if (value < 1000) return `${Math.round(value)}ms`;
  return `${(value / 1000).toFixed(value < 10000 ? 1 : 0)}s`;
};

const formatSinceBoot = (ms) => {
  const value = Number(ms);
  if (!Number.isFinite(value) || value <= 0) return '—';
  return `${formatUptime(Math.round(value / 1000))} since boot`;
};

const wifiSignalFromRssi = (rssi) => {
  const value = Number(rssi);
  if (!Number.isFinite(value)) return null;
  return Math.max(0, Math.min(100, Math.round(((value + 90) / 60) * 100)));
};

const formatWifiSignal = (network) => {
  if (!network) return '—';
  const rssi = Number(network.rssi);
  const quality = wifiSignalFromRssi(rssi);
  if (quality == null) return '—';
  return `${quality}% · ${rssi} dBm`;
};

const formatWifiLink = (qualityValue, rssiValue) => {
  const quality = Number(qualityValue);
  const rssi = Number(rssiValue);
  if (Number.isFinite(quality) && Number.isFinite(rssi)) {
    return `${Math.round(quality)}% · ${Math.round(rssi)} dBm`;
  }
  if (Number.isFinite(rssi)) {
    const derived = wifiSignalFromRssi(rssi);
    return derived == null ? `${Math.round(rssi)} dBm` : `${derived}% · ${Math.round(rssi)} dBm`;
  }
  return '—';
};

const isSameDay = (left, right) => (
  left.getFullYear() === right.getFullYear()
  && left.getMonth() === right.getMonth()
  && left.getDate() === right.getDate()
);

const formatCompactDateTime = (date) => {
  const now = new Date();
  const time = date.toLocaleTimeString([], { hour: 'numeric', minute: '2-digit', second: '2-digit' });
  if (isSameDay(date, now)) {
    return time;
  }

  const dateOptions = date.getFullYear() === now.getFullYear()
    ? { month: 'short', day: 'numeric' }
    : { month: 'short', day: 'numeric', year: 'numeric' };
  const day = date.toLocaleDateString([], dateOptions);
  const shortTime = date.toLocaleTimeString([], { hour: 'numeric', minute: '2-digit' });
  return `${day} ${shortTime}`;
};

const formatRfReceivedLabel = (unixTime, receivedMs, ageMs) => {
  const age = formatAge(ageMs);
  const ageLabel = age === '—' ? 'just now' : `${age} ago`;
  const unix = Number(unixTime) || 0;
  if (unix > 0) {
    const date = new Date(unix * 1000);
    return `${formatCompactDateTime(date)} · ${ageLabel}`;
  }
  const uptimeSeconds = Math.round((Number(receivedMs) || 0) / 1000);
  return uptimeSeconds > 0 ? `${uptimeSeconds}s since boot · ${ageLabel}` : ageLabel;
};

const formatRfReceivedTitle = (unixTime, receivedMs) => {
  const unix = Number(unixTime) || 0;
  if (unix > 0) {
    return new Date(unix * 1000).toLocaleString();
  }
  const uptimeSeconds = Math.round((Number(receivedMs) || 0) / 1000);
  return uptimeSeconds > 0 ? `${uptimeSeconds}s since boot` : '';
};

const formatLastUsedLabel = (unixTime, usedMs) => {
  const unix = Number(unixTime) || 0;
  if (unix > 0) {
    return formatCompactDateTime(new Date(unix * 1000));
  }
  const uptimeSeconds = Math.round((Number(usedMs) || 0) / 1000);
  return uptimeSeconds > 0 ? `${uptimeSeconds}s since boot` : 'Used';
};

const updateActivityHighlight = (card, ageMs, hasActivity = true) => {
  if (!card) return;
  const age = Number(ageMs);
  const active = hasActivity && Number.isFinite(age) && age >= 0 && age < ACTIVITY_HIGHLIGHT_MS;
  card.classList.toggle('is-activity-active', active);
  if (hasActivity && Number.isFinite(age) && age >= 0) {
    card.dataset.activityAgeMs = String(age);
    card.dataset.activityRenderedAtMs = String(Date.now());
  } else {
    delete card.dataset.activityAgeMs;
    delete card.dataset.activityRenderedAtMs;
  }
};

const refreshActivityHighlights = () => {
  document.querySelectorAll('.credential-card[data-activity-age-ms]').forEach((card) => {
    const baseAgeMs = Number(card.dataset.activityAgeMs || 0);
    const renderedAtMs = Number(card.dataset.activityRenderedAtMs || 0);
    const ageMs = baseAgeMs + Math.max(0, Date.now() - renderedAtMs);
    updateActivityHighlight(card, ageMs, true);
  });
};

const updateRfReceivedTimestamps = () => {
  document.querySelectorAll('.rf-last-received').forEach((el) => {
    const baseAgeMs = Number(el.dataset.ageMs || 0);
    const renderedAtMs = Number(el.dataset.renderedAtMs || 0);
    const ageMs = baseAgeMs + Math.max(0, Date.now() - renderedAtMs);
    el.textContent = formatRfReceivedLabel(el.dataset.unixTime, el.dataset.receivedMs, ageMs);
    const title = formatRfReceivedTitle(el.dataset.unixTime, el.dataset.receivedMs);
    if (title) {
      el.title = title;
    }
    updateActivityHighlight(el.closest('.credential-card'), ageMs, true);
  });
  refreshActivityHighlights();
};

const ensureRfTimestampTimer = () => {
  if (!App.rfTimestampTimer) {
    App.rfTimestampTimer = setInterval(updateRfReceivedTimestamps, 1000);
  }
};

const buildLastUsedMetric = (lastUsed, emptyText = 'Not used yet') => {
  if (!lastUsed) {
    return buildRfUserMetric('Last used', emptyText, true);
  }
  const usedMs = Number(lastUsed.used_ms) || 0;
  const unixTime = Number(lastUsed.unixTime) || 0;
  const title = formatRfReceivedTitle(unixTime, usedMs);
  return `
    <div class="rf-card-metric">
      <span class="label">Last used</span>
      <span class="value credential-last-used"
        data-unix-time="${unixTime}"
        data-received-ms="${usedMs}"
        title="${escapeHtml(title)}">${formatLastUsedLabel(unixTime, usedMs)}</span>
    </div>
  `;
};

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

const ACTIVE_PAGE_STORAGE_KEY = 'ac_active_page';

const getStoredActivePageId = () => {
  try {
    const stored = localStorage.getItem(ACTIVE_PAGE_STORAGE_KEY);
    const known = App.elements.navItems.map((item) => item.dataset.target);
    return known.includes(stored) ? stored : 'device';
  } catch (error) {
    return 'device';
  }
};

const setActivePage = (targetId) => {
  App.elements.pages.forEach((section) => {
    section.classList.toggle('active', section.id === `page-${targetId}`);
  });

  App.elements.navItems.forEach((item) => {
    item.classList.toggle('active', item.dataset.target === targetId);
  });

  try {
    localStorage.setItem(ACTIVE_PAGE_STORAGE_KEY, targetId);
  } catch (error) {
    // Private browsing / storage disabled -- the tab just won't survive a refresh.
  }
};

const bindNavigation = () => {
  App.elements.navItems.forEach((button) => {
    button.addEventListener('click', () => {
      const target = button.dataset.target;
      setActivePage(target);
      onPageActivated(target);
    });
  });
};

const getActivePageId = () => document.querySelector('.page.active')?.id?.replace(/^page-/, '') || 'device';

const onPageActivated = (targetId) => {
  if (!targetId) return;

  if (targetId === 'device') {
    if (!App.pageBoot.device) {
      App.pageBoot.device = true;
    }
    scheduleDeviceCredentialLoad();
    startSignalPolling();
  } else {
    stopSignalPolling();
    stopWiegandPolling();
  }

  if (targetId === 'system') {
    if (!App.pageBoot.system) {
      App.pageBoot.system = true;
      loadLogs();
    }
    if (!App.logsTimer) {
      App.logsTimer = setInterval(loadLogs, 30000);
    }
  } else if (App.logsTimer) {
    clearInterval(App.logsTimer);
    App.logsTimer = null;
  }

  if (targetId === 'settings') {
    App.pageBoot.settings = true;
    loadWifiList({ force: true, scan: true });
  }
};

const applyHeaderIpLink = (linkId, valueId, label, value) => {
  const link = document.getElementById(linkId);
  const valueEl = document.getElementById(valueId);
  const ip = String(value || '').trim();

  if (valueEl) valueEl.textContent = ip || '—';
  if (!link) return;

  if (!ip) {
    link.removeAttribute('href');
    link.setAttribute('aria-disabled', 'true');
    link.setAttribute('aria-label', `${label} interface unavailable`);
    link.setAttribute('title', `${label} interface is not available yet`);
    link.classList.add('is-disabled');
    return;
  }

  const host = ip.includes(':') && !ip.startsWith('[') ? `[${ip}]` : ip;
  const url = `http://${host}/`;
  link.setAttribute('href', url);
  link.removeAttribute('aria-disabled');
  link.setAttribute('aria-label', `Open ${label} interface at ${ip}`);
  link.setAttribute('title', `Open ${url}`);
  link.classList.remove('is-disabled');
};

const applyDeviceInfo = (device = {}) => {
  const uuidEl = document.getElementById('uuid');
  if (uuidEl) {
    uuidEl.textContent = device.uuid || '—';
  }

  const network = device.network || {};
  const setText = (id, value) => {
    const el = document.getElementById(id);
    if (el) el.textContent = value || '—';
  };

  setText('wifiStaIp', network.wifi_sta_ip);
  setText('wifiApIp', network.wifi_ap_ip);
  setText('ethIp', network.eth_ip);
  setText('wifiStaMac', network.wifi_sta_mac);
  setText('wifiApMac', network.wifi_ap_mac);
  setText('ethMac', network.eth_mac);
  applyHeaderIpLink('headerStaLink', 'headerStaIp', 'STA', network.wifi_sta_ip);
  applyHeaderIpLink('headerApLink', 'headerApIp', 'AP', network.wifi_ap_ip);
};

const applyServerInfo = (server = {}) => {
  const serverUrl = server.url || 'https://open-automation.org/devices';
  const setText = (id, value) => {
    const el = document.getElementById(id);
    if (el) el.textContent = value || '—';
  };
  setText('systemServerUrl', serverUrl);
  setText('headerServerUrl', serverUrl);

  const input = document.getElementById('serverUrl');
  if (input && document.activeElement !== input) {
    input.value = serverUrl;
  }

  const requireInput = document.getElementById('serverRequireReachable');
  if (requireInput && document.activeElement !== requireInput) {
    requireInput.checked = server.requireReachable !== false;
  }
};

const applySystemInfo = (system = {}) => {
  const uptimeEl = document.getElementById('systemUptime');
  const uptimeSeconds = Number(system.uptimeSeconds);
  if (Number.isFinite(uptimeSeconds)) {
    App.systemUptimeBaseSeconds = Math.max(0, Math.floor(uptimeSeconds));
    App.systemUptimeBaseMs = Date.now();
  }

  const unixTime = Number(system.unixTime);
  App.headerClockResolved = Number.isFinite(unixTime) && unixTime > 0;
  if (App.headerClockResolved) {
    App.headerClockBaseUnixSeconds = Math.floor(unixTime);
    App.headerClockUtcOffsetSeconds = Number(system.utcOffsetSeconds) || 0;
    App.headerClockBaseMs = Date.now();
  }
  updateHeaderClock();
  if (uptimeEl) {
    uptimeEl.textContent = formatUptime(
      Number.isFinite(uptimeSeconds) ? uptimeSeconds : App.systemUptimeBaseSeconds
    );
  }

  const firmware = system.firmware || {};
  const setText = (id, value) => {
    const el = document.getElementById(id);
    if (el) el.textContent = value || '—';
  };

  setText('firmwareBranch', firmware.gitBranch);
  setText('firmwareCommit', firmware.gitCommit ? `${firmware.gitCommit}${firmware.gitDirty ? ' · dirty' : ''}` : '');
  setText('firmwareVersion', firmware.projectVersion);
  setText('firmwareSlot', `${formatPartition(firmware.runningPartition)} · ${firmware.otaState || 'unknown'}`);
  setText('firmwareNextSlot', formatPartition(firmware.nextUpdatePartition));
  setText(
    'firmwareRollback',
    firmware.rollbackEnabled
      ? `Enabled${firmware.rollbackPossible ? ' · ready' : ''}`
      : 'Disabled'
  );
};

const updateSystemUptimeClock = () => {
  if (!App.systemUptimeBaseMs) return;
  const uptimeEl = document.getElementById('systemUptime');
  if (!uptimeEl) return;
  const elapsedSeconds = Math.floor((Date.now() - App.systemUptimeBaseMs) / 1000);
  uptimeEl.textContent = formatUptime(App.systemUptimeBaseSeconds + elapsedSeconds);
};

const startUptimeClock = () => {
  if (App.uptimeTimer) return;
  updateSystemUptimeClock();
  App.uptimeTimer = setInterval(updateSystemUptimeClock, 1000);
};

const stopUptimeClock = () => {
  if (!App.uptimeTimer) return;
  clearInterval(App.uptimeTimer);
  App.uptimeTimer = null;
};

// Shows the device's own local time (NTP-synced unix time, shifted by the UTC offset it resolved
// from IP geolocation) in the header -- not the viewer's browser time zone, since the schedule
// feature this mirrors is about what time the DEVICE thinks it is. Uses the same
// shift-then-read-as-UTC trick as the firmware's schedule_allows_access() (getUTCHours/Minutes on
// a pre-shifted timestamp) so it can't accidentally double-apply the browser's own local zone.
const updateHeaderClock = () => {
  const clockEl = document.getElementById('headerClock');
  if (!clockEl) return;
  if (!App.headerClockResolved || !App.headerClockBaseMs) {
    clockEl.textContent = '—';
    return;
  }
  const elapsedSeconds = Math.floor((Date.now() - App.headerClockBaseMs) / 1000);
  const shiftedUnixSeconds = App.headerClockBaseUnixSeconds + App.headerClockUtcOffsetSeconds + elapsedSeconds;
  const shifted = new Date(shiftedUnixSeconds * 1000);
  const hh = String(shifted.getUTCHours()).padStart(2, '0');
  const mm = String(shifted.getUTCMinutes()).padStart(2, '0');
  clockEl.textContent = formatTime12h(`${hh}:${mm}`);
};

const startHeaderClock = () => {
  if (App.headerClockTimer) return;
  updateHeaderClock();
  App.headerClockTimer = setInterval(updateHeaderClock, 1000);
};

const stopHeaderClock = () => {
  if (!App.headerClockTimer) return;
  clearInterval(App.headerClockTimer);
  App.headerClockTimer = null;
};

const applySignalDot = (elementId, value, activeText = 'Signal active', inactiveText = 'Signal inactive') => {
  const el = document.getElementById(elementId);
  if (!el) return;

  el.classList.remove('status-ok', 'status-alert');
  if (typeof value === 'boolean') {
    el.classList.add(value ? 'status-ok' : 'status-alert');
    el.title = value ? activeText : inactiveText;
  } else {
    el.title = 'Signal state unknown';
  }

  const section = el.closest('.control-section');
  if (section && typeof value === 'boolean') {
    section.classList.toggle('is-signal-active', value);
  }
};

const setEnableButtonState = (button, enabled) => {
  if (!button) return;
  button.classList.toggle('is-enabled', !!enabled);
  button.classList.toggle('is-disabled', !enabled);
  button.setAttribute('aria-pressed', enabled ? 'true' : 'false');
  button.title = enabled ? 'Disable' : 'Enable';
};

const setCardEnabledState = (enableId, enabled) => {
  const enableEl = document.getElementById(enableId);
  const section = enableEl?.closest('.control-section');
  const button = document.querySelector(`[data-enable-target="${enableId}"]`);
  if (enableEl) enableEl.checked = !!enabled;
  if (section) section.classList.toggle('is-card-disabled', !enabled);
  setEnableButtonState(button, !!enabled);
};

const normalizeCardMode = (mode, latch) => {
  if (mode === 'momentary' || mode === 'toggle' || mode === 'latch') return mode;
  return latch ? 'latch' : 'momentary';
};

const normalizeChannelMask = (value, fallback = 1) => {
  const mask = Number(value);
  return mask >= 1 && mask <= 3 ? mask : fallback;
};

const ALERT_TARGET_CONTROLLER = 1;
const ALERT_TARGET_WG1 = 2;
const ALERT_TARGET_WG2 = 4;
const ALERT_TARGET_KEYPAD = ALERT_TARGET_WG1 | ALERT_TARGET_WG2;
const ALERT_TARGET_BOTH = ALERT_TARGET_CONTROLLER | ALERT_TARGET_KEYPAD;

const normalizeAlertTarget = (value, fallbackAlert = true) => {
  if (typeof value === 'number' || /^\d+$/.test(String(value || ''))) {
    const mask = Number(value);
    return mask >= 0 && mask <= ALERT_TARGET_BOTH
      ? mask
      : (fallbackAlert ? ALERT_TARGET_BOTH : 0);
  }
  const raw = String(value || '').trim().toLowerCase();
  if (!raw || raw === 'none' || raw === 'off') return fallbackAlert && !raw ? ALERT_TARGET_BOTH : 0;
  if (raw === 'controller' || raw === 'buzzer') return ALERT_TARGET_CONTROLLER;
  if (raw === 'keypad' || raw === 'keypads' || raw === 'wg') return ALERT_TARGET_KEYPAD;
  if (raw === 'both') return ALERT_TARGET_BOTH;
  let mask = 0;
  raw.split(/[\s,+|]+/).forEach((part) => {
    if (part === 'controller' || part === 'buzzer') mask |= ALERT_TARGET_CONTROLLER;
    if (part === 'wg1' || part === 'wiegand1' || part === 'keypad1') mask |= ALERT_TARGET_WG1;
    if (part === 'wg2' || part === 'wiegand2' || part === 'keypad2') mask |= ALERT_TARGET_WG2;
  });
  return mask || (fallbackAlert ? ALERT_TARGET_BOTH : 0);
};

const alertFromTarget = (target) => normalizeAlertTarget(target) !== 0;

const readSectionLabel = (id, fallback) => {
  const input = document.getElementById(id);
  const value = input?.value?.trim();
  return value || fallback;
};

const lockLabel = (channel) => readSectionLabel(`label_enableLock_${channel}`, `Lock ${channel}`);

const wiegandLabel = (channel) => readSectionLabel(`label_wiegandDevice_${channel}`, `Wiegand ${channel}`);

const enabledLockOptions = () => {
  const locks = Array.isArray(App.data?.locks) ? App.data.locks : [];
  const options = [1, 2].map((channel) => ({
    bit: channel,
    label: lockLabel(channel),
    enabled: locks.length ? locks.some((lock) => Number(lock.channel) === channel && lock.enable !== false) : true,
  })).filter((option) => option.enabled);
  return options.length ? options : [1, 2].map((channel) => ({ bit: channel, label: lockLabel(channel), enabled: true }));
};

const enabledWiegandOptions = () => {
  const devices = Array.isArray(App.data?.wiegand?.devices) ? App.data.wiegand.devices : [];
  return [1, 2]
    .map((channel) => ({
      bit: channel === 1 ? ALERT_TARGET_WG1 : ALERT_TARGET_WG2,
      label: wiegandLabel(channel),
      enabled: devices.length
        ? devices.some((device) => Number(device.channel) === channel && device.enable !== false)
        : true,
    }))
    .filter((option) => option.enabled);
};

const multiSelectOptions = (kind) => {
  if (kind === 'lock-target') return enabledLockOptions();
  if (kind === 'alert-target') {
    return [
      { bit: ALERT_TARGET_CONTROLLER, label: 'Controller', enabled: true },
      ...enabledWiegandOptions(),
    ];
  }
  return [];
};

const normalizeMaskForOptions = (value, kind, fallback = 1) => {
  const options = multiSelectOptions(kind);
  const availableMask = options.reduce((mask, option) => mask | option.bit, 0);
  const rawMask = kind === 'alert-target'
    ? normalizeAlertTarget(value, true)
    : normalizeChannelMask(value, fallback);
  if (kind === 'alert-target' && rawMask === 0) return 0;
  const selected = rawMask & availableMask;
  if (selected) return selected;
  if (kind === 'alert-target') return availableMask & ALERT_TARGET_CONTROLLER ? ALERT_TARGET_CONTROLLER : availableMask;
  return options[0]?.bit || fallback;
};

const multiSelectSummary = (kind, value) => {
  const options = multiSelectOptions(kind);
  const mask = normalizeMaskForOptions(value, kind);
  const labels = options.filter((option) => (mask & option.bit) !== 0).map((option) => option.label);
  if (!labels.length) return 'None';
  if (kind === 'lock-target' && labels.length === 2) return 'Both locks';
  if (kind === 'alert-target' && labels.length === options.length) return 'All outputs';
  return labels.join(' + ');
};

const renderMultiCheckboxControl = (label, className, value, kind, id = '') => {
  const normalized = normalizeMaskForOptions(value, kind);
  const inputAttrs = [
    'type="hidden"',
    `class="${escapeHtml(className)} multi-select-value"`,
    `value="${normalized}"`,
    `data-multi-kind="${escapeHtml(kind)}"`,
  ];
  if (id) inputAttrs.push(`id="${escapeHtml(id)}"`);
  const options = multiSelectOptions(kind);
  return `
    <label class="stacked multi-select-field">
      <span>${escapeHtml(label)}</span>
      <input ${inputAttrs.join(' ')}>
      <details class="multi-select-dropdown" data-multi-kind="${escapeHtml(kind)}">
        <summary><span class="multi-select-summary">${escapeHtml(multiSelectSummary(kind, normalized))}</span></summary>
        <div class="multi-select-menu">
          ${options.map((option) => `
            <label class="multi-select-option">
              <input type="checkbox" data-bit="${option.bit}" ${(normalized & option.bit) ? 'checked' : ''}>
              <span>${escapeHtml(option.label)}</span>
            </label>
          `).join('')}
        </div>
      </details>
    </label>
  `;
};

const refreshMultiSelectControl = (input, value = input?.value) => {
  if (!input?.classList?.contains('multi-select-value')) return;
  const field = input.closest('.multi-select-field');
  const details = field?.querySelector('.multi-select-dropdown');
  const summary = field?.querySelector('.multi-select-summary');
  const menu = field?.querySelector('.multi-select-menu');
  const kind = input.dataset.multiKind;
  const normalized = normalizeMaskForOptions(value, kind);
  input.value = String(normalized);
  if (summary) summary.textContent = multiSelectSummary(kind, normalized);
  if (menu) {
    menu.innerHTML = multiSelectOptions(kind).map((option) => `
      <label class="multi-select-option">
        <input type="checkbox" data-bit="${option.bit}" ${(normalized & option.bit) ? 'checked' : ''}>
        <span>${escapeHtml(option.label)}</span>
      </label>
    `).join('');
  }
};

const refreshAllMultiSelectControls = () => {
  document.querySelectorAll('.multi-select-value').forEach((input) => {
    refreshMultiSelectControl(input, input.value);
  });
};

const isMultiSelectOpen = (root = document) => !!root.querySelector?.('.multi-select-dropdown[open]');

const shouldDeferCredentialRender = () => {
  const root = App.elements.credentialUserList;
  if (!root) return false;
  if (isMultiSelectOpen(root)) return true;
  const active = document.activeElement;
  return !!active && root.contains(active) && active.matches('input, select, textarea, summary, button');
};

const setSelectValueIfIdle = (id, value) => {
  const el = document.getElementById(id);
  if (el?.classList?.contains('multi-select-value')) {
    if (!el.closest('.multi-select-field')?.querySelector('.multi-select-dropdown')?.open) {
      refreshMultiSelectControl(el, value);
    }
    return;
  }
  if (el && document.activeElement !== el) {
    el.value = String(value);
  }
};

const setCardModeState = (modeId, mode, latch) => {
  const modeEl = document.getElementById(modeId);
  if (modeEl && document.activeElement !== modeEl) {
    modeEl.value = normalizeCardMode(mode, latch);
  }
};

const setCardTargetState = (targetId, channelMask, fallback = 1) => {
  setSelectValueIfIdle(targetId, normalizeChannelMask(channelMask, fallback));
};

const setCardAlertTargetState = (targetId, alertTarget, alert = true) => {
  setSelectValueIfIdle(targetId, normalizeAlertTarget(alertTarget, alert));
};

const setLockArmState = (ch, armed) => {
  const label = document.getElementById(`armLabel_${ch}`);
  if (!label) return;
  // Show the state right in the arm button: locked+Armed vs unlocked+Disarmed.
  label.textContent = armed ? '🔒 Armed' : '🔓 Disarmed';
  const pill = label.closest('.lock-toggle-pill');
  if (pill) pill.classList.toggle('is-disarmed', !armed);
};

const applyLockState = (locks = []) => {
  locks.forEach((lock) => {
    const ch = lock.channel;
    const enableEl = document.getElementById(`enableLock_${ch}`);
    const armEl = document.getElementById(`arm_${ch}`);
    const contactEl = document.getElementById(`enableContactAlert_${ch}`);
    const failSecureEl = document.getElementById(`failSecure_${ch}`);
    const contactStatusEl = document.getElementById(`lockContact_${ch}`);
    const senseStatusEl = document.getElementById(`lockSense_${ch}`);

    if (enableEl) enableEl.checked = !!lock.enable;
    setCardEnabledState(`enableLock_${ch}`, !!lock.enable);
    if (armEl) armEl.checked = !!lock.arm;
    setLockArmState(ch, !!lock.arm, !!lock.enable);
    if (contactEl) contactEl.checked = !!lock.enableContactAlert;
    setCardAlertTargetState(`alertTargetLock_${ch}`, lock.alert_target, !!lock.enableContactAlert);
    if (failSecureEl) failSecureEl.checked = !!lock.failSecure;

    /* Ch1: contact state is in API "sense". Ch2: contact state is in API "contact". */
    const contactState = ch === 1 ? lock.sense : lock.contact;
    const signalState = ch === 1 ? lock.contact : lock.sense;

    if (contactStatusEl) applySignalDot(`lockContact_${ch}`, contactState, 'Contact closed', 'Contact open');
    if (senseStatusEl) applySignalDot(`lockSense_${ch}`, signalState);
  });
};

const applyExitState = (exits = []) => {
  exits.forEach((exit) => {
    const ch = exit.channel;
    const enableEl = document.getElementById(`enableExit_${ch}`);
    const alertEl = document.getElementById(`alertExit_${ch}`);
    const latchEl = document.getElementById(`latchExit_${ch}`);
    const invertEl = document.getElementById(`invertExit_${ch}`);
    const delayEl = document.getElementById(`armDelay_${ch}`);

    if (enableEl) enableEl.checked = !!exit.enable;
    setCardEnabledState(`enableExit_${ch}`, !!exit.enable);
    if (alertEl) alertEl.checked = !!exit.alert;
    setCardTargetState(`targetExit_${ch}`, exit.channel_mask, ch);
    setCardAlertTargetState(`alertTargetExit_${ch}`, exit.alert_target, !!exit.alert);
    if (latchEl) latchEl.checked = !!exit.latch;
    if (invertEl) invertEl.checked = !!exit.invert;
    setCardModeState(`modeExit_${ch}`, exit.mode, !!exit.latch);
    if (delayEl) delayEl.value = exit.delay ?? 0;
    applySignalDot(`exitSignal_${ch}`, exit.signal);
  });
};

const applyFobState = (fobs = []) => {
  fobs.forEach((fob) => {
    const ch = fob.channel;
    const enableEl = document.getElementById(`enableFob_${ch}`);
    const alertEl = document.getElementById(`alertFob_${ch}`);
    const latchEl = document.getElementById(`latchFob_${ch}`);
    const delayEl = document.getElementById(`fobDelay_${ch}`);

    if (enableEl) enableEl.checked = !!fob.enable;
    setCardEnabledState(`enableFob_${ch}`, !!fob.enable);
    if (alertEl) alertEl.checked = !!fob.alert;
    setCardTargetState(`targetFob_${ch}`, fob.channel_mask, ch);
    setCardAlertTargetState(`alertTargetFob_${ch}`, fob.alert_target, !!fob.alert);
    if (latchEl) latchEl.checked = !!fob.latch;
    setCardModeState(`modeFob_${ch}`, fob.mode, !!fob.latch);
    if (delayEl) delayEl.value = fob.delay ?? 4;
    applySignalDot(`fobSignal_${ch}`, fob.signal);
  });
};

const applyKeypadState = (keypads = []) => {
  keypads.forEach((pad) => {
    const ch = pad.channel;
    const enableEl = document.getElementById(`enableKeypad_${ch}`);
    const alertEl = document.getElementById(`alertKeypad_${ch}`);
    const latchEl = document.getElementById(`latchKeypad_${ch}`);
    const delayEl = document.getElementById(`keypadDelay_${ch}`);

    if (enableEl) enableEl.checked = !!pad.enable;
    setCardEnabledState(`enableKeypad_${ch}`, !!pad.enable);
    if (alertEl) alertEl.checked = !!pad.alert;
    setCardTargetState(`targetKeypad_${ch}`, pad.channel_mask, ch);
    setCardAlertTargetState(`alertTargetKeypad_${ch}`, pad.alert_target, !!pad.alert);
    if (latchEl) latchEl.checked = !!pad.latch;
    setCardModeState(`modeKeypad_${ch}`, pad.mode, !!pad.latch);
    if (delayEl) delayEl.value = pad.delay ?? 0;
    applySignalDot(`keypadSignal_${ch}`, pad.signal);
  });
};

const applyMotionState = (motions = []) => {
  motions.forEach((motion) => {
    const ch = motion.channel;
    const enableEl = document.getElementById(`enableMotion_${ch}`);
    const alertEl = document.getElementById(`alertMotion_${ch}`);
    const latchEl = document.getElementById(`latchMotion_${ch}`);
    const delayEl = document.getElementById(`motionDelay_${ch}`);

    if (enableEl) enableEl.checked = !!motion.enable;
    setCardEnabledState(`enableMotion_${ch}`, !!motion.enable);
    if (alertEl) alertEl.checked = !!motion.alert;
    setCardTargetState(`targetMotion_${ch}`, motion.channel_mask, ch);
    setCardAlertTargetState(`alertTargetMotion_${ch}`, motion.alert_target, !!motion.alert);
    if (latchEl) latchEl.checked = !!motion.latch;
    setCardModeState(`modeMotion_${ch}`, motion.mode, !!motion.latch);
    if (delayEl) delayEl.value = motion.delay ?? 4;
    applySignalDot(`motionSignal_${ch}`, motion.signal);
  });
};

const pinEntriesHaveActiveCode = (entries = []) =>
  Array.isArray(entries) && entries.some((entry) => entry?.active && String(entry.code || '').length > 0);

const signalStateHasCredentialActivity = (state = {}) => {
  const anyActive = (items = [], field = 'signal') =>
    Array.isArray(items) && items.some((item) => !!item?.[field]);
  return anyActive(state.exits) ||
    anyActive(state.fobs) ||
    anyActive(state.keypads) ||
    anyActive(state.motions) ||
    pinEntriesHaveActiveCode(state.wiegand?.pinEntries);
};

const applyFastSignalState = (state = {}) => {
  (state.locks || []).forEach((lock) => {
    const ch = lock.channel;
    const contactState = ch === 1 ? lock.sense : lock.contact;
    const signalState = ch === 1 ? lock.contact : lock.sense;
    applySignalDot(`lockContact_${ch}`, contactState, 'Contact closed', 'Contact open');
    applySignalDot(`lockSense_${ch}`, signalState);
  });

  (state.exits || []).forEach((exit) => applySignalDot(`exitSignal_${exit.channel}`, exit.signal));
  (state.fobs || []).forEach((fob) => applySignalDot(`fobSignal_${fob.channel}`, fob.signal));
  (state.keypads || []).forEach((pad) => applySignalDot(`keypadSignal_${pad.channel}`, pad.signal));
  (state.motions || []).forEach((motion) => applySignalDot(`motionSignal_${motion.channel}`, motion.signal));
  const previousPinActive = pinEntriesHaveActiveCode(App.data?.wiegand?.pinEntries);
  const nextPinActive = pinEntriesHaveActiveCode(state.wiegand?.pinEntries);
  if (App.data) {
    App.data.wiegand = mergeCredentialStateFromSummary(App.data.wiegand || {}, state.wiegand || {});
  }
  renderLivePinEntries(state.wiegand?.pinEntries || []);
  renderWiegandDevices(state.wiegand?.pinEntries || [], state.wiegand?.devices || App.data?.wiegand?.devices || []);
  if (Array.isArray(App.data?.keypadUsers)) {
    renderKeypadUsers(App.data.keypadUsers);
  }
  if (previousPinActive && !nextPinActive) {
    loadKeypadUsers().catch((error) => {
      console.warn('Failed to refresh PIN codes after keypad submit', error);
    });
  }
  if (signalStateHasCredentialActivity(state)) {
    refreshCredentialActivityData().catch((error) => {
      console.warn('Failed to refresh credential activity data', error);
    });
  }
  applyCredentialActivityState(state);
};

const findCredentialCard = (containerId, id) => {
  const container = document.getElementById(containerId) || App.elements.credentialUserList;
  if (!container || !id) return null;
  return Array.from(container.querySelectorAll('.credential-card'))
    .find((card) => card.dataset.id === String(id));
};

const applyCredentialActivityState = (state = {}) => {
  (state.wiegand?.users || []).forEach((user) => {
    const id = user.id || '';
    const card = findCredentialCard('wiegandUserList', id);
    const ageMs = Number(user.lastUsed?.age_ms);
    if (card) {
      updateActivityHighlight(card, ageMs, !!user.lastUsed);
    }
  });

  (state.rf?.users || []).forEach((user) => {
    const id = user.id || '';
    const card = findCredentialCard('rfUserList', id);
    const ageMs = Number(user.lastRx?.age_ms);
    if (card) {
      updateActivityHighlight(card, ageMs, !!user.lastRx);
    }
  });
  if (Array.isArray(App.data?.keypadUsers)) {
    applyPinCredentialActivityState(App.data.keypadUsers);
  }
  refreshActivityHighlights();
};

const renderCredentialEnableButton = (enabled, action, id) => `
  <button type="button"
    class="card-enable-toggle ${enabled ? 'is-enabled' : 'is-disabled'}"
    data-action="${action}"
    data-id="${escapeHtml(id || '')}"
    aria-pressed="${enabled ? 'true' : 'false'}"
    aria-label="${enabled ? 'Disable' : 'Enable'}"
    title="${enabled ? 'Disable' : 'Enable'}">
    <span class="card-enable-icon" aria-hidden="true"></span>
  </button>
`;

const renderCredentialIconButton = (action, id, label, iconClass, extraAttrs = '') => `
  <button type="button"
    class="credential-icon-button ${iconClass}"
    data-action="${action}"
    data-id="${escapeHtml(id || '')}"
    aria-label="${escapeHtml(label)}"
    title="${escapeHtml(label)}"
    ${extraAttrs}>
    <span aria-hidden="true"></span>
  </button>
`;

const renderCredentialAlertToggle = (className, checked, label = 'Alert beep') => `
  <label class="credential-icon-button credential-alert-toggle${checked ? ' is-active' : ''}" title="${escapeHtml(label)}" aria-label="${escapeHtml(label)}">
    <input type="checkbox" class="${className}" aria-label="${escapeHtml(label)}" ${checked ? 'checked' : ''}>
    <span class="credential-speaker-icon" aria-hidden="true"></span>
  </label>
`;

const renderLockTargetSelect = (className, channelMask) =>
  renderMultiCheckboxControl('Target', className, channelMask, 'lock-target');

const renderKeypadAccessSelect = (className, keypadMask) => `
  <label class="stacked">
    <span>Keypad access</span>
    <select class="${className}">
      <option value="1" ${Number(keypadMask) === 1 ? 'selected' : ''}>Keypad 1</option>
      <option value="2" ${Number(keypadMask) === 2 ? 'selected' : ''}>Keypad 2</option>
      <option value="3" ${Number(keypadMask) === 3 ? 'selected' : ''}>Both keypads</option>
    </select>
  </label>
`;

const renderAlertTargetSelect = (className, alertTarget, alert = true) =>
  renderMultiCheckboxControl('Alert output', className, normalizeAlertTarget(alertTarget, alert), 'alert-target');

const setupCredentialIconControls = () => {
  const removeButtons = [
    { button: App.elements.wiegandRemoveAllBtn, label: 'Remove all RFID cards' },
    { button: App.elements.keypadRemoveAllBtn, label: 'Remove all PIN codes' },
    { button: App.elements.rfRemoveAllBtn, label: 'Remove all remotes' },
  ];

  removeButtons.forEach(({ button, label }) => {
    if (!button) return;
    button.classList.add('credential-icon-button', 'credential-remove-icon', 'credential-section-remove');
    button.setAttribute('aria-label', label);
    button.title = label;
    button.innerHTML = '<span aria-hidden="true"></span>';
  });
};

const setupMultiSelectControls = () => {
  document.addEventListener('change', (event) => {
    const checkbox = event.target.closest('.multi-select-menu input[type="checkbox"][data-bit]');
    if (!checkbox) return;
    const field = checkbox.closest('.multi-select-field');
    const input = field?.querySelector('.multi-select-value');
    if (!input) return;
    const kind = input.dataset.multiKind;
    const checked = Array.from(field.querySelectorAll('.multi-select-menu input[type="checkbox"][data-bit]:checked'));
    let mask = checked.reduce((sum, item) => sum | Number(item.dataset.bit || 0), 0);
    if (kind === 'lock-target' && mask === 0) {
      checkbox.checked = true;
      mask = Number(checkbox.dataset.bit || 0);
    }
    input.value = String(mask);
    refreshMultiSelectControl(input, mask);
    input.dispatchEvent(new Event('change', { bubbles: true }));
  });
};

const findRenderedCredentialCard = (container) => {
  const id = container?.getAttribute('data-id');
  if (!id) return null;
  const typeClass = ['credential-card--rfid', 'credential-card--remote', 'credential-card--pin']
    .find((className) => container.classList.contains(className));
  return Array.from(document.querySelectorAll('.credential-card'))
    .find((candidate) => (
      candidate.getAttribute('data-id') === id
      && (!typeClass || candidate.classList.contains(typeClass))
    )) || null;
};

const credentialCardTimerKey = (container) => {
  const id = container?.getAttribute('data-id');
  if (!id) return container;
  const typeClass = ['credential-card--rfid', 'credential-card--remote', 'credential-card--pin']
    .find((className) => container.classList.contains(className)) || 'credential-card';
  return `${typeClass}:${id}`;
};

const scheduleCredentialNameSave = (input, saveHandler) => {
  const container = input?.closest('.user-row');
  if (!container || !input.value.trim()) return;
  const pendingValue = input.value;
  const timerKey = credentialCardTimerKey(container);
  const existing = App.credentialAutoSaveTimers.get(timerKey);
  if (existing) clearTimeout(existing);

  const timer = setTimeout(async () => {
    App.credentialAutoSaveTimers.delete(timerKey);
    const targetContainer = container.isConnected ? container : findRenderedCredentialCard(container);
    if (!targetContainer) return;
    const targetInput = targetContainer.querySelector('.user-name-input');
    if (targetContainer !== container && targetInput && !targetInput.matches(':focus')) {
      targetInput.value = pendingValue;
    }
    try {
      await saveHandler(targetContainer);
    } catch (error) {
      // The save handler already surfaces the failure.
    }
  }, 650);

  App.credentialAutoSaveTimers.set(timerKey, timer);
};

const renderWiegandDevices = (pinEntries = [], devices = App.data?.wiegand?.devices || []) => {
  [1, 2].forEach((channel) => {
    const entry = Array.isArray(pinEntries)
      ? pinEntries.find((item) => Number(item?.channel) === channel)
      : null;
    const device = Array.isArray(devices)
      ? devices.find((item) => Number(item?.channel) === channel)
      : null;
    const active = !!entry?.active;
    const enabled = device ? device.enable !== false : true;
    const codeEl = document.getElementById(`wiegandDeviceCode_${channel}`);
    const lengthEl = document.getElementById(`wiegandDeviceLength_${channel}`);
    const enableEl = document.getElementById(`enableWiegandDevice_${channel}`);
    const card = enableEl?.closest('.wiegand-device-card');
    if (codeEl) codeEl.textContent = entry?.code ? entry.code : '—';
    if (lengthEl) lengthEl.textContent = String(Number(entry?.length) || 0);
    if (enableEl) enableEl.checked = enabled;
    if (card) card.classList.toggle('is-card-disabled', !enabled);
    setCardEnabledState(`enableWiegandDevice_${channel}`, enabled);
    applySignalDot(`wiegandDeviceSignal_${channel}`, active, 'PIN digits active', 'Idle');
  });
};

const buildWiegandUserRow = (user, existingValue) => {
  if (!user) return '';
  const meta = WIEGAND_STATUS_META[user.status] || WIEGAND_STATUS_META[2];
  const statusClass = meta.className ? `status-chip ${meta.className}` : 'status-chip';
  const rawCode = user.code || '';
  const hexCode = binaryToHex(rawCode);
  const preserved = existingValue && typeof existingValue === 'object' ? existingValue : { name: existingValue };
  // Use existing form values if user was editing, otherwise use stored config
  const name = escapeHtml(preserved.name !== undefined ? preserved.name : (user.name || ''));
  const channelNum = user.channel || 0;
  const userId = escapeHtml(user.id || '');
  const alert = preserved.alert !== undefined ? !!preserved.alert : (user.alert !== false);
  const alertTarget = normalizeAlertTarget(preserved.alert_target ?? user.alert_target, alert);
  const channelMask = normalizeChannelMask(preserved.channel_mask ?? user.channel_mask, channelNum ? (1 << (channelNum - 1)) : 3);
  const mode = preserved.mode || user.mode || 'momentary';
  const enabled = user.status === 1;
  const metrics = `
    <div class="rf-card-metrics wiegand-card-metrics">
      ${buildLastUsedMetric(user.lastUsed)}
    </div>
  `;

  return `
    <div class="user-row credential-card credential-card--rfid ${enabled ? '' : 'is-card-disabled'}" data-id="${userId}" data-channel="${channelNum}" data-enabled="${enabled ? 'true' : 'false'}">
      <div class="credential-card-header">
        <div class="credential-card-title">
          <span class="credential-kind">RFID</span>
          <span class="user-code">${escapeHtml(hexCode)}</span>
        </div>
        <div class="credential-card-actions">
          ${renderCredentialEnableButton(enabled, 'toggle-wiegand-enabled', user.id || '')}
          ${renderCredentialIconButton('delete-wiegand', user.id || '', 'Delete RFID card', 'credential-remove-icon')}
        </div>
      </div>
      <div class="user-info">
        <label class="stacked">
          <span>Name</span>
          <input type="text" class="user-name-input" value="${name}" placeholder="Enter name...">
        </label>
        <div class="credential-meta-row">
          <span class="user-channel">${escapeHtml(formatChannelLabel(channelNum))}</span>
          <span class="${statusClass}">${meta.label}</span>
        </div>
        <label class="stacked">
          <span>Mode</span>
          <select class="wiegand-mode-select" data-previous-value="${escapeHtml(mode)}">
            <option value="momentary" ${mode === 'momentary' ? 'selected' : ''}>Momentary</option>
            <option value="toggle" ${mode === 'toggle' ? 'selected' : ''}>Toggle</option>
            <option value="latch" ${mode === 'latch' ? 'selected' : ''}>Latch</option>
          </select>
        </label>
        <div class="user-config">
          ${renderLockTargetSelect('wiegand-channel-select', channelMask)}
          ${renderAlertTargetSelect('wiegand-alert-target-select', alertTarget, alert)}
        </div>
        ${metrics}
      </div>
    </div>
  `;
};

const startWiegandPolling = () => {
  if (App.wiegandPollTimer || !isDevicePageActive() || document.hidden) return;
  App.wiegandPollTimer = setInterval(async () => {
    if (!isDevicePageActive() || document.hidden) return;
    try {
      const wiegand = await fetchJSON('api/wiegand');
      if (App.data) {
        App.data.wiegand = wiegand;
      }
      renderWiegand(wiegand);
    } catch (error) {
      console.warn('Failed to refresh Wiegand state', error);
      stopWiegandPolling();
    }
  }, 10000);
};

const stopWiegandPolling = () => {
  if (App.wiegandPollTimer) {
    clearInterval(App.wiegandPollTimer);
    App.wiegandPollTimer = null;
  }
};

const isDevicePageActive = () => document.getElementById('page-device')?.classList.contains('active');

let deviceCredentialDetailsInFlight = null;
let deviceCredentialRetryTimer = null;
let deviceCredentialLoadTimer = null;

const mergeCredentialStateFromSummary = (previous = {}, incoming = {}) => {
  if (!incoming || typeof incoming !== 'object') {
    return previous || {};
  }

  const merged = { ...(previous || {}), ...incoming };
  const previousUsers = Array.isArray(previous?.users) ? previous.users : null;
  const incomingUsers = Array.isArray(incoming.users) ? incoming.users : null;
  const incomingIsEmptyPlaceholder =
    !incomingUsers ||
    incoming.busy === true ||
    incoming.summary === true ||
    incomingUsers.length === 0;

  if (previousUsers && incomingIsEmptyPlaceholder) {
    merged.users = previousUsers;
  }

  return merged;
};

const mergeStateCredentialDetails = (previous = {}, incoming = {}) => ({
  ...incoming,
  wiegand: mergeCredentialStateFromSummary(previous?.wiegand, incoming?.wiegand),
  rf: mergeCredentialStateFromSummary(previous?.rf, incoming?.rf),
  keypadUsers: Array.isArray(incoming?.keypadUsers)
    ? incoming.keypadUsers
    : (Array.isArray(previous?.keypadUsers) ? previous.keypadUsers : incoming?.keypadUsers),
  // /api/state never includes schedules (it's only ever served by /api/schedules), so without this
  // fallback every 30s state poll wipes App.data.schedules and the next renderSchedules() call
  // (e.g. clicking any schedule chip) renders as if every custom profile had been deleted.
  schedules: incoming?.schedules !== undefined ? incoming.schedules : previous?.schedules,
});

const scheduleDeviceCredentialRetry = () => {
  if (deviceCredentialRetryTimer || !isDevicePageActive()) return;
  deviceCredentialRetryTimer = setTimeout(() => {
    deviceCredentialRetryTimer = null;
    if (!isDevicePageActive()) return;
    loadDeviceCredentialDetails();
  }, 1500);
};

const scheduleDeviceCredentialLoad = () => {
  if (deviceCredentialLoadTimer || deviceCredentialDetailsInFlight) return;
  deviceCredentialLoadTimer = setTimeout(() => {
    deviceCredentialLoadTimer = null;
    if (!isDevicePageActive()) return;
    loadDeviceCredentialDetails().catch((error) => {
      console.warn('Failed to load credential details', error);
      scheduleDeviceCredentialRetry();
    });
  }, 1200);
};

const loadDeviceCredentialDetails = async () => {
  if (deviceCredentialDetailsInFlight) return deviceCredentialDetailsInFlight;
  deviceCredentialDetailsInFlight = (async () => {
    await loadCredentialDetails();
    await sleep(150);
    await loadKeypadUsers();
    await sleep(150);
    await loadSchedules();
  })().finally(() => {
    deviceCredentialDetailsInFlight = null;
  });
  return deviceCredentialDetailsInFlight;
};

const loadCredentialDetails = async () => {
  if (!App.data) App.data = {};
  let wiegandResult;
  let rfResult;

  try {
    wiegandResult = { status: 'fulfilled', value: await fetchJSON(`api/wiegand?t=${Date.now()}`) };
  } catch (error) {
    wiegandResult = { status: 'rejected', reason: error };
  }

  if (wiegandResult.status === 'fulfilled') {
    App.data.wiegand = wiegandResult.value;
    renderWiegand(wiegandResult.value);
  }

  await sleep(150);

  try {
    rfResult = { status: 'fulfilled', value: await fetchJSON(`api/rf?t=${Date.now()}`) };
  } catch (error) {
    rfResult = { status: 'rejected', reason: error };
  }

  if (rfResult.status === 'fulfilled') {
    App.data.rf = rfResult.value;
    renderRf(rfResult.value);
  }
  if (wiegandResult.status !== 'fulfilled' || rfResult.status !== 'fulfilled') {
    console.warn('Failed to load credential details', {
      wiegand: wiegandResult.status === 'rejected' ? wiegandResult.reason : null,
      rf: rfResult.status === 'rejected' ? rfResult.reason : null,
    });
    scheduleDeviceCredentialRetry();
  }
};

const applyEnrollmentUpdate = (update = {}) => {
  if (!App.data) App.data = {};
  if (update.enrollment) {
    App.data.enrollment = update.enrollment;
    renderEnrollment(update.enrollment);
  }
  if (update.rf) {
    App.data.rf = update.rf;
    renderRf(update.rf);
  }
  if (update.wiegand) {
    App.data.wiegand = update.wiegand;
    renderWiegand(update.wiegand);
  }
  if (Array.isArray(update.keypadUsers)) {
    App.data.keypadUsers = update.keypadUsers;
    renderKeypadUsers(update.keypadUsers);
  }
};

const refreshEnrollmentState = async () => {
  const [enrollmentResult, wiegandResult, rfResult, keypadUsersResult] = await Promise.allSettled([
    fetchJSON(`api/enrollment?t=${Date.now()}`),
    fetchJSON(`api/wiegand?t=${Date.now()}`),
    fetchJSON(`api/rf?t=${Date.now()}`),
    fetchJSON(`api/keypad/users?t=${Date.now()}`),
  ]);

  const update = {};
  if (enrollmentResult.status === 'fulfilled') update.enrollment = enrollmentResult.value;
  if (wiegandResult.status === 'fulfilled') update.wiegand = wiegandResult.value;
  if (rfResult.status === 'fulfilled') update.rf = rfResult.value;
  if (keypadUsersResult.status === 'fulfilled' && Array.isArray(keypadUsersResult.value)) {
    update.keypadUsers = keypadUsersResult.value;
  }
  applyEnrollmentUpdate(update);

  if (enrollmentResult.status !== 'fulfilled') {
    throw enrollmentResult.reason;
  }
};

const startEnrollmentPolling = () => {
  if (App.enrollmentPollTimer) return;
  App.enrollmentPollTimer = setInterval(async () => {
    try {
      await refreshEnrollmentState();
    } catch (error) {
      console.warn('Failed to refresh enrollment state', error);
      stopEnrollmentPolling();
    }
  }, 900);
};

const stopEnrollmentPolling = () => {
  if (App.enrollmentPollTimer) {
    clearInterval(App.enrollmentPollTimer);
    App.enrollmentPollTimer = null;
  }
};

const deleteItemsSequentially = async (items, endpoint, payloadKey) => {
  let latest = null;
  for (const item of items) {
    const id = typeof item === 'string' ? item : item?.[payloadKey];
    if (!id) continue;
    latest = await fetchJSON(endpoint, {
      method: endpoint.includes('keypad') ? 'DELETE' : 'POST',
      body: JSON.stringify({ [payloadKey]: id }),
    });
  }
  return latest;
};

const renderWiegand = (wiegand = {}) => {
  const {
    registrationActive = false,
    registrationChannel = 0,
    registrationPending = 0,
    lastDuplicateCode = '',
    users = [],
    pinEntries = [],
    devices = [],
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
    registerBtn.hidden = registrationActive;
    registerBtn.disabled = registrationActive;
  }
  if (channelSelect) {
    channelSelect.disabled = registrationActive;
  }
  if (stopBtn) {
    stopBtn.hidden = !registrationActive;
    stopBtn.disabled = !registrationActive;
    stopBtn.classList.toggle('is-listening', registrationActive);
  }
  if (App.elements.wiegandRemoveAllBtn) {
    App.elements.wiegandRemoveAllBtn.disabled = registrationActive || !users.length;
  }
  renderWiegandDevices(pinEntries, devices);
  renderLivePinEntries(pinEntries);

  if (listEl) {
    if (!users || users.length === 0) {
      listEl.innerHTML = '<p class="empty-state muted">No RFID cards registered yet. Click "Register" to add cards.</p>';
    } else {
      // Preserve name input values that user may be editing
      const existingValues = {};
      let focusedId = null;
      listEl.querySelectorAll('.user-row').forEach((row) => {
        const id = row.getAttribute('data-id');
        if (!id) return;
        const nameInput = row.querySelector('.user-name-input');
        const modeInput = row.querySelector('.wiegand-mode-select');
        const channelInput = row.querySelector('.wiegand-channel-select');
        const alertTargetInput = row.querySelector('.wiegand-alert-target-select');
        if (nameInput || modeInput || channelInput || alertTargetInput) {
          existingValues[id] = {
            name: nameInput ? nameInput.value : undefined,
            mode: modeInput ? modeInput.value : undefined,
            channel_mask: channelInput ? Number(channelInput.value) : undefined,
            alert_target: alertTargetInput ? alertTargetInput.value : undefined,
            alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
          };
          if (document.activeElement === nameInput) {
            focusedId = id;
          }
        }
      });

      listEl.innerHTML = users
        .map((user) => buildWiegandUserRow(user, existingValues[user.id]))
        .join('');
      applyCredentialActivityState({ wiegand });

      // Restore focus if user was editing
      if (focusedId) {
        const row = listEl.querySelector(`.user-row[data-id="${focusedId}"]`);
        const input = row?.querySelector('.user-name-input');
        if (input) {
          input.focus();
          input.setSelectionRange(input.value.length, input.value.length);
        }
      }
    }
  }

  renderCredentialUsers();
  updateRfReceivedTimestamps();
  ensureRfTimestampTimer();
  startWiegandPolling();
};

// Remote FOBs (433 MHz)
const buildRfUserRow = (user, existingValue) => {
  if (!user) return '';
  const name = escapeHtml(existingValue?.name !== undefined ? existingValue.name : (user.name || ''));
  const code = user.code ? `0x${escapeHtml(user.code)}` : '—';
  const mode = existingValue?.mode || user.mode || 'momentary';
  const channelMask = existingValue?.channel_mask || user.channel_mask || 1;
  const exitSeconds = existingValue?.exit_seconds ?? user.exit_seconds ?? 4;
  const alert = existingValue?.alert ?? (user.alert ?? true);
  const alertTarget = normalizeAlertTarget(existingValue?.alert_target ?? user.alert_target, alert);
  const enabled = existingValue?.enabled ?? (user.enabled !== false);
  const metrics = buildRfUserMetrics(user);

  return `
    <div class="user-row credential-card credential-card--remote ${enabled ? '' : 'is-card-disabled'}" data-id="${escapeHtml(user.id || '')}" data-enabled="${enabled ? 'true' : 'false'}">
      <div class="credential-card-header">
        <div class="credential-card-title">
          <span class="credential-kind">Remote</span>
          <span class="user-code">${code}</span>
        </div>
        <div class="credential-card-actions">
          ${renderCredentialEnableButton(enabled, 'toggle-rf-enabled', user.id || '')}
          ${renderCredentialIconButton('delete-rf', user.id || '', 'Delete remote', 'credential-remove-icon')}
        </div>
      </div>
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
              <option value="momentary" ${mode === 'momentary' ? 'selected' : ''}>Momentary</option>
              <option value="latch" ${mode === 'latch' ? 'selected' : ''}>Latch</option>
              <option value="exit" ${mode === 'exit' ? 'selected' : ''}>Exit pulse</option>
              <option value="power_on" ${mode === 'power_on' ? 'selected' : ''}>Power ON</option>
              <option value="power_off" ${mode === 'power_off' ? 'selected' : ''}>Power OFF</option>
            </select>
          </label>
          <label class="stacked">
            <span>Target</span>
            <select class="rf-channel-select">
              <option value="1" ${channelMask === 1 ? 'selected' : ''}>Lock 1</option>
              <option value="2" ${channelMask === 2 ? 'selected' : ''}>Lock 2</option>
              <option value="3" ${channelMask === 3 ? 'selected' : ''}>Both locks</option>
            </select>
          </label>
          ${renderAlertTargetSelect('rf-alert-target-select', alertTarget, alert)}
          <label class="stacked">
            <span>Exit duration (s)</span>
            <input type="number" class="rf-exit-seconds" min="1" step="1" value="${exitSeconds}">
          </label>
        </div>
        ${metrics}
      </div>
    </div>
  `;
};

const getRfUserDefaults = (user = {}) => ({
  id: user.id || '',
  name: user.name || 'Remote Fob',
  mode: user.mode || 'momentary',
  channel_mask: Number(user.channel_mask) || 1,
  exit_seconds: Number(user.exit_seconds) || 4,
  alert: user.alert ?? true,
  alert_target: normalizeAlertTarget(user.alert_target, user.alert ?? true),
  enabled: user.enabled !== false,
});

const buildRfUserMetric = (label, value, muted = false) => `
  <div class="rf-card-metric${muted ? ' muted' : ''}">
    <span class="label">${label}</span>
    <span class="value">${value}</span>
  </div>
`;

const buildRfUserMetrics = (user = {}) => {
  const rx = user.lastRx || null;
  if (!rx) {
    return `
      <div class="rf-card-metrics">
        ${buildRfUserMetric('Last packet', 'No packets yet', true)}
      </div>
    `;
  }

  const qualityScore = Number(rx.qualityScore);
  const quality = Number.isFinite(qualityScore)
    ? `${escapeHtml(rx.qualityLabel || 'Signal')} · ${qualityScore}/100`
    : '—';
  const shortUs = Number(rx.shortUs) || 0;
  const longUs = Number(rx.longUs) || 0;
  const timing = shortUs && longUs ? `${shortUs}us / ${longUs}us` : '—';
  const decodeOk = Number(rx.decodeOkCount) || 0;
  const captures = Number(rx.captureCount) || 0;

  return `
    <div class="rf-card-metrics">
      ${buildRfUserMetric('RF quality', quality, !qualityScore)}
      <div class="rf-card-metric">
        <span class="label">Last received</span>
        <span class="value rf-last-received"
          data-unix-time="${Number(rx.unixTime) || 0}"
          data-received-ms="${Number(rx.received_ms) || 0}"
          data-age-ms="${Number(rx.age_ms) || 0}"
          data-rendered-at-ms="${Date.now()}">${formatRfReceivedLabel(rx.unixTime, rx.received_ms, rx.age_ms)}</span>
      </div>
      ${buildRfUserMetric('Timing', timing, timing === '—')}
      ${buildRfUserMetric('Jitter', formatPercent(rx.jitterPercent, 1), !shortUs)}
      ${buildRfUserMetric('Repeats', String(rx.repeatCount || 0), !rx.repeatCount)}
      ${buildRfUserMetric('Noise', `${formatPercent(rx.noisePercent)} · ${formatRate(rx.noiseRatePerSecond)}`, false)}
      ${buildRfUserMetric('Decode', `${decodeOk}/${captures} · ${formatPercent(rx.decodeSuccessRatePercent)}`, !decodeOk)}
      ${buildRfUserMetric('Capture', `${rx.lastCapturePulses || 0} pulses · sync ${rx.syncCount || 0}`, !rx.lastCapturePulses)}
    </div>
  `;
};

const autoSaveRfUser = async (user) => {
  const config = getRfUserDefaults(user);
  if (!config.id || App.rfAutoSavedIds.has(config.id) || App.rfAutoSavingIds.has(config.id)) {
    return;
  }

  App.rfAutoSavingIds.add(config.id);
  try {
    if (config.name.trim()) {
      await fetchJSON('api/rf/rename', {
        method: 'POST',
        body: JSON.stringify({ id: config.id, name: config.name.trim() }),
      });
    }
    await fetchJSON('api/rf/config', {
      method: 'POST',
        body: JSON.stringify({
          id: config.id,
          mode: config.mode,
          channel_mask: config.channel_mask,
          exit_seconds: config.exit_seconds,
          alert: !!config.alert,
          alert_target: config.alert_target,
          enabled: config.enabled !== false,
        }),
    });
    App.rfAutoSavedIds.add(config.id);
  } catch (error) {
    console.warn('Failed to auto-save remote defaults', error);
  } finally {
    App.rfAutoSavingIds.delete(config.id);
  }
};

const setMetricValue = (element, value, muted = false) => {
  if (!element) return;
  element.textContent = value;
  element.classList.toggle('muted', muted || value === '—');
};

const renderRfDiagnostics = (receiver = {}) => {
  const score = Number(receiver.qualityScore);
  const label = receiver.qualityLabel || 'No signal';
  const qualityText = Number.isFinite(score) ? `${label} · ${score}/100` : '—';
  setMetricValue(App.elements.rfQuality, qualityText, !score);

  const codeText = formatRemoteCode(receiver.lastCode);
  setMetricValue(App.elements.rfLastCode, codeText, codeText === '—');

  const shortUs = Number(receiver.lastShortUs) || 0;
  const longUs = Number(receiver.lastLongUs) || 0;
  setMetricValue(
    App.elements.rfTiming,
    shortUs && longUs ? `${shortUs}us / ${longUs}us` : '—',
    !(shortUs && longUs)
  );

  const hasDecodedFrame = Number(receiver.lastCode) > 0 && shortUs > 0 && longUs > 0;
  setMetricValue(
    App.elements.rfJitter,
    hasDecodedFrame ? formatPercent(receiver.lastJitterPercent, 1) : '—',
    !hasDecodedFrame
  );
  setMetricValue(App.elements.rfRepeats, String(receiver.lastRepeatCount || 0), !receiver.lastRepeatCount);
  setMetricValue(
    App.elements.rfNoise,
    `${formatPercent(receiver.noisePercent)} · ${formatRate(receiver.noiseRatePerSecond)}`,
    !receiver.edgeCount
  );
  setMetricValue(
    App.elements.rfDecode,
    `${receiver.decodeOkCount || 0}/${receiver.captureCount || 0} · ${formatPercent(receiver.decodeSuccessRatePercent)}`,
    !receiver.decodeOkCount
  );
  setMetricValue(
    App.elements.rfCaptures,
    `${receiver.lastCapturePulses || 0} pulses · sync ${receiver.syncCount || 0}`,
    !receiver.captureCount
  );
  setMetricValue(
    App.elements.rfLastInput,
    `${formatAge(receiver.lastDecodeAgeMs)} · edge ${formatRate(receiver.edgeRatePerSecond)}`,
    !receiver.lastCode
  );
  setMetricValue(
    App.elements.rfReceiver,
    `GPIO${receiver.gpio ?? '—'} · ${receiver.isrAddResult || '—'}`,
    receiver.isrAddResult !== 'ESP_OK'
  );
};

const renderRf = (rf = {}) => {
  const {
    registrationActive = false,
    registrationPending = 0,
    lastDuplicateCode = '',
    users = [],
  } = rf;

  if (App.data) {
    App.data.rf = rf;
  }

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
  if (App.elements.rfRemoveAllBtn) {
    App.elements.rfRemoveAllBtn.disabled = registrationActive || !users.length;
  }

  renderRfDiagnostics(rf.receiver || {});

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
        const alertTargetInput = row.querySelector('.rf-alert-target-select');
        existingValues[id] = {
          name: nameInput ? nameInput.value : undefined,
          mode: modeSel ? modeSel.value : undefined,
          channel_mask: chSel ? Number(chSel.value) : undefined,
          exit_seconds: exitInput ? Number(exitInput.value) : undefined,
          alert_target: alertTargetInput ? alertTargetInput.value : undefined,
          alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
          enabled: row.dataset.enabled !== 'false',
        };
      });

      listEl.innerHTML = users
        .map((u) => buildRfUserRow(u, existingValues[u.id]))
        .join('');

      users.forEach((user) => autoSaveRfUser(user));
    }
  }

  renderCredentialUsers();
  updateRfReceivedTimestamps();
  ensureRfTimestampTimer();

  if (registrationActive) {
    if (!App.rfPollTimer) {
      App.rfPollTimer = setInterval(() => {
        refreshEnrollmentState().catch((error) => {
          console.warn('Failed to refresh RF registration state', error);
        });
      }, 900);
    }
  } else if (App.rfPollTimer) {
    clearInterval(App.rfPollTimer);
    App.rfPollTimer = null;
  }
};

const renderEnrollment = (enrollment = {}) => {
  const active = !!enrollment.active;
  const startBtn = App.elements.enrollStartBtn;
  const stopBtn = App.elements.enrollStopBtn;
  const selectEl = App.elements.enrollUserSelect;
  const newUserName = document.getElementById('enrollNewUserName');

  if (startBtn) {
    startBtn.hidden = active;
    startBtn.disabled = active;
  }
  if (stopBtn) {
    stopBtn.hidden = !active;
    stopBtn.disabled = !active;
    stopBtn.classList.toggle('is-listening', active);
  }
  if (selectEl) selectEl.disabled = active;
  if (newUserName) newUserName.disabled = active;

  if (active) {
    startEnrollmentPolling();
  } else {
    stopEnrollmentPolling();
  }
};

const renderState = (state = {}) => {
  applyDeviceInfo(state.device || {});
  applyServerInfo(state.server || {});
  applySystemInfo(state.system || {});
  applyLockState(state.locks || []);
  applyExitState(state.exits || []);
  applyFobState(state.fobs || []);
  applyKeypadState(state.keypads || []);
  applyMotionState(state.motions || []);
  renderWiegand(state.wiegand || {});
  renderRf(state.rf || {});
  renderEnrollment(state.enrollment || {});
  // Keypad users and logs are heavy; loaded via dedicated endpoints so state polling
  // can't fragment heap on the ESP32 or wipe UI lists when omitted from /api/state.
  if (Array.isArray(state.keypadUsers)) {
    renderKeypadUsers(state.keypadUsers);
  }
  if (Array.isArray(state.logs)) {
    renderLogs(state.logs);
  }
  renderWifi(state.wifi || {}, state.device?.network || {}, state.system || {});
  refreshAllMultiSelectControls();
};

const DEVICE_STATE_ERROR_THROTTLE_MS = 15000;
let lastDeviceStateErrorToast = 0;
let stateInFlight = null;
let consecutiveStateFailures = 0;
let nextStateToastAt = 0;
const DEVICE_STATE_TOAST_BACKOFF_MS = 120000;
let wifiNetworksCache = null;
let wifiScanCache = null;
let wifiListLoaded = false;
let wifiScanLoaded = false;
let wifiListLoading = false;
let wifiScanLoading = false;
let wifiListError = null;
let wifiScanError = null;
let wifiScanRetryTimer = null;
let signalsInFlight = null;
let signalPollFailures = 0;
let signalPollRetryTimer = null;
let signalPollStartTimer = null;

const renderWifiFromCache = () => {
  const wifi = (App.data && App.data.wifi) ? App.data.wifi : {};
  renderWifi(
    { ...wifi, networks: wifiNetworksCache || [], scanned: wifiScanCache || [] },
    App.data?.device?.network || {},
    App.data?.system || {}
  );
};

const scheduleWifiScanRetry = () => {
  if (wifiScanRetryTimer || document.hidden || getActivePageId() !== 'settings') return;
  wifiScanRetryTimer = setTimeout(() => {
    wifiScanRetryTimer = null;
    if (!document.hidden && getActivePageId() === 'settings') {
      loadWifiList({ force: true, scan: true });
    }
  }, 5000);
};

const applyWifiListSnapshot = (networks = wifiNetworksCache || [], scanned = wifiScanCache || []) => {
  wifiNetworksCache = Array.isArray(networks) ? networks : [];
  wifiScanCache = Array.isArray(scanned) ? scanned : [];
  wifiListLoaded = Array.isArray(networks) || wifiListLoaded;
  if (App.data) {
    App.data.wifi = App.data.wifi || {};
    App.data.wifi.networks = wifiNetworksCache;
    App.data.wifi.scanned = wifiScanCache;
  }
  renderWifiFromCache();
};

const loadState = async (options = {}) => {
  const { suppressErrorToast = false, timeoutMs = 8000 } = options;
  if (stateInFlight) return stateInFlight;

  stateInFlight = (async () => {
  try {
    const data = await fetchJSON(`api/state?t=${Date.now()}`, { timeoutMs });
    App.data = mergeStateCredentialDetails(App.data || {}, data);
    consecutiveStateFailures = 0;
    nextStateToastAt = 0;
    if (wifiNetworksCache !== null) {
      App.data.wifi = App.data.wifi || {};
      App.data.wifi.networks = wifiNetworksCache;
    }
    if (wifiScanCache !== null) {
      App.data.wifi = App.data.wifi || {};
      App.data.wifi.scanned = wifiScanCache;
    }
    renderState(App.data);
    if (App.elements.toast && !App.elements.toast.hidden) {
      App.elements.toast.hidden = true;
      App.elements.toast.classList.remove('show');
    }
    return true;
  } catch (error) {
    consecutiveStateFailures++;
    const now = Date.now();
    const shouldToast =
      consecutiveStateFailures === 1 || (nextStateToastAt > 0 && now >= nextStateToastAt);
    if (!suppressErrorToast && shouldToast && now - lastDeviceStateErrorToast >= DEVICE_STATE_ERROR_THROTTLE_MS) {
      lastDeviceStateErrorToast = now;
      nextStateToastAt = now + DEVICE_STATE_TOAST_BACKOFF_MS;
      handleError(error, 'Unable to load device state');
    }
    return false;
  }
  })().finally(() => {
    stateInFlight = null;
  });

  return stateInFlight;
};

const stopNetworkHandoffPolling = () => {
  if (!App.networkHandoffTimer) return;
  clearTimeout(App.networkHandoffTimer);
  App.networkHandoffTimer = null;
};

const startNetworkHandoffPolling = () => {
  stopNetworkHandoffPolling();
  const deadline = Date.now() + 90000;

  // Do not let a pre-reboot STA address look like the result of this handoff.
  // The next successful state request will restore the live value.
  if (App.data?.device?.network) {
    App.data.device.network.wifi_sta_ip = null;
    App.data.device.network.wifi_sta_connected = false;
  }
  applyHeaderIpLink('headerStaLink', 'headerStaIp', 'STA', null);

  const poll = async () => {
    App.networkHandoffTimer = null;
    const loaded = await loadState({ suppressErrorToast: true, timeoutMs: 2500 });
    const staIp = loaded ? String(App.data?.device?.network?.wifi_sta_ip || '').trim() : '';
    if (staIp) {
      showToast(`STA ready at ${staIp}. Click the STA badge to switch.`);
      return;
    }
    if (Date.now() < deadline) {
      App.networkHandoffTimer = setTimeout(poll, 1500);
    }
  };

  // The controller schedules its reboot one second after saving Wi-Fi. Start
  // after that boundary so an old pre-reboot STA address cannot end the watcher.
  App.networkHandoffTimer = setTimeout(poll, 2000);
};

const pollSignals = async () => {
  if (signalsInFlight || document.hidden) return signalsInFlight;

  signalsInFlight = (async () => {
    try {
      const data = await fetchJSON(`api/signals?t=${Date.now()}`, { timeoutMs: 2500 });
      signalPollFailures = 0;
      applyFastSignalState(data || {});
    } catch (error) {
      signalPollFailures++;
      console.warn('Failed to load signal state', error);
      if (signalPollFailures >= 2) {
        stopSignalPolling();
        if (!signalPollRetryTimer && !document.hidden) {
          signalPollRetryTimer = setTimeout(() => {
            signalPollRetryTimer = null;
            signalPollFailures = 0;
            startSignalPolling();
          }, 10000);
        }
      }
    }
  })().finally(() => {
    signalsInFlight = null;
  });

  return signalsInFlight;
};

const startSignalPolling = () => {
  if (!isDevicePageActive()) return;
  if (App.signalPollTimer || signalPollStartTimer) return;
  signalPollStartTimer = setTimeout(() => {
    signalPollStartTimer = null;
    if (!isDevicePageActive() || document.hidden) return;
    pollSignals();
    App.signalPollTimer = setInterval(pollSignals, 750);
  }, 500);
};

const stopSignalPolling = () => {
  if (signalPollStartTimer) {
    clearTimeout(signalPollStartTimer);
    signalPollStartTimer = null;
  }
  if (App.signalPollTimer) {
    clearInterval(App.signalPollTimer);
    App.signalPollTimer = null;
  }
  if ((!isDevicePageActive() || document.hidden) && signalPollRetryTimer) {
    clearTimeout(signalPollRetryTimer);
    signalPollRetryTimer = null;
  }
};

let wifiListInFlight = null;
const loadWifiList = async ({ force = false, scan = true } = {}) => {
  if (wifiListInFlight && !force) return wifiListInFlight;
  wifiListInFlight = (async () => {
    if (wifiScanRetryTimer) {
      clearTimeout(wifiScanRetryTimer);
      wifiScanRetryTimer = null;
    }
    wifiListLoading = true;
    wifiListError = null;
    renderWifiFromCache();
    try {
      const networks = await fetchJSON(`api/wifi/list?t=${Date.now()}`, { timeoutMs: 3000 });
      wifiNetworksCache = Array.isArray(networks) ? networks : [];
      wifiListLoaded = true;
    } catch (error) {
      wifiListError = error;
      console.warn('Failed to load Wi-Fi list', error);
    } finally {
      wifiListLoading = false;
      renderWifiFromCache();
    }
    if (scan) {
      wifiScanLoading = true;
      wifiScanError = null;
      renderWifiFromCache();
      try {
        const scanned = await fetchJSON(`api/wifi/scan?t=${Date.now()}`, { timeoutMs: 12000 });
        wifiScanCache = Array.isArray(scanned) ? scanned : [];
        wifiScanLoaded = true;
      } catch (scanError) {
        wifiScanError = scanError;
        console.warn('Failed to scan Wi-Fi networks', scanError);
        if (!(wifiScanCache && wifiScanCache.length)) {
          scheduleWifiScanRetry();
        }
      } finally {
        wifiScanLoading = false;
        renderWifiFromCache();
      }
    }
  })().finally(() => {
    wifiListInFlight = null;
  });
  return wifiListInFlight;
};

let keypadUsersInFlight = null;
const loadKeypadUsers = async () => {
  if (keypadUsersInFlight) return keypadUsersInFlight;
  keypadUsersInFlight = (async () => {
    try {
      const users = await fetchJSON(`api/keypad/users?t=${Date.now()}`);
      renderKeypadUsers(Array.isArray(users) ? users : []);
      if (App.data) App.data.keypadUsers = Array.isArray(users) ? users : [];
    } catch (error) {
      // Don't toast spam; state load already covers connectivity issues.
      console.warn('Failed to load keypad users', error);
      scheduleDeviceCredentialRetry();
    }
  })().finally(() => {
    keypadUsersInFlight = null;
  });
  return keypadUsersInFlight;
};

let credentialActivityRefreshInFlight = null;
const refreshCredentialActivityData = async () => {
  if (credentialActivityRefreshInFlight) return credentialActivityRefreshInFlight;
  credentialActivityRefreshInFlight = (async () => {
    const [wiegandResult, rfResult, keypadUsersResult] = await Promise.allSettled([
      fetchJSON(`api/wiegand?t=${Date.now()}`),
      fetchJSON(`api/rf?t=${Date.now()}`),
      fetchJSON(`api/keypad/users?t=${Date.now()}`),
    ]);

    if (wiegandResult.status === 'fulfilled') {
      if (App.data) App.data.wiegand = wiegandResult.value;
      renderWiegand(wiegandResult.value);
    }
    if (rfResult.status === 'fulfilled') {
      if (App.data) App.data.rf = rfResult.value;
      renderRf(rfResult.value);
    }
    if (keypadUsersResult.status === 'fulfilled' && Array.isArray(keypadUsersResult.value)) {
      if (App.data) App.data.keypadUsers = keypadUsersResult.value;
      renderKeypadUsers(keypadUsersResult.value);
    }
  })().finally(() => {
    credentialActivityRefreshInFlight = null;
  });
  return credentialActivityRefreshInFlight;
};

let logsInFlight = null;
const loadLogs = async () => {
  if (logsInFlight) return logsInFlight;
  logsInFlight = (async () => {
    try {
      const logs = await fetchJSON(`api/logs?t=${Date.now()}`);
      renderLogs(Array.isArray(logs) ? logs : []);
      if (App.data) App.data.logs = Array.isArray(logs) ? logs : [];
    } catch (error) {
      console.warn('Failed to load logs', error);
    }
  })().finally(() => {
    logsInFlight = null;
  });
  return logsInFlight;
};

const updateLock = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const locks = await fetchJSON('api/lock', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    if (App.data) App.data.locks = Array.isArray(locks) ? locks : [];
    applyLockState(locks || []);
    refreshAllMultiSelectControls();
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

const updateMotion = async (channel, updates) => {
  const body = { channel, ...updates };
  try {
    const motions = await fetchJSON('api/motion', {
      method: 'POST',
      body: JSON.stringify(body),
    });
    applyMotionState(motions || []);
  } catch (error) {
    handleError(error, 'Failed to update motion state');
  }
};

const testServiceInput = async (channel, config) => {
  if (!config?.endpoint || !config?.apply) return;
  try {
    const state = await fetchJSON(`api/${config.endpoint}`, {
      method: 'POST',
      body: JSON.stringify({ channel, test: true }),
      timeoutMs: 5000,
    });
    config.apply(state || []);
    showToast(`${config.label} test sent.`);
  } catch (error) {
    handleError(error, `Failed to test ${config.label.toLowerCase()}`);
    throw error;
  }
};

const createCardEnableButton = (enableId, label) => {
  const button = document.createElement('button');
  button.type = 'button';
  button.className = 'card-enable-toggle';
  button.dataset.enableTarget = enableId;
  button.setAttribute('aria-label', `${label} enabled`);
  button.innerHTML = '<span class="card-enable-icon" aria-hidden="true"></span><span class="card-enable-text">Enabled</span>';
  return button;
};

const createCardTestButton = (label) => {
  const button = document.createElement('button');
  button.type = 'button';
  button.className = 'card-test-toggle';
  button.setAttribute('aria-label', `Test ${label} input`);
  button.title = `Test ${label} input`;
  button.innerHTML = '<span class="card-test-icon" aria-hidden="true"></span>';
  return button;
};

const createCardModeSelect = (modeId, latchId) => {
  const wrap = document.createElement('label');
  wrap.className = 'card-mode-select stacked';
  wrap.innerHTML = `
    <span>Mode</span>
    <select id="${modeId}" data-latch-target="${latchId}">
      <option value="momentary">Momentary</option>
      <option value="toggle">Toggle</option>
      <option value="latch">Latch</option>
    </select>
  `;
  return wrap;
};

const createCardTargetSelect = (targetId) => {
  const holder = document.createElement('div');
  holder.innerHTML = renderMultiCheckboxControl('Target', '', 3, 'lock-target', targetId).trim();
  return holder.firstElementChild;
};

const createCardAlertTargetSelect = (targetId) => {
  const holder = document.createElement('div');
  holder.innerHTML = renderMultiCheckboxControl('Alert output', '', ALERT_TARGET_BOTH, 'alert-target', targetId).trim();
  return holder.firstElementChild;
};

const setupControlCardChrome = () => {
  const configs = [
    { label: 'Lock 1', enableId: 'enableLock_1', alertId: 'enableContactAlert_1', alertTargetId: 'alertTargetLock_1', alertStateKey: 'enableContactAlert', update: updateLock },
    { label: 'Lock 2', enableId: 'enableLock_2', alertId: 'enableContactAlert_2', alertTargetId: 'alertTargetLock_2', alertStateKey: 'enableContactAlert', update: updateLock },
    { label: 'Exit 1', enableId: 'enableExit_1', alertId: 'alertExit_1', latchId: 'latchExit_1', modeId: 'modeExit_1', targetId: 'targetExit_1', alertTargetId: 'alertTargetExit_1', delayId: 'armDelay_1', update: updateExit, endpoint: 'exit', apply: applyExitState },
    { label: 'Exit 2', enableId: 'enableKeypad_1', alertId: 'alertKeypad_1', latchId: 'latchKeypad_1', modeId: 'modeKeypad_1', targetId: 'targetKeypad_1', alertTargetId: 'alertTargetKeypad_1', delayId: 'keypadDelay_1', update: updateKeypad, endpoint: 'keypad', apply: applyKeypadState },
    { label: 'Exit 3', enableId: 'enableFob_1', alertId: 'alertFob_1', latchId: 'latchFob_1', modeId: 'modeFob_1', targetId: 'targetFob_1', alertTargetId: 'alertTargetFob_1', delayId: 'fobDelay_1', update: updateFob, endpoint: 'fob', apply: applyFobState },
    { label: 'Exit 4', enableId: 'enableMotion_1', alertId: 'alertMotion_1', latchId: 'latchMotion_1', modeId: 'modeMotion_1', targetId: 'targetMotion_1', alertTargetId: 'alertTargetMotion_1', delayId: 'motionDelay_1', update: updateMotion, endpoint: 'motion', apply: applyMotionState },
    { label: 'Exit 5', enableId: 'enableExit_2', alertId: 'alertExit_2', latchId: 'latchExit_2', modeId: 'modeExit_2', targetId: 'targetExit_2', alertTargetId: 'alertTargetExit_2', delayId: 'armDelay_2', update: updateExit, endpoint: 'exit', apply: applyExitState },
    { label: 'Exit 6', enableId: 'enableKeypad_2', alertId: 'alertKeypad_2', latchId: 'latchKeypad_2', modeId: 'modeKeypad_2', targetId: 'targetKeypad_2', alertTargetId: 'alertTargetKeypad_2', delayId: 'keypadDelay_2', update: updateKeypad, endpoint: 'keypad', apply: applyKeypadState },
    { label: 'Exit 7', enableId: 'enableFob_2', alertId: 'alertFob_2', latchId: 'latchFob_2', modeId: 'modeFob_2', targetId: 'targetFob_2', alertTargetId: 'alertTargetFob_2', delayId: 'fobDelay_2', update: updateFob, endpoint: 'fob', apply: applyFobState },
    { label: 'Exit 8', enableId: 'enableMotion_2', alertId: 'alertMotion_2', latchId: 'latchMotion_2', modeId: 'modeMotion_2', targetId: 'targetMotion_2', alertTargetId: 'alertTargetMotion_2', delayId: 'motionDelay_2', update: updateMotion, endpoint: 'motion', apply: applyMotionState },
  ];

  configs.forEach((config) => {
    const enableEl = document.getElementById(config.enableId);
    const section = enableEl?.closest('.control-section');
    if (!enableEl || !section || section.querySelector('.control-card-header')) return;

    const channel = Number(config.enableId.split('_').pop()) || 0;
    const titleEl = section.querySelector('.section-label, h4');
    const header = document.createElement('div');
    header.className = 'control-card-header';

    const titleWrap = document.createElement('div');
    titleWrap.className = 'control-card-title';
    if (titleEl?.classList?.contains('section-label')) {
      titleWrap.appendChild(titleEl);
    } else {
      if (titleEl) titleEl.remove();
      const title = document.createElement('input');
      title.type = 'text';
      title.id = `label_${config.enableId}`;
      title.className = 'section-label';
      title.value = config.label;
      title.setAttribute('aria-label', `${config.label} label`);
      titleWrap.appendChild(title);
    }
    header.appendChild(titleWrap);
    const fieldGrid = document.createElement('div');
    fieldGrid.className = 'control-card-fields';

    if (config.latchId && config.modeId) {
      const modeWrap = createCardModeSelect(config.modeId, config.latchId);
      const modeSelect = modeWrap.querySelector('select');
      const latchEl = document.getElementById(config.latchId);

      const modeRow = document.createElement('div');
      modeRow.className = 'control-card-mode-row';
      modeRow.appendChild(modeWrap);

      let delayField = null;
      if (config.delayId) {
        const delayInput = document.getElementById(config.delayId);
        const oldInputRow = delayInput?.closest('.input-row');
        oldInputRow?.querySelector('button')?.remove();
        if (delayInput) {
          delayField = document.createElement('label');
          delayField.className = 'card-delay-field stacked';
          const delaySpan = document.createElement('span');
          delaySpan.textContent = 'Re-arm delay (seconds)';
          delayField.appendChild(delaySpan);
          delayField.appendChild(delayInput);
          modeRow.appendChild(delayField);
          delayInput.addEventListener('change', () => {
            config.update(channel, { delay: parseInt(delayInput.value, 10) || 0 });
          });
        }
        oldInputRow?.remove();
      }

      if (modeSelect && latchEl) {
        modeSelect.value = normalizeCardMode(null, latchEl.checked);
        const syncDelayVisibility = () => {
          delayField?.classList.toggle('hidden-card-control', modeSelect.value !== 'momentary');
        };
        syncDelayVisibility();
        modeSelect.addEventListener('change', (event) => {
          const mode = normalizeCardMode(event.target.value, latchEl.checked);
          const latch = mode === 'latch';
          latchEl.checked = latch;
          syncDelayVisibility();
          config.update(channel, { mode, latch });
        });
      }
      fieldGrid.appendChild(modeRow);
      latchEl?.closest('label')?.classList.add('hidden-card-control');
    }

    if (config.targetId) {
      const targetWrap = createCardTargetSelect(config.targetId);
      const targetInput = targetWrap.querySelector('.multi-select-value');
      if (targetInput) {
        refreshMultiSelectControl(targetInput, 3);
        targetInput.addEventListener('change', (event) => {
          config.update(channel, { channel_mask: Number(event.target.value) });
        });
      }
      fieldGrid.appendChild(targetWrap);
    }

    if (config.alertTargetId) {
      const alertWrap = createCardAlertTargetSelect(config.alertTargetId);
      const alertInput = alertWrap.querySelector('.multi-select-value');
      const alertEl = document.getElementById(config.alertId);
      if (alertInput) {
        refreshMultiSelectControl(alertInput, alertEl?.checked === false ? 0 : ALERT_TARGET_BOTH);
        alertInput.addEventListener('change', (event) => {
          const alert_target = normalizeAlertTarget(event.target.value, true);
          if (alertEl) alertEl.checked = alertFromTarget(alert_target);
          const enabledKey = config.alertStateKey || 'alert';
          config.update(channel, { alert_target, [enabledKey]: alertFromTarget(alert_target) });
        });
      }
      fieldGrid.appendChild(alertWrap);
      alertEl?.closest('label')?.classList.add('hidden-card-control');
    }

    if (config.endpoint) {
      const testButton = createCardTestButton(config.label);
      testButton.addEventListener('click', async () => {
        testButton.disabled = true;
        try {
          await testServiceInput(channel, config);
        } finally {
          testButton.disabled = false;
        }
      });
      header.appendChild(testButton);
    }

    const enableButton = createCardEnableButton(config.enableId, config.label);
    enableButton.addEventListener('click', () => {
      config.update(channel, { enable: !enableEl.checked });
    });
    header.appendChild(enableButton);
    if (fieldGrid.childElementCount > 0) {
      header.appendChild(fieldGrid);
    }
    section.insertBefore(header, section.firstChild);
    enableEl.closest('label')?.classList.add('hidden-card-control');
    setCardEnabledState(config.enableId, enableEl.checked);
  });
};

const updateWiegandDevice = async (channel, patch) => {
  try {
    const wiegand = await fetchJSON('api/wiegand/device', {
      method: 'POST',
      body: JSON.stringify({ channel, ...patch }),
    });
    if (App.data) App.data.wiegand = wiegand;
    renderWiegand(wiegand);
    refreshAllMultiSelectControls();
    return wiegand;
  } catch (error) {
    handleError(error, `Failed to update Wiegand ${channel}`);
    throw error;
  }
};

const setupWiegandDeviceControls = () => {
  [1, 2].forEach((channel) => {
    const signalEl = document.getElementById(`wiegandDeviceSignal_${channel}`);
    const card = signalEl?.closest('.wiegand-device-card');
    const titleRow = signalEl?.closest('.wiegand-device-title');
    if (!card || !titleRow || document.getElementById(`enableWiegandDevice_${channel}`)) return;

    const existingTitle = titleRow.querySelector('h4, .section-label');
    if (existingTitle && !existingTitle.classList.contains('section-label')) {
      const input = document.createElement('input');
      input.type = 'text';
      input.id = `label_wiegandDevice_${channel}`;
      input.className = 'section-label';
      input.value = `Wiegand ${channel}`;
      input.setAttribute('aria-label', `Wiegand ${channel} label`);
      existingTitle.replaceWith(input);
    }

    const enableInput = document.createElement('input');
    enableInput.type = 'checkbox';
    enableInput.id = `enableWiegandDevice_${channel}`;
    enableInput.className = 'hidden-card-control';
    enableInput.checked = true;
    card.appendChild(enableInput);

    const enableButton = createCardEnableButton(enableInput.id, `Wiegand ${channel}`);
    enableButton.addEventListener('click', async () => {
      const next = !enableInput.checked;
      enableButton.disabled = true;
      try {
        await updateWiegandDevice(channel, { enable: next });
      } finally {
        enableButton.disabled = false;
      }
    });
    titleRow.appendChild(enableButton);
    setCardEnabledState(enableInput.id, true);
  });
};

const setupLockHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableLock_${ch}`);
    const armEl = document.getElementById(`arm_${ch}`);
    const contactEl = document.getElementById(`enableContactAlert_${ch}`);
    const failSecureEl = document.getElementById(`failSecure_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateLock(ch, { enable: event.target.checked });
      });
    }
    if (armEl) {
      armEl.addEventListener('change', (event) => {
        const enabled = document.getElementById(`enableLock_${ch}`)?.checked !== false;
        setLockArmState(ch, event.target.checked, enabled);
        updateLock(ch, { arm: event.target.checked });
      });
    }
    if (contactEl) {
      contactEl.addEventListener('change', (event) => {
        updateLock(ch, {
          enableContactAlert: event.target.checked,
          alert_target: event.target.checked ? ALERT_TARGET_BOTH : 0,
        });
      });
    }
    if (failSecureEl) {
      failSecureEl.addEventListener('change', (event) => {
        updateLock(ch, { failSecure: event.target.checked });
      });
    }
  });
};

const setupExitHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableExit_${ch}`);
    const alertEl = document.getElementById(`alertExit_${ch}`);
    const latchEl = document.getElementById(`latchExit_${ch}`);
    const invertEl = document.getElementById(`invertExit_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateExit(ch, { enable: event.target.checked });
      });
    }
    if (invertEl) {
      invertEl.addEventListener('change', (event) => {
        updateExit(ch, { invert: event.target.checked });
      });
    }
    if (alertEl) {
      alertEl.addEventListener('change', (event) => {
        updateExit(ch, { alert: event.target.checked });
      });
    }
    if (latchEl) {
      latchEl.addEventListener('change', (event) => {
        updateExit(ch, { latch: event.target.checked, mode: normalizeCardMode(null, event.target.checked) });
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
        updateFob(ch, { latch: event.target.checked, mode: normalizeCardMode(null, event.target.checked) });
      });
    }
  });
};

const setupKeypadHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableKeypad_${ch}`);
    const alertEl = document.getElementById(`alertKeypad_${ch}`);
    const latchEl = document.getElementById(`latchKeypad_${ch}`);

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
    if (latchEl) {
      latchEl.addEventListener('change', (event) => {
        updateKeypad(ch, { latch: event.target.checked, mode: normalizeCardMode(null, event.target.checked) });
      });
    }
  });
};

const setupMotionHandlers = () => {
  [1, 2].forEach((ch) => {
    const enableEl = document.getElementById(`enableMotion_${ch}`);
    const alertEl = document.getElementById(`alertMotion_${ch}`);
    const latchEl = document.getElementById(`latchMotion_${ch}`);

    if (enableEl) {
      enableEl.addEventListener('change', (event) => {
        updateMotion(ch, { enable: event.target.checked });
      });
    }
    if (alertEl) {
      alertEl.addEventListener('change', (event) => {
        updateMotion(ch, { alert: event.target.checked });
      });
    }
    if (latchEl) {
      latchEl.addEventListener('change', (event) => {
        updateMotion(ch, { latch: event.target.checked, mode: normalizeCardMode(null, event.target.checked) });
      });
    }
  });
};

const setupEnrollmentHandlers = () => {
  const newUserName = document.getElementById('enrollNewUserName');
  const userSelect = document.getElementById('enrollUserSelect');
  const startBtn = document.getElementById('enrollStartBtn');
  const stopBtn = document.getElementById('enrollStopBtn');

  const addUser = async () => {
    const name = (newUserName?.value || '').trim();
    if (!name) return null;

    try {
      // Reuse an existing user with this name (PIN-based or RFID/remote-only) instead
      // of unconditionally minting a new one -- typing a name that already exists
      // should join that person, not create a duplicate.
      const uuid = await ensureCredentialUserForName(name);
      if (!uuid) return null;
      const users = await fetchJSON(`api/keypad/users?t=${Date.now()}`);
      const list = Array.isArray(users) ? users : [];
      if (App.data) App.data.keypadUsers = list;
      renderKeypadUsers(list);
      if (userSelect) userSelect.value = `uuid:${uuid}`;
      if (newUserName) newUserName.value = '';
      return uuid;
    } catch (error) {
      handleError(error, 'Failed to add user');
      return null;
    }
  };

  const resolveSelectedUserUuid = async () => {
    const selected = userSelect?.value || '';
    if (selected.startsWith('uuid:')) return selected.slice('uuid:'.length);
    if (selected.startsWith('name:')) {
      const group = buildCredentialUserGroups().find((candidate) => candidate.key === selected);
      return ensureCredentialUserForName(group?.name || 'Default User');
    }
    // No existing users at all yet (dropdown fell back to the generic placeholder) --
    // resolve/create the real "Default User" record so it's reusable next time
    // instead of relying on the device to silently reuse whichever user happens to
    // be record #1.
    return ensureCredentialUserForName('Default User');
  };

  if (startBtn) {
    startBtn.addEventListener('click', async () => {
      let userUuid = '';
      if ((newUserName?.value || '').trim()) {
        userUuid = await addUser() || '';
        if (!userUuid) return;
      } else {
        userUuid = await resolveSelectedUserUuid() || '';
      }

      startBtn.disabled = true;
      try {
        const state = await fetchJSON('api/enrollment/start', {
          method: 'POST',
          body: JSON.stringify({ userUuid }),
        });
        if (state) applyEnrollmentUpdate(state);
        showToast('Listening for credentials.');
      } catch (error) {
        handleError(error, 'Failed to start enrollment');
        startBtn.disabled = false;
      }
    });
  }

  if (stopBtn) {
    stopBtn.addEventListener('click', async () => {
      stopBtn.disabled = true;
      try {
        const state = await fetchJSON('api/enrollment/stop', {
          method: 'POST',
          body: JSON.stringify({}),
        });
        if (state) applyEnrollmentUpdate(state);
        await Promise.allSettled([loadKeypadUsers(), loadState()]);
        showToast('Enrollment stopped.');
      } catch (error) {
        handleError(error, 'Failed to stop enrollment');
        stopBtn.disabled = false;
      }
    });
  }
};

const setupForms = () => {
  const wifiForm = document.getElementById('wifiForm');
  const serverForm = document.getElementById('serverForm');
  const wifiList = document.getElementById('wifiNetworks');
  const wifiAvailableList = document.getElementById('wifiAvailableNetworks');
  const wifiScanBtn = document.getElementById('wifiScanBtn');

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
          showToast('Wi‑Fi saved. Waiting for the STA link...');
          startNetworkHandoffPolling();
          wifiNetworksCache = null;
          wifiScanCache = null;
          wifiListLoaded = false;
          wifiScanLoaded = false;
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
          showToast('Connecting... waiting for the STA link.');
          startNetworkHandoffPolling();
          wifiNetworksCache = null;
          wifiScanCache = null;
          wifiListLoaded = false;
          wifiScanLoaded = false;
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
          const result = await fetchJSON('api/wifi/delete', {
            method: 'POST',
            body: JSON.stringify({ ssid }),
          });
          if (Array.isArray(result?.networks)) {
            applyWifiListSnapshot(result.networks, wifiScanCache || []);
          }
          showToast('Wi‑Fi removed.');
          if (result?.reboot) {
            wifiNetworksCache = null;
            wifiScanCache = null;
          } else {
            await loadWifiList({ force: true, scan: false });
          }
        } catch (error) {
          handleError(error, 'Failed to delete Wi‑Fi');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });
  }

  if (wifiScanBtn) {
    wifiScanBtn.addEventListener('click', async () => {
      const previousText = wifiScanBtn.textContent;
      wifiScanBtn.disabled = true;
      wifiScanBtn.textContent = 'Scanning...';
      try {
        await loadWifiList({ force: true, scan: true });
        showToast('Wi-Fi scan refreshed.');
      } catch (error) {
        handleError(error, 'Failed to scan Wi-Fi networks');
      } finally {
        wifiScanBtn.disabled = false;
        wifiScanBtn.textContent = previousText;
      }
    });
  }

  if (wifiAvailableList) {
    wifiAvailableList.addEventListener('click', (event) => {
      const networkBtn = event.target.closest('button[data-action="wifi-select"]');
      if (!networkBtn) return;
      const ssid = networkBtn.getAttribute('data-ssid') || '';
      const ssidInput = document.getElementById('wifiName');
      const passwordInput = document.getElementById('wifiPassword');
      if (ssidInput) ssidInput.value = ssid;
      if (passwordInput) passwordInput.focus();
    });
  }

  if (serverForm) {
    serverForm.addEventListener('submit', async (event) => {
      event.preventDefault();
      const serverUrl = (document.getElementById('serverUrl')?.value || 'https://open-automation.org/devices').trim() || 'https://open-automation.org/devices';
      const requireReachable = document.getElementById('serverRequireReachable')?.checked !== false;

      try {
        await fetchJSON('api/server', {
          method: 'POST',
          body: JSON.stringify({ serverUrl, requireReachable }),
        });
        showToast('Server information updated. Device will reboot to apply changes.');
      } catch (error) {
        handleError(error, 'Failed to update server information');
      }
    });
  }
};

const setupCredentialUserHandlers = () => {
  const listEl = App.elements.credentialUserList;
  if (!listEl) return;

  listEl.addEventListener('click', async (event) => {
    const deleteUserBtn = event.target.closest('button[data-action="delete-user"]');
    if (deleteUserBtn) {
      const key = deleteUserBtn.getAttribute('data-id');
      if (!key) return;
      const group = buildCredentialUserGroups().find((candidate) => candidate.key === key);
      if (!group) return;

      const total = group.rfid.length + group.pins.length + group.remotes.length;
      const confirmMessage = total
        ? `Delete ${group.name} and ${total} credential${total === 1 ? '' : 's'}? This can't be undone.`
        : `Delete ${group.name}? This can't be undone.`;
      if (!window.confirm(confirmMessage)) return;

      deleteUserBtn.disabled = true;
      try {
        for (const user of group.rfid) {
          if (!user.id) continue;
          const wiegand = await fetchJSON('api/wiegand/delete', {
            method: 'POST',
            body: JSON.stringify({ id: user.id }),
          });
          renderWiegand(wiegand);
        }

        for (const user of group.remotes) {
          if (!user.id) continue;
          const rf = await fetchJSON('api/rf/delete', {
            method: 'POST',
            body: JSON.stringify({ id: user.id }),
          });
          renderRf(rf);
        }

        if (group.uuid) {
          const latestUsers = await fetchJSON('api/keypad/user', {
            method: 'DELETE',
            body: JSON.stringify({ uuid: group.uuid }),
          });
          renderKeypadUsers(Array.isArray(latestUsers) ? latestUsers : []);
        }

        App.credentialUserOpenKeys.delete(key);
        renderCredentialUsers();
        showToast(`${group.name} deleted.`);
      } catch (error) {
        handleError(error, `Failed to delete ${group.name}`);
      } finally {
        deleteUserBtn.disabled = false;
      }
      return;
    }

    const toggle = event.target.closest('button[data-action="toggle-user-group"]');
    if (!toggle) return;
    const key = toggle.getAttribute('data-user-key');
    if (!key) return;

    const isOpen = toggle.getAttribute('aria-expanded') === 'true';
    if (isOpen) {
      App.credentialUserOpenKeys.delete(key);
    } else {
      App.credentialUserOpenKeys.add(key);
    }
    // Re-render is skipped whenever focus sits inside the list (see
    // shouldDeferCredentialRender) so it doesn't clobber an in-progress edit.
    // The toggle button itself just received focus from this click, which
    // would otherwise defer the very render that's supposed to reflect it.
    toggle.blur();
    renderCredentialUsers();
  });

  listEl.addEventListener('change', async (event) => {
    const picker = event.target.closest('.user-schedule-picker');
    if (!picker) return;
    const uuid = picker.getAttribute('data-user-uuid');
    if (!uuid) return;
    const scheduleId = picker.value;
    const previous = App.data?.schedules?.assignments?.[uuid] || '';

    picker.disabled = true;
    try {
      const schedules = await fetchJSON('api/schedules/assign', {
        method: 'POST',
        body: JSON.stringify({ uuid, schedule_id: scheduleId }),
      });
      if (App.data) App.data.schedules = schedules;
      showToast(`Access schedule set to ${scheduleNameFor(scheduleId)}.`);
      renderCredentialUsers();
    } catch (error) {
      handleError(error, 'Failed to update access schedule');
      picker.value = previous;
    } finally {
      picker.disabled = false;
    }
  });
};

const setupWiegandHandlers = () => {
  const registerBtn = App.elements.wiegandRegisterBtn;
  const stopBtn = App.elements.wiegandStopBtn;
  const channelSelect = App.elements.wiegandChannelSelect;
  const listEl = App.elements.wiegandUserList || App.elements.credentialUserList;
  const removeAllBtn = App.elements.wiegandRemoveAllBtn;

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

  if (removeAllBtn) {
    removeAllBtn.addEventListener('click', async () => {
      const users = App.data?.wiegand?.users || [];
      if (!users.length) {
        showToast('No RFID cards to remove.');
        return;
      }

      removeAllBtn.disabled = true;
      try {
        const wiegand = await fetchJSON('api/wiegand/delete-all', {
          method: 'POST',
          body: JSON.stringify({}),
        });
        renderWiegand(wiegand || { users: [] });
        await loadState();
        showToast(`Removed ${users.length} RFID card${users.length === 1 ? '' : 's'}.`);
      } catch (error) {
        handleError(error, error.message || 'Failed to remove RFID cards');
      } finally {
        removeAllBtn.disabled = false;
      }
    });
  }

  if (listEl) {
    const saveWiegandCard = async (container, trigger, { quiet = false } = {}) => {
      if (!container) return null;
      const input = container.querySelector('.user-name-input');
      const modeInput = container.querySelector('.wiegand-mode-select');
      const channelInput = container.querySelector('.wiegand-channel-select');
      const alertTargetInput = container.querySelector('.wiegand-alert-target-select');
      if (!input) return null;

      const id = container.getAttribute('data-id');
      const existing = (App.data?.wiegand?.users || []).find((user) => user.id === id) || {};
      const name = input.value.trim() || existing.name || 'RFID Card';
      const existingUserUuid = credentialOwnerUuid(existing);
      const existingName = credentialOwnerName(existing);
      let userUuid = existingUserUuid;
      if (!userUuid || credentialUserNameKey(name) !== credentialUserNameKey(existingName)) {
        userUuid = await ensureCredentialUserForName(name);
      }
      const channel = parseInt(container.getAttribute('data-channel') || `${existing.channel || 0}`, 10) || 0;
      const mode = modeInput?.value || existing.mode || 'momentary';
      const channel_mask = channelInput ? Number(channelInput.value) : (existing.channel_mask || 3);
      const alert_target = normalizeAlertTarget(alertTargetInput?.value || existing.alert_target, existing.alert !== false);
      const alert = alertFromTarget(alert_target);
      const enabled = container.dataset.enabled !== 'false';
      if (!id) return null;

      if (trigger) trigger.disabled = true;
      container.classList.add('saving');
      try {
        const wiegand = await fetchJSON('api/wiegand/rename', {
          method: 'POST',
          body: JSON.stringify({ id, name, userUuid, channel, channel_mask, alert, alert_target, enabled, mode }),
        });
        renderWiegand(wiegand);
        if (!quiet) showToast('RFID card updated.');
        return wiegand;
      } catch (error) {
        handleError(error, error.message || 'Failed to update RFID card');
        throw error;
      } finally {
        if (trigger) trigger.disabled = false;
        container.classList.remove('saving');
      }
    };

    listEl.addEventListener('click', async (event) => {
      const toggleEnabledBtn = event.target.closest('button[data-action="toggle-wiegand-enabled"]');
      const deleteBtn = event.target.closest('button[data-action="delete-wiegand"]');

      if (toggleEnabledBtn) {
        const container = toggleEnabledBtn.closest('.user-row');
        if (!container) return;
        const previousEnabled = container.dataset.enabled !== 'false';
        const nextEnabled = !previousEnabled;
        container.dataset.enabled = nextEnabled ? 'true' : 'false';
        container.classList.toggle('is-card-disabled', !nextEnabled);
        setEnableButtonState(toggleEnabledBtn, nextEnabled);
        try {
          await saveWiegandCard(container, toggleEnabledBtn, { quiet: true });
          showToast(`RFID card ${nextEnabled ? 'enabled' : 'disabled'}.`);
        } catch (error) {
          container.dataset.enabled = previousEnabled ? 'true' : 'false';
          container.classList.toggle('is-card-disabled', !previousEnabled);
          setEnableButtonState(toggleEnabledBtn, previousEnabled);
        }
        return;
      }

      if (deleteBtn) {
        const id = deleteBtn.getAttribute('data-id');
        if (!id) return;

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

    listEl.addEventListener('input', (event) => {
      const nameInput = event.target.closest('.user-name-input');
      if (!nameInput || !nameInput.closest('.credential-card--rfid')) return;
      scheduleCredentialNameSave(nameInput, (container) => saveWiegandCard(container, null, { quiet: true }));
    });

    listEl.addEventListener('change', async (event) => {
      const control = event.target.closest('.wiegand-mode-select, .wiegand-channel-select, .wiegand-alert-target-select');
      if (!control) return;
      const container = control.closest('.user-row');
      if (!container) return;

      const previous = control.classList.contains('wiegand-mode-select') ? control.dataset.previousValue || '' : control.value;
      control.disabled = true;
      try {
        await saveWiegandCard(container, control, { quiet: true });
        if (control.classList.contains('wiegand-mode-select')) {
          control.dataset.previousValue = control.value;
          showToast(`RFID mode set to ${control.options[control.selectedIndex].text}.`);
        } else {
          showToast('RFID card updated.');
        }
      } catch (error) {
        control.value = previous;
      } finally {
        control.disabled = false;
      }
    });
  }
};

const setupRfHandlers = () => {
  const registerBtn = App.elements.rfRegisterBtn;
  const stopBtn = App.elements.rfStopBtn;
  const listEl = App.elements.rfUserList || App.elements.credentialUserList;
  const removeAllBtn = App.elements.rfRemoveAllBtn;

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

  if (removeAllBtn) {
    removeAllBtn.addEventListener('click', async () => {
      const users = App.data?.rf?.users || [];
      if (!users.length) {
        showToast('No remotes to remove.');
        return;
      }

      removeAllBtn.disabled = true;
      try {
        const rf = await fetchJSON('api/rf/delete-all', {
          method: 'POST',
          body: JSON.stringify({}),
        });
        renderRf(rf || { users: [] });
        await loadState();
        showToast(`Removed ${users.length} remote FOB${users.length === 1 ? '' : 's'}.`);
      } catch (error) {
        handleError(error, error.message || 'Failed to remove remotes');
      } finally {
        removeAllBtn.disabled = false;
      }
    });
  }

  if (listEl) {
    const saveRfCard = async (container, trigger, { quiet = false } = {}) => {
      if (!container) return null;
      const nameInput = container.querySelector('.user-name-input');
      const modeSelect = container.querySelector('.rf-mode-select');
      const channelSelect = container.querySelector('.rf-channel-select');
      const exitInput = container.querySelector('.rf-exit-seconds');
      const alertTargetSelect = container.querySelector('.rf-alert-target-select');
      const id = container.getAttribute('data-id');
      const name = nameInput?.value.trim();
      const mode = modeSelect?.value || 'momentary';
      const channel_mask = channelSelect ? Number(channelSelect.value) : 0;
      const exit_seconds = exitInput ? Number(exitInput.value || 0) : 0;
      const alert_target = normalizeAlertTarget(alertTargetSelect?.value, true);
      const alert = alertFromTarget(alert_target);
      const enabled = container.dataset.enabled !== 'false';

      if (!id) {
        showToast('Missing id');
        return null;
      }
      if (!name) {
        showToast('Please provide a name before saving.');
        return null;
      }

      if (trigger) trigger.disabled = true;
      container.classList.add('saving');
      try {
        await fetchJSON('api/rf/rename', {
          method: 'POST',
          body: JSON.stringify({ id, name }),
        });
        const rf = await fetchJSON('api/rf/config', {
          method: 'POST',
          body: JSON.stringify({ id, mode, channel_mask, exit_seconds, alert, alert_target, enabled }),
        });
        renderRf(rf);
        if (!quiet) showToast('Remote updated.');
        return rf;
      } catch (error) {
        handleError(error, error.message || 'Failed to update remote');
        throw error;
      } finally {
        if (trigger) trigger.disabled = false;
        container.classList.remove('saving');
      }
    };

    listEl.addEventListener('click', async (event) => {
      const toggleEnabledBtn = event.target.closest('button[data-action="toggle-rf-enabled"]');
      const deleteBtn = event.target.closest('button[data-action="delete-rf"]');

      if (toggleEnabledBtn) {
        const container = toggleEnabledBtn.closest('.user-row');
        if (!container) return;
        const previousEnabled = container.dataset.enabled !== 'false';
        const nextEnabled = !previousEnabled;
        container.dataset.enabled = nextEnabled ? 'true' : 'false';
        container.classList.toggle('is-card-disabled', !nextEnabled);
        setEnableButtonState(toggleEnabledBtn, nextEnabled);
        try {
          await saveRfCard(container, toggleEnabledBtn, { quiet: true });
          showToast(`Remote ${nextEnabled ? 'enabled' : 'disabled'}.`);
        } catch (error) {
          container.dataset.enabled = previousEnabled ? 'true' : 'false';
          container.classList.toggle('is-card-disabled', !previousEnabled);
          setEnableButtonState(toggleEnabledBtn, previousEnabled);
        }
        return;
      }

      if (deleteBtn) {
        const id = deleteBtn.getAttribute('data-id');
        if (!id) return;
        const container = deleteBtn.closest('.user-row');
        if (container) container.remove();
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
          await loadState();
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });

    listEl.addEventListener('input', (event) => {
      const nameInput = event.target.closest('.user-name-input');
      if (!nameInput || !nameInput.closest('.credential-card--remote')) return;
      scheduleCredentialNameSave(nameInput, (container) => saveRfCard(container, null, { quiet: true }));
    });

    listEl.addEventListener('change', async (event) => {
      const control = event.target.closest('.rf-mode-select, .rf-channel-select, .rf-exit-seconds, .rf-alert-target-select');
      if (!control) return;

      const container = control.closest('.user-row');
      if (!container) return;

      control.disabled = true;
      try {
        await saveRfCard(container, control, { quiet: true });
      } catch (error) {
        await loadState();
      } finally {
        control.disabled = false;
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
const getPinUserDefaults = (user = {}) => ({
  id: user.uuid || '',
  name: user.name || 'PIN User',
  pin: Array.isArray(user.pins) && user.pins.length ? String(user.pins[0] || '') : String(user.pin || ''),
  mode: user.mode || 'momentary',
  channel_mask: Number(user.channel_mask || user.channel || 1) || 1,
  keypad_mask: Number(user.keypad_mask || 3) || 3,
  exit_seconds: Number(user.exit_seconds || user.delay || 4) || 4,
  alert: user.alert ?? true,
  alert_target: normalizeAlertTarget(user.alert_target, user.alert ?? true),
  enabled: user.enabled !== false,
});

const expandPinUserCredentials = (users = []) => {
  const credentials = [];
  (Array.isArray(users) ? users : []).forEach((user, userIndex) => {
    const pins = Array.isArray(user.pins)
      ? user.pins.map((pin) => String(pin || '')).filter(Boolean)
      : [];
    const sourcePins = pins.length ? pins : (user.pin ? [String(user.pin)] : []);

    if (!sourcePins.length) {
      credentials.push({
        ...user,
        credentialId: `${user.uuid || `user-${userIndex}`}:pin:none`,
        pinIndex: -1,
        pin: '',
        pins: [],
      });
      return;
    }

    sourcePins.forEach((pin, pinIndex) => {
      credentials.push({
        ...user,
        credentialId: `${user.uuid || `user-${userIndex}`}:pin:${pinIndex}`,
        pinIndex,
        pin,
        pins: [pin],
      });
    });
  });
  return credentials;
};

const livePinCredentialRows = () => {
  const enrollmentActive = !!App.data?.enrollment?.active;
  const entries = App.data?.wiegand?.pinEntries || [];
  if (!enrollmentActive || !Array.isArray(entries)) {
    return [];
  }
  return entries
    .filter((entry) => entry?.active && entry.code)
    .map((entry) => ({
      pending: true,
      credentialId: `pending-pin-ch${Number(entry.channel) || 0}`,
      uuid: '',
      name: `${formatChannelLabel(Number(entry.channel) || 0)} PIN`,
      pin: String(entry.code || ''),
      pinIndex: -1,
      mode: 'typing',
      channel_mask: Number(entry.channel) === 2 ? 2 : 1,
      keypad_mask: Number(entry.channel) === 2 ? 2 : 1,
      exit_seconds: 4,
      alert: true,
      alert_target: 'both',
      enabled: true,
    }));
};

const buildPendingPinRow = (entry) => `
  <div class="user-row credential-card credential-card--pin credential-card--pending" data-id="${escapeHtml(entry.credentialId)}" data-pending="true">
    <div class="credential-card-header">
      <div class="credential-card-title">
        <span class="credential-kind">PIN</span>
        <span class="user-code">${escapeHtml(entry.pin)}</span>
      </div>
      <div class="credential-card-actions">
        <span class="status-chip pending">Typing</span>
      </div>
    </div>
    <div class="user-info">
      <div class="credential-meta-row">
        <span class="user-channel">${escapeHtml(entry.name)}</span>
        <span class="status-chip pending">Press # to save</span>
      </div>
    </div>
  </div>
`;

const buildKeypadUserRow = (user, index, existingValue) => {
  if (user?.pending) {
    return buildPendingPinRow(user);
  }
  const defaults = getPinUserDefaults(user);
  const preserved = existingValue && typeof existingValue === 'object' ? existingValue : {};
  const name = escapeHtml(preserved.name !== undefined ? preserved.name : (defaults.name || `User ${index + 1}`));
  const pin = escapeHtml(preserved.pin !== undefined ? preserved.pin : defaults.pin);
  const mode = preserved.mode || defaults.mode || 'momentary';
  const channelMask = Number(preserved.channel_mask ?? defaults.channel_mask) || 1;
  const keypadMask = Number(preserved.keypad_mask ?? defaults.keypad_mask) || 3;
  const exitSeconds = Number(preserved.exit_seconds ?? defaults.exit_seconds) || 4;
  const alert = preserved.alert !== undefined ? !!preserved.alert : !!defaults.alert;
  const alertTarget = normalizeAlertTarget(preserved.alert_target ?? defaults.alert_target, alert);
  const enabled = preserved.enabled !== undefined ? !!preserved.enabled : !!defaults.enabled;
  const uuid = escapeHtml(user.uuid || '');
  const credentialId = escapeHtml(user.credentialId || user.uuid || '');
  const pinIndex = Number.isInteger(user.pinIndex) ? user.pinIndex : -1;
  
  return `
    <div class="user-row credential-card credential-card--pin ${enabled ? '' : 'is-card-disabled'}" data-id="${credentialId}" data-uuid="${uuid}" data-pin-index="${pinIndex}" data-enabled="${enabled ? 'true' : 'false'}">
      <div class="credential-card-header">
        <div class="credential-card-title">
          <span class="credential-kind">PIN</span>
          <span class="user-code">${pin || 'No PIN'}</span>
        </div>
        <div class="credential-card-actions">
          ${renderCredentialEnableButton(enabled, 'toggle-pin-enabled', user.uuid || '')}
          ${renderCredentialIconButton('delete-pin', user.uuid || '', 'Delete PIN code', 'credential-remove-icon', `data-pin-index="${pinIndex}"`)}
        </div>
      </div>
      <div class="user-info">
        <label class="stacked">
          <span>Name</span>
          <input type="text" class="user-name-input" value="${name}" placeholder="Enter name...">
        </label>
        <label class="stacked">
          <span>PIN Code</span>
          <input type="text" class="pin-code-input" value="${pin}" maxlength="8" pattern="[0-9]*" inputmode="numeric" placeholder="1234">
        </label>
        <div class="user-config">
          <label class="stacked">
            <span>Mode</span>
            <select class="pin-mode-select">
              <option value="toggle" ${mode === 'toggle' ? 'selected' : ''}>Toggle</option>
              <option value="momentary" ${mode === 'momentary' ? 'selected' : ''}>Momentary</option>
              <option value="latch" ${mode === 'latch' ? 'selected' : ''}>Latch</option>
              <option value="exit" ${mode === 'exit' ? 'selected' : ''}>Exit pulse</option>
              <option value="power_on" ${mode === 'power_on' ? 'selected' : ''}>Power ON</option>
              <option value="power_off" ${mode === 'power_off' ? 'selected' : ''}>Power OFF</option>
            </select>
          </label>
          ${renderLockTargetSelect('pin-channel-select', channelMask)}
          ${renderKeypadAccessSelect('pin-keypad-select', keypadMask)}
          ${renderAlertTargetSelect('pin-alert-target-select', alertTarget, alert)}
          <label class="stacked">
            <span>Exit duration (s)</span>
            <input type="number" class="pin-exit-seconds" min="1" step="1" value="${exitSeconds}">
          </label>
        </div>
      </div>
    </div>
  `;
};

const normalizeCredentialUserName = (value) => {
  const name = String(value || '').trim();
  return name || 'Default User';
};

const credentialUserNameKey = (name) => normalizeCredentialUserName(name).toLowerCase();

const credentialOwnerUuid = (item = {}) => (
  item.userUuid || item.user_uuid || item.uuid || ''
);

const credentialOwnerName = (item = {}) => normalizeCredentialUserName(item.userName || item.name);

const findCredentialUserByName = (name) => {
  const key = credentialUserNameKey(name);
  const direct = (Array.isArray(App.data?.keypadUsers) ? App.data.keypadUsers : [])
    .find((user) => credentialUserNameKey(user?.name) === key);
  if (direct) return direct;

  // No PIN/keypad record yet, but the name may already exist purely as an RFID or
  // remote credential tag -- treat that as the same person so a second credential
  // added under the same displayed name joins them instead of minting a new user.
  const group = buildCredentialUserGroups().find((candidate) => credentialUserNameKey(candidate.name) === key);
  return group?.uuid ? { uuid: group.uuid, name: group.name } : null;
};

const ensureCredentialUserForName = async (name) => {
  const normalized = normalizeCredentialUserName(name);
  const key = credentialUserNameKey(normalized);
  const existing = findCredentialUserByName(normalized);
  if (existing?.uuid) return existing.uuid;
  const pending = App.credentialUserCreatePromises.get(key);
  if (pending) return pending;

  const createPromise = (async () => {
    let users = await fetchJSON(`api/keypad/users?t=${Date.now()}`);
    let list = Array.isArray(users) ? users : [];
    if (App.data) App.data.keypadUsers = list;
    const latest = list.find((user) => credentialUserNameKey(user?.name) === key);
    if (latest?.uuid) return latest.uuid;

    users = await fetchJSON('api/keypad/user', {
      method: 'POST',
      body: JSON.stringify({ name: normalized, pin: '' }),
    });
    if (!Array.isArray(users) || !users.length) {
      users = await fetchJSON(`api/keypad/users?t=${Date.now()}`);
    }

    list = Array.isArray(users) ? users : [];
    if (App.data) App.data.keypadUsers = list;

    const created = [...list].reverse()
      .find((user) => credentialUserNameKey(user?.name) === key);
    return created?.uuid || '';
  })();

  App.credentialUserCreatePromises.set(key, createPromise);
  try {
    return await createPromise;
  } finally {
    App.credentialUserCreatePromises.delete(key);
  }
};

const collectCredentialFormValues = () => {
  const root = App.elements.credentialUserList
    || App.elements.wiegandUserList
    || App.elements.keypadUserList
    || App.elements.rfUserList;
  const values = {
    rfid: {},
    pin: {},
    remote: {},
    focused: null,
  };
  if (!root) return values;

  root.querySelectorAll('.credential-card').forEach((row) => {
    const id = row.getAttribute('data-id');
    if (!id || row.dataset.pending === 'true') return;

    if (row.classList.contains('credential-card--rfid')) {
      const nameInput = row.querySelector('.user-name-input');
      const modeInput = row.querySelector('.wiegand-mode-select');
      const channelInput = row.querySelector('.wiegand-channel-select');
      const alertTargetInput = row.querySelector('.wiegand-alert-target-select');
      values.rfid[id] = {
        name: nameInput ? nameInput.value : undefined,
        mode: modeInput ? modeInput.value : undefined,
        channel_mask: channelInput ? Number(channelInput.value) : undefined,
        alert_target: alertTargetInput ? alertTargetInput.value : undefined,
        alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
      };
    } else if (row.classList.contains('credential-card--remote')) {
      const nameInput = row.querySelector('.user-name-input');
      const modeSel = row.querySelector('.rf-mode-select');
      const chSel = row.querySelector('.rf-channel-select');
      const exitInput = row.querySelector('.rf-exit-seconds');
      const alertTargetInput = row.querySelector('.rf-alert-target-select');
      values.remote[id] = {
        name: nameInput ? nameInput.value : undefined,
        mode: modeSel ? modeSel.value : undefined,
        channel_mask: chSel ? Number(chSel.value) : undefined,
        exit_seconds: exitInput ? Number(exitInput.value) : undefined,
        alert_target: alertTargetInput ? alertTargetInput.value : undefined,
        alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
        enabled: row.dataset.enabled !== 'false',
      };
    } else if (row.classList.contains('credential-card--pin')) {
      const nameInput = row.querySelector('.user-name-input');
      const pinInput = row.querySelector('.pin-code-input');
      const modeInput = row.querySelector('.pin-mode-select');
      const channelInput = row.querySelector('.pin-channel-select');
      const keypadInput = row.querySelector('.pin-keypad-select');
      const exitInput = row.querySelector('.pin-exit-seconds');
      const alertTargetInput = row.querySelector('.pin-alert-target-select');
      values.pin[id] = {
        name: nameInput ? nameInput.value : undefined,
        pin: pinInput ? pinInput.value : undefined,
        mode: modeInput ? modeInput.value : undefined,
        channel_mask: channelInput ? Number(channelInput.value) : undefined,
        keypad_mask: keypadInput ? Number(keypadInput.value) : undefined,
        exit_seconds: exitInput ? Number(exitInput.value) : undefined,
        alert_target: alertTargetInput ? alertTargetInput.value : undefined,
        alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
        enabled: row.dataset.enabled !== 'false',
      };
    }

    if (document.activeElement && row.contains(document.activeElement)) {
      let type = null;
      if (row.classList.contains('credential-card--rfid')) type = 'rfid';
      if (row.classList.contains('credential-card--remote')) type = 'remote';
      if (row.classList.contains('credential-card--pin')) type = 'pin';
      let selector = null;
      [
        '.user-name-input',
        '.pin-code-input',
        '.wiegand-mode-select',
        '.wiegand-channel-select',
        '.wiegand-alert-target-select',
        '.rf-mode-select',
        '.rf-channel-select',
        '.rf-alert-target-select',
        '.rf-exit-seconds',
        '.pin-mode-select',
        '.pin-channel-select',
        '.pin-keypad-select',
        '.pin-alert-target-select',
        '.pin-exit-seconds',
      ].some((candidate) => {
        if (document.activeElement.matches(candidate)) {
          selector = candidate;
          return true;
        }
        return false;
      });
      if (type && selector) values.focused = { type, id, selector };
    }
  });

  return values;
};

const buildCredentialUserGroups = () => {
  const keypadUsers = Array.isArray(App.data?.keypadUsers) ? App.data.keypadUsers : [];
  const wiegandUsers = Array.isArray(App.data?.wiegand?.users) ? App.data.wiegand.users : [];
  const rfUsers = Array.isArray(App.data?.rf?.users) ? App.data.rf.users : [];
  const nameToUser = new Map();
  const groups = new Map();

  keypadUsers.forEach((user, index) => {
    const uuid = user.uuid || '';
    const name = normalizeCredentialUserName(user.name || `User ${index + 1}`);
    if (uuid) nameToUser.set(credentialUserNameKey(name), { uuid, name });
  });

  const groupKeyFor = (uuid, name) => {
    if (uuid) return `uuid:${uuid}`;
    const known = nameToUser.get(credentialUserNameKey(name));
    if (known?.uuid) return `uuid:${known.uuid}`;
    return `name:${credentialUserNameKey(name)}`;
  };

  const ensureGroup = ({ uuid = '', name = '' } = {}) => {
    const known = !uuid ? nameToUser.get(credentialUserNameKey(name)) : null;
    const groupUuid = uuid || known?.uuid || '';
    const groupName = normalizeCredentialUserName(name || known?.name);
    const key = groupKeyFor(groupUuid, groupName);
    if (!groups.has(key)) {
      groups.set(key, {
        key,
        uuid: groupUuid,
        name: groupName,
        pins: [],
        rfid: [],
        remotes: [],
      });
    } else if (groupUuid && !groups.get(key).uuid) {
      groups.get(key).uuid = groupUuid;
    }
    return groups.get(key);
  };

  keypadUsers.forEach((user, index) => {
    ensureGroup({
      uuid: user.uuid || '',
      name: user.name || `User ${index + 1}`,
    });
  });

  const pinCredentials = [
    ...expandPinUserCredentials(keypadUsers).filter((user) => String(user.pin || '').trim()),
    ...livePinCredentialRows().map((user) => ({
      ...user,
      uuid: App.data?.enrollment?.userUuid || '',
      name: App.data?.enrollment?.userName || user.name,
    })),
  ];

  pinCredentials.forEach((user) => {
    ensureGroup({
      uuid: user.uuid || '',
      name: user.name,
    }).pins.push(user);
  });

  wiegandUsers.forEach((user) => {
    ensureGroup({
      uuid: credentialOwnerUuid(user),
      name: credentialOwnerName(user),
    }).rfid.push(user);
  });

  rfUsers.forEach((user) => {
    ensureGroup({
      uuid: credentialOwnerUuid(user),
      name: credentialOwnerName(user),
    }).remotes.push(user);
  });

  const assignments = App.data?.schedules?.assignments || {};
  groups.forEach((group) => {
    group.scheduleId = group.uuid ? (assignments[group.uuid] || '') : '';
  });

  return Array.from(groups.values())
    .sort((left, right) => left.name.localeCompare(right.name));
};

const credentialGroupSummary = (group) => {
  const parts = [
    [group.rfid.length, 'RFID'],
    [group.pins.length, 'PIN'],
    [group.remotes.length, 'Remote'],
  ]
    .filter(([count]) => count > 0)
    .map(([count, label]) => `${count} ${label}${count === 1 || label === 'RFID' ? '' : 's'}`);
  return parts.length ? parts.join(' · ') : 'No credentials';
};

const groupContainsFocusedCredential = (group, focused) => {
  if (!focused) return false;
  const id = String(focused.id || '');
  if (!id) return false;
  if (focused.type === 'rfid') return group.rfid.some((user) => String(user.id || '') === id);
  if (focused.type === 'remote') return group.remotes.some((user) => String(user.id || '') === id);
  if (focused.type === 'pin') return group.pins.some((user) => String(user.credentialId || user.uuid || '') === id);
  return false;
};

const groupMatchesActiveEnrollment = (group) => {
  const enrollment = App.data?.enrollment || {};
  if (!enrollment.active) return false;
  if (enrollment.userUuid && group.uuid === enrollment.userUuid) return true;
  return credentialUserNameKey(group.name) === credentialUserNameKey(enrollment.userName || '');
};

// ---- Schedule profiles ----

const BUILTIN_SCHEDULES = [
  { id: '', name: 'Always', start: null, end: null },
  { id: 'day', name: 'Day', start: '06:00', end: '18:00' },
  { id: 'night', name: 'Night', start: '18:00', end: '06:00' },
];

const formatTime12h = (hhmm) => {
  const match = /^(\d{1,2}):(\d{2})$/.exec(String(hhmm || ''));
  if (!match) return hhmm || '';
  let hour = Number(match[1]);
  const minute = match[2];
  const suffix = hour >= 12 ? 'PM' : 'AM';
  hour %= 12;
  if (hour === 0) hour = 12;
  return `${hour}:${minute} ${suffix}`;
};

const formatUtcOffset = (offsetSeconds) => {
  const total = Number(offsetSeconds) || 0;
  const sign = total < 0 ? '−' : '+';
  const abs = Math.abs(total);
  const hours = Math.floor(abs / 3600);
  const minutes = Math.floor((abs % 3600) / 60);
  return `UTC${sign}${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}`;
};

const renderTimezoneNote = () => {
  const resolved = !!App.data?.schedules?.utc_offset_resolved;
  const offset = App.data?.schedules?.utc_offset_seconds;
  return resolved
    ? `<p class="schedule-editor-utc-note">Times shown in the device's local time zone (${formatUtcOffset(offset)}), detected from its network location.</p>`
    : '<p class="schedule-editor-utc-note">Detecting time zone from the network — showing UTC until that finishes.</p>';
};

const SCHEDULE_DAYS = [
  { key: 'sun', label: 'Sun' },
  { key: 'mon', label: 'Mon' },
  { key: 'tue', label: 'Tue' },
  { key: 'wed', label: 'Wed' },
  { key: 'thu', label: 'Thu' },
  { key: 'fri', label: 'Fri' },
  { key: 'sat', label: 'Sat' },
];

const customScheduleProfiles = () => (Array.isArray(App.data?.schedules?.profiles) ? App.data.schedules.profiles : []);

const allScheduleOptions = () => [
  ...BUILTIN_SCHEDULES.map(({ id, name }) => ({ id, name })),
  ...customScheduleProfiles().map((profile) => ({ id: profile.id, name: profile.name || 'Untitled profile' })),
];

const scheduleNameFor = (id) => {
  const match = allScheduleOptions().find((option) => option.id === (id || ''));
  return match ? match.name : 'Always';
};

const renderUserScheduleSelect = (group) => {
  if (!group.uuid) return '';
  const options = allScheduleOptions();
  const current = group.scheduleId || '';
  return `
    <label class="user-schedule-select">
      <span>Access schedule</span>
      <select class="user-schedule-picker" data-user-uuid="${escapeHtml(group.uuid)}">
        ${options.map((option) => `
          <option value="${escapeHtml(option.id)}" ${option.id === current ? 'selected' : ''}>${escapeHtml(option.name)}</option>
        `).join('')}
      </select>
    </label>
  `;
};

const renderScheduleChips = () => {
  const options = allScheduleOptions();
  const editingId = App.scheduleEditingId;
  const chips = options.map((option) => `
    <button type="button" class="schedule-profile-chip ${option.id === editingId ? 'is-active' : ''}" data-action="select-schedule-profile" data-schedule-id="${escapeHtml(option.id)}">${escapeHtml(option.name)}</button>
  `).join('');
  return `${chips}<button type="button" class="schedule-profile-add" data-action="add-schedule-profile" aria-label="Add schedule profile" title="Add schedule profile">+</button>`;
};

const renderScheduleEditor = () => {
  const editingId = App.scheduleEditingId;
  if (editingId === null || editingId === undefined) return '';

  const builtin = BUILTIN_SCHEDULES.find((option) => option.id === editingId);
  if (builtin) {
    const windowText = builtin.start
      ? `Every day, ${formatTime12h(builtin.start)} – ${formatTime12h(builtin.end)}.`
      : 'No restrictions — access allowed at all times.';
    return `
      <div class="schedule-profile-name">${escapeHtml(builtin.name)}</div>
      <p class="schedule-editor-readonly-note">${escapeHtml(windowText)} Built-in schedules can't be edited or deleted — add a custom profile instead.</p>
      ${builtin.start ? renderTimezoneNote() : ''}
    `;
  }

  const profile = customScheduleProfiles().find((candidate) => candidate.id === editingId);
  if (!profile) return '';

  const days = profile.days || {};
  const dayRows = SCHEDULE_DAYS.map(({ key, label }) => {
    const day = days[key] || { enabled: true, start: '09:00', end: '17:00' };
    const start = day.start || '09:00';
    const end = day.end || '17:00';
    return `
      <div class="schedule-day-row" data-day="${key}" data-day-disabled="${day.enabled ? 'false' : 'true'}">
        <label class="form-switch">
          <input type="checkbox" class="schedule-day-enabled" ${day.enabled ? 'checked' : ''}>
          <span>${label}</span>
        </label>
        <span class="schedule-day-time-group">
          <input type="time" class="schedule-day-time schedule-day-start" value="${escapeHtml(start)}">
          <span class="schedule-day-time-ampm">${escapeHtml(formatTime12h(start))}</span>
        </span>
        <span class="schedule-day-sep">to</span>
        <span class="schedule-day-time-group">
          <input type="time" class="schedule-day-time schedule-day-end" value="${escapeHtml(end)}">
          <span class="schedule-day-time-ampm">${escapeHtml(formatTime12h(end))}</span>
        </span>
      </div>
    `;
  }).join('');

  return `
    <div class="schedule-editor-head">
      <input type="text" class="schedule-profile-name" data-schedule-id="${escapeHtml(profile.id)}" value="${escapeHtml(profile.name || '')}" placeholder="Profile name" aria-label="Profile name">
      ${renderCredentialIconButton('delete-schedule-profile', profile.id, 'Delete schedule profile', 'credential-remove-icon')}
    </div>
    ${renderTimezoneNote()}
    <div class="schedule-day-grid">${dayRows}</div>
  `;
};

const renderSchedules = () => {
  const stripEl = App.elements.scheduleProfileStrip;
  const editorEl = App.elements.scheduleProfileEditor;
  if (!stripEl || !editorEl) return;

  stripEl.innerHTML = renderScheduleChips();

  const editorHtml = renderScheduleEditor();
  editorEl.innerHTML = editorHtml;
  editorEl.hidden = !editorHtml;
};

const loadSchedules = async () => {
  try {
    const schedules = await fetchJSON(`api/schedules?t=${Date.now()}`);
    if (App.data) App.data.schedules = schedules;
    renderSchedules();
    renderCredentialUsers();
  } catch (error) {
    console.warn('Failed to load schedules', error);
  }
};

const collectDayGridFromDom = (editorEl) => {
  const days = {};
  editorEl.querySelectorAll('.schedule-day-row').forEach((row) => {
    const key = row.dataset.day;
    if (!key) return;
    const enabled = row.querySelector('.schedule-day-enabled')?.checked !== false;
    const start = row.querySelector('.schedule-day-start')?.value || '09:00';
    const end = row.querySelector('.schedule-day-end')?.value || '17:00';
    days[key] = { enabled, start, end };
  });
  return days;
};

const saveScheduleProfile = async (id, patch) => {
  try {
    const schedules = await fetchJSON('api/schedules', {
      method: 'PUT',
      body: JSON.stringify({ id, ...patch }),
    });
    if (App.data) App.data.schedules = schedules;
    renderSchedules();
    renderCredentialUsers();
  } catch (error) {
    handleError(error, 'Failed to update schedule profile');
  }
};

let scheduleNameSaveTimer = null;

const setupScheduleHandlers = () => {
  const stripEl = App.elements.scheduleProfileStrip;
  const editorEl = App.elements.scheduleProfileEditor;
  if (!stripEl || !editorEl) return;

  stripEl.addEventListener('click', async (event) => {
    const addBtn = event.target.closest('button[data-action="add-schedule-profile"]');
    if (addBtn) {
      addBtn.disabled = true;
      try {
        const schedules = await fetchJSON('api/schedules', { method: 'POST', body: JSON.stringify({}) });
        if (App.data) App.data.schedules = schedules;
        const profiles = schedules.profiles || [];
        const created = profiles[profiles.length - 1];
        App.scheduleEditingId = created ? created.id : null;
        renderSchedules();
        showToast('Schedule profile created.');
        const nameInput = editorEl.querySelector('.schedule-profile-name');
        if (nameInput) {
          nameInput.focus();
          nameInput.select();
        }
      } catch (error) {
        handleError(error, 'Failed to create schedule profile');
      } finally {
        addBtn.disabled = false;
      }
      return;
    }

    const chip = event.target.closest('button[data-action="select-schedule-profile"]');
    if (chip) {
      const id = chip.getAttribute('data-schedule-id') || '';
      App.scheduleEditingId = App.scheduleEditingId === id ? null : id;
      renderSchedules();
    }
  });

  editorEl.addEventListener('click', async (event) => {
    const deleteBtn = event.target.closest('button[data-action="delete-schedule-profile"]');
    if (!deleteBtn) return;
    const id = deleteBtn.getAttribute('data-id');
    if (!id) return;

    deleteBtn.disabled = true;
    try {
      const schedules = await fetchJSON('api/schedules', {
        method: 'DELETE',
        body: JSON.stringify({ id }),
      });
      if (App.data) App.data.schedules = schedules;
      App.scheduleEditingId = null;
      renderSchedules();
      renderCredentialUsers();
      showToast('Schedule profile deleted.');
    } catch (error) {
      handleError(error, 'Failed to delete schedule profile');
      deleteBtn.disabled = false;
    }
  });

  editorEl.addEventListener('input', (event) => {
    const timeInput = event.target.closest('.schedule-day-time');
    if (timeInput) {
      const ampmLabel = timeInput.parentElement?.querySelector('.schedule-day-time-ampm');
      if (ampmLabel) ampmLabel.textContent = formatTime12h(timeInput.value);
      return;
    }

    const nameInput = event.target.closest('.schedule-profile-name');
    if (!nameInput || nameInput.tagName !== 'INPUT') return;
    const id = nameInput.getAttribute('data-schedule-id');
    if (!id || !nameInput.value.trim()) return;
    clearTimeout(scheduleNameSaveTimer);
    scheduleNameSaveTimer = setTimeout(() => {
      saveScheduleProfile(id, { name: nameInput.value.trim() });
    }, 650);
  });

  editorEl.addEventListener('change', (event) => {
    const dayControl = event.target.closest('.schedule-day-enabled, .schedule-day-start, .schedule-day-end');
    if (!dayControl) return;
    const nameInput = editorEl.querySelector('.schedule-profile-name');
    const id = nameInput?.getAttribute('data-schedule-id');
    if (!id) return;
    const row = dayControl.closest('.schedule-day-row');
    if (row) {
      const enabled = row.querySelector('.schedule-day-enabled')?.checked !== false;
      row.dataset.dayDisabled = enabled ? 'false' : 'true';
    }
    saveScheduleProfile(id, { days: collectDayGridFromDom(editorEl) });
  });
};

let lastRenderedCredentialUserHtml = null;

const renderCredentialUsers = () => {
  const listEl = App.elements.credentialUserList;
  if (!listEl) return;
  if (shouldDeferCredentialRender()) return;

  const preserved = collectCredentialFormValues();
  const groups = buildCredentialUserGroups();
  const html = !groups.length
    ? '<p class="empty-state muted">No users or credentials yet.</p>'
    : groups.map((group) => {
    const shouldOpen =
      App.credentialUserOpenKeys.has(group.key)
      || groupContainsFocusedCredential(group, preserved.focused)
      || groupMatchesActiveEnrollment(group);
    const credentialCards = [
      ...group.rfid.map((user) => buildWiegandUserRow(user, preserved.rfid[user.id])),
      ...group.pins.map((user, index) => buildKeypadUserRow(user, index, preserved.pin[user.credentialId || user.uuid])),
      ...group.remotes.map((user) => buildRfUserRow(user, preserved.remote[user.id])),
    ].join('');

    return `
      <section class="credential-user-group ${shouldOpen ? 'is-open' : ''}" data-user-key="${escapeHtml(group.key)}">
        <div class="credential-user-header">
          <button type="button" class="credential-user-toggle" data-action="toggle-user-group" data-user-key="${escapeHtml(group.key)}" aria-expanded="${shouldOpen ? 'true' : 'false'}">
            <span class="credential-user-title">
              <strong>${escapeHtml(group.name)}</strong>
            </span>
            <span class="credential-user-summary">${escapeHtml(credentialGroupSummary(group))}</span>
            <span class="credential-user-chevron" aria-hidden="true"></span>
          </button>
          ${renderCredentialIconButton('delete-user', group.key, `Delete ${group.name} and all their credentials`, 'credential-remove-icon credential-user-remove')}
        </div>
        <div class="credential-user-body" ${shouldOpen ? '' : 'hidden'}>
          ${renderUserScheduleSelect(group)}
          ${credentialCards
            ? `<div class="credential-user-grid">${credentialCards}</div>`
            : '<p class="empty-state muted">No credentials for this user yet.</p>'}
        </div>
      </section>
    `;
  }).join('');

  if (html !== lastRenderedCredentialUserHtml) {
    lastRenderedCredentialUserHtml = html;
    listEl.innerHTML = html;
  }

  applyCredentialActivityState({
    wiegand: App.data?.wiegand || {},
    rf: App.data?.rf || {},
  });

  if (Array.isArray(App.data?.keypadUsers)) {
    applyPinCredentialActivityState(App.data.keypadUsers);
  }

  const focused = preserved.focused;
  if (focused) {
    const row = Array.from(listEl.querySelectorAll('.credential-card'))
      .find((candidate) => candidate.getAttribute('data-id') === focused.id);
    const input = row?.querySelector(focused.selector);
    if (input) {
      input.focus();
      if (typeof input.setSelectionRange === 'function') {
        input.setSelectionRange(input.value.length, input.value.length);
      }
    }
  }
};

const applyPinCredentialActivityState = (users = []) => {
  expandPinUserCredentials(users).forEach((user) => {
    const card = findCredentialCard('keypadUserList', user.credentialId || user.uuid || '');
    if (!card) return;
    const lastUsed = user.lastUsed || null;
    const lastUsedPin = String(lastUsed?.pin || user.last_used_pin || '');
    const credentialPin = String(user.pin || '');
    const matchesPin = !lastUsedPin || !credentialPin || lastUsedPin === credentialPin;
    const ageMs = Number(lastUsed?.age_ms);
    updateActivityHighlight(card, ageMs, !!lastUsed && matchesPin);
  });
};

const renderLivePinEntries = (entries = []) => {
  const el = App.elements.livePinEntries;
  if (!el) return;
  const list = Array.isArray(entries) ? entries : [];
  if (!list.length) {
    el.innerHTML = '';
    el.hidden = true;
    return;
  }
  el.hidden = false;
  el.innerHTML = list.map((entry) => {
    const active = !!entry.active && entry.code;
    const code = active ? String(entry.code) : '';
    return `
      <div class="live-pin-entry ${active ? 'is-active' : ''}">
        <span>${escapeHtml(formatChannelLabel(Number(entry.channel) || 0))}</span>
        <strong>${escapeHtml(active ? code : 'Idle')}</strong>
      </div>
    `;
  }).join('');
};

const renderKeypadUsers = (users = []) => {
  const listEl = App.elements.keypadUserList;
  if (App.data) {
    App.data.keypadUsers = Array.isArray(users) ? users : [];
  }
  renderEnrollmentUserOptions();
  if (App.elements.keypadRemoveAllBtn) {
    App.elements.keypadRemoveAllBtn.disabled = !users.length;
  }
  const credentialRows = [
    ...expandPinUserCredentials(users),
  ];

  if (listEl && !credentialRows.length) {
    listEl.innerHTML = '<p class="empty-state muted">No PIN codes configured yet.</p>';
  } else if (listEl) {
    // Preserve input values that user may be editing
    const existingValues = {};
    let focusedUuid = null;
    let focusedSelector = null;
    listEl.querySelectorAll('.user-row').forEach((row) => {
      if (row.dataset.pending === 'true') return;
      const credentialId = row.getAttribute('data-id');
      if (!credentialId) return;
      const nameInput = row.querySelector('.user-name-input');
      const pinInput = row.querySelector('.pin-code-input');
      const modeInput = row.querySelector('.pin-mode-select');
      const channelInput = row.querySelector('.pin-channel-select');
      const keypadInput = row.querySelector('.pin-keypad-select');
      const exitInput = row.querySelector('.pin-exit-seconds');
      const alertTargetInput = row.querySelector('.pin-alert-target-select');
      existingValues[credentialId] = {
        name: nameInput ? nameInput.value : undefined,
        pin: pinInput ? pinInput.value : undefined,
        mode: modeInput ? modeInput.value : undefined,
        channel_mask: channelInput ? Number(channelInput.value) : undefined,
        keypad_mask: keypadInput ? Number(keypadInput.value) : undefined,
        exit_seconds: exitInput ? Number(exitInput.value) : undefined,
        alert_target: alertTargetInput ? alertTargetInput.value : undefined,
        alert: alertTargetInput ? alertFromTarget(alertTargetInput.value) : undefined,
        enabled: row.dataset.enabled !== 'false',
      };
      if (document.activeElement && row.contains(document.activeElement)) {
        focusedUuid = credentialId;
        if (document.activeElement.classList.contains('pin-code-input')) focusedSelector = '.pin-code-input';
        else if (document.activeElement.classList.contains('user-name-input')) focusedSelector = '.user-name-input';
        else if (document.activeElement.classList.contains('pin-exit-seconds')) focusedSelector = '.pin-exit-seconds';
      }
    });

    listEl.innerHTML = credentialRows
      .map((user, idx) => buildKeypadUserRow(user, idx, existingValues[user.credentialId || user.uuid]))
      .join('');
    applyPinCredentialActivityState(users);

    if (focusedUuid && focusedSelector) {
      const row = Array.from(listEl.querySelectorAll('.user-row'))
        .find((candidate) => candidate.getAttribute('data-id') === focusedUuid);
      const input = row?.querySelector(focusedSelector);
      if (input) {
        input.focus();
        if (typeof input.setSelectionRange === 'function') {
          input.setSelectionRange(input.value.length, input.value.length);
        }
      }
    }
  }
  renderCredentialUsers();
};

const renderEnrollmentUserOptions = () => {
  const select = App.elements.enrollUserSelect;
  if (!select) return;

  const previous = select.value;
  // Source from the full merged user list (not just keypadUsers) so a person who so
  // far only has an RFID card or remote still shows up as a selectable existing user
  // instead of forcing every next credential into a brand-new "Default User".
  const groups = buildCredentialUserGroups();
  if (!groups.length) {
    select.innerHTML = '<option value="">Default User</option>';
    return;
  }

  select.innerHTML = groups
    .map((group) => `<option value="${escapeHtml(group.key)}">${escapeHtml(group.name)}</option>`)
    .join('');

  if (previous && groups.some((group) => group.key === previous)) {
    select.value = previous;
  }
};

const renderWifi = (wifi = {}, networkState = {}, system = {}) => {
  const list = document.getElementById('wifiNetworks');
  const availableList = document.getElementById('wifiAvailableNetworks');
  const activeEl = document.getElementById('wifiActive');
  if (!list) return;
  const networks = Array.isArray(wifi.networks) ? wifi.networks : [];
  const scanned = Array.isArray(wifi.scanned) ? wifi.scanned : [];
  const configuredSsid = wifi.active_ssid || '';
  const staConnected = networkState.wifi_sta_connected === true;
  const active = staConnected ? configuredSsid : '';
  const savedSsids = new Set(networks.map((network) => network.ssid).filter(Boolean));
  const activeSaved = networks.find((network) => network.ssid === configuredSsid) || null;
  const activeScan = scanned
    .filter((network) => network.ssid === configuredSsid)
    .sort((left, right) => (Number(right.rssi) || -999) - (Number(left.rssi) || -999))[0] || null;
  if (activeEl) {
    if (!configuredSsid) {
      activeEl.innerHTML = '<p class="empty-state muted">No active station network.</p>';
    } else if (!staConnected) {
      const targetSignal = formatWifiSignal(activeScan);
      const targetDetails = [
        ['Status', 'Station not connected'],
        ['AP mode', networkState.wifi_ap_ip || '192.168.4.1'],
        ['Target seen', activeScan ? targetSignal : 'No'],
        ['STA IP', '—'],
        ['Gateway', '—'],
        ['STA MAC', networkState.wifi_sta_mac || '—'],
        ['AP BSSID', activeScan?.bssid || '—'],
        ['Channel', activeScan?.channel || '—'],
        ['Security', activeScan?.auth || (activeScan?.secure ? 'Secured' : '—')],
      ];
      const strength = wifiSignalFromRssi(activeScan?.rssi) ?? 0;
      activeEl.innerHTML = `
        <div class="wifi-active-summary">
          <div class="wifi-active-title">${escapeHtml(configuredSsid)}</div>
          <div class="wifi-active-signal">
            <span class="wifi-strength" aria-hidden="true"><span style="width:${strength}%"></span></span>
            <strong>${escapeHtml(activeScan ? targetSignal : 'Not connected')}</strong>
          </div>
        </div>
        <div class="wifi-active-grid">
          ${targetDetails.map(([label, value]) => `
            <div class="wifi-detail">
              <span>${escapeHtml(label)}</span>
              <strong>${escapeHtml(value || '—')}</strong>
            </div>
          `).join('')}
        </div>
      `;
    } else {
      const connectedAtMs = activeSaved?.last_used_ms;
      const connectedForSeconds = Number.isFinite(Number(connectedAtMs))
        ? Math.max(0, (Number(system.uptimeSeconds) || 0) - Math.round(Number(connectedAtMs) / 1000))
        : null;
      const liveSignal = formatWifiLink(networkState.wifi_sta_quality, networkState.wifi_sta_rssi);
      const activeDetails = [
        ['Link quality', liveSignal !== '—' ? liveSignal : formatWifiSignal(activeScan)],
        ['Connected', connectedForSeconds != null ? `${formatUptime(connectedForSeconds)} ago` : formatSinceBoot(connectedAtMs)],
        ['STA IP', networkState.wifi_sta_ip || '—'],
        ['Gateway', networkState.wifi_sta_gateway || '—'],
        ['STA MAC', networkState.wifi_sta_mac || '—'],
        ['AP BSSID', networkState.wifi_sta_bssid || activeScan?.bssid || '—'],
        ['Channel', networkState.wifi_sta_channel || activeScan?.channel || '—'],
        ['Security', networkState.wifi_sta_auth || activeScan?.auth || (activeScan?.secure ? 'Secured' : '—')],
      ];
      const strength = Number.isFinite(Number(networkState.wifi_sta_quality))
        ? Math.max(0, Math.min(100, Math.round(Number(networkState.wifi_sta_quality))))
        : (wifiSignalFromRssi(activeScan?.rssi) ?? 0);
      activeEl.innerHTML = `
        <div class="wifi-active-summary">
          <div class="wifi-active-title">${escapeHtml(active)}</div>
          <div class="wifi-active-signal">
            <span class="wifi-strength" aria-hidden="true"><span style="width:${strength}%"></span></span>
            <strong>${escapeHtml(activeDetails[0][1])}</strong>
          </div>
        </div>
        <div class="wifi-active-grid">
          ${activeDetails.map(([label, value]) => `
            <div class="wifi-detail">
              <span>${escapeHtml(label)}</span>
              <strong>${escapeHtml(value || '—')}</strong>
            </div>
          `).join('')}
        </div>
      `;
    }
  }

  if (availableList) {
    if (!scanned.length) {
      if (wifiScanLoading) {
        availableList.innerHTML = '<p class="empty-state muted">Scanning nearby networks...</p>';
      } else if (wifiScanError && !wifiScanLoaded) {
        availableList.innerHTML = '<p class="empty-state muted">Wi-Fi scan is retrying...</p>';
      } else if (!wifiScanLoaded) {
        availableList.innerHTML = '<p class="empty-state muted">Open Settings or click Scan to refresh nearby networks.</p>';
      } else {
        availableList.innerHTML = '<p class="empty-state muted">No nearby networks found.</p>';
      }
    } else {
      availableList.innerHTML = scanned
        .map((network) => {
          const ssid = network.ssid || '';
          const saved = savedSsids.has(ssid);
          const strength = wifiSignalFromRssi(network.rssi) ?? 0;
          return `
            <button type="button" class="wifi-network-card" data-action="wifi-select" data-ssid="${escapeHtml(ssid)}">
              <span class="wifi-network-name">${escapeHtml(ssid)}</span>
              <span class="wifi-network-meta">${escapeHtml(network.auth || (network.secure ? 'Secured' : 'Open'))}${saved ? ' · Saved' : ''}</span>
              <span class="wifi-strength" aria-hidden="true"><span style="width:${strength}%"></span></span>
            </button>
          `;
        })
        .join('');
    }
  }

  if (!networks.length) {
    if (wifiListLoading) {
      list.innerHTML = '<p class="empty-state muted">Loading saved Wi-Fi networks...</p>';
    } else if (wifiListError && !wifiListLoaded) {
      list.innerHTML = '<p class="empty-state muted">Saved Wi-Fi list is retrying...</p>';
    } else {
      list.innerHTML = '<p class="empty-state muted">No saved Wi-Fi networks.</p>';
    }
    return;
  }
  list.innerHTML = networks
    .map(
      (n) => `
      <div class="wifi-saved-card ${active === n.ssid ? 'is-active' : ''}" data-ssid="${escapeHtml(n.ssid || '')}">
        <div class="wifi-saved-info">
          <strong>${escapeHtml(n.ssid || '')}</strong>
          <span>${active === n.ssid ? 'Active now' : 'Saved network'}</span>
          <div class="meta muted">${n.last_used_ms ? `Last used: ${formatSinceBoot(n.last_used_ms)}` : 'Not used since boot'}</div>
        </div>
        <div class="wifi-saved-actions">
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
  const listEl = App.elements.keypadUserList || App.elements.credentialUserList;
  const removeAllBtn = App.elements.keypadRemoveAllBtn;

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

      if (!/^\d{4,8}$/.test(pin)) {
        showToast('PIN must be 4-8 digits.');
        return;
      }

      saveNewBtn.disabled = true;
      try {
        const users = await fetchJSON('api/keypad/user', {
          method: 'POST',
          body: JSON.stringify({ name, pin }),
        });
        renderKeypadUsers(Array.isArray(users) ? users : []);
        showToast('PIN code added successfully.');
        addForm.hidden = true;
        addBtn.disabled = false;
        nameInput.value = '';
        pinInput.value = '';
        if (App.data) App.data.keypadUsers = Array.isArray(users) ? users : [];
        // Refresh list from flash to match persisted state
        loadKeypadUsers();
      } catch (error) {
        handleError(error, 'Failed to add PIN code');
      } finally {
        saveNewBtn.disabled = false;
      }
    });
  }

  if (removeAllBtn) {
    removeAllBtn.addEventListener('click', async () => {
      const users = App.data?.keypadUsers || [];
      if (!users.length) {
        showToast('No PIN codes to remove.');
        return;
      }

      removeAllBtn.disabled = true;
      try {
        const latest = await fetchJSON('api/keypad/users/delete-all', {
          method: 'POST',
          body: JSON.stringify({}),
        });
        const list = Array.isArray(latest) ? latest : [];
        renderKeypadUsers(list);
        if (App.data) App.data.keypadUsers = list;
        await loadKeypadUsers();
        await loadState();
        showToast(`Removed ${users.length} PIN code${users.length === 1 ? '' : 's'}.`);
      } catch (error) {
        handleError(error, 'Failed to remove users');
      } finally {
        removeAllBtn.disabled = false;
      }
    });
  }

  if (listEl) {
    const savePinUser = async (container, trigger, { quiet = false } = {}) => {
      const nameInput = container?.querySelector('.user-name-input');
      const pinInput = container?.querySelector('.pin-code-input');
      const modeInput = container?.querySelector('.pin-mode-select');
      const channelInput = container?.querySelector('.pin-channel-select');
      const keypadInput = container?.querySelector('.pin-keypad-select');
      const exitInput = container?.querySelector('.pin-exit-seconds');
      const alertTargetInput = container?.querySelector('.pin-alert-target-select');
      const uuid = container?.getAttribute('data-uuid');
      const pinIndex = Number(container?.dataset.pinIndex ?? -1);
      const name = nameInput?.value.trim();
      const pin = pinInput?.value.trim();
      const mode = modeInput?.value || 'momentary';
      const channelMask = Number(channelInput?.value || 1);
      const keypadMask = Number(keypadInput?.value || 3);
      const exitSeconds = Number(exitInput?.value || 4);
      const enabled = container?.dataset.enabled !== 'false';
      const alert_target = normalizeAlertTarget(alertTargetInput?.value, true);
      const alert = alertFromTarget(alert_target);

      if (!uuid || !name) {
        showToast('Please provide a name.');
        return null;
      }
      if (!/^\d{4,8}$/.test(pin || '')) {
        showToast('PIN must be 4-8 digits.');
        return null;
      }

      if (trigger) trigger.disabled = true;
      try {
        const users = await fetchJSON('api/keypad/user', {
          method: 'PUT',
          body: JSON.stringify({
            uuid,
            name,
            pin,
            pinIndex,
            mode,
            channel_mask: channelMask,
            keypad_mask: keypadMask,
            exit_seconds: exitSeconds > 0 ? exitSeconds : 4,
            alert,
            alert_target,
            enabled,
          }),
        });
        renderKeypadUsers(Array.isArray(users) ? users : []);
        if (!quiet) showToast('PIN code updated.');
        if (App.data) App.data.keypadUsers = Array.isArray(users) ? users : [];
        return users;
      } catch (error) {
        handleError(error, 'Failed to update user');
        throw error;
      } finally {
        if (trigger) trigger.disabled = false;
      }
    };

    listEl.addEventListener('click', async (event) => {
      const toggleEnabledBtn = event.target.closest('button[data-action="toggle-pin-enabled"]');
      const deleteBtn = event.target.closest('button[data-action="delete-pin"]');

      if (toggleEnabledBtn) {
        const container = toggleEnabledBtn.closest('.credential-card--pin');
        if (!container) return;
        const previousEnabled = container.dataset.enabled !== 'false';
        const nextEnabled = !previousEnabled;
        container.dataset.enabled = nextEnabled ? 'true' : 'false';
        container.classList.toggle('is-card-disabled', !nextEnabled);
        toggleEnabledBtn.classList.toggle('is-enabled', nextEnabled);
        toggleEnabledBtn.classList.toggle('is-disabled', !nextEnabled);
        toggleEnabledBtn.setAttribute('aria-pressed', nextEnabled ? 'true' : 'false');
        try {
          await savePinUser(container, toggleEnabledBtn, { quiet: true });
          showToast(`PIN code ${nextEnabled ? 'enabled' : 'disabled'}.`);
        } catch (error) {
          container.dataset.enabled = previousEnabled ? 'true' : 'false';
          container.classList.toggle('is-card-disabled', !previousEnabled);
        }
        return;
      }

      if (deleteBtn) {
        const container = deleteBtn.closest('.credential-card--pin');
        const uuid = container?.getAttribute('data-uuid') || deleteBtn.getAttribute('data-id');
        const pinIndex = Number(container?.dataset.pinIndex ?? deleteBtn.getAttribute('data-pin-index') ?? -1);
        if (!uuid) return;

        deleteBtn.disabled = true;
        try {
          const users = await fetchJSON('api/keypad/user', {
            method: 'DELETE',
            body: JSON.stringify({ uuid, pinIndex }),
          });
          renderKeypadUsers(Array.isArray(users) ? users : []);
          showToast('PIN code deleted.');
          if (App.data) App.data.keypadUsers = Array.isArray(users) ? users : [];
          loadKeypadUsers();
        } catch (error) {
          handleError(error, 'Failed to delete PIN code');
        } finally {
          deleteBtn.disabled = false;
        }
      }
    });

    listEl.addEventListener('input', (event) => {
      const input = event.target.closest('.user-name-input, .pin-code-input, .pin-exit-seconds');
      if (!input || !input.closest('.credential-card--pin')) return;
      scheduleCredentialNameSave(input, (container) => savePinUser(container, null, { quiet: true }));
    });

    listEl.addEventListener('change', (event) => {
      const control = event.target.closest('.pin-mode-select, .pin-channel-select, .pin-keypad-select, .pin-exit-seconds, .pin-alert-target-select');
      if (!control) return;
      const container = control.closest('.credential-card--pin');
      if (!container) return;
      savePinUser(container, control, { quiet: true }).catch(() => {
        loadKeypadUsers();
      });
    });
  }
};

const setOtaProgress = (percent) => {
  const bar = App.elements.otaProgressBar;
  if (bar) {
    bar.style.width = `${Math.max(0, Math.min(100, percent))}%`;
  }
};

const setOtaStatus = (message) => {
  if (App.elements.otaStatus) {
    App.elements.otaStatus.textContent = message;
  }
};

const waitForDeviceAfterOta = async () => {
  await sleep(3500);
  for (let attempt = 0; attempt < 45; attempt++) {
    try {
      const state = await fetchJSON(`api/state?t=${Date.now()}`);
      if (state?.system?.firmware) {
        App.data = state;
        renderState(state);
        return true;
      }
    } catch (error) {
      // Reboot drops requests briefly.
    }
    await sleep(2000);
  }
  return false;
};

const uploadOtaFile = (file) => new Promise((resolve, reject) => {
  const href = window.location.href;
  const baseHref = href.endsWith('/') ? href : `${href}/`;
  const url = new URL('api/ota/upload', baseHref);
  const xhr = new XMLHttpRequest();

  xhr.open('POST', url.toString());
  xhr.setRequestHeader('Content-Type', 'application/octet-stream');
  xhr.setRequestHeader('X-Firmware-Filename', file.name || 'firmware.bin');

  xhr.upload.onprogress = (event) => {
    if (event.lengthComputable) {
      setOtaProgress((event.loaded / event.total) * 100);
      setOtaStatus(`Uploading ${formatBytes(event.loaded)} of ${formatBytes(event.total)}`);
    }
  };

  xhr.onload = () => {
    const text = xhr.responseText || '';
    if (xhr.status >= 200 && xhr.status < 300) {
      try {
        resolve(text ? JSON.parse(text) : {});
      } catch (error) {
        resolve({});
      }
      return;
    }
    reject(new Error(text || `OTA upload failed: HTTP ${xhr.status}`));
  };

  xhr.onerror = () => reject(new Error('OTA upload connection failed'));
  xhr.ontimeout = () => reject(new Error('OTA upload timed out'));
  xhr.timeout = 120000;
  xhr.send(file);
});

const setupOtaHandlers = () => {
  const form = App.elements.otaForm;
  const fileInput = App.elements.otaFile;
  const uploadBtn = App.elements.otaUploadBtn;
  const fileName = App.elements.otaFileName;
  const fileSize = App.elements.otaFileSize;
  const dropzone = App.elements.otaDropzone;
  const browseBtn = App.elements.otaBrowseBtn;
  let selectedFile = null;

  if (!form || !fileInput || !uploadBtn) return;

  const setSelectedFile = (file) => {
    selectedFile = file || null;
    uploadBtn.disabled = !file;
    if (fileName) fileName.textContent = file ? file.name : 'No file selected';
    if (fileSize) fileSize.textContent = file ? formatBytes(file.size) : '—';
    setOtaProgress(0);
    setOtaStatus(file ? 'Ready' : 'Idle');
  };

  const pickFile = () => {
    if (!fileInput.disabled) fileInput.click();
  };

  fileInput.addEventListener('change', () => {
    setSelectedFile(fileInput.files?.[0] || null);
  });

  if (browseBtn) {
    browseBtn.addEventListener('click', pickFile);
  }

  if (dropzone) {
    dropzone.addEventListener('click', (event) => {
      if (event.target === browseBtn) return;
      pickFile();
    });
    dropzone.addEventListener('keydown', (event) => {
      if (event.key !== 'Enter' && event.key !== ' ') return;
      event.preventDefault();
      pickFile();
    });
    dropzone.addEventListener('dragenter', (event) => {
      event.preventDefault();
      dropzone.classList.add('drag-over');
    });
    dropzone.addEventListener('dragover', (event) => {
      event.preventDefault();
      dropzone.classList.add('drag-over');
    });
    dropzone.addEventListener('dragleave', (event) => {
      if (!dropzone.contains(event.relatedTarget)) {
        dropzone.classList.remove('drag-over');
      }
    });
    dropzone.addEventListener('drop', (event) => {
      event.preventDefault();
      dropzone.classList.remove('drag-over');
      if (fileInput.disabled) return;
      const file = event.dataTransfer?.files?.[0] || null;
      if (!file) return;
      if (!file.name.toLowerCase().endsWith('.bin')) {
        showToast('Select a firmware .bin file.');
        return;
      }
      setSelectedFile(file);
    });
  }

  form.addEventListener('submit', async (event) => {
    event.preventDefault();
    const file = selectedFile || fileInput.files?.[0] || null;
    if (!file) {
      showToast('Select a firmware binary.');
      return;
    }

    uploadBtn.disabled = true;
    fileInput.disabled = true;
    if (browseBtn) browseBtn.disabled = true;
    setOtaProgress(0);
    setOtaStatus('Starting OTA upload');

    try {
      const result = await uploadOtaFile(file);
      setOtaProgress(100);
      setOtaStatus(`Installed ${formatBytes(result.bytes || file.size)} to ${result.partition || 'OTA slot'}. Rebooting.`);
      showToast('Firmware uploaded. Rebooting.');
      const online = await waitForDeviceAfterOta();
      if (online) {
        setOtaStatus('Back online');
        showToast('Controller is back online.');
        fileInput.value = '';
        setSelectedFile(null);
        setOtaProgress(0);
      } else {
        setOtaStatus('Reboot is taking longer than expected');
      }
    } catch (error) {
      setOtaStatus(error.message || 'OTA upload failed');
      handleError(error, 'OTA upload failed');
    } finally {
      fileInput.disabled = false;
      if (browseBtn) browseBtn.disabled = false;
      uploadBtn.disabled = !(fileInput.files && fileInput.files.length);
      uploadBtn.disabled = !selectedFile;
    }
  });
};

const setupEditableLabels = () => {
  const LABEL_STORAGE_KEY = 'ac_section_labels';

  // Load saved labels
  let savedLabels = {};
  try {
    savedLabels = JSON.parse(localStorage.getItem(LABEL_STORAGE_KEY) || '{}');
  } catch (e) { /* ignore */ }

  document.querySelectorAll('.section-label').forEach((input) => {
    const id = input.id;
    // Restore saved value if present
    if (savedLabels[id]) {
      input.value = savedLabels[id];
    }

    // Save on change
    input.addEventListener('change', () => {
      let labels = {};
      try {
        labels = JSON.parse(localStorage.getItem(LABEL_STORAGE_KEY) || '{}');
      } catch (e) { labels = {}; }
      labels[id] = input.value;
      localStorage.setItem(LABEL_STORAGE_KEY, JSON.stringify(labels));
      refreshAllMultiSelectControls();
    });
    input.addEventListener('input', refreshAllMultiSelectControls);
  });
  refreshAllMultiSelectControls();
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
    credentialUserList: document.getElementById('credentialUserList'),
    scheduleProfileStrip: document.getElementById('scheduleProfileStrip'),
    scheduleProfileEditor: document.getElementById('scheduleProfileEditor'),
    wiegandUserList: document.getElementById('wiegandUserList'),
    wiegandRegisterBtn: document.getElementById('wiegandRegisterBtn'),
    wiegandStopBtn: document.getElementById('wiegandStopBtn'),
    wiegandChannelSelect: document.getElementById('wiegandChannelSelect'),
    wiegandRemoveAllBtn: document.getElementById('wiegandRemoveAllBtn'),
    rfStatus: document.getElementById('rfStatus'),
    rfStatusBar: document.getElementById('rfStatusBar'),
    rfPending: document.getElementById('rfPending'),
    rfDuplicate: document.getElementById('rfDuplicate'),
    rfUserList: document.getElementById('rfUserList'),
    rfRegisterBtn: document.getElementById('rfRegisterBtn'),
    rfStopBtn: document.getElementById('rfStopBtn'),
    rfRemoveAllBtn: document.getElementById('rfRemoveAllBtn'),
    rfQuality: document.getElementById('rfQuality'),
    rfLastCode: document.getElementById('rfLastCode'),
    rfTiming: document.getElementById('rfTiming'),
    rfJitter: document.getElementById('rfJitter'),
    rfRepeats: document.getElementById('rfRepeats'),
    rfNoise: document.getElementById('rfNoise'),
    rfDecode: document.getElementById('rfDecode'),
    rfCaptures: document.getElementById('rfCaptures'),
    rfLastInput: document.getElementById('rfLastInput'),
    rfReceiver: document.getElementById('rfReceiver'),
    enrollUserSelect: document.getElementById('enrollUserSelect'),
    enrollStartBtn: document.getElementById('enrollStartBtn'),
    enrollStopBtn: document.getElementById('enrollStopBtn'),
    keypadUserList: document.getElementById('keypadUserList'),
    livePinEntries: document.getElementById('livePinEntries'),
    keypadAddBtn: document.getElementById('keypadAddBtn'),
    keypadAddForm: document.getElementById('keypadAddForm'),
    keypadCancelBtn: document.getElementById('keypadCancelBtn'),
    keypadSaveNewBtn: document.getElementById('keypadSaveNewBtn'),
    keypadRemoveAllBtn: document.getElementById('keypadRemoveAllBtn'),
    logItems: document.getElementById('logItems'),
    logEmptyState: document.getElementById('logEmptyState'),
    wifiNetworks: document.getElementById('wifiNetworks'),
    wifiActive: document.getElementById('wifiActive'),
    otaForm: document.getElementById('otaForm'),
    otaFile: document.getElementById('otaFile'),
    otaDropzone: document.getElementById('otaDropzone'),
    otaBrowseBtn: document.getElementById('otaBrowseBtn'),
    otaUploadBtn: document.getElementById('otaUploadBtn'),
    otaFileName: document.getElementById('otaFileName'),
    otaFileSize: document.getElementById('otaFileSize'),
    otaProgressBar: document.getElementById('otaProgressBar'),
    otaStatus: document.getElementById('otaStatus'),
  };

  // Restore whichever tab was active before the last refresh immediately, so the
  // page doesn't flash to Device before switching to the remembered tab.
  const initialPageId = getStoredActivePageId();
  setActivePage(initialPageId);

  bindNavigation();
  setupCredentialIconControls();
  setupMultiSelectControls();
  setupControlCardChrome();
  setupWiegandDeviceControls();
  setupLockHandlers();
  setupExitHandlers();
  setupFobHandlers();
  setupKeypadHandlers();
  setupEnrollmentHandlers();
  setupForms();
  setupCredentialUserHandlers();
  setupScheduleHandlers();
  setupWiegandHandlers();
  setupRfHandlers();
  setupKeypadPinHandlers();
  setupMotionHandlers();
  setupOtaHandlers();
  setupEditableLabels();

  // Defer heavy endpoints until their tabs are opened; tunneling adds overhead
  // and these requests can clobber the ESP32 heap when fired all at once.
  const startStatePolling = () => {
    if (App.stateTimer) return;
    App.stateTimer = setInterval(loadState, 30000);
  };
  const stopStatePolling = () => {
    if (App.stateTimer) {
      clearInterval(App.stateTimer);
      App.stateTimer = null;
    }
  };

  document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
      stopStatePolling();
      stopEnrollmentPolling();
      stopSignalPolling();
      stopWiegandPolling();
      stopUptimeClock();
      stopHeaderClock();
    } else {
      loadState().finally(() => {
        onPageActivated(getActivePageId());
        startStatePolling();
        startUptimeClock();
        startHeaderClock();
      });
    }
  });

  loadState().finally(() => {
    onPageActivated(getActivePageId());
    startStatePolling();
    startUptimeClock();
    startHeaderClock();
  });
});
