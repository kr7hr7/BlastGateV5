**Main.cpp Overview**

- Global variable reference: [GLOBALS_DOC.md](GLOBALS_DOC.md)

- **File:** [src/main.cpp](src/main.cpp)
- **Purpose:** Initializes hardware, network (Wi‑Fi/MQTT), display and OTA, configures pins, and performs a single startup sensor check that may open or home the gate. The `loop()` is intentionally empty; main behaviour is event driven and uses FreeRTOS tasks.

**High-level Flow (setup)**n
1. `settings()`
   - Loads project settings and network/MQTT parameters (see `settings.cpp`).

2. `delay(100); Serial.begin(115200);` — serial for debug output.

3. Display initialization
   - `Wire.begin();`
   - `display.begin(SSD1306_SWITCHCAPVCC, 0x3C);`
   - `trace` set to `"Start"` then `displayStat()` called.

4. Network and MQTT
   - `WiFiConnect()` — connects to Wi‑Fi (uses `ssid`/`password`).
   - `mac` and `wifiDB` captured from `WiFi` object.

5. EEPROM
   - `EEPROM.begin(EEPROM_SIZE)` and early return on failure.
   - `boardconfiguration()` reads/writes board-specific EEPROM data and configures pins and behavior.

6. `prepNames()` — prepares topics, device names, OTA hostname, etc.

7. Pin configuration
   - `pinMode()` calls for analog input, relays, LEDs, H‑bridge control, stepper pins, and link pin.
   - Initial `digitalWrite()` states set for LEDs, relays and enable pins.

8. Timing and MQTT setup
   - `closeDelayTime = 1000 * gateDelaySeconds;`
   - `client.setKeepAlive(keepAlive); client.setServer(mqtt_server, 1883);` then `MQTTconnect()`.

9. FreeRTOS tasks
   - `xTaskCreatePinnedToCore()` creates background tasks: `pingBroker`, `keepWiFiAlive`, `keepMqttAlive`.

10. Diagnostics printing
   - Prints `gateType`, board id/version/config, pin assignments, local IP, etc.

11. `writeToBootLog()` — uploads boot info (HTTP) if configured.

12. OTA
   - Calls `OTA()` to initialize ArduinoOTA and calls `ArduinoOTA.handle()` once here. (Note: `ArduinoOTA.handle()` must be invoked regularly; the project may call it in other utilities/loops as well.)

13. Controller mode behavior
   - If `gateType == "S"` → static pressure sensor mode: early `return` (no gate control).
   - If `gateType == "P"` → passive data logger mode: reads analog and decides `toolOn()`/`toolOff()` then returns.
   - If `gateType == "M"` → manual gate mode: reads analog (inverted) and updates display/state, then returns.
   - Otherwise (normal controllers): reads sensor, compares to `trigger` (+ `triggerDelta`) and either `openGate()` or `homePosition()`.

14. `client.publish(mID, db, false);` — publishes a small boot message.

**Loop**
- `loop()` is empty: the app relies on background tasks and interrupts.

**Key Symbols (from `globals.h`)**
- `ANALOG_PIN_IN` — sensor analog input (ADC).
- `gateType` — mode selector (`A,B,C,D,M,S,P`).
- `gateOn`, `gateOff`, `stepPin`, `dirPin`, `enablePin` — motor/H‑bridge & stepper pins.
- `client` — MQTT client (PubSubClient).
- `display` — Adafruit SSD1306 instance.
- `ArduinoOTA` — OTA functionality.

**Important Notes & Troubleshooting**
- Boot straps: avoid GPIOs that affect ESP32 boot mode (0,2,4,5,12,15). The project has already moved `gateOff` and `dirPin` off strapping pins; if you rewire, ensure you don't use strap pins or that external circuits don't pull them low.
- `ArduinoOTA.handle()` must be called periodically to process OTA requests. In this project it's called in `OTA()` and in various utility functions — ensure the code path is exercised while waiting for an OTA upload.
- If the device resets on boot: open serial monitor at `115200` to see reset reason; verify strapping pins and external hardware.
- Display: SSD1306 address is `0x3C`. If display fails to initialize, check I2C wiring and `Wire.begin()`.
- MQTT: If `MQTTconnect()` fails repeatedly, verify `mqtt_server`, `ssid`, `password`, and network reachability.

**Where to look next**
- `src/settings.cpp` — default network and trigger values.
- `src/boardConfig.cpp` — pin assignments per board ID.
- `src/utilities.cpp` — OTA init and `logCycle()` plus HTTP upload logic.
- `src/controlFunctions.cpp` — `openGate()`, `homePosition()`, `toolOn()`/`toolOff()`.

**Quick dev commands**

Build:
```bash
pio run
```

Upload:
```bash
pio run -t upload
```

Serial monitor:
```bash
pio device monitor -b 115200
```

---

If you want I can also: add inline doxygen comments into `main.cpp`, or insert the above summary as a header comment at the top of `src/main.cpp`. Which would you prefer?