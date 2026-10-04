# Access Controller Test Suite

Comprehensive automated test suite for the ESP32-S3 Access Controller. Covers API endpoints, configuration toggles, bulk user operations, Playwright browser tests, and interactive physical hardware verification.

## Quick Start

```bash
cd code/controller/tests
npm install
npx playwright install chromium
```

## Offline verification

These commands do not contact a device:

| Command | Checks |
|---|---|
| `npm run test:wiegand-format` | Native Wiegand format handling |
| `npm run test:boot-audio` | Startup sampling, timers and quiet mode |
| `npm run test:ui-network` | Desktop/mobile navigation, IP badges and simulated reboot recovery |

Set `CHROMIUM_PATH=/usr/bin/google-chrome` to use installed Chrome.
UI screenshots go to `artifacts/network-ui/` or `$ARTIFACT_DIR`.

## Attached-device usage

The commands below contact a controller and may change lock/input settings.


```bash
npm test              # Full automated suite (API + stress + UI)
npm run test:api      # API state & config only
npm run test:quick    # Smoke test (state + connectivity)
npm run test:ui       # Playwright browser tests only
npm run test:physical # Interactive hardware walkthrough
```

Or with environment variables:
```bash
DEVICE_URL=http://192.168.1.131 npm test
```

## Test Suites

| Suite | File | Tests | Description |
|-------|------|-------|-------------|
| **Build Verification** | `runner.mjs` | 5 | Firmware binaries exist |
| **API State & Monitoring** | `suites/api-state.mjs` | 19 | GET endpoints: state, logs, wiegand, rf, wifi, caching |
| **API Configuration** | `suites/api-config.mjs` | 29 | POST endpoints: lock, exit, fob, keypad toggles + PIN user CRUD + channel isolation |
| **Stress & Bulk** | `suites/stress-bulk.mjs` | 39 | 30-user bulk CRUD, comprehensive toggle sweep with log verification, Wiegand/RF registration flows |
| **Web UI (Playwright)** | `suites/ui-playwright.mjs` | 32 | Every checkbox on both channels, PIN user CRUD, tab navigation, WiFi/Server forms |
| **Physical Hardware** | `suites/physical-hardware.mjs` | 11 | Interactive: exit buttons, contacts, keypads, fobs, RFID scan, motion, buzzer |

## Device Auto-Detection

The runner automatically finds the device:
1. Checks `DEVICE_URL` environment variable
2. Tries `http://192.168.4.1` (AP mode default)
3. Any other reachable IP

Connect to the device's WiFi AP (`ac_xxxx`, password `pyfitech`) or set `DEVICE_URL`.

## Log Verification

The stress-bulk suite verifies that every arm/disarm action produces a corresponding log entry on the controller with the correct source tag (`[api]`, `[exit_auto]`, `[fob_auto]`, etc.).

## Report

After each run, `test-report.html` is generated with:
- Summary cards (total, passed, failed)
- Per-suite tables with test names, results, durations
- System information (UUID, channel counts, user counts)

## Physical Hardware Tests

Requires hardware connected. Each test:
1. Prints a large banner with instructions
2. Waits for you to press Enter (or type `skip`)
3. Verifies state/logs changed

```bash
npm run test:physical
```

## Files

```
tests/
├── runner.mjs              # Main entry point
├── package.json            # npm config
├── lib/
│   ├── api-client.mjs      # Full API client (28 endpoints)
│   ├── config.mjs          # Device URL, timeouts
│   └── report.mjs          # HTML report generator
├── suites/
│   ├── api-state.mjs       # GET state/monitoring tests
│   ├── api-config.mjs      # POST config toggle tests
│   ├── stress-bulk.mjs     # Bulk CRUD + toggle sweep
│   ├── ui-playwright.mjs   # Browser UI tests
│   └── physical-hardware.mjs # Interactive hardware tests
└── test-report.html        # Generated report
```
