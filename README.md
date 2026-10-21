# ESP32 Relay Controller 02

ESP32 firmware for controlling three relays, monitoring the presence of AC electricity, providing a local web dashboard, supporting browser-based OTA updates, and integrating automatically with Home Assistant through MQTT Discovery.

This project is configured as the **second controller** in the installation. Its Home Assistant device ID, MQTT client ID, discovery IDs, and MQTT topics are different from device 01, so the two devices do not overwrite each other.

## Features

- Three independently controlled relays
- Local responsive web dashboard
- AC electricity ON/cut-off detection on GPIO 25
- Noise filtering and three-second state confirmation
- JSON HTTP APIs for relay and electricity states
- MQTT relay control and retained state publication
- Home Assistant MQTT Discovery
- Separate Home Assistant electricity binary sensor
- Browser-based OTA firmware updates
- Automatic Wi-Fi and MQTT reconnection

## Hardware

| Function | ESP32 pin | Direction | Notes |
| --- | ---: | --- | --- |
| Relay 1 | GPIO 5 | Output | Active HIGH |
| Relay 2 | GPIO 18 | Output | Active HIGH |
| Relay 3 | GPIO 19 | Output | Active HIGH |
| AC detector `H_PF` | GPIO 25 | Input | Falling-edge pulse input with internal pull-up |

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

The dashboard displays the electricity supply independently from the three relays. It also provides controls for every relay and a link to the OTA update page.

### Local HTTP API

| Method | Endpoint | Purpose |
| --- | --- | --- |
| GET | `/api/relay_states` | Returns the three relay states as a JSON array |
| GET | `/api/electricity_status` | Returns `{"electricity_on":true}` or `false` |
| GET | `/api/relay/1/on` | Turns relay 1 on |
| GET | `/api/relay/1/off` | Turns relay 1 off |
| GET | `/api/relay/2/on` | Turns relay 2 on |
| GET | `/api/relay/2/off` | Turns relay 2 off |
| GET | `/api/relay/3/on` | Turns relay 3 on |
| GET | `/api/relay/3/off` | Turns relay 3 off |
| GET | `/update` | Opens the browser OTA page |

## OTA firmware update

1. Build the project and locate `.pio/build/esp32dev/firmware.bin`.
2. Open `http://ESP32_IP/update`.
3. Select `firmware.bin` and start the upload.
4. Keep the ESP32 powered and connected until it reports success and reboots.

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
```

Open **Settings > Devices & services > MQTT > Devices**. The device should appear as **ESP32 Relay Controller 02** with four entities:

- Relay 1
- Relay 2
- Relay 3
- Electricity Supply

To display electricity on a dashboard, edit the dashboard, add a **Tile** or **Entities** card, and select **Electricity Supply**.

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

Discovery configurations are published as retained messages below `homeassistant/switch/.../config` and `homeassistant/binary_sensor/.../config`.

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
  web_server.h
  wifi_manager.h
src/
  ac_monitor.cpp
  main.cpp
  mqtt_manager.cpp
  relay_manager.cpp
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

