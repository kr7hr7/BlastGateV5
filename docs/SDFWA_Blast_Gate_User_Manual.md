# SDFWA Blast Gate System — User Manual

| Field | Value |
|---|---|
| Document Version | [1.0] |
| Date | [Month DD, YYYY] |
| Prepared By | [Author Name] |
| Audience | Maintenance Team & Support Technicians |

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [How to Add a Controller to the System](#2-how-to-add-a-controller-to-the-system)
3. [How to Remove a Tool from the System](#3-how-to-remove-a-tool-from-the-system)
- [Appendix](#appendix--quick-reference)

---

## 1. System Overview

The SDFWA Blast Gate System provides automated dust collection management for the woodshop. Motorized blast gates are installed at each tool and are controlled via smart home automation, eliminating the need to manually open or close gates when switching between tools.

### 1.1 Purpose

This manual serves two purposes: it provides operational guidance for maintenance team members responsible for the day-to-day upkeep of the blast gate system, and it serves as technical documentation for future reengineering  to configure, extend, or troubleshoot the system. It covers system architecture, firmware configuration via VS Code, and Home Assistant integration.

### 1.2 System Components

- Blast gate controllers (one per tool).   
  - There are about 25 blast gate controllers in the system.  There is one for each stationary power tool.  The controller:
    - monitors the power cord for the tool to sense is if the tool is on or off
    - if the tool is on, the controller opens the gate, closes the signal relay contacts that are monitored by the Jace
    - when the tool is turned off, the controller begins a countdown (software configurable but normally 15 seconds). After the countdown the gate is closed and the JACE signal 
    - communicates via MQTT to the HA the current state of the gate (open, closed, opening, closing, unknown) to home assistant
    - There are multiple versions of the controllers. addition details available in the appendix

- System sensors 
- Central Power Supply / low voltage & signal wiring 
- Wiring harness 
- Central Industrial Controller:  The "JACE" 
- Home Assistant instance — [specify location / device it runs on]
- VS Code development environment — [specify where code is maintained]
- Dust collector — [specify make/model if relevant]
- Wi-Fi / network infrastructure

### 1.3 How It Works

1. A member activates a tool (turns it on).
2. The controller for that tool detects the activation signal.
3. The controller sends a message to Home Assistant via [protocol — e.g., MQTT, ESPHome].
4. Home Assistant triggers an automation that opens the appropriate blast gate and closes all others.
5. The dust collector runs and collects dust through the open gate.
6. When the tool is turned off, the gate returns to its default (closed) position.

### 1.4 Network & Communication

| Setting | Value |
|---|---|
| Communication Protocol | [e.g., MQTT / ESPHome / Wi-Fi] |
| MQTT Broker (if used) | [IP address or hostname] |
| Home Assistant URL | [http://homeassistant.local:8123 or IP] |
| Firmware Platform | [e.g., ESPHome / Arduino / Tasmota] |

> **NOTE:** All controllers must be connected to the shop Wi-Fi network before they can communicate with Home Assistant.

---

## 2. How to Add a Controller to the System

Follow the steps in this section when installing a blast gate controller for a new tool. Both the VS Code firmware configuration and the Home Assistant configuration must be updated.

> **WARNING:** Complete the VS Code changes and flash the firmware before making changes in Home Assistant.

### 2.1 Before You Begin

Gather the following information before starting:

| Field | Value |
|---|---|
| Tool Name | [Descriptive name, e.g., Table Saw] |
| Controller Device Name | [Unique ID, e.g., blast-gate-tablesaw] |
| Physical Location | [Where the gate is installed] |
| Gate Number / ID | [Hardware identifier] |
| Wi-Fi Network | [SSID the controller will join] |

---

### 2.2 VS Code Changes

The firmware configuration files live in the project repository. Open the project in VS Code before proceeding.

#### Step 1 — Open the Project

1. Launch VS Code.
2. Open the blast gate project folder: **File > Open Folder > [path to project]**.
3. Confirm you are on the correct branch: `[branch name]`.

#### Step 2 — Create the Controller Configuration File

1. In the Explorer panel, navigate to the `[folder name]` directory.
2. Right-click the folder and select **New File**.
3. Name the file: `[device-name].yaml` (e.g., `blast-gate-tablesaw.yaml`).
4. Copy the contents of `[template-file.yaml]` into the new file as a starting point.

#### Step 3 — Edit the Configuration

Update the following fields in the new `.yaml` file:

| Field | Value |
|---|---|
| `name:` | [device-name] — must be unique across all controllers |
| `friendly_name:` | [Human-readable tool name] |
| `ssid:` | [Wi-Fi SSID] |
| `password:` | [Wi-Fi password — use secrets.yaml reference] |
| `[gate pin]:` | [GPIO pin number connected to gate actuator] |
| `[sensor pin]:` | [GPIO pin number for tool detection sensor] |

> **NOTE:** Sensitive values such as Wi-Fi passwords should always be stored in `secrets.yaml`, not directly in the device file.

#### Step 4 — Validate the Configuration

1. Open the VS Code terminal (`Ctrl+`` ` or **View > Terminal**).
2. Run the linter / validator:
   ```
   [command to validate — e.g., esphome config blast-gate-tablesaw.yaml]
   ```
3. Resolve any errors before proceeding.

#### Step 5 — Flash the Firmware

1. Connect the controller hardware to your computer via USB.
2. Run the flash command:
   ```
   [command to flash — e.g., esphome run blast-gate-tablesaw.yaml]
   ```
3. Monitor the output log and confirm the device boots and connects to Wi-Fi.
4. Disconnect the USB cable and install the controller in the shop.

#### Step 6 — Commit the Configuration

1. Stage the new file in Git: `git add [device-name].yaml`
2. Commit with a descriptive message: `git commit -m "Add controller for [Tool Name]"`
3. Push to the repository: `git push`

---

### 2.3 Home Assistant Changes

Once the controller is online, add it to Home Assistant so it can participate in automations.

#### Step 1 — Discover the New Device

1. Open Home Assistant in your browser: `[Home Assistant URL]`.
2. Navigate to **Settings > Devices & Services**.
3. If the device was auto-discovered, it will appear in the **Discovered** section. Click **Configure** and follow the prompts.
4. If not auto-discovered, click **Add Integration** and select `[integration name — e.g., ESPHome]`.
5. Enter the device hostname or IP address: `[device-name].local` or `[IP]`.

#### Step 2 — Verify Entities

1. Open the newly added device from **Settings > Devices & Services > [Integration]**.
2. Confirm the following entities are present:
   - Switch or Cover entity for the blast gate actuator
   - Binary sensor entity for tool detection
   - [Any additional entities expected]
3. Test each entity manually by toggling from the device page.

#### Step 3 — Add to the Dashboard (Optional)

1. Navigate to your Home Assistant dashboard.
2. Click the three-dot menu > **Edit Dashboard**.
3. Add a card for the new tool's blast gate. Recommended card type: [e.g., Button, Tile, Entity].
4. Configure the card to target the gate switch entity.
5. Save the dashboard.

#### Step 4 — Update or Create Automations

1. Navigate to **Settings > Automations & Scenes**.
2. Open the existing blast gate automation: `[Automation Name]`.
3. Add a new condition/action block for the new tool:
   - Trigger: [tool detection sensor] turns ON
   - Action: Open [new gate entity], close all others
4. Repeat for the "tool off" automation if applicable.
5. Save and test the automation by activating the tool.

> **NOTE:** Test the full end-to-end flow (tool on → gate opens → dust collector runs → tool off → gate closes) before considering the installation complete.

---

## 3. How to Remove a Tool from the System

Follow the steps in this section when decommissioning a tool or replacing a controller. Remove it from Home Assistant first, then clean up the firmware files.

> **WARNING:** Removing a device from Home Assistant will delete its history and entity data. Ensure you no longer need this data before proceeding.

---

### 3.1 VS Code Changes

#### Step 1 — Open the Project

1. Launch VS Code and open the blast gate project folder.
2. Confirm you are on the correct branch: `[branch name]`.

#### Step 2 — Remove the Configuration File

1. In the Explorer panel, locate the device configuration file: `[device-name].yaml`.
2. Right-click the file and select **Delete** (or move it to an archive folder if you want to preserve it).
3. If the device name appears in any shared configuration files (e.g., a device list or group file), remove those references now.

#### Step 3 — Commit the Removal

1. Stage the deletion: `git rm [device-name].yaml`
2. Commit: `git commit -m "Remove controller for [Tool Name]"`
3. Push: `git push`

---

### 3.2 Home Assistant Changes

#### Step 1 — Update Automations

1. Navigate to **Settings > Automations & Scenes**.
2. Open each automation that references the tool being removed.
3. Delete or disable the trigger/action blocks that reference the removed tool's entities.
4. Save each automation.

> **WARNING:** Do not delete automations that are still needed for other tools — only remove the blocks specific to the tool being decommissioned.

#### Step 2 — Remove the Device

1. Navigate to **Settings > Devices & Services**.
2. Find the integration the controller belongs to (e.g., ESPHome).
3. Click the device to open its detail page.
4. Click the three-dot menu and select **Delete**.
5. Confirm the deletion when prompted.

#### Step 3 — Clean Up the Dashboard

1. Navigate to your Home Assistant dashboard.
2. Click **Edit Dashboard**.
3. Locate any cards that referenced the removed tool's entities.
4. Delete those cards.
5. Save the dashboard.

#### Step 4 — Verify System Integrity

1. Cycle through each remaining tool and confirm its automation still triggers correctly.
2. Check the Home Assistant log (**Settings > System > Logs**) for any errors referencing the removed device.
3. Resolve any residual errors before considering the removal complete.

> **NOTE:** If the physical controller hardware will be reused for a different tool, follow Section 2 to re-configure and re-register it under the new tool name.

---

## Appendix — Quick Reference

### A. Installed Controllers

| Tool Name | Controller Device Name |
|---|---|
| [Tool 1] | [device-name-1] |
| [Tool 2] | [device-name-2] |
| [Tool 3] | [device-name-3] |
| [Add more…] | |

### B. Key File Locations

| Item | Location |
|---|---|
| Project Repository | [Git URL or local path] |
| Device Configs Folder | [relative path within project] |
| Secrets File | `secrets.yaml` — [location] |
| Home Assistant Config | [path or URL] |

### C. Useful Commands

| Task | Command |
|---|---|
| Validate config | `[e.g., esphome config <device>.yaml]` |
| Flash firmware | `[e.g., esphome run <device>.yaml]` |
| View device logs | `[e.g., esphome logs <device>.yaml]` |
| Git status | `git status` |
| Git push | `git push` |

### D. Contacts

| Role | Contact |
|---|---|
| System Administrator | [Name — email / phone] |
| Home Assistant Admin | [Name — email / phone] |
| General Enquiries | [SDFWA contact] |
