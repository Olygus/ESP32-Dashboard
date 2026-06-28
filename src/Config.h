#pragma once

// ============================================================
// BLE — must match linux-service/config.toml exactly
// ============================================================
#define BLE_DEVICE_NAME         "ESP32-Status"
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_MTU                 512

// ============================================================
// I2C bus 0 — Main display (128×64 SH1106)
// SW-I2C constructor order: (rotation, clock, data, reset)
// ============================================================
#define I2C0_SCL  22
#define I2C0_SDA  21

// ============================================================
// I2C bus 1 — Stylus display (128×32 SSD1306)
// ============================================================
#define I2C1_SCL  17
#define I2C1_SDA  16

// ============================================================
// Buttons (active-LOW via INPUT_PULLUP)
// ============================================================
#define BTN1_PIN  12   // Wake / reset timeout
#define BTN2_PIN  13   // Cycle pages / reset timeout

// ============================================================
// Power management
// ============================================================
#define DISPLAY_TIMEOUT_MS  10000UL   // 10 s inactivity → OLEDs off
