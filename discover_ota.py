#!/usr/bin/env python3
"""OTA device discovery script for PlatformIO.

Selection order:
1) Use configured UPLOAD_PORT if present.
2) Discover mDNS Arduino OTA devices and auto-select if exactly one.
3) If multiple are found, allow selection by OTA_DEVICE_NAME env var.
4) Otherwise fail with a clear list of candidates.
"""
import socket
import sys
import time

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
        if info:
            address = socket.inet_ntoa(info.addresses[0])
            hostname = name.replace('._arduino._tcp.local.', '')
            self.devices.append({
                'name': hostname,
                'ip': address,
                'port': info.port
            })
    
    def update_service(self, zc, type_, name):
        pass
    
    def remove_service(self, zc, type_, name):
        pass

def discover_ota_devices(timeout=3):
    """Discover OTA devices on the network"""
    print("\n" + "=" * 60)
    print("Scanning network for OTA devices...")
    print("=" * 60)

    if Zeroconf is None:
        print("zeroconf package is not installed in PlatformIO Python environment")
        print("Install with: pio pkg install --global --tool zeroconf")
        return []
    
    zeroconf = Zeroconf()
    listener = OTADeviceListener()
    browser = ServiceBrowser(zeroconf, "_arduino._tcp.local.", listener)
    
    time.sleep(timeout)
    
    browser.cancel()
    zeroconf.close()
    
    return listener.devices


def prompt_for_device_selection(devices):
    """Prompt user to choose an OTA target or abort upload."""
    if not sys.stdin.isatty():
        return None

    print("\nSelect upload target:")
    print("  0. Abort upload")
    for i, device in enumerate(devices, 1):
        print(f"  {i}. {device['name']:<30} ({device['ip']})")

    while True:
        choice = input("\nEnter number (or 'q' to abort): ").strip().lower()
        if choice in ("q", "quit", "x", "abort"):
            return -1
        if choice == "0":
            return -1
        if choice.isdigit():
            idx = int(choice)
            if 1 <= idx <= len(devices):
                return idx - 1
        print("Invalid selection. Enter 1-{} or 0/q to abort.".format(len(devices)))

def select_ota_device(*args, **kwargs):
    """Deterministic non-interactive device selection for OTA uploads."""
    devices = discover_ota_devices()

    if not devices:
        print("\nNo OTA devices found on network")
        print("Make sure device is powered on and connected to WiFi")
        print("Or set upload_port in platformio.ini for env:esp32dev-ota")
        env.Exit(1)
        return

    # Sort by hostname for consistent ordering
    devices.sort(key=lambda x: x["name"])

    configured_name = env.GetProjectOption("custom_ota_device_name", "").strip()
    if configured_name:
        for device in devices:
            if device["name"].lower() == configured_name.lower():
                env.Replace(UPLOAD_PORT=device["ip"])
                print(f"Using configured OTA device '{device['name']}' at {device['ip']}")
                return

    # In interactive mode, always offer explicit selection/abort, even for one device.
    if sys.stdin.isatty():
        selected_index = prompt_for_device_selection(devices)
        if selected_index == -1:
            print("Upload aborted by user.")
            env.Exit(1)
            return
        if selected_index is not None:
            selected = devices[selected_index]
            env.Replace(UPLOAD_PORT=selected["ip"])
            print(f"Using selected OTA device '{selected['name']}' at {selected['ip']}")
            return

    if len(devices) == 1:
        selected = devices[0]
        env.Replace(UPLOAD_PORT=selected["ip"])
        print(f"Using discovered OTA device '{selected['name']}' at {selected['ip']}")
        return

    print(f"\nFound {len(devices)} OTA devices:\n")
    for i, device in enumerate(devices, 1):
        print(f"  {i}. {device['name']:<30} ({device['ip']})")

    print("\nNo interactive terminal available.")
    print("Set one of these in platformio.ini for env:esp32dev-ota:")
    print("  upload_port = <device ip>")
    print("or")
    print("  custom_ota_device_name = <exact mdns hostname>")
    env.Exit(1)

# Register with PlatformIO
Import("env")
# Only run discovery if upload_port not already set
if not env.get("UPLOAD_PORT"):
    env.AddPreAction("upload", select_ota_device)
