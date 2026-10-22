# Changelog

All notable changes to this project are documented in this file.

## 2026-10-22

### Added

- Added active-HIGH float-switch inputs on GPIO 12, 13, 14, and 27.
- Added water-level calculation using Float 1 and Float 2:
  - `FULL` and 100% when both inputs are HIGH.
  - `MIDDLE` and 50% when Float 1 is LOW and Float 2 is HIGH.
  - `LOW` and 0% for the remaining combinations.
- Added the `/api/water_level` JSON endpoint with calculated level, percentage, and four raw float states.
- Added retained MQTT water-level state and percentage topics.
- Added Home Assistant MQTT Discovery sensors for water-level state and percentage.
- Added a water-level card to the local dashboard.
- Added a **Back to Dashboard** button to the OTA page.

### Changed

- Redesigned the OTA update page to match the main dashboard's dark interface.
- Improved the OTA file selector, upload button, progress display, and status log.
- Limited the OTA file picker to `.bin` files.
- Declared UTF-8 explicitly in the dashboard and OTA pages.
- Replaced the dashboard's em dash separator with an ASCII hyphen to prevent encoding artifacts.

### Verification

- Successfully built the firmware for the PlatformIO `esp32dev` environment.
