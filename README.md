# F411 IO Mod Project

![Build Status](https://github.com/kshypachov/f411_io_mod/actions/workflows/c-cpp.yml/badge.svg)

**F411 IO Mod** is firmware for the STM32F411CEUX microcontroller, designed for digital input/output control, network functionality, and MQTT communication with Home Assistant integration. The device supports over-the-air firmware and web interface updates, runs FreeRTOS for multitasking, and provides a REST API for remote management.

## Key Features

- **MQTT**: Data publishing and command reception, compatible with Home Assistant.
- **REST API**: Device control, configuration, updates, and log management.
- **FreeRTOS**: Tasks for IO, networking, settings, and display.
- **File System**: LittleFS for settings and web interface storage.

## Device Setup

1. Download from the [Releases](https://github.com/kshypachov/f411_io_mod/releases) section:
   - **`f411_io_mod_bootloader.hex`** — bootloader.
   - **`f411_io_mod.hex`** — main firmware.

2. Flash the bootloader:
   - Hold the **Boot** button and connect the board to USB.
   - Flash **`f411_io_mod_bootloader.hex`** using **STM32CubeProgrammer**.

3. Flash the main firmware:
   - Flash **`f411_io_mod.hex`** using **STM32CubeProgrammer**.

4. Reboot the device (**Reset** button):
   - The LED will blink 4 times, and the firmware version will be displayed on the OLED.
   - After connecting Ethernet, the device will acquire an IP address and display it.

## Uploading the Web Interface

1. Download:
   - **`web_interface.zip`** — web interface files.
   - **`web_interface_uploader_win.exe`** or **`web_interface_uploader_mac.app.zip`** depending on your OS.

2. Extract **web_interface.zip**.

3. Launch the uploader, enter the device IP (`http://<ip>`) and login as **admin/admin**.

4. Select the extracted web interface folder and click **Upload**.

## Quick Start

- The project is built using **STM32CubeIDE**.
- MQTT configuration is available via the web interface.
- Connect the device to your network for access.

## Web API

Full API documentation is available here: **[API.md](API.md)**

---

## FreeRTOS Tasks

- **Ethernet Task**: Manages network and MQTT.
- **IO Task**: Monitors inputs, controls outputs.
- **Settings Task**: Manages configuration.
- **Display Task**: Displays data (optional).

---

## License

MIT License — see [LICENSE](LICENSE).

## Contributions

PRs and suggestions are welcome. Feedback via [Issues](https://github.com/kshypachov/f411_io_mod/issues).

## Acknowledgments

- [STM32 HAL and CMSIS](https://www.st.com/)
- [FreeRTOS](https://www.freertos.org/)
- [Mongoose](https://www.cesanta.com/)
- [Home Assistant](https://www.home-assistant.io/)

