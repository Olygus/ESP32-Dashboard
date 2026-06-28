# ESP32 dashboard
<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-black?style=flat&logo=espressif&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![Python](https://img.shields.io/badge/Python-3776AB?style=flat&logo=python&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)
![Bluetooth](https://img.shields.io/badge/Bluetooth-0082FC?style=flat&logo=bluetooth&logoColor=white)
</div>
this is an ESP32 dashboard that communicates with Linux laptops over Bluetooth to show system statistics. The device receives periodic status updates from the laptop machine and displays the information across two OLED screens.

## Table of Contents

1. [Hardware Requirements](#hardware-requirements)
2. [Key Features & Architecture](#key-features--architecture)
3. [Repository Structure](#repository-structure)
4. [Installation & Setup](#installation--setup)
5. [Data Payload Format](#data-payload-format)

## Hardware Requirements

* **Microcontroller**: ESP32 DevKit V1 (flashed with C++).
* **Displays**: One 128x64 OLED using the `sh1107` library (if you are using an SSD1306 oled screen, make sure you change the libraries for it) and one 128x32 OLED using the `SSD1306` library.
* **Inputs**: 2 physical buttons for Wake and Page Cycle functionality.
* **Misc**: Connecting wires, an optional breadboard, double-sided tape, and a micro-USB battery.

## Key Features & Architecture

* **Dual I2C Buses**: Avoids display conflicts by explicitly passing hardware identifier instances (`0` and `1`) during object construction, separating the main and stylus display buses. 
* **Instant Wake**: Utilizes `esp_light_sleep_start()` instead of deep sleep. This keeps the internal RAM and BLE connection active while saving power, allowing the display to wake and update instantly on button press avoiding the tedious reconection.
* **Readable Payloads**: the laptop drops compressed binary frames in favor of raw, human readable JSON packages for easier debugging and data transfer. Shoutout to my boi Jason
* **Linux Host Daemon**: The backend runs as a background systemd service (I am happy to change this if anyone with the knowledge is willing to help me) using a Python Bleak monitor to grab battery, uptime, and stylus data.

## Repository Structure

```text
ESP32-Dashboard/
├── LICENSE
├── linux-service
│   ├── ble_client.py
│   ├── collectors
│   │   ├── battery.py
│   │   ├── stylus.py
│   │   └── uptime.py
│   ├── config.toml
│   ├── esp32_status.py
│   ├── esp32-status.service
│   └── serializer.py
├── platformio.ini
└── src
    ├── BLEManager.cpp
    ├── DisplayManager.cpp
    ├── InputManager.cpp
    ├── main.cpp
    └── pages/

```

## Installation & Setup

### 1. Flash the ESP32 Firmware

Ensure you have PlatformIO installed, navigate to your project directory, and upload the firmware to your board:

```bash
# Navigate to the project directory
cd ESP32-Dashboard

# Flash firmware via PlatformIO
pio run --target upload

```

### 2. Install the Python Host Service

To avoid any externally managed environment errors on most modern Linux distributions (like Arch Linux), the host script runs inside an isolated virtual environment so you are safer. (remember to disable this when you decide to stop using it because it will stay there indefinitely)

```bash
# Create application directory and copy service files
mkdir -p ~/.local/share/esp32-status
cp -r linux-service/* ~/.local/share/esp32-status/

# Create venv and install dependencies
python3 -m venv ~/.local/share/esp32-status/venv
~/.local/share/esp32-status/venv/bin/pip install bleak

```

### 3. Enable the systemd Service

Configure the daemon to run automatically in the background for your user session. (make sure to remember to kill it once you deice to stop using this)

```bash
# Copy systemd service unit file
mkdir -p ~/.config/systemd/user
cp linux-service/esp32-status.service ~/.config/systemd/user/

# Reload the daemon and enable the service
systemctl --user daemon-reload
systemctl --user enable --now esp32-status

# Verify it is running and watch the logs
journalctl --user -u esp32-status -f

```

## Data Payload Format

Data is transferred from the Linux machine to the ESP32 formatted as a standard JSON strings .

```json
{
  "date": "2026-06-27",
  "time": "13:15",
  "uptime": "3d 7h",
  "batteryPercent": 84,
  "batteryRemaining": "4h 22m",
  "stylusPercent": 61
}

```
