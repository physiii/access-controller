# Power and Energy Harvesting

## Status

This document is a design investigation for a battery-powered or energy-harvesting
variant of the access controller. **Nothing in this document is implemented.**

Current repository state, for contrast:

- The active firmware (`code/controller/main/services/station.c:241`) hardcodes
  `esp_wifi_set_ps(WIFI_PS_NONE)` — WiFi power save is explicitly disabled, logged
  as `"WiFi power save disabled for low-latency controller traffic"`.
- `code/controller/sdkconfig` has `# CONFIG_PM_ENABLE is not set` — automatic
  light sleep is not built in.
- The controller target is ESP32-S3 (`CONFIG_IDF_TARGET="esp32s3"` in both
  `code/controller` and `code/controller_mini`).
- There is no battery, energy-harvesting circuit, or 802.15.4/Zigbee radio
  anywhere in `circuits/`. `circuits/controller/power.kicad_sch` is a step-down
  DC-DC regulator fed from PoE/`VIN` — see [Hardware](HARDWARE.md). The board is
  mains/PoE-powered today.

Everything below is calculated from vendor datasheets, ESP-IDF documentation, and
cited third-party sources — not measured on this hardware. Treat figures as
planning estimates with the stated margins, not guarantees.

The motivating scenario throughout: a battery-powered or harvesting-powered
satellite device (e.g. mounted at/near a doorknob) that needs to react to or
report a WiFi/radio event, separate from the existing mains-powered controller.

---

## 1. WiFi Power-Save Architecture for Sub-1-Second Response

Goal: minimize average current on an ESP32-S3 station while guaranteeing it can
receive and react to an inbound event within 1 second.

### Why not deep sleep

Deep sleep fully de-associates from the AP. Waking on a fixed short interval (e.g. every ~900ms) to reconnect, check for work, and sleep again is **worse** than staying associated: reconnect (scan/auth/assoc, ~150–300ms+ at ~150mA+) paid every cycle costs far more than briefly waking to check a TIM bit while still associated.

Rough estimate: deep-sleep-and-reconnect every 900ms lands around 30+ mA average — roughly 15–30x worse than the modem-sleep approach below. **Stay associated; don't deep-sleep-and-reconnect.**

### Listen interval math

Beacon interval is 100 TU = 102.4ms (1 TU = 1024µs). Station `listen_interval`
(`wifi_config_t.sta.listen_interval`, set via `esp_wifi_set_config`) is a
multiplier of that:

- `listen_interval = 10` → wakes every **1024ms** — fails a strict 1-second
  budget by 24ms.
- `listen_interval = 9` → wakes every **921.6ms** — under budget with ~78ms
  margin. **Use 9, not 10.**

This governs unicast delivery latency (`WIFI_PS_MAX_MODEM`). Broadcast/multicast
delivery is instead gated by the AP's own DTIM period, which is a router-side
setting.

### Current draw (ESP32-S3, official Espressif figures, 160MHz + DFS)

| Mode | DTIM 1 | DTIM 3 | DTIM 10 (~listen_interval 9–10) |
|---|---|---|---|
| Min Modem-sleep only | 20.7 mA | 19.9 mA | 19.5 mA |
| **+ Auto Light-sleep** | 2.45 mA | 1.33 mA | **~0.9–1.0 mA** |

Working figures used throughout this document:

- **Ideal** (bare chip, datasheet conditions): **~1mA** average.
- **Practical** (real board — antenna/PCB losses, LDO quiescent draw, occasional
  event handling): **~2mA** average, used as the primary design figure.

Peak current: ESP32-S3 datasheet TX is ~310mA; real boards report spikes to
400–600mA. This does not change the average-current math but does constrain
regulator/battery choice (see §2 and §3).

### What implementing this requires

- `code/controller/sdkconfig`: enable `CONFIG_PM_ENABLE=y`.
- Firmware: `esp_pm_configure(.light_sleep_enable = true, .max_freq_mhz = 160, ...)`.
- `station.c:241`: replace `esp_wifi_set_ps(WIFI_PS_NONE)` with
  `esp_wifi_set_ps(WIFI_PS_MAX_MODEM)` and set `wifi_config.sta.listen_interval = 9`
  before connecting.
- Regulator: use a **nanopower** buck/LDO (e.g. TPS7A02-class, ~25nA Iq, or
  TPS62840-class buck, ~60nA Iq). A generic regulator idling at 50–100µA would
  eat 5–10% of the entire ~1mA budget by itself.

