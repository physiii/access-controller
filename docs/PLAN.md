# Access Controller Modernization Plan

## Objectives
- Add an interactive Wiegand interface in the device UI that supports live RFID tag enrollment (`Register`/`Stop`) and renaming users, backed by ESP32 firmware.
- Provide a dedicated keypad management section under the Wiegand area.
- Refresh the hosted web application with a Material Design inspired layout including new `Device`, `System`, and `Settings` views.
- Ensure end-to-end wiring between the ESP32 services and the refreshed UI so that stored Wiegand credentials are validated against authorised codes.

## Current State (as observed)
- The ESP32-S3 firmware under `code/controller/main` serves a single-page UI from `public/index.html`, with lock, exit, FOB, WiFi, and server settings exposed through `/api/*` routes defined in `services/api.c`.
- Wiegand keypad handling exists in `services/wiegand.c` but only arms/disarms channels based on keypad input; there is no enrollment state machine or persisted user metadata.
- User storage helpers in `services/store.c` are mostly stubs (`load_user_from_flash`, `store_user_to_flash`, etc.), meaning renaming and RFID persistence will need full implementations.
- The UI is table-based with custom CSS; there is no navigation framework or Material-style components.

## Work Plan

### 1. Establish Baseline
- Build and flash the current firmware (`/home/andy/devices/access-controller/code/controller`) to confirm existing endpoints and UI behaviour.
- Capture current REST contract for `/api/state`, `/api/lock`, `/api/exit`, `/api/fob`, `/api/wifi`, and `/api/server` for regression comparison after refactor.
- Document available FreeRTOS tasks and queues relevant to Wiegand (`wiegand_main`, `addServiceMessageToQueue`) to avoid conflict with new logic.

### 2. Design Data Structures for Wiegand Enrollment
- Define an internal representation for enrolled RFID/tag users (UUID/code, display name, channel, timestamps, enrolment status).
- Specify flash/NVS storage keys and maximum counts; decide on JSON vs binary storage using the existing `store_*` helpers.
- Plan a lightweight in-memory cache that mirrors persisted entries for quick lookup during access checks.

### 3. Implement Flash Persistence Utilities
- Replace stub functions in `services/store.c` with real NVS-backed implementations for:
  - `store_user_to_flash`, `load_user_from_flash`, `modify_user_from_flash`, `delete_user_from_flash`.
  - `find_pin_in_flash` (and equivalent lookups for RFID codes if stored separately).
- Add helper routines to list all enrolled users and to synchronise cache ↔ flash on boot.
- Write unit-style tests (hosted or on-target) where feasible to validate serialization/deserialization logic.

### 4. Extend Wiegand Service Behaviour
- Introduce a registration state machine in `services/wiegand.c`:
  - `Register` command arms capture mode, accumulates distinct tag IDs, labels them sequentially (`User 1`, `User 2`, …), and surfaces events via a queue or observer.
  - `Stop` command exits capture mode, freezes the list, and re-enables normal authorisation checks.
- Ensure captured codes are immediately validated against persisted authorised codes when `Stop` is pressed and flag duplicates.
- Provide functions to rename users and update storage, ensuring active authorisation checks pull the latest data.
- Guard concurrency with mutexes/semaphores to avoid interfering with ISR-driven keypad reads.

### 5. Add Firmware APIs & Messaging Hooks
- Define new HTTP endpoints (e.g. `/api/wiegand/register`, `/api/wiegand/users`, `/api/wiegand/rename`, `/api/wiegand/capture`) inside `services/api.c` alongside response builders.
- Extend websocket/server message pathways (`addClientMessageToQueue`, `addServiceMessageToQueue`) if live push updates are desired during registration.
- Update `build_state_snapshot` (or create a dedicated snapshot) to expose current Wiegand/keypad status, active registration flag, and enrolled users for the UI.
- Add keypad-specific configuration endpoints if additional settings are needed (timeouts, alerts, etc.).

### 6. Front-End Refactor (Material Design Layout)
- Replace `public/index.html`, `script.js`, and `style.css` with a modular layout:
  - Introduce a top app bar + side navigation (or tabs) for `Device`, `System`, and `Settings` views using Material-inspired components (pure CSS or a lightweight library that fits ESP32 constraints).
  - `Device` view: channel 1/2 toggles, lock/exit/FOB controls grouped with improved UX.
  - `System` view: display UUID, firmware info, and any diagnostics from `/api/state`.
  - `Settings` view: WiFi and server forms with validation and feedback.
- Under `Device`, add a dedicated Wiegand section featuring:
  - `Register` button to start capture, live list of detected tags (auto-labelled `User #` until renamed), `Stop` button, and rename inputs per entry.
  - Visual state showing whether new tags are accepted or normal mode is active.
- Add a sibling keypad subsection for keypad-specific configuration/status (e.g. last keypress, PIN slots, alert toggles).
- Refactor `script.js` into smaller modules (state loading, API clients, UI controllers) and update requests to use new endpoints.
- Ensure styles remain lightweight to meet ESP32 hosting limitations (minify assets as needed).

### 7. End-to-End Wiring & Validation
- On firmware boot, load saved Wiegand/keypad users into memory and expose them via the API snapshot.
- During registration, push updates to the web UI (polling or websocket) so newly tapped tags appear immediately.
- After renaming, confirm both UI state and backend storage reflect changes; ensure authorisation logic uses updated names/codes.
- Add logging/alerts for duplicate or invalid tags when exiting registration.

### 8. Testing Strategy
- Firmware: unit/integration tests for storage helpers; on-device manual tests for registration flow, rename persistence, and keypad behaviour.
- UI: browser-based smoke tests for navigation, responsive layout, registration interactions, and settings forms.
- Regression: confirm legacy lock/exit/FOB toggles still function via `/api/*` endpoints and that WiFi/server updates trigger expected reboot behaviour.

### 9. Documentation & Delivery
- Update firmware README (`code/controller/README.md`) with instructions for new build flags/endpoints and registration workflow.
- Capture UI screenshots demonstrating the new Material design layout and Wiegand workflow.
- Provide a release checklist: build firmware, flash device, verify UI cache busting, test Register/Stop cycle, and confirm persistent storage across reboots.


