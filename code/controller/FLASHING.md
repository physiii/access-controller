# Deploy / OTA / USB — Access Controller firmware

Authoritative reference for getting firmware onto the controller. **Day-to-day updates
need no buttons and no USB — use OTA.** The USB/serial paths are recovery only.

Related docs: `docs/CLOUD_TUNNEL.md` (tunnel + Device Manager proxy internals) ·
`docs/CONTROLLER_DEPLOY_AND_TEST.md` (full provisioning/test runbook).

## Which method do I use?

| Situation | Method | One-liner |
|---|---|---|
| Controller on the **same LAN** as you | LAN OTA — [§2a](#2a-lan-ota-normal-path-) | `python tools/ota_client.py --host <ip> --yes` |
| Controller on a **remote / NAT'd** network, you have Device Manager | DM tunnel OTA — [§2b](#2b-off-lan-ota-through-device-manager) | `curl -X POST http://<dm-host>:8102/api/devices/<id>/access-controller/ota -H 'x-firmware-filename: controller.bin' --data-binary @build/controller.bin` |
| **In the field, standing next to the device**, no infrastructure | AP-mode OTA — [§2c](#2c-ap-mode-ota-in-the-field-no-infrastructure) | join `ac_*` / `pyfitech`, `curl -X POST http://192.168.4.1/api/ota/upload …` |
| Device silent / off-network after a serial flash | Buttonless reset — [§3a](#3a-buttonless-reset-to-the-app) | `./reset_to_app.sh` |
| **Bricked / unreachable**, OTA impossible | Serial flash (needs PROG button) — [§3b](#3b-serial-flash--recovery-only-requires-the-prog-button) | `ESP_PORT=/dev/ttyUSB0 ./flash_now.sh` |

All OTA methods POST `build/controller.bin` to the device's `POST /api/ota/upload`. The
device writes the **inactive** slot (`app0`/`app1`), verifies it, reboots into it, and marks
it valid ~10 s after a clean startup. Rollback is enabled, so a bad image reverts on its own.

---

## 1. Build

```bash
cd code/controller
source ~/esp/esp-idf/export.sh
idf.py build                 # -> build/controller.bin (~1.5 MB)
```

- Toolchain: **ESP-IDF v6.0**. This repo is validated against esp-idf commit `3cc59d2c`.
- First-time setup of ESP-IDF: see [§4 Field-laptop setup](#4-field-laptop-setup-from-scratch).

The build stamps a git-describe version (e.g. `341c98f`, or `341c98f-dirty` with uncommitted
changes) into the app descriptor. That string is how you confirm an OTA landed — see [§5](#5-verify-an-ota-landed).

---

## 2. OTA (buttonless, no USB)

### 2a. LAN OTA (normal path) ✅

The device just has to be reachable on the LAN.

```bash
cd code/controller && source ~/esp/esp-idf/export.sh
idf.py build
python tools/ota_client.py --host <device-ip> --yes
```

`ota_client.py` uploads `build/controller.bin` (its default) to `POST /api/ota/upload`.
Omit `--host` to scan the LAN and list any controllers it finds. This is the method to use every time it applies.

### 2b. Off-LAN OTA through Device Manager

When the controller is on a NAT'd / remote network (e.g. `ec3409`) and not LAN-reachable, OTA
still works buttonlessly **through the Device Manager tunnel proxy** — the DM streams the image
to the device over the reverse tunnel. Run this on the DM host (or any host that can reach it):

```bash
idf.py build                                    # build/controller.bin
DMID=f7f34b9f-7ee3-5519-a8db-f703581931c0       # the device's Device Manager id
curl -X POST http://<dm-host>:8102/api/devices/$DMID/access-controller/ota \
  -H 'x-firmware-filename: controller.bin' \
  --data-binary @build/controller.bin
```

Send **no `Origin` header** (that passes the DM's control-origin check). The device logs a
"streamed tunnel OTA to partition app0/app1", marks the image valid after a clean boot, and
reboots. Full path + ops: `docs/CLOUD_TUNNEL.md` → *Device control tunnel & Device Manager HTTP proxy*.

### 2c. AP-mode OTA (in the field, no infrastructure)

The controller always runs its own SoftAP, so a laptop can update it with nothing but Wi-Fi range.

- **SSID:** `ac_<last 3 bytes of the SoftAP MAC>` — e.g. `ac_cb9aa1`
- **Password:** `pyfitech` (WPA2) — hardcoded in `main/main.c` (`ap_main(ap_ssid, "pyfitech")`)
- **Address:** the controller is `192.168.4.1`; it hands out `192.168.4.x` via DHCP

Join the AP, then upload straight to it (the web server binds all interfaces, so the full API
+ OTA are served on the AP exactly as on the LAN):

```bash
# join the controller's AP (example with NetworkManager on a laptop)
nmcli device wifi connect ac_cb9aa1 password pyfitech ifname <wlan-iface>
# the AP has no internet — keep it off the default route so your other traffic is unaffected:
nmcli connection modify ac_cb9aa1 ipv4.never-default yes ipv6.never-default yes
nmcli connection up ac_cb9aa1

curl -X POST http://192.168.4.1/api/ota/upload \
  -H 'Content-Type: application/octet-stream' \
  -H 'X-Firmware-Filename: controller.bin' \
  --data-binary @build/controller.bin
```

Notes:
- A laptop with a **single Wi-Fi NIC leaves the LAN** while it is joined to the AP (it comes back
  when you reconnect to your normal network). If you are driving this over SSH, run the join →
  upload → reconnect as one detached script so it survives the network switch.
- The AP is only in range within a few meters of the controller (small ESP32 radio). If a scan
  doesn't show `ac_*`, move closer.

---

## 3. USB / serial (recovery only)

> **Serial-port gotcha:** the controller (ESP32-S3) enumerates as the **CP210x on
> `/dev/ttyUSB0`**. A `/dev/ttyACM0` on the same host is a *different* board (an ESP32-C6 dev
> unit), not the controller — don't flash or reset it by mistake.

### 3a. Buttonless reset to the app

The USB-serial **RTS line is wired to EN (reset)**, so the board can be rebooted into the running
app over USB with no button press:

```bash
code/controller/reset_to_app.sh        # ESP_PORT=/dev/ttyUSB0 by default
```

It pulses EN via RTS while holding DTR low (GPIO0 high → boots the app, not ROM download). Use it
to recover a chip left sitting in download mode (silent, off the network) after a serial flash.

### 3b. Serial flash — recovery only (requires the PROG button)

Only needed when the device is bricked/unreachable and OTA is impossible. **This board's
USB-serial DTR line is *not* wired to GPIO0**, so esptool cannot pull the chip into download mode
on its own — entering download mode requires the physical **PROG** button. (Reset/EN *is*
automated via RTS; only download-mode entry needs the button.)

```bash
cd code/controller && source ~/esp/esp-idf/export.sh
# Arm the flasher, then on the board: hold PROG -> tap REBOOT -> release PROG
ESP_PORT=/dev/ttyUSB0 ./flash_now.sh
```

`flash_now.sh` builds first, then runs esptool's connect sequence (the "Connecting…" dots are
real retries — press the buttons during them). After a successful flash the chip may sit in
download mode; run [§3a `reset_to_app.sh`](#3a-buttonless-reset-to-the-app) to boot the app.

> Future hardware revision: wire USB-serial **DTR → GPIO0** (standard two-transistor auto-reset
> network) alongside the existing **RTS → EN** to make serial flashing fully buttonless. Until
> then, keep the device reachable so OTA is always available and the PROG button is never needed.

---

## 4. Field-laptop setup (from scratch)

To build + AP-OTA from a laptop you carry to the device:

```bash
# 1. clone
mkdir -p ~/projects && cd ~/projects
git clone git@github.com:physiii/access-controller.git

# 2. ESP-IDF v6.0 (match commit 3cc59d2c)
mkdir -p ~/esp && cd ~/esp
git clone https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout 3cc59d2c && git submodule update --init --recursive
./install.sh esp32s3

# 3. build
source ~/esp/esp-idf/export.sh
cd ~/projects/access-controller/code/controller
idf.py build

# 4. upload over the AP  (see §2c)
```

`install.sh esp32s3` pulls only the ESP32-S3 toolchain. `1.6 T`-ish free is plenty; the toolchain
is ~8 GB installed under `~/.espressif`. (Alternative: rsync a working machine's `~/esp/esp-idf`
+ `~/.espressif` — the Python venv relocates cleanly if both machines use the same `$HOME` path.)

### Gotchas hit on a real bring-up

The clean flow above hides three things that bit a from-scratch build on the asus field laptop:

- **`cmake` / `ninja` missing and `apt` can't install them** (the laptop runs an EOL Ubuntu whose
  repos are gone → `Package 'cmake' has no installation candidate`). Install them into the ESP-IDF
  Python env instead — `export.sh` puts it on `PATH`:
  ```bash
  source ~/esp/esp-idf/export.sh && pip install cmake ninja
  ```
- **`EXTRA_COMPONENT_DIRS doesn't exist: .../esp-protocols/...`** — `CMakeLists.txt` references
  `~/esp/esp-protocols/components/esp_websocket_client` by absolute path, so that repo must exist at
  `~/esp/esp-protocols` (`git clone https://github.com/espressif/esp-protocols.git ~/esp/esp-protocols`,
  or rsync it for an exact version match).
- **`httpd_ws_frame_t undeclared`** — needs `CONFIG_HTTPD_WS_SUPPORT=y` (in `sdkconfig.defaults` as of
  `eba517a`). If a stale generated `sdkconfig` predates that: `rm sdkconfig && idf.py build`.

### Driving the AP-OTA over SSH (single-NIC laptop)

Running the OTA *on* the laptop interactively is just §2c. Driving it *over SSH* from another machine
is fiddlier — the laptop drops the LAN (and your SSH) the instant it joins the AP, and a detached SSH
session lacks the polkit auth to create a Wi-Fi connection (`Not authorized to control networking`).
The `ap_ota.sh` staged on asus handles it end to end: run it **detached** and it (a) `nmcli device
disconnect`s first to free the single radio for a clean 2.4 GHz scan (the controller's AP beacons are
sparse in AP+STA mode, so a radio camped on 5 GHz LAN won't see them), (b) uses **`sudo nmcli`** to
authorize the join, (c) OTAs, then (d) restores the LAN (240 s watchdog as backstop). Verify from a
third machine or after the laptop reconnects. Validated 2026-07-25: `7be4c43-dirty`/app0 → `eba517a`/app1.

---

## 5. Verify an OTA landed

After any OTA, confirm three things flipped:

1. **Version string changed** — e.g. `7be4c43-dirty` → `341c98f`.
2. **`runningPartition` flipped** — `app0` ↔ `app1`.
3. **Log line** — `OTA image marked valid after successful startup` (permanent, no rollback).

Read the live state through whichever path you used:

```bash
# over the AP
curl -s http://192.168.4.1/api/state | jq '.system.firmware.projectVersion, .system.firmware.runningPartition.label'

# through Device Manager
curl -s http://<dm-host>:8102/api/devices/<DMID>/access-controller/state \
  | jq '.state.system.firmware.projectVersion, .state.system.firmware.runningPartition.label, .state.system.uptimeSeconds'
```

A fresh low `uptimeSeconds` plus the new version string on the other partition = the upload took,
the device came back up, and the commit changed.