This is scoped to a hypothetical battery-powered variant. `WIFI_PS_NONE` is
presumably correct as-is for the existing mains/PoE-powered controller, where
minimum latency at zero power cost is free.

---

## 2. Battery Sizing — 1 Year on a Single Charge

Target: WiFi-only baseline (no additional sensors — see formula at the end of
this section for extending the budget), rechargeable, must run 1 year without
recharging.

### Energy budget methodology

Load over 1 year (8760h) at the §1 current figures:

| | Ideal (1mA) | Practical (2mA) |
|---|---|---|
| Load over 1 year | 8,760 mAh | 17,520 mAh |

A single-charge, 1-year design must also budget for two things a short-runtime
design can ignore:

1. **Self-discharge.** Li-ion 18650s run ~1–2%/month at room temperature
   (Battery University), worse if hot. Design assumption: **~20%/year**.
2. **Usable capacity reserve.** Don't discharge to 0V; leave margin for
   regulator dropout near end-of-discharge. Design assumption: **~90% usable**.

Combined: only **~72%** of nameplate capacity is available to the load over a
year unattended. Required pack capacity = Load ÷ 0.72:

| | Ideal | Practical |
|---|---|---|
| Required pack capacity | 12,167 mAh | **24,333 mAh** |

### 18650 cell count

Genuine 18650s top out ~3,500–3,600mAh (Samsung 36G, LG M36); the common
reliable grade (Samsung 30Q, Panasonic NCR18650B) is ~3,000mAh. **Anything
listed above ~4,000mAh is fake** — no current 18650 chemistry reaches it.

| Scenario | Cell grade | Cells (parallel, 1S-NP) | Pack capacity | Pack energy |
|---|---|---|---|---|
| Practical, **recommended** | Premium 3,500mAh | **7** | 24,500mAh | ~91 Wh |
| Practical | Common 3,000mAh | **9** | 27,000mAh | ~100 Wh |
| Ideal (datasheet floor) | Premium 3,500mAh | 4 | 14,000mAh | ~52 Wh |
| Ideal (datasheet floor) | Common 3,000mAh | 5 | 15,000mAh | ~56 Wh |

**100Wh note:** the 9-cell/common-grade row lands almost exactly on 100Wh, the
standard threshold above which lithium packs need special handling for
shipping (UN38.3, airline carriage rules). The 7-cell premium-cell option
(~91Wh) meets the same 1-year target with margin under that line — prefer it
over assuming cheap 3,000mAh cells will be available.

**Working recommendation: 7× 18650 in parallel (premium cells), single 3.7V
nominal group.**

### Regulator implication

A single series (1S) Li-ion group droops from 4.2V (full) to ~3.0V (empty)
over the discharge. Holding a stable 3.3V rail through the whole year requires
a **buck-boost** converter, not a buck-only design — or accepting a higher
cutoff voltage (already partially reflected in the 10% usable-capacity reserve
above).

### Extending the budget for sensors

The battery-only figures above are explicitly a floor — additional sensors
will add continuous or duty-cycled current. Reuse the same math:

```
extra_mAh_per_year = sensor_avg_mA × 8760
extra_pack_mAh     = extra_mAh_per_year / 0.72
extra_cells        = extra_pack_mAh / cell_capacity_mAh
```

An always-on 1mA sensor roughly doubles the whole budget; something mostly
idle (PIR, reed switch) barely moves it.

---

## 3. Doorknob Energy Harvesting (Battery-Dead Fallback)

Goal: if the battery above is depleted, harvest energy from manually twisting a
doorknob-mounted mechanism to send one "I'm alive, battery's dead" message,
using less than one full turn.

### Physics correction: gearing does not create energy

Available mechanical energy is **E = torque applied × angle rotated**, full
stop — set by human biomechanics, not by gear ratio. Gearing conserves energy
(minus friction losses) and reshapes torque↔speed to match a small generator's
efficient operating RPM. It's a real and necessary efficiency lever, but it is
not a way to extract more energy than was put in.

### Recommended architecture

1. **Decouple a free-spinning collar/dial from the latch mechanism** — don't
   rely on the latch's natural ~50° throw. A separate free-spin collar allows
   much more angle without affecting normal door operation.
2. **Step-up gear train** (~20–50:1, 3–4 stages) into a small brushed DC
   gearmotor run as a generator, spun into the thousands-of-RPM range where it
   generates a few usable volts directly.
