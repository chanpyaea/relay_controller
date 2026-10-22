# ESP32 Relay Controller 02

ESP32 firmware for controlling three relays, monitoring AC electricity and tank water level, providing a local web dashboard, supporting browser-based OTA updates, and integrating automatically with Home Assistant through MQTT Discovery.

This project is configured as the **second controller** in the installation. Its Home Assistant device ID, MQTT client ID, discovery IDs, and MQTT topics are different from device 01, so the two devices do not overwrite each other.

## Features

- Three independently controlled relays
- Local responsive web dashboard
- AC electricity ON/cut-off detection on GPIO 25
- Noise filtering and three-second state confirmation
- Four active-HIGH float-switch inputs
- Water-level states and percentage reporting (`LOW` 0%, `MIDDLE` 50%, `FULL` 100%)
- JSON HTTP APIs for relay, electricity, and water-level states
- MQTT relay control and retained state publication
- Home Assistant MQTT Discovery
- Separate Home Assistant electricity and water-level sensors
- Browser-based OTA firmware updates with progress display and dashboard navigation
- Automatic Wi-Fi and MQTT reconnection

## Hardware

| Function | ESP32 pin | Direction | Notes |
| --- | ---: | --- | --- |
| Relay 1 | GPIO 5 | Output | Active HIGH |
| Relay 2 | GPIO 18 | Output | Active HIGH |
| Relay 3 | GPIO 19 | Output | Active HIGH |
| AC detector `H_PF` | GPIO 25 | Input | Falling-edge pulse input with internal pull-up |
| Float 1 | GPIO 12 | Input | Active HIGH; used for level calculation |
| Float 2 | GPIO 13 | Input | Active HIGH; used for level calculation |
| Float 3 | GPIO 14 | Input | Read by the API; reserved for future level logic |
| Float 4 | GPIO 27 | Input | Read by the API; reserved for future level logic |

The relay outputs initialize LOW/OFF when the ESP32 starts.

> **Electrical safety:** Never connect AC mains voltage directly to GPIO 25 or any ESP32 pin. GPIO 25 must receive only a safe, isolated, 3.3 V-compatible signal from an optocoupler or properly designed AC detection circuit. Mains wiring should be completed by a qualified person and housed in a suitable insulated enclosure.

## AC electricity detection

The `H_PF` detector output is connected to GPIO 25. A falling-edge interrupt counts its pulses without blocking the rest of the firmware.

Every second, the firmware checks the pulse count:

- One or more pulses means an AC-present candidate reading.
- Zero pulses means an AC-off candidate reading.
- Three consecutive candidate readings are required before the public state changes.
- The input uses `INPUT_PULLUP` to reduce false edges from a floating pin.

Consequently, Home Assistant and the web dashboard normally take about three seconds to confirm either an AC restoration or a power cut. These values can be adjusted in `include/config.h`:

```cpp
#define AC_CHECK_INTERVAL_MS     1000
#define AC_MIN_PULSES_PER_CHECK  1
#define AC_ON_CONFIRM_CHECKS     3
#define AC_OFF_CONFIRM_CHECKS    3
```

## Water-level detection

The firmware reads all four float-switch inputs every 250 milliseconds. The switches must provide a safe 3.3 V-compatible HIGH or LOW signal; the inputs are configured as `INPUT`, so external pull-up or pull-down circuitry must prevent floating signals.

The current tank-level calculation uses Float 1 and Float 2:

| Float 1 (GPIO 12) | Float 2 (GPIO 13) | Water level | Percentage |
| --- | --- | --- | ---: |
| HIGH | HIGH | `FULL` | 100% |
| LOW | HIGH | `MIDDLE` | 50% |
| LOW | LOW | `LOW` | 0% |
| HIGH | LOW | `LOW` | 0% |

The Float 1 HIGH / Float 2 LOW combination is treated as `LOW` because it does not match a valid configured level. Float 3 and Float 4 are initialized and exposed in the HTTP API, but they do not currently affect the calculated level.

## Software requirements

- Visual Studio Code with the PlatformIO extension, or PlatformIO Core
- An ESP32 development board compatible with `esp32dev`
- Home Assistant and an MQTT broker for Home Assistant integration

The project dependencies are declared in `platformio.ini`:

- ESPAsyncWebServer-esphome
- PubSubClient
- ArduinoJson

## Configuration

Edit `include/config.h` before building.

### Wi-Fi

