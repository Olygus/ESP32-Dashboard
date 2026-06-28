---
tags: [esp32, bluetooth, linux, dashboard, oled, system-monitor, cpp, python]
---

# ESP32-Dashboard

An ESP32-based display system that communicates with Linux-based laptops over Bluetooth to show system statistics. The device receives periodic status updates from the host machine and displays the information across two distinct OLED screens.

## Table of Contents

1. [Hardware Requirements](#hardware-requirements)
2. [Key Features & Architecture](#key-features--architecture)
3. [Repository Structure](#repository-structure)
4. [Installation & Setup](#installation--setup)
5. [Data Payload Format](#data-payload-format)

## Hardware Requirements

* **Microcontroller**: ESP32 DevKit V1 (flashed with C++).
* **Displays**: One 128x64 OLED using the `sh1107` library and one 128x32 OLED using the `SSD1306` library.
* **Inputs**: 2 physical buttons for Wake and Page Cycle functionality.
* **Misc**: Connecting wires, an optional breadboard, double-sided tape, and a micro-USB battery.

## Key Features & Architecture

* **Dual I2C Buses**: Avoids display conflicts by explicitly passing hardware identifier instances (`0` and `1`) during object construction, separating the main and stylus display buses. 
* **Instant Wake**: Utilizes `esp_light_sleep_start()` instead of deep sleep. This keeps the internal RAM and BLE connection active while saving power, allowing the display to wake and update instantly on button press.
* **Readable Payloads**: Drops compressed binary frames in favor of raw, human-readable JSON packages for easier debugging and data transfer.
* **Linux Host Daemon**: The backend runs as a background systemd service using a Python Bleak monitor to grab battery, uptime, and stylus data.

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

To avoid externally-managed-environment errors on modern Linux distributions (like Arch Linux), the host script runs inside an isolated virtual environment.

```bash
# Create application directory and copy service files
mkdir -p ~/.local/share/esp32-status
cp -r linux-service/* ~/.local/share/esp32-status/

# Create venv and install dependencies
python3 -m venv ~/.local/share/esp32-status/venv
~/.local/share/esp32-status/venv/bin/pip install bleak

```

### 3. Enable the systemd Service

Configure the daemon to run automatically in the background for your user session.

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

Data is transferred from the Linux machine to the ESP32 formatted as a standard JSON string.

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