3. **Full-bridge rectifier** (handles either twist direction) → **BQ25570-class
   nano-power boost/harvesting IC** (TI; cold-starts ~330–600mV, harvests down
   to 100mV, 488nA quiescent) → charges a **coin-type supercapacitor
   (0.1–1F, 5.5V)**.
4. **ESP-NOW (or 802.15.4, see §4) to a nearby always-powered hub** — not a
   full WiFi/AP join. The existing mains/PoE-powered controller is a natural
   always-on peer at the same doorway.

### Energy cost of sending a message (cold boot from a dead battery)

| Path | What happens | Time | Energy |
|---|---|---|---|
| **ESP-NOW → local hub** | Boot + WiFi PHY init (no scan/auth/DHCP) + send | ~100–150ms | **~0.1 J** |
| **Full WiFi STA join → publish**, optimized (static IP, cached channel/BSSID, IRAM opt, minimal logging) | Boot + connect + publish | ~0.5–1s | **~0.3–0.5 J** |
| **Full WiFi STA join → publish**, unoptimized/default | Boot + scan + auth + DHCP + publish | ~3s (commonly reported) | **~1.5 J** |

The AP-join-vs-local-hub choice is a 4–15x energy swing — the single biggest
lever in this whole design.

### Mechanical energy available

**Angle:**
- Natural latch throw only (no mechanism change): ~50° (0.87 rad)
- Dedicated free-spin collar: ~300° / 0.83 turn (5.24 rad) — under one turn

**Torque** (grounded in ANSI/BHMA hardware accessibility limits and hand/wrist
ergonomics literature):
- **Gentle/accessible**: 1.5 N·m — comparable to ergonomics-study recommended
  jar-lid-opening torque (~1.0–2.0 N·m) for the general population including
  reduced grip strength.
- **Normal firm**: 3 N·m — matches ANSI/BHMA A156.2's 28 in-lb (3.16 N·m) limit,
  i.e. the maximum torque door code allows a lever handle to require.
- **Hard deliberate**: 8 N·m — approaching clinical max voluntary wrist
  pronation/supination torque (~8–15 N·m); a real "wringing" effort, not a
  normal-case design target.

**Conversion efficiency** (gear friction × generator conversion × rectifier/PMIC
efficiency, stacked): pessimistic ~20%, **typical ~40%**, optimistic ~60%.
(Sanity check: commercial small hand-crank harvesters report 0.3–1.9W
sustained, consistent with these figures over a few seconds of cranking.)

### Scenario matrix (stored energy at typical 40% efficiency)

| Mechanism | Twist effort | Mech. energy in | Stored (40%) | Active radio time | ESP-NOW msgs | WiFi-opt msgs | WiFi-naive msgs |
|---|---|---|---|---|---|---|---|
| Natural throw (50°) | Gentle (1.5N·m) | 1.31 J | 0.52 J | ~1.1s | **5** | 1 | 0 |
| Natural throw (50°) | Normal (3N·m) | 2.61 J | 1.04 J | ~2.1s | **10** | 2 | 0 |
| Natural throw (50°) | Hard (8N·m) | 6.96 J | 2.78 J | ~5.6s | **27** | 6 | 1 |
| Free-spin collar (300°) | Gentle (1.5N·m) | 7.86 J | 3.14 J | ~6.3s | **31** | 7 | 2 |
| Free-spin collar (300°) | Normal (3N·m) | 15.7 J | 6.29 J | ~12.7s | **62** | 15 | 4 |
| Free-spin collar (300°) | Hard (8N·m) | 41.9 J | 16.8 J | ~34s | **167** | 41 | 11 |

Takeaways:

- Even the laziest twist (natural 50° throw, gentle 1.5N·m) yields 5 ESP-NOW
  messages — enough for a message plus a retry/ACK handshake.
- A dedicated free-spin collar turns "barely works" into comfortable margin:
  even gentle effort gets 7 optimized-WiFi messages; a code-legal "normal"
  twist (3N·m) gets 15.
- The ESP-NOW-vs-full-AP-join choice matters more than any torque or gearing
  decision.
- Budget exists for a brief RX listen window for a delivery ACK (~50–100ms,
  ~0.03–0.05J) — worth adding given this is a security-relevant "I'm dead"
  signal; every scenario above absorbs that cost easily.

### Practical notes