```cpp
#define WIFI_SSID     "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### MQTT

```cpp
#define MQTT_BROKER      "HOME_ASSISTANT_IP"
#define MQTT_PORT        1883
#define MQTT_USERNAME    "YOUR_MQTT_USER"
#define MQTT_PASSWORD    "YOUR_MQTT_PASSWORD"
```

`MQTT_BROKER` must be the LAN address of the computer running Mosquitto, normally the Home Assistant server. Reserve that address in the router so it does not change.

Credentials are stored as plain text in the compiled firmware. Use a dedicated, non-administrator MQTT account and do not commit real Wi-Fi or MQTT passwords to a public repository.

### Unique device identity and multiple controllers

Only change `DEVICE_ID_SUFFIX` for each physical controller:

```cpp
#define DEVICE_ID_SUFFIX "02"
```

For a third controller, change it to:

```cpp
#define DEVICE_ID_SUFFIX "03"
```

The firmware automatically derives all of these values from the suffix:

| Derived value for suffix `03` | Result |
| --- | --- |
| MQTT client ID | `ESP32_RELAY_03` |
| Home Assistant device ID | `esp32_relay_03` |
| Home Assistant display name | `ESP32 Relay Controller 03` |
| MQTT base topic | `home/esp32_relay_03` |
| Availability topic | `home/esp32_relay_03/availability` |
| Electricity topic | `home/esp32_relay_03/electricity/state` |
| Water-level state topic | `home/esp32_relay_03/water_level/state` |
| Water-level percentage topic | `home/esp32_relay_03/water_level/percentage` |

Every physical ESP32 must use a different suffix. Duplicate suffixes cause MQTT disconnections and Home Assistant entities overwriting each other. Build and flash each device separately after changing the suffix.

## Build and upload

Using PlatformIO in Visual Studio Code:

1. Open this project folder.
2. Connect the ESP32 through USB.
3. Edit `include/config.h`.
4. Select **PlatformIO: Build**.
5. Select **PlatformIO: Upload**.
6. Open Serial Monitor at `115200` baud.

Using PlatformIO Core:

```sh
pio run
pio run --target upload
pio device monitor --baud 115200
```

The compiled binary is generated at:

```text
.pio/build/esp32dev/firmware.bin
```

## Local web dashboard

After Wi-Fi connects, Serial Monitor prints the ESP32 IP address:

```text
ESP32 IP Address : 192.168.x.x
```

Open that address in a browser on the same network:

```text
http://192.168.x.x/
```

The dashboard displays the electricity supply, water-level name and percentage, and three relay controls. It also provides a link to the OTA update page.

### Local HTTP API

| Method | Endpoint | Purpose |
| --- | --- | --- |
| GET | `/api/relay_states` | Returns the three relay states as a JSON array |
| GET | `/api/electricity_status` | Returns `{"electricity_on":true}` or `false` |
| GET | `/api/water_level` | Returns level, percentage, and all four raw float states |
| GET | `/api/relay/1/on` | Turns relay 1 on |
| GET | `/api/relay/1/off` | Turns relay 1 off |
| GET | `/api/relay/2/on` | Turns relay 2 on |
| GET | `/api/relay/2/off` | Turns relay 2 off |
| GET | `/api/relay/3/on` | Turns relay 3 on |
| GET | `/api/relay/3/off` | Turns relay 3 off |
| GET | `/update` | Opens the browser OTA page |

Example water-level response:

```json
{"level":"MIDDLE","percentage":50,"float1":false,"float2":true,"float3":false,"float4":false}
```

## OTA firmware update

1. Build the project and locate `.pio/build/esp32dev/firmware.bin`.
2. Open `http://ESP32_IP/update`.
3. Select `firmware.bin` and start the upload.
4. Keep the ESP32 powered and connected until it reports success and reboots.

Select **Back to Dashboard** to return to the controller home page without starting an update.

Use USB recovery if an interrupted or incompatible OTA update prevents normal startup.

## Home Assistant connection

Home Assistant configuration is automatic through MQTT Discovery. No entity YAML is required.

### 1. Install Mosquitto Broker

For Home Assistant OS:

1. Open **Settings > Apps > Install app**.
2. Install **Mosquitto broker**.
3. Start it.
4. Enable **Start on boot** and **Watchdog**.

If Mosquitto runs on another server, use that server's IP address for `MQTT_BROKER`.

### 2. Create a dedicated MQTT user

In Home Assistant:

1. Enable **Advanced mode** in the user profile if the user-management options are hidden.
2. Open **Settings > People > Users**.
3. Create a non-administrator MQTT user.
4. Put the same username and password in `include/config.h`.

The Mosquitto app can alternatively create a local login through its `logins` configuration.

### 3. Enable the MQTT integration

1. Open **Settings > Devices & services**.
2. Select the discovered **MQTT** integration, or choose **Add integration > MQTT**.
3. Enter the broker details if requested.
4. Ensure MQTT Discovery is enabled with the default prefix `homeassistant`.

### 4. Start the ESP32

Restart the ESP32 after Mosquitto and the MQTT integration are running. Serial Monitor should show messages similar to:

```text
MQTT Connected
Published discovery: homeassistant/switch/esp32_relay_02_relay_1/config
Published discovery: homeassistant/switch/esp32_relay_02_relay_2/config
Published discovery: homeassistant/switch/esp32_relay_02_relay_3/config
Published discovery: homeassistant/binary_sensor/esp32_relay_02_ac_power/config
Published discovery: homeassistant/sensor/esp32_relay_02_water_level/config
Published discovery: homeassistant/sensor/esp32_relay_02_water_level_percentage/config
```

