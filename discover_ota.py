#!/usr/bin/env python3
"""
OTA Device Discovery Script for PlatformIO
Discovers all ESP32 devices with OTA enabled on the network via mDNS
"""
import socket
import time
from zeroconf import ServiceBrowser, Zeroconf, ServiceListener

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
    print("\n" + "="*60)
    print("🔍 Scanning network for OTA devices...")
    print("="*60)
    
    zeroconf = Zeroconf()
    listener = OTADeviceListener()
    browser = ServiceBrowser(zeroconf, "_arduino._tcp.local.", listener)
    
    time.sleep(timeout)
    
    browser.cancel()
    zeroconf.close()
    
    return listener.devices

def select_ota_device(*args, **kwargs):
    """Interactive device selection"""
    devices = discover_ota_devices()
    
    if not devices:
        print("\n❌ No OTA devices found on network!")
        print("   Make sure devices are powered on and connected to WiFi")
        env.Exit(1)
        return
    
    # Sort by hostname for consistent ordering
    devices.sort(key=lambda x: x['name'])
    
    print(f"\n📡 Found {len(devices)} OTA device(s):\n")
    for i, device in enumerate(devices, 1):
        print(f"  {i}. {device['name']:<30} ({device['ip']})")
    
    # Get user selection
    while True:
        try:
            selection = input(f"\n👉 Select device [1-{len(devices)}]: ").strip()
            idx = int(selection) - 1
            if 0 <= idx < len(devices):
                break
            print(f"   Please enter a number between 1 and {len(devices)}")
        except (ValueError, KeyboardInterrupt):
            print("\n\n❌ Upload cancelled")
            env.Exit(1)
            return
    
    selected = devices[idx]
    print(f"\n✅ Selected: {selected['name']} at {selected['ip']}")
    print("="*60 + "\n")
    
    # Update upload port dynamically
    env.Replace(UPLOAD_PORT=selected['ip'])

# Register with PlatformIO
Import("env")
# Only run discovery if upload_port not already set
if not env.get("UPLOAD_PORT"):
    env.AddPreAction("upload", select_ota_device)
