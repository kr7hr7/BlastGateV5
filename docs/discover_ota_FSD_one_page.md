# One-Page FSD: discover_ota.py

## Feature
Interactive upload target selection for ESP32 firmware upload in PlatformIO.

## Objective
Ensure each upload run makes an explicit, safe target choice:
- Abort
- USB flash to a selected serial port
- OTA upload to a discovered LAN device

## Integration
- Script: discover_ota.py
- Hook: pre-upload script via PlatformIO extra_scripts
- Activation rule: only when upload protocol is espota and upload_port is placeholder (empty or 0.0.0.0)

## Primary User Flow
1. Upload is started.
2. Script discovers USB ports.
3. Script discovers OTA devices via mDNS (_arduino._tcp.local.).
4. Script shows one combined menu:
   - 0 Abort
   - 1..N USB options
   - Next options OTA devices
5. User selects target.
6. Script executes selected branch:
   - Abort: stop upload (exit non-zero)
   - USB: run esptool flash of firmware.bin to selected port, return esptool exit code
   - OTA: set UPLOAD_PORT to selected device IP and continue normal PlatformIO OTA upload

## Functional Requirements (Condensed)
- OTA discovery shall run for a short timeout (default 3 seconds).
- OTA devices shall include hostname, IP, and service port.
- USB discovery shall use pyserial first, then PlatformIO CLI JSON fallback.
- Preferred USB port from project option custom_usb_port (default COM3) shall be included even if not currently detected.
- Menu shall be interactive and reject invalid input until valid selection, abort, or EOF.
- Non-interactive stdin shall force safe abort with message.
- USB path shall fail clearly if firmware.bin is missing.
- USB path shall resolve esptool from PlatformIO package paths with fallback to python module invocation.

## Dependencies
- Required: PlatformIO SCons environment
- Optional: zeroconf (OTA discovery), pyserial (USB discovery)
- Runtime tools: platformio CLI fallback, esptool

## Error and Safety Behavior
- Missing zeroconf: show install hint; continue with no OTA entries.
- Invalid menu input: reprompt.
- EOF or explicit abort: terminate upload safely.
- Missing firmware binary on USB path: terminate with failure code.
- Discovery exceptions: fail soft where possible and keep menu usable.

## Non-Functional Targets
- Fast selection readiness (OTA scan roughly 3 seconds).
- Deterministic menu ordering (USB first, OTA sorted by name).
- No upload proceeds without explicit interactive choice in selector flow.

## Acceptance Criteria
1. OTA environment with placeholder upload port shows selector before upload.
2. Menu includes Abort and available USB/OTA targets.
3. Abort stops upload with non-zero result.
4. USB selection flashes via esptool and returns subprocess status.
5. OTA selection updates UPLOAD_PORT and proceeds OTA upload.
6. Non-interactive execution aborts with clear guidance.