Open **Settings > Devices & services > MQTT > Devices**. The device should appear as **ESP32 Relay Controller 02** with six entities:

- Relay 1
- Relay 2
- Relay 3
- Electricity Supply
- Water Level
- Water Level Percentage

To display these readings on a Home Assistant dashboard, add **Tile** or **Entities** cards and select **Electricity Supply**, **Water Level**, and **Water Level Percentage**.

## MQTT topics

All state messages are retained by the broker.

| Purpose | Topic | Payload |
| --- | --- | --- |
| Availability | `home/esp32_relay_02/availability` | `online` |
| Relay 1 command | `home/esp32_relay_02/relay/1/set` | `ON` or `OFF` |
| Relay 1 state | `home/esp32_relay_02/relay/1/state` | `ON` or `OFF` |
| Relay 2 command | `home/esp32_relay_02/relay/2/set` | `ON` or `OFF` |
| Relay 2 state | `home/esp32_relay_02/relay/2/state` | `ON` or `OFF` |
| Relay 3 command | `home/esp32_relay_02/relay/3/set` | `ON` or `OFF` |
| Relay 3 state | `home/esp32_relay_02/relay/3/state` | `ON` or `OFF` |
| Electricity state | `home/esp32_relay_02/electricity/state` | `ON` or `OFF` |
| Water-level state | `home/esp32_relay_02/water_level/state` | `LOW`, `MIDDLE`, or `FULL` |
| Water-level percentage | `home/esp32_relay_02/water_level/percentage` | `0`, `50`, or `100` |

Discovery configurations are published as retained messages below `homeassistant/switch/.../config`, `homeassistant/binary_sensor/.../config`, and `homeassistant/sensor/.../config`.

## Troubleshooting

### Device does not appear in Home Assistant

- Confirm Serial Monitor prints `MQTT Connected`.
- Confirm the MQTT broker IP and port are correct.
- Confirm the username and password match the broker account.
- Confirm the ESP32 and Home Assistant can reach each other on the LAN.
- Confirm MQTT Discovery is enabled and its prefix is `homeassistant`.
- Restart the ESP32 to republish retained discovery messages.
- Look for `FAILED to publish discovery` in Serial Monitor.

The firmware sets the PubSubClient buffer to 768 bytes because Home Assistant discovery JSON does not fit in the library's default 256-byte buffer.

### Only three relay entities appear

Flash the latest firmware and restart the ESP32. The latest version publishes a fourth `binary_sensor` discovery message for **Electricity Supply** and reports whether each discovery publication succeeds.

### Electricity always reports OFF

- Verify the detector's safe output is connected to GPIO 25 and shares the required signal ground/reference with the ESP32 side of the isolated circuit.
- Confirm the detector produces falling-edge pulses when AC is present.
- Observe Serial Monitor for `Electricity: ON`.
- If the valid detector frequency is different, tune the `AC_*` settings in `include/config.h`.

### Electricity changes ON/OFF unexpectedly

- Do not leave GPIO 25 connected to a long unshielded wire.
- Verify the detector output has a clean 3.3 V-compatible logic level.
- Keep low-voltage signal wiring away from relay contacts and mains conductors.
- Increase `AC_ON_CONFIRM_CHECKS` or `AC_OFF_CONFIRM_CHECKS` if additional filtering is needed.

### MQTT repeatedly reconnects

- Check that Mosquitto is running and port 1883 is reachable.
- Check the broker logs for authentication failures.
- Ensure every controller has a different `DEVICE_ID_SUFFIX`; duplicate suffixes produce duplicate MQTT client IDs.

### Water level is incorrect or unstable

- Confirm the float-switch signals are 3.3 V compatible and share the required ground/reference with the ESP32.
- Confirm Float 1 is connected to GPIO 12 and Float 2 to GPIO 13.
- Add suitable external pull-up or pull-down resistors; the firmware does not enable internal pulls for these inputs.
- Open `/api/water_level` to inspect the raw `float1` through `float4` readings.
- Confirm the switches produce HIGH when active. If the hardware is active LOW, the input logic must be inverted in the firmware.

### Dashboard does not open

- Read the current ESP32 address from Serial Monitor.
- Make sure the browser is on a network that can reach the ESP32.
- Reserve the ESP32 address in the router if a stable URL is required.

## Source layout

```text
include/
  ac_monitor.h
  config.h
  mqtt_manager.h
  relay_manager.h
  water_level.h
  web_server.h
  wifi_manager.h
src/
  ac_monitor.cpp
  main.cpp
  mqtt_manager.cpp
  relay_manager.cpp
  water_level.cpp
  web_server.cpp
  wifi_manager.cpp
platformio.ini
```

## Security notes

- Do not expose the web dashboard, OTA endpoint, or MQTT port directly to the internet.
- Use trusted local network access or a VPN for remote access.
- Use a dedicated MQTT account with a strong password.
- Remove real credentials before publishing or sharing the repository.
- The current web and MQTT connections are unencrypted LAN connections; protect the network accordingly.

