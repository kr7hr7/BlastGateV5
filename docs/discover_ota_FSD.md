# Functional Specification Document (FSD)

## Feature
Interactive upload target selection for PlatformIO OTA workflows

## Source Artifact
- Script: `discover_ota.py`
- Integration point: `platformio.ini` via `extra_scripts = pre:discover_ota.py`

## 1. Purpose
Provide a pre-upload selection workflow so firmware can be uploaded to either:
1. A USB serial port using esptool
2. A discovered OTA-capable ESP32 device on the local network
3. Abort (safe cancellation)

This feature prevents accidental uploads to the wrong target and allows one consistent upload command for both USB and OTA paths.

## 2. Scope
In scope:
- Discovery of OTA devices announced through mDNS service `_arduino._tcp.local.`
- Discovery of USB serial ports from pyserial or PlatformIO fallback
- Interactive terminal menu to choose upload target
- USB flashing path using esptool and `firmware.bin`
- OTA path by replacing PlatformIO `UPLOAD_PORT` with selected device IP
- Aborting upload when user cancels or interactivity is unavailable

Out of scope:
- Firmware build logic
- OTA authentication policy (`upload_flags` remains configured by PlatformIO)
- Persistent storage of selected target
- Network diagnostics beyond discovery timeout behavior

## 3. Trigger Conditions
The pre-action is enabled only when all conditions are true:
- `upload_protocol == espota`
- `upload_port` is a placeholder (`""` or `"0.0.0.0"`)

When enabled, the script registers a pre-action for the `upload` target that runs selection before upload proceeds.

## 4. Inputs
Configuration/runtime inputs:
- PlatformIO environment object (`env`)
- Project option: `custom_usb_port` (default `COM3`)
- Upload protocol and port values from PlatformIO variables
- Local USB device enumeration data
- mDNS announcements for `_arduino._tcp.local.`
- Terminal stdin capability (`sys.stdin.isatty()`)

## 5. Outputs
- Console menu and status messages
- For USB path: direct esptool process execution and exit code propagation
- For OTA path: `env.Replace(UPLOAD_PORT=<selected_ip>)`
- Process termination through `env.Exit(code)` for abort/error/USB completion

## 6. Functional Requirements

### FR-1: OTA Discovery
- The system shall scan the LAN for Arduino OTA services using Zeroconf for a default 3-second window.
- The system shall parse each discovered service into `name`, `ip`, and `port`.
- The system shall return an empty OTA list when no devices are found.

### FR-2: Zeroconf Dependency Handling
- If Zeroconf is unavailable, the system shall display installation guidance and continue with an empty OTA list.

### FR-3: USB Port Discovery
- The system shall discover serial ports using `serial.tools.list_ports` when available.
- If none are found, the system shall attempt PlatformIO JSON device listing (`platformio device list --json-output`).
- The system shall include `custom_usb_port` as a selectable option even if not currently detected.
- The resulting USB list shall be deduplicated while preserving order.

### FR-4: Interactive Menu
- The system shall display a combined numbered menu in this order:
  1. Abort option (index 0)
  2. USB options
  3. OTA options (sorted by device name)
- The system shall accept `q/quit/x/abort/0` as abort commands.
- The system shall repeatedly prompt until a valid selection or abort/EOF.

### FR-5: Non-interactive Guard
- If stdin is non-interactive, the system shall abort upload with a clear message and non-zero exit.

### FR-6: USB Upload Path
- On USB selection, the system shall locate `firmware.bin` in `$BUILD_DIR`.
- If `firmware.bin` is missing, the system shall fail with exit code 1.
- The system shall resolve esptool command path using PlatformIO package lookup with fallback to `python -m esptool`.
- The system shall invoke esptool with ESP32 parameters and propagate subprocess return code via `env.Exit(rc)`.

### FR-7: OTA Upload Path
- On OTA selection, the system shall set PlatformIO `UPLOAD_PORT` to selected device IP.
- The system shall print the selected device summary and return control to PlatformIO for normal OTA upload execution.

### FR-8: Safe Abort Behavior
- On explicit abort or EOF input, the system shall terminate upload using `env.Exit(1)`.

## 7. Decision Flow
1. Determine whether script should activate based on upload protocol/placeholder port.
2. On upload pre-action: read preferred USB port.
3. Discover USB ports.
4. Discover OTA devices.
5. Validate interactive terminal availability.
6. Show combined menu and read selection.
7. Branch:
   - Abort/EOF -> exit 1
   - USB -> run esptool flash -> exit with esptool code
   - OTA -> set `UPLOAD_PORT` and continue standard PlatformIO OTA upload

## 8. External Dependencies
Required/optional runtime components:
- PlatformIO SCons `env` object (required)
- `zeroconf` package (optional but needed for OTA discovery)
- `pyserial` package (optional for primary USB enumeration)
- `platformio` CLI executable (fallback USB discovery path)
- `esptool.py` package/tool from PlatformIO packages or Python module fallback

## 9. Error Handling Requirements
- Missing Zeroconf: user guidance shown, no hard failure unless user cannot proceed.
- Missing `firmware.bin` for USB: fail with explicit error.
- Invalid user input: prompt again without crashing.
- EOF/no interactive stdin: abort with explicit reason.
- Unexpected exceptions in discovery helpers: fail soft where possible and continue with reduced data.

## 10. Non-Functional Requirements
- Discovery responsiveness: default OTA scan completes in approximately 3 seconds.
- Deterministic ordering: USB first, OTA sorted by name.
- Safety: never perform upload without explicit user choice in interactive flow.
- Compatibility: works with Windows-style COM ports and generic serial naming.

## 11. Acceptance Criteria
1. In `esp32dev-ota` with placeholder upload port, upload command shows menu before upload action.
2. Menu includes Abort, at least one USB option (preferred port if available), and discovered OTA devices when present.
3. Choosing Abort exits upload with non-zero status.
4. Choosing USB runs esptool against selected port and exits with esptool return code.
5. Choosing OTA sets target IP and proceeds with PlatformIO OTA upload.
6. Running in non-interactive context aborts with clear message and does not upload.

## 12. Known Constraints
- OTA discovery depends on mDNS visibility and LAN conditions.
- Only first resolved service address is used per device entry.
- Script activates only for espota/placeholder-port pattern; fixed-IP OTA environments bypass menu by design.

## 13. Suggested Verification Matrix
- Interactive terminal + both USB and OTA available
- Interactive terminal + USB only
- Interactive terminal + OTA only
- No zeroconf installed
- Non-interactive invocation (CI/task runner without TTY)
- Missing firmware binary for USB path
- Invalid menu input then valid selection
