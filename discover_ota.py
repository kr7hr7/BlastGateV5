#!/usr/bin/env python3
"""Upload target selection for PlatformIO OTA uploads.

Interactive menu shown on every OTA upload:
  0. Abort
  1. USB (<port>)
  2+. Discovered OTA devices on the local network
"""

import os
import socket
import subprocess
import sys
import time
import json

try:
    from serial.tools import list_ports
except ImportError:
    list_ports = None

try:
    from zeroconf import ServiceBrowser, Zeroconf, ServiceListener
except ImportError:
    ServiceBrowser = None
    Zeroconf = None
    ServiceListener = object


class OTADeviceListener(ServiceListener):
    def __init__(self):
        self.devices = []

    def add_service(self, zc, type_, name):
        info = zc.get_service_info(type_, name)
        if info and info.addresses:
            address = socket.inet_ntoa(info.addresses[0])
            hostname = name.replace("._arduino._tcp.local.", "")
            self.devices.append({
                "name": hostname,
                "ip": address,
                "port": info.port,
            })

    def update_service(self, zc, type_, name):
        pass

    def remove_service(self, zc, type_, name):
        pass


def discover_ota_devices(timeout=3):
    """Discover Arduino OTA devices on the LAN via mDNS."""
    print("\n" + "=" * 60)
    print("Scanning network for OTA devices...")
    print("=" * 60)

    if Zeroconf is None:
        print("zeroconf package not installed.")
        print("Install: pio pkg install --global --tool zeroconf")
        return []

    zeroconf = Zeroconf()
    listener = OTADeviceListener()
    browser = ServiceBrowser(zeroconf, "_arduino._tcp.local.", listener)
    time.sleep(timeout)
    browser.cancel()
    zeroconf.close()
    return listener.devices


def _candidate_esptool_paths():
    """Yield likely esptool.py locations from PlatformIO package dirs."""
    package_names = ("tool-esptoolpy", "tool-esptool-unofficial", "tool-esptool")

    # Preferred: ask PlatformIO for package locations.
    try:
        pio_platform = env.PioPlatform()
        for pkg in package_names:
            pkg_dir = pio_platform.get_package_dir(pkg)
            if pkg_dir:
                yield os.path.join(pkg_dir, "esptool.py")
    except Exception:
        pass

    # Fallback: check common PlatformIO home dirs.
    base_dirs = []
    try:
        core_dir = env.get("PROJECT_CORE_DIR")
        if core_dir:
            base_dirs.append(str(core_dir))
    except Exception:
        pass
    try:
        pio_home = env.subst("$PIOHOME_DIR")
        if pio_home:
            base_dirs.append(str(pio_home))
    except Exception:
        pass
    base_dirs.append(os.path.join(os.path.expanduser("~"), ".platformio"))

    for base in base_dirs:
        for pkg in package_names:
            yield os.path.join(base, "packages", pkg, "esptool.py")


def _find_esptool_cmd():
    """Return command prefix for invoking esptool via PlatformIO's Python."""
    python_exe = env.subst("$PYTHONEXE") if env.get("PYTHONEXE") else sys.executable

    for candidate in _candidate_esptool_paths():
        if os.path.exists(candidate):
            return [python_exe, candidate]

    # Last resort fallback.
    return [python_exe, "-m", "esptool"]


def _run_usb_upload(usb_port):
    """Flash firmware.bin to app partition via USB/esptool.

    Returns process exit code (0 = success).
    """
    build_dir = env.subst("$BUILD_DIR")
    firmware = os.path.join(build_dir, "firmware.bin")
    if not os.path.exists(firmware):
        print(f"ERROR: Firmware binary not found: {firmware}")
        return 1

    cmd = _find_esptool_cmd() + [
        "--chip",
        "esp32",
        "--port",
        usb_port,
        "--baud",
        "921600",
        "--before",
        "default_reset",
        "--after",
        "hard_reset",
        "write_flash",
        "-z",
        "--flash_mode",
        "dio",
        "--flash_freq",
        "80m",
        "--flash_size",
        "4MB",
        "0x10000",
        firmware,
    ]

    print(f"\nUploading firmware via USB -> {usb_port}")
    result = subprocess.run(cmd)
    return result.returncode