- **Supercap, not battery, as the buffer.** ESR of a small coin supercap
  (~0.05Ω for a 1F/5.5V cell) means even a 300–500mA WiFi TX pulse only sags
  ~15–25mV — a battery this small couldn't source that pulse at all.
- **No standby shelf-life concern.** The supercap only holds charge for the
  seconds between twist and transmit; it doesn't sit charged for months like
  the battery pack in §2 does, so self-discharge is irrelevant here.
- **Cold tolerance.** Supercaps are commonly rated to -40°C, more cold-tolerant
  than the Li-ion pack this mechanism is backing up — relevant for an exterior
  door.
- **Fully repeatable.** Nothing here is consumed permanently; every twist
  regenerates the same budget, so a "twist again if it didn't work" retry UX
  is free.

---

## 4. Radio Protocol Choice: ESP-NOW vs. Zigbee (802.15.4)

Zigbee/802.15.4 is lower energy per message than ESP-NOW by roughly 3–10x, but
requires different silicon — it is not a firmware-only switch.

### Current draw comparison

| | ESP32-S3 (WiFi) | ESP32-H2 (802.15.4/Zigbee) |
|---|---|---|
| TX, typical/default power | ~120–200mA active, up to **310mA peak** (20dBm) | **36mA** @ 0dBm (802.15.4 standard output); 140mA only if pushed to 20dBm |
| RX | ~95–130mA | **25mA** |
| Light sleep | ~1–2.5mA (with WiFi PS, from §1) | 25–85µA |
| Deep sleep | ~7µA | 7µA |

Root cause: WiFi's 20MHz-wide OFDM/QAM signal needs a bigger, more linear, more
power-hungry PA than 802.15.4's narrow 2MHz O-QPSK signal — a PHY-level
difference, not primarily a protocol-overhead one. Figures are from the
official ESP32-H2 datasheet (Table 5-7, RF Current Consumption in Active Mode).

### Per-message energy budget

| Path | Analogous to | Energy per message |
|---|---|---|
| Raw 802.15.4 send, no network join | ESP-NOW | **~15–30mJ** |
| ESP-NOW (WiFi) | — | ~100mJ |
| Zigbee rejoin (cached PAN ID/channel/network key) + send | Optimized WiFi STA join | **~25–70mJ** typical, up to ~150–250mJ |
| Full WiFi STA join + publish, optimized | — | ~300–500mJ |
| Full WiFi STA join + publish, naive | — | ~1,500mJ |

Zigbee's join is lighter than WiFi's beyond the raw PHY current difference too:
pre-shared AES-128 network key (no WPA2 4-way handshake), no DHCP-equivalent.

Concrete impact on the worst-case §3 scenario (natural 50° throw, gentle
1.5N·m twist, 0.52J stored): 5 ESP-NOW messages becomes **~20–30 messages** on
raw 802.15.4 — or the harvester itself (gear train, torque requirement) could
be scaled down for the same reliability target.

### The hardware trade-off

- **Harvesting satellite device**: needs **ESP32-H2** specifically (not C6).
  ESP32-S3 has no 802.15.4 radio. H2 has no WiFi radio at all — less RF
  silicon, lower baseline current, purpose-built by Espressif for
  coin-cell/harvested Zigbee-Thread-Matter end devices. C6 bundles WiFi 6 +
  802.15.4, reintroducing the power/cost this path is trying to avoid.
- **Hub side**: the existing mains-powered controller doesn't speak 802.15.4.
  Needs an ESP32-H2/C6 co-processor (UART/SPI, a few dollars of BOM) running a
  Zigbee coordinator stack. ESP-NOW's hub side is a firmware-only change to
  radio hardware already present.

**Net: Zigbee wins decisively on energy-per-twist; ESP-NOW wins on zero new
hardware.** Given how much margin even the worst-case §3 scenario has on
ESP-NOW (5 messages minimum), the Zigbee path is the right call specifically
if the goal is shrinking the harvester mechanism itself (smaller generator,
less torque, less mechanism) rather than banking extra message margin.

### Bonus option: BLE advertising

ESP32-H2/C6 share the same RF front end for BLE and 802.15.4, so BLE TX
current matches the 802.15.4 figures above. A BLE advertising beacon
(broadcast-only, no pairing/connection) skips even the lightweight Zigbee
rejoin — "wake, broadcast one packet, done." Worth considering if Zigbee's
mesh/network-key model isn't needed and the goal is the cheapest possible
"I'm alive" chirp for a hub to catch.

