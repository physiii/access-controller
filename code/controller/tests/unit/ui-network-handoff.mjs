import assert from 'node:assert/strict';
import { readFile, mkdir } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium } from 'playwright';

const here = dirname(fileURLToPath(import.meta.url));
const assets = resolve(here, '../../main/public');
const artifacts = process.env.ARTIFACT_DIR || resolve(here, '../artifacts/network-ui');
await mkdir(artifacts, { recursive: true });
const browser = await chromium.launch({
  headless: true,
  ...(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {}),
});

try {
  for (const viewport of [{ width: 1440, height: 900 }, { width: 390, height: 844 }]) {
    const page = await browser.newPage({ viewport });
    const errors = [];
    let handoff = false;
    let outageSeen = false;
    let saveCount = 0;
    let stateRequests = 0;
    page.on('pageerror', (error) => errors.push(error.message));
    page.on('console', (message) => {
      // A deliberately failed state request models the controller reboot.
      if (message.type() === 'error' && !message.text().includes('503')) errors.push(message.text());
    });
    await page.route('**/*', async (route) => {
      const url = new URL(route.request().url());
      const path = url.pathname;
      if (url.hostname !== 'controller.invalid') {
        return route.fulfill({ contentType: 'text/html', body: '<title>IP destination fixture</title>' });
      }
      if (path === '/api/state') {
        stateRequests++;
        if (handoff && !outageSeen) {
          outageSeen = true;
          return route.fulfill({ status: 503, body: 'rebooting' });
        }
        const ip = handoff ? '192.0.2.20' : '192.0.2.10';
        return route.fulfill({ json: {
          device: { uuid: 'fixture', network: { wifi_ap_ip: '192.168.4.1', wifi_sta_ip: ip } },
          wifi: { networks: [] }, locks: [], exits: [], fobs: [], motions: [], keypads: [],
          system: {},
        } });
      }
      if (path === '/api/wifi/add') {
        assert.deepEqual(route.request().postDataJSON(), { ssid: 'Fixture LAN', password: 'fixture-only' });
        saveCount++;
        handoff = true;
        return route.fulfill({ json: { ok: true } });
      }
      if (path.startsWith('/api/')) return route.fulfill({ json: { networks: [], scanned: [] } });
      const name = path === '/' ? 'index.html' : path.slice(1);
      const types = { 'index.html': 'text/html', 'script.js': 'text/javascript', 'style.css': 'text/css' };
      if (types[name]) return route.fulfill({ contentType: types[name], body: await readFile(resolve(assets, name)) });
      return route.fulfill({ status: 200, body: '' });
    });
    await page.goto('http://controller.invalid/');
    await page.waitForFunction(() => document.querySelector('#headerStaLink')?.getAttribute('href') === 'http://192.0.2.10/');
    assert.equal(await page.locator('#headerApLink').getAttribute('href'), 'http://192.168.4.1/');
    for (const name of ['device', 'settings', 'system']) {
      await page.locator(`.nav-item[data-target="${name}"]`).click();
      assert.equal(await page.locator(`#page-${name}`).isVisible(), true);
    }
    await page.locator('.nav-item[data-target="settings"]').click();
    await page.locator('#wifiName').fill('Fixture LAN');
    await page.locator('#wifiPassword').fill('fixture-only');
    await page.locator('#wifiForm button[type="submit"]').click();
    await page.waitForFunction(() => document.querySelector('#headerStaLink')?.getAttribute('aria-disabled') === 'true');
    assert.equal(await page.locator('#headerStaLink').getAttribute('href'), null, 'old STA address must clear on save');
    await page.waitForFunction(() => document.querySelector('#headerStaLink')?.getAttribute('href') === 'http://192.0.2.20/', null, { timeout: 15000 });
    assert.equal(saveCount, 1);
    assert.equal(outageSeen, true, 'exercise a reboot outage before recovery');
    assert.ok(stateRequests >= 3);
    await page.waitForFunction(() => document.querySelector('#toast')?.textContent.includes('STA ready at 192.0.2.20'));
    await page.screenshot({ path: resolve(artifacts, `network-${viewport.width}.png`), fullPage: true });
    await Promise.all([
      page.waitForURL('http://192.0.2.20/'),
      page.locator('#headerStaLink').click(),
    ]);
    assert.equal(await page.title(), 'IP destination fixture');
    assert.deepEqual(errors, []);
    await page.close();
  }
  console.log('ui-network-handoff: desktop/mobile navigation, stale-IP clearing, reboot recovery and IP navigation passed');
} finally {
  await browser.close();
}