def _discover_usb_ports(preferred_port=None):
    """Return ordered list of serial ports for USB upload selection."""
    ports = []
    if list_ports is not None:
        try:
            for port in list_ports.comports():
                if port.device:
                    ports.append(port.device)
        except Exception:
            pass

    # Fallback: ask PlatformIO for detected serial devices.
    if not ports:
        try:
            pio_cmd = env.subst("$PYTHONEXE") if env.get("PYTHONEXE") else sys.executable
            pio_bin = os.path.join(os.path.dirname(pio_cmd), "platformio.exe")
            if not os.path.exists(pio_bin):
                pio_bin = "platformio"
            result = subprocess.run(
                [pio_bin, "device", "list", "--json-output"],
                capture_output=True,
                text=True,
                check=False,
            )
            if result.returncode == 0 and result.stdout.strip():
                devices = json.loads(result.stdout)
                for dev in devices:
                    port = dev.get("port")
                    if port:
                        ports.append(str(port))
        except Exception:
            pass

    # Add preferred port from project options even if not currently detected.
    if preferred_port and preferred_port not in ports:
        ports.insert(0, preferred_port)

    # Deduplicate while preserving order.
    deduped = []
    for port in ports:
        if port not in deduped:
            deduped.append(port)
    return deduped


def _prompt_combined_menu(usb_ports, ota_devices):
    """Show numbered menu with USB ports first and OTA devices after.

    Returns selected option dict, None to abort, or "__EOF__" when input stream
    cannot be read interactively.
    """
    options = []
    for usb_port in usb_ports:
        options.append(
            {"type": "usb", "label": f"USB ({usb_port})", "port": usb_port}
        )
    for device in ota_devices:
        options.append(
            {
                "type": "ota",
                "label": f"{device['name']} ({device['ip']})",
                "device": device,
            }
        )

    print("\nSelect upload target:")
    print("  0. Abort upload")
    for i, option in enumerate(options, 1):
        print(f"  {i}. {option['label']}")

    while True:
        try:
            raw = input("\nEnter number (or 'q' to abort): ").strip().lower()
        except EOFError:
            return "__EOF__"

        if raw in ("q", "quit", "x", "abort", "0"):
            return None
        if raw.isdigit():
            idx = int(raw)
            if 1 <= idx <= len(options):
                return options[idx - 1]
        print(f"Invalid selection. Enter 1-{len(options)} or 0/q to abort.")


def select_upload_target(*args, **kwargs):
    """PlatformIO upload pre-action: choose USB or OTA target."""
    try:
        preferred_usb_port = env.GetProjectOption("custom_usb_port", "COM3").strip()
    except Exception:
        preferred_usb_port = "COM3"

    usb_ports = _discover_usb_ports(preferred_usb_port)

    ota_devices = discover_ota_devices()
    ota_devices.sort(key=lambda item: item["name"])

    if not sys.stdin.isatty():
        print("\nInteractive selection is required, but no interactive terminal is available.")
        print("Upload aborted. Re-run from an interactive terminal/task to choose USB/OTA or Abort.")
        env.Exit(1)
        return

    selected = _prompt_combined_menu(usb_ports, ota_devices)
    if selected == "__EOF__" or selected is None:
        print("Upload aborted by user.")
        env.Exit(1)
        return

    if selected["type"] == "usb":
        rc = _run_usb_upload(selected["port"])
        env.Exit(rc)
        return

    device = selected["device"]
    env.Replace(UPLOAD_PORT=device["ip"])
    print(f"Using OTA device '{device['name']}' at {device['ip']}")
    return


Import("env")

upload_protocol = str(env.get("UPLOAD_PROTOCOL", "")).strip().lower()
upload_port_value = str(env.get("UPLOAD_PORT", "")).strip()
placeholder_ports = {"", "0.0.0.0"}

# Activate only for OTA env that uses placeholder upload port.
if upload_protocol == "espota" and upload_port_value in placeholder_ports:
    env.AddPreAction("upload", select_upload_target)
