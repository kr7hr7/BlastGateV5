# Functional Specification Document (One-Page)

## Feature
Runtime behavior for compiled artifact __pycache__/discover_ota.cpython-312.pyc.

## Purpose
Define expected behavior of the Python 3.12 bytecode cache generated from discover_ota.py for PlatformIO upload target selection.

## Artifact Context
- Compiled artifact: __pycache__/discover_ota.cpython-312.pyc
- Source of truth: discover_ota.py
- Related source-level spec: docs/discover_ota_FSD.md

## Trigger and Activation
- Script logic is active only when upload protocol is espota and upload port is placeholder (empty or 0.0.0.0).
- In that state, a pre-upload action runs target selection before upload.

## Core Functional Requirements
1. Functional equivalence: .pyc behavior must match discover_ota.py from which it was generated.
2. Version compatibility: artifact is valid for compatible CPython 3.12 runtime.
3. Discovery: enumerate USB ports (pyserial, with PlatformIO fallback) and OTA devices via mDNS when zeroconf is available.
4. Menu flow: present Abort, USB options, then OTA options; reprompt on invalid input.
5. USB path: execute esptool against firmware.bin and return subprocess exit code.
6. OTA path: set UPLOAD_PORT to selected device IP and continue normal OTA upload.
7. Non-interactive safety: auto-select only when exactly one OTA target exists; otherwise abort with non-zero exit.
8. Abort handling: explicit abort or EOF exits with non-zero status.

## Error Handling
- Missing zeroconf: continue with USB-only capability and guidance message.
- Missing firmware.bin for USB: fail with exit code 1.
- No available targets: abort with explicit message.
- Helper exceptions: fail soft where possible and keep deterministic outcome.

## Dependencies
- PlatformIO SCons env
- pyserial (optional)
- zeroconf (optional)
- platformio CLI (fallback discovery)
- esptool (package path or module fallback)

## Non-Functional Requirements
- Deterministic ordering (USB first, OTA sorted by name)
- Safe target selection (no ambiguous silent uploads)
- Fast discovery (default OTA scan about 3 seconds)
- Windows COM port compatibility

## Acceptance Criteria
1. In espota + placeholder-port mode, pre-upload selector is invoked.
2. Interactive mode supports Abort, USB, and OTA selection.
3. USB selection executes esptool and exits with esptool status.
4. OTA selection sets UPLOAD_PORT and proceeds.
5. Non-interactive mode auto-selects only a single OTA target; otherwise aborts safely.
6. Observed .pyc behavior matches discover_ota.py logic.

## Constraints
- Do not edit .pyc directly.
- Make changes in discover_ota.py; regenerate bytecode through normal execution.