---

## Open Questions / Next Steps

- Target runtime (§2) is fixed at 1 year single-charge; sensor load to be
  added to that budget is not yet defined (formula provided in §2).
- Final twist torque/angle for the doorknob mechanism (§3) is a mechanical
  design choice, not yet made — the scenario matrix is meant to inform that
  choice, not replace it.
- ESP-NOW vs. Zigbee (§4) is not yet decided — depends on whether minimizing
  new hardware/BOM or minimizing harvested-energy requirement is the priority.
- None of this has a KiCad representation yet. If a prototype direction is
  chosen, it belongs under a new sheet in `circuits/controller` or as a new
  board, per [Hardware](HARDWARE.md) conventions.
- If this moves from investigation to implementation, it's a candidate for a
  formal spec (`spec-generator`) rather than continued ad-hoc documentation
  here.

---

## Sources

**§1 — WiFi power-save:**
- [Introduction to Low Power Mode in Wi-Fi Scenarios — ESP32-S3 (ESP-IDF docs)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/low-power-mode/low-power-mode-wifi.html)
- [Sleep Modes — ESP32-S3 (ESP-IDF docs)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/sleep_modes.html)
- [ESP32-S3 Series Datasheet (Espressif)](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [ESP32 WiFi and Low Power Modes — MetaShunt (Hackaday.io)](https://hackaday.io/project/193628-metashunt-high-dynamic-range-current-measurement/log/225599-example-esp32-wifi-and-low-power-modes)
- [How to Profile ESP32 Power Consumption Accurately (Hubble Network)](https://hubble.com/community/guides/how-to-profile-esp32-power-consumption-accurately/)

**§2 — Battery sizing:**
- [Battery University BU-802b: What does Elevated Self-discharge Do?](https://www.batteryuniversity.com/article/bu-802b-what-does-elevated-self-discharge-do/)
- [Li-ion Battery Self Discharge Rate — UFine Battery](https://www.ufinebattery.com/blog/what-is-the-self-discharge-rate-of-li-ion-battery/)
- [18650 Maximum Capacity 2026 — UFine Battery](https://www.ufinebattery.com/blog/the-18650-maximum-capacity/)
- [18650 Fake vs Genuine Battery — Zbotic](https://zbotic.in/18650-fake-vs-genuine-battery-how-to-identify-test/)

**§3 — Doorknob energy harvesting:**
- [ANSI/BHMA A156.2 lever hardware operating force — I Dig Hardware](https://idighardware.com/2014/03/decoded-operational-force-for-door-hardware/)
- [Mismatch between jar opening demands and wrist torque strength — ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0003687021000685)
- [Multi-digit maximum voluntary torque production on a circular object — PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC2821118/)
- [BQ25570 nano-power boost charger/buck converter datasheet — TI](https://www.ti.com/lit/ds/symlink/bq25570.pdf)
- [Hand Crank Energy Harvesting for Small Projects — Tindie Blog](https://blog.tindie.com/2024/01/hand-crank-energy-harvesting-for-small-projects/)
- [Energy Harvesting from Upper-Limb Pulling Motions — PMC](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4541857/)
- [ESP-NOW Protocol latency — Zbotic](https://zbotic.in/esp-now-protocol-ultra-fast-peer-to-peer-esp32-communication/)
- [Benchmarking Latency Across Common Wireless Links for MCUs — Hackaday](https://hackaday.com/2024/02/11/benchmarking-latency-across-common-wireless-links-for-mcus/)
- [5.5V Coin Cell Supercapacitor datasheet — Abracon](https://abracon.com/datasheets/ADCH-S05R5S.pdf)

**§4 — ESP-NOW vs. Zigbee:**
- [ESP32-H2 Series Datasheet — Espressif](https://documentation.espressif.com/esp32-h2_datasheet_en.html)
- [ESP32-C6 Series Datasheet — Espressif](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [Zigbee Introduction — Network Activities (Form, Join, Rejoin, Leave) — Silicon Labs](https://github.com/SiliconLabsSoftware/zigbee_applications/blob/master/zigbee_concepts/Zigbee-Introduction/Zigbee%20Introduction%20-%20Network%20Activities%20(Form,%20Join,%20Rejoin,%20Leave).md)
- [Deep sleep without rejoin using RTC FAST Memory — espressif/esp-zigbee-sdk#501](https://github.com/espressif/esp-zigbee-sdk/issues/501)
