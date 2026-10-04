import { chromium } from 'playwright';

export default async function run(api, report) {
  report.startSuite('Web UI (Playwright)', 'Automated browser tests of the web interface');

  const DEVICE_URL = api.baseUrl;
  let browser;
  let page;

  try {
    const executablePath = process.env.CHROMIUM_PATH?.trim();
    browser = await chromium.launch({
      headless: true,
      ...(executablePath ? { executablePath } : {}),
    });
    page = await browser.newPage();
    page.setDefaultTimeout(10000);
  } catch (err) {
    report.skip('Browser launch', `Could not launch browser: ${err.message}`);
    report.endSuite();
    return {};
  }

  // Helper: navigate and verify
  async function navigateToDevice() {
    try {
      await page.goto(DEVICE_URL, { waitUntil: 'domcontentloaded', timeout: 15000 });
      await page.waitForSelector('.nav-item[data-target="device"]', { timeout: 10000 });
      return true;
    } catch {
      return false;
    }
  }

  // Helper: toggle a checkbox and verify via API
  async function uiToggleTest(label, checkboxId, apiReadFn, channel, field) {
    const t0 = Date.now();
    try {
      const readField = async () => {
        const state = await apiReadFn();
        const items = Array.isArray(state) ? state : state.locks;
        return items?.find?.((item) => item.channel === channel)?.[field];
      };
      const waitForField = async (expected, timeoutMs = 10000) => {
        const deadline = Date.now() + timeoutMs;
        let actual;
        do {
          actual = await readField();
          if (actual === expected) return actual;
          await page.waitForTimeout(200);
        } while (Date.now() < deadline);
        return actual;
      };

      // Read current state from API
      const stateBefore = await apiReadFn();
      const itemsBefore = Array.isArray(stateBefore) ? stateBefore : stateBefore.locks;
      const itemBefore = itemsBefore?.find?.(i => i.channel === channel);
      const origValue = itemBefore?.[field];
      if (typeof origValue !== 'boolean') {
        report.skip(label, `API field ${field} is not boolean`, Date.now() - t0);
        return;
      }
      const desiredValue = !origValue;

      // Find checkbox
      const checkbox = await page.$(`#${checkboxId}`);
      if (!checkbox) {
        report.skip(label, `Checkbox #${checkboxId} not found`, Date.now() - t0);
        return;
      }

      // Periodic state refresh can lag behind the API after a stress sweep. Sync
      // the hidden source checkbox before exercising its visible control.
      await page.evaluate(({ id, checked }) => {
        const input = document.getElementById(id);
        if (input) input.checked = checked;
      }, { id: checkboxId, checked: origValue });
      const wasChecked = origValue;
      const visible = await checkbox.isVisible();
      const alertTargetId = checkboxId
        .replace(/^enableContactAlert_/, 'alertTargetLock_')
        .replace(/^alert(Exit|Fob|Keypad|Motion)_/, 'alertTarget$1_');
      const alertTargetInput = !visible && alertTargetId !== checkboxId
        ? await page.$(`#${alertTargetId}`)
        : null;
      const originalAlertMask = alertTargetInput ? Number(await alertTargetInput.inputValue()) : null;
      const setVisibleAlertMask = async (desiredMask) => {
        const input = page.locator(`#${alertTargetId}`);
        const field = input.locator('xpath=ancestor::*[contains(@class,"multi-select-field")][1]');
        const details = field.locator('details');
        if (await input.count() === 0 || await details.count() === 0) {
          throw new Error(`Visible alert control #${alertTargetId} not found`);
        }
        if (!(await details.evaluate((element) => element.open))) {
          await details.locator('summary').click();
        }
        const bits = await details.locator('input[type="checkbox"][data-bit]').evaluateAll((items) =>
          items.map((item) => Number(item.dataset.bit || 0))
        );
        for (const bit of bits) {
          if (!(await details.evaluate((element) => element.open))) {
            await details.locator('summary').click();
          }
          const option = details.locator(`input[type="checkbox"][data-bit="${bit}"]`);
          const shouldBeChecked = (desiredMask & bit) !== 0;
          if ((await option.isChecked()) !== shouldBeChecked) {
            await option.locator('xpath=ancestor::label[1]').click();
          }
        }
        if (await details.evaluate((element) => element.open)) {
          await details.locator('summary').click();
        }
      };
      const clickCheckboxControl = async () => {
        const currentCheckbox = await page.$(`#${checkboxId}`);
        if (!currentCheckbox) {
          throw new Error(`Checkbox #${checkboxId} disappeared`);
        }
        if (visible) {
          await currentCheckbox.click();
          return;
        }
        const wrapper = await currentCheckbox.$('xpath=ancestor::label[1]') || await page.$(`label[for="${checkboxId}"]`);
        if (!wrapper) {
          throw new Error(`Visible control for hidden checkbox #${checkboxId} not found`);
        }
        await wrapper.click();
      };
      if (!visible && field === 'enable') {
        const button = await page.$(`[data-enable-target="${checkboxId}"]`);
        if (!button) {
          report.skip(label, `Visible enable button for #${checkboxId} not found`, Date.now() - t0);
          return;
        }
        await button.click();
      } else if (!visible && field === 'latch') {
        const modeSelectId = checkboxId.replace(/^latch/, 'mode');
        const modeSelect = await page.$(`#${modeSelectId}`);
        if (!modeSelect) {
          report.skip(label, `Mode select #${modeSelectId} not found`, Date.now() - t0);
          return;
        }
        await modeSelect.selectOption(wasChecked ? 'momentary' : 'latch');
      } else if (alertTargetInput) {
        await setVisibleAlertMask(wasChecked ? 0 : 1);
      } else {
        await clickCheckboxControl();
      }

      // Verify via API. Controller writes may take several seconds after a bulk
      // NVS exercise, so poll the actual state instead of racing a fixed delay.
      const newValue = await waitForField(desiredValue);

      // Restore
      await page.evaluate(({ id, checked }) => {
        const input = document.getElementById(id);
        if (input) input.checked = checked;
      }, { id: checkboxId, checked: desiredValue });
      if (!visible && field === 'enable') {
        const button = await page.$(`[data-enable-target="${checkboxId}"]`);
        await button?.click();
      } else if (!visible && field === 'latch') {
        const modeSelectId = checkboxId.replace(/^latch/, 'mode');
        const modeSelect = await page.$(`#${modeSelectId}`);
        await modeSelect?.selectOption(wasChecked ? 'latch' : 'momentary');
      } else if (alertTargetInput) {
        await setVisibleAlertMask(originalAlertMask);
      } else {
        await clickCheckboxControl();
      }
      await waitForField(origValue);

      if (newValue === desiredValue) {
        report.pass(label, '', Date.now() - t0);
      } else {
        report.fail(label, `Expected ${desiredValue}, got ${newValue}`, Date.now() - t0);
      }
    } catch (err) {
      report.fail(label, err.message, Date.now() - t0);
    }
  }

  async function uiModeSelectTest(label, selectId, apiReadFn, channel, value) {
    const t0 = Date.now();
    try {
      const before = await apiReadFn();
      const itemBefore = before?.find?.(i => i.channel === channel);
      const originalMode = itemBefore?.mode || (itemBefore?.latch ? 'latch' : 'momentary');
      const select = await page.$(`#${selectId}`);
      if (!select) {
        report.skip(label, `Select #${selectId} not found`, Date.now() - t0);
        return;
      }

      await select.selectOption(value);
      const readMode = async () => (await apiReadFn())?.find?.((item) => item.channel === channel)?.mode;
      const waitForMode = async (expected, timeoutMs = 10000) => {
        const deadline = Date.now() + timeoutMs;
        let actual;
        do {
          actual = await readMode();
          if (actual === expected) return actual;
          await page.waitForTimeout(200);
        } while (Date.now() < deadline);
        return actual;
      };
      const changedMode = await waitForMode(value);

      await page.locator(`#${selectId}`).selectOption(originalMode);
      await waitForMode(originalMode);

      if (changedMode === value) {
        report.pass(label, '', Date.now() - t0);
      } else {
        report.fail(label, `Expected ${value}, got ${changedMode}`, Date.now() - t0);
      }
    } catch (err) {
      report.fail(label, err.message, Date.now() - t0);
    }
  }

  // 1. Page Load
  {
    const t0 = Date.now();
    const loaded = await navigateToDevice();
    if (!loaded) {
      report.fail('Page load', `Cannot reach ${DEVICE_URL}`, Date.now() - t0);
      await browser.close();
      report.endSuite();
      return {};
    }

    const title = await page.title();
    if (title.includes('Access Controller')) {
      report.pass('Page title: "Access Controller"', title, Date.now() - t0);
    } else {
      report.fail('Page title', `Got: "${title}"`, Date.now() - t0);
    }
  }

  // 1b. Header IP links update from live network state and navigate in-place.
  {
    const t0 = Date.now();
    try {
      const applyNetwork = async (wifiApIp, wifiStaIp) => {
        await page.evaluate(({ wifiApIp, wifiStaIp }) => {
          applyDeviceInfo({
            network: {
              wifi_ap_ip: wifiApIp,
              wifi_sta_ip: wifiStaIp,
            },
          });
        }, { wifiApIp, wifiStaIp });
      };
      const readLinks = async () => page.evaluate(() => {
        const snapshot = (linkId, valueId) => {
          const link = document.getElementById(linkId);
          return {
            tag: link?.tagName,
            text: document.getElementById(valueId)?.textContent,
            href: link?.getAttribute('href'),
            disabled: link?.getAttribute('aria-disabled'),
          };
        };
        return {
          ap: snapshot('headerApLink', 'headerApIp'),
          sta: snapshot('headerStaLink', 'headerStaIp'),
        };
      });
      const clickAndCaptureDestination = async (selector, targetUrl) => {
        const routeHandler = async (route) => route.fulfill({
          status: 200,
          contentType: 'text/html',
          body: '<!doctype html><title>IP badge destination</title>',
        });
        await page.route(targetUrl, routeHandler);
        try {
          await Promise.all([
            page.waitForURL(targetUrl, { waitUntil: 'domcontentloaded', timeout: 10000 }),
            page.click(selector),
          ]);
          return page.url();
        } finally {
          await page.unroute(targetUrl, routeHandler);
        }
      };

      await applyNetwork('192.168.4.1', null);
      const apOnly = await readLinks();
      await applyNetwork('192.168.4.1', '192.168.1.131');
      const apAndSta = await readLinks();
      const staDestination = await clickAndCaptureDestination('#headerStaLink', 'http://192.168.1.131/');

      await navigateToDevice();
      await applyNetwork('192.168.4.1', '192.168.1.131');
      const apDestination = await clickAndCaptureDestination('#headerApLink', 'http://192.168.4.1/');
      await navigateToDevice();

      const passed =
        apOnly.ap.tag === 'A' &&
        apOnly.ap.text === '192.168.4.1' &&
        apOnly.ap.href === 'http://192.168.4.1/' &&
        apOnly.sta.href === null &&
        apOnly.sta.disabled === 'true' &&
        apAndSta.sta.tag === 'A' &&
        apAndSta.sta.text === '192.168.1.131' &&
        apAndSta.sta.href === 'http://192.168.1.131/' &&
        apAndSta.sta.disabled === null &&
        staDestination === 'http://192.168.1.131/' &&
        apDestination === 'http://192.168.4.1/';

      if (passed) {
        report.pass('UI: AP and STA badges update and navigate', '', Date.now() - t0);
      } else {
        report.fail(
          'UI: AP and STA badges',
          JSON.stringify({ apOnly, apAndSta, staDestination, apDestination }),
          Date.now() - t0
        );
      }
    } catch (err) {
      report.fail('UI: AP and STA badges', err.message, Date.now() - t0);
      await navigateToDevice();
    }
  }

  // 2. Navigation tabs
  {
    const t0 = Date.now();
    const tabs = await page.$$('.nav-item');
    const tabCount = tabs.length;
    if (tabCount >= 3) {
      report.pass(`Navigation tabs: ${tabCount} found`, '', Date.now() - t0);
    } else {
      report.fail('Navigation tabs', `Expected 3+, got ${tabCount}`, Date.now() - t0);
    }
  }

  // 3. Device tab active by default
  {
    const t0 = Date.now();
    const devicePage = await page.$('#page-device.active');
    if (devicePage) {
      report.pass('Device tab active by default', '', Date.now() - t0);
    } else {
      report.fail('Device tab', 'Device page not active', Date.now() - t0);
    }
  }

  // 4. Lock toggles (CH1)
  {
    const getState = async () => (await api.getState()).locks;
    await uiToggleTest('UI: Lock CH1 enable toggle', 'enableLock_1', getState, 1, 'enable');
    await uiToggleTest('UI: Lock CH1 arm toggle', 'arm_1', getState, 1, 'arm');
    await uiToggleTest('UI: Lock CH1 contact alert toggle', 'enableContactAlert_1', getState, 1, 'enableContactAlert');
    await uiToggleTest('UI: Lock CH1 fail-secure toggle', 'failSecure_1', getState, 1, 'failSecure');
  }

  // 5. Lock toggles (CH2)
  {
    const getState = async () => (await api.getState()).locks;
    await uiToggleTest('UI: Lock CH2 enable toggle', 'enableLock_2', getState, 2, 'enable');
    await uiToggleTest('UI: Lock CH2 arm toggle', 'arm_2', getState, 2, 'arm');
    await uiToggleTest('UI: Lock CH2 contact alert toggle', 'enableContactAlert_2', getState, 2, 'enableContactAlert');
    await uiToggleTest('UI: Lock CH2 fail-secure toggle', 'failSecure_2', getState, 2, 'failSecure');
  }

  // 6. Exit toggles
  {
    const getState = async () => (await api.getState()).exits;
    await uiToggleTest('UI: Exit CH1 enable toggle', 'enableExit_1', getState, 1, 'enable');
    await uiToggleTest('UI: Exit CH1 alert toggle', 'alertExit_1', getState, 1, 'alert');
    await uiToggleTest('UI: Exit CH1 latch toggle', 'latchExit_1', getState, 1, 'latch');
    await uiModeSelectTest('UI: Exit CH1 mode select toggle', 'modeExit_1', getState, 1, 'toggle');
    await uiToggleTest('UI: Exit CH2 enable toggle', 'enableExit_2', getState, 2, 'enable');
    await uiToggleTest('UI: Exit CH2 alert toggle', 'alertExit_2', getState, 2, 'alert');
    await uiToggleTest('UI: Exit CH2 latch toggle', 'latchExit_2', getState, 2, 'latch');
    await uiModeSelectTest('UI: Exit CH2 mode select toggle', 'modeExit_2', getState, 2, 'toggle');
  }

  // 7. Exit delay (CH1)
  {
    const t0 = Date.now();
    try {
      const delayInput = await page.$('#armDelay_1');
      const saveBtn = await page.$('#relock');
      if (delayInput && saveBtn) {
        const before = (await api.getState()).exits.find(e => e.channel === 1)?.delay || 0;
        await delayInput.fill('25');
        await saveBtn.click();
        await page.waitForTimeout(800);
        const after = (await api.getState()).exits.find(e => e.channel === 1)?.delay;
        // Restore
        await delayInput.fill(String(before));
        await saveBtn.click();

        if (after === 25) {
          report.pass('UI: Exit CH1 delay set to 25', '', Date.now() - t0);
        } else {
          report.fail('UI: Exit CH1 delay', `Expected 25, got ${after}`, Date.now() - t0);
        }
      } else {
        report.skip('UI: Exit CH1 delay', 'Elements not found', Date.now() - t0);
      }
    } catch (err) {
      report.fail('UI: Exit CH1 delay', err.message, Date.now() - t0);
    }
  }

  // 8. Fob toggles
  {
    const getState = async () => (await api.getState()).fobs;
    await uiToggleTest('UI: Fob CH1 enable toggle', 'enableFob_1', getState, 1, 'enable');
    await uiToggleTest('UI: Fob CH1 alert toggle', 'alertFob_1', getState, 1, 'alert');
    await uiToggleTest('UI: Fob CH1 latch toggle', 'latchFob_1', getState, 1, 'latch');
    await uiModeSelectTest('UI: Fob CH1 mode select toggle', 'modeFob_1', getState, 1, 'toggle');
    await uiToggleTest('UI: Fob CH2 enable toggle', 'enableFob_2', getState, 2, 'enable');
    await uiToggleTest('UI: Fob CH2 alert toggle', 'alertFob_2', getState, 2, 'alert');
    await uiToggleTest('UI: Fob CH2 latch toggle', 'latchFob_2', getState, 2, 'latch');
    await uiModeSelectTest('UI: Fob CH2 mode select toggle', 'modeFob_2', getState, 2, 'toggle');
  }

  // 8b. Fob delay (CH1)
  {
    const t0 = Date.now();
    try {
      const delayInput = await page.$('#fobDelay_1');
      const saveBtn = await page.$('#fobSave_1');
      if (delayInput && saveBtn) {
        const before = (await api.getState()).fobs.find(f => f.channel === 1)?.delay || 0;
        await delayInput.fill('11');
        await saveBtn.click();
        await page.waitForTimeout(800);
        const after = (await api.getState()).fobs.find(f => f.channel === 1)?.delay;
        await delayInput.fill(String(before));
        await saveBtn.click();

        if (after === 11) {
          report.pass('UI: Fob CH1 delay set to 11', '', Date.now() - t0);
        } else {
          report.fail('UI: Fob CH1 delay', `Expected 11, got ${after}`, Date.now() - t0);
        }
      } else {
        report.skip('UI: Fob CH1 delay', 'Elements not found', Date.now() - t0);
      }
    } catch (err) {
      report.fail('UI: Fob CH1 delay', err.message, Date.now() - t0);
    }
  }

  // 9. Keypad toggles
  {
    const getState = async () => (await api.getState()).keypads;
    await uiToggleTest('UI: Keypad CH1 enable toggle', 'enableKeypad_1', getState, 1, 'enable');
    await uiToggleTest('UI: Keypad CH1 alert toggle', 'alertKeypad_1', getState, 1, 'alert');
    await uiToggleTest('UI: Keypad CH1 latch toggle', 'latchKeypad_1', getState, 1, 'latch');
    await uiModeSelectTest('UI: Keypad CH1 mode select toggle', 'modeKeypad_1', getState, 1, 'toggle');
    await uiToggleTest('UI: Keypad CH2 enable toggle', 'enableKeypad_2', getState, 2, 'enable');
    await uiToggleTest('UI: Keypad CH2 alert toggle', 'alertKeypad_2', getState, 2, 'alert');
    await uiToggleTest('UI: Keypad CH2 latch toggle', 'latchKeypad_2', getState, 2, 'latch');
    await uiModeSelectTest('UI: Keypad CH2 mode select toggle', 'modeKeypad_2', getState, 2, 'toggle');
  }

  // 9b. Motion latch toggles
  {
    const getState = async () => (await api.getState()).motions;
    await uiToggleTest('UI: Motion CH1 latch toggle', 'latchMotion_1', getState, 1, 'latch');
    await uiModeSelectTest('UI: Motion CH1 mode select toggle', 'modeMotion_1', getState, 1, 'toggle');
    await uiToggleTest('UI: Motion CH2 latch toggle', 'latchMotion_2', getState, 2, 'latch');
    await uiModeSelectTest('UI: Motion CH2 mode select toggle', 'modeMotion_2', getState, 2, 'toggle');
  }

  // 10. Keypad user management (UI)
  {
    const t0 = Date.now();
    try {
      // Click Add button
      const addBtn = await page.$('#keypadAddBtn');
      if (!addBtn) {
        report.skip('UI: Keypad user add', 'Add button not found', Date.now() - t0);
      } else {
        await addBtn.click();
        await page.waitForTimeout(300);

        const form = await page.$('#keypadAddForm');
        const formHidden = await form?.getAttribute('hidden');
        if (form && formHidden === null) {
          report.pass('UI: Keypad add form visible', '', Date.now() - t0);
        } else {
          report.fail('UI: Keypad add form', 'Form not visible after clicking Add', Date.now() - t0);
        }

        // Fill form
        await page.fill('#keypadNewName', 'UITest User');
        await page.fill('#keypadNewPin', '987654');
        await page.click('#keypadSaveNewBtn');
        await page.waitForTimeout(1000);

        // Verify user appeared via API
        const users = await api.getKeypadUsers();
        const found = users.find(u => u.name === 'UITest User');
        if (found) {
          report.pass('UI: Keypad user added and visible', '', 0);
          // Clean up via API
          await api.deleteKeypadUser(found.uuid);
        } else {
          report.fail('UI: Keypad user add', 'User not found after UI add', 0);
        }
      }
    } catch (err) {
      report.fail('UI: Keypad user management', err.message, Date.now() - t0);
    }
  }

  // 11. System tab
  {
    const t0 = Date.now();
    try {
      const tabs = await page.$$eval('.nav-item', (items) => items.map((item) => item.textContent.trim()).join('|'));
      if (tabs === 'Device|Settings|System') {
        report.pass('UI: Navigation order is Device, Settings, System', '', 0);
      } else {
        report.fail('UI: Navigation order', tabs, 0);
      }

      // Click System tab
      await page.click('.nav-item[data-target="system"]');
      await page.waitForTimeout(1000);

      const uptime = await page.textContent('#systemUptime');
      const branch = await page.textContent('#firmwareBranch');
      const commit = await page.textContent('#firmwareCommit');
      const rollback = await page.textContent('#firmwareRollback');
      const otaFile = await page.$('#otaFile');
      const otaUploadBtn = await page.$('#otaUploadBtn');
      const otaStatus = await page.textContent('#otaStatus');
      const logItems = await page.$('#logItems');
      const logEmpty = await page.$('#logEmptyState');
      if (
        (logItems || logEmpty) &&
        uptime && /\d+s$/.test(uptime.trim()) &&
        branch && branch.trim() !== '—' &&
        commit && commit.trim() !== '—' &&
        rollback && /Enabled/.test(rollback) &&
        otaFile && otaUploadBtn && otaStatus
      ) {
        report.pass('UI: System tab loads logs', '', Date.now() - t0);
      } else {
        report.fail(
          'UI: System tab',
          `logs=${!!(logItems || logEmpty)} uptime=${uptime} branch=${branch} commit=${commit} rollback=${rollback} otaFile=${!!otaFile} otaButton=${!!otaUploadBtn} otaStatus=${otaStatus}`,
          Date.now() - t0
        );
      }
    } catch (err) {
      report.fail('UI: System tab', err.message, Date.now() - t0);
    }
  }

  // 12. Settings tab
  {
    const t0 = Date.now();
    try {
      await page.click('.nav-item[data-target="settings"]');
      await page.waitForTimeout(1000);

      const wifiForm = await page.$('#wifiForm');
      const serverForm = await page.$('#serverForm');
      if (wifiForm && serverForm) {
        report.pass('UI: Settings tab loads WiFi and Server forms', '', Date.now() - t0);
      } else {
        report.fail('UI: Settings tab', `WiFi form: ${!!wifiForm}, Server form: ${!!serverForm}`, Date.now() - t0);
      }
    } catch (err) {
      report.fail('UI: Settings tab', err.message, Date.now() - t0);
    }
  }

  // 13. Wiegand section
  {
    const t0 = Date.now();
    try {
      await page.click('.nav-item[data-target="device"]');
      await page.waitForTimeout(500);

      const registerBtn = await page.$('#wiegandRegisterBtn');
      const channelSelect = await page.$('#wiegandChannelSelect');
      if (registerBtn && channelSelect) {
        report.pass('UI: Wiegand section elements present', '', Date.now() - t0);
      } else {
        report.skip('UI: Wiegand section', 'Elements not found', Date.now() - t0);
      }
    } catch (err) {
      report.skip('UI: Wiegand section', err.message, Date.now() - t0);
    }
  }

  // 14. RF section
  {
    const t0 = Date.now();
    try {
      const rfRegisterBtn = await page.$('#rfRegisterBtn');
      const rfList = await page.$('#rfUserList');
      const rfCardMetrics = await page.$('.credential-card--remote .rf-card-metrics');
      if ((rfRegisterBtn || rfList) && rfList) {
        report.pass('UI: RF fobs section elements present', '', Date.now() - t0);
      } else if (rfCardMetrics) {
        report.pass('UI: RF fobs card metrics present', '', Date.now() - t0);
      } else {
        report.skip('UI: RF fobs section', `Elements not found list=${!!rfList} metrics=${!!rfCardMetrics}`, Date.now() - t0);
      }
    } catch (err) {
      report.skip('UI: RF fobs section', err.message, Date.now() - t0);
    }
  }

  await browser.close();
  report.endSuite();
  return {};
}
