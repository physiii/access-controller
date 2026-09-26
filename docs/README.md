# Access Controller Documentation

This folder documents the current shape of the access-controller project as of
July 6, 2026. It is based on the checked-out repository contents, including the
ESP32 firmware, the Node.js tunnel server, KiCad hardware projects,
manufacturing outputs, enclosure models, images, and existing planning notes.

The project is a commercial-style access controller for electric strikes and magnetic locks.

The main device idea is a networked ESP32-S3 controller that can manage two lock channels, read Wiegand RFID/keypad devices, accept physical inputs such as exit buttons, fobs, and motion, expose a local web UI, store credential/configuration state, and optionally connect outward to a tunnel gateway so the device UI can be reached through a server.

## Documentation map

| Guide | Purpose |
|---|---|
| [Project overview](PROJECT.md) | Product and subsystem architecture |
| [Repository map](REPOSITORY_MAP.md) | Source, hardware and model locations |
| [Software](SOFTWARE.md) | Firmware, UI, APIs, tests and tunnel |
| [Network provisioning](NETWORK_PROVISIONING.md) | Wi-Fi, AP fallback and server reachability |
| [Deploy and test](CONTROLLER_DEPLOY_AND_TEST.md) | Programming, OTA and attached-device verification |
| [Hardware](HARDWARE.md) | KiCad, manufacturing and enclosure artifacts |
| [Power study](POWER_AND_ENERGY_HARVESTING.md) | Proposed battery/harvesting variant; not implemented |
| [Workflows](WORKFLOWS.md) | Build, verification and maintenance commands |
| [Modernization plan](PLAN.md) | Wiegand, keypad and UI planning record |
| [Consolidation audit](WORKSPACE_CLEANUP.md) | Reconciled source, branches and recovery |

## Current Project Snapshot

The repository is organized around these major areas:

- `code/controller`: the active ESP32-S3 access-controller firmware and embedded
  web UI.
- `code/controller_mini`: an alternate/experimental ESP32-S3 firmware layout
  with service-manager, radar, OTA, and network-manager modules.
- `code/tunnel`: a Node.js reverse HTTP tunnel server and mock ESP32 client.
- `circuits/controller`: the main controller KiCad design, generated PCB data,
  gerbers, BOM/placement files, symbols, footprints, and an adapter board.
- `circuits/strike`: a smaller strike-controller KiCad design with its own
  gerbers and manufacturing exports.
- `model`: enclosure and mechanical artifacts, including current controller
  STL/3MF/Blender/G-code files and older reference models.
- `images`: rendered board/schematic images and a PoE controller PDF reference.
- `docs`: this documentation set plus the existing modernization plan.

## Notes About Source State

Source changes were reconciled onto `master` during the September 2026 cleanup.
Update these guides with hardware, firmware or UI changes; dated installation
notes and generated evidence describe their recorded revision.
