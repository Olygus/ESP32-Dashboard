#include "DisplayManager.h"
#include "BLEManager.h"
#include "Config.h"

// ============================================================
// Constructor — pin assignments come from Config.h.
// SW-I2C constructor: (rotation, clock, data, reset)
// ============================================================
DisplayManager::DisplayManager(PageManager& pages, StatusManager& status, BLEManager& ble)
    : _pages(pages), _status(status), _ble(ble),
      _dispMain   (U8G2_R0, I2C0_SCL, I2C0_SDA, U8X8_PIN_NONE),
      _dispStylus (U8G2_R0, I2C1_SCL, I2C1_SDA, U8X8_PIN_NONE)
{}

// ============================================================
// begin — initialise both displays independently.
// A failed display logs an error and is silently skipped;
// the system continues with whatever remains functional.
// ============================================================
bool DisplayManager::begin() {
    Serial.println("[Display] Initialising displays...");

    if (_dispMain.begin()) {
        _mainOk = true;
        _dispMain.setContrast(200);
        Serial.println("[Display] Main display (128×64) OK.");
    } else {
        Serial.println("[Display] WARNING: Main display (128×64) not found.");
    }

    if (_dispStylus.begin()) {
        _stylOk = true;
        _dispStylus.setContrast(200);
        Serial.println("[Display] Stylus display (128×32) OK.");
    } else {
        Serial.println("[Display] WARNING: Stylus display (128×32) not found.");
    }

    _on = _mainOk || _stylOk;
    return _on;
}

// ============================================================
// Power control — setPowerSave(1) blanks the OLED without
// erasing the buffer; setPowerSave(0) restores it.
// ============================================================
void DisplayManager::turnOff() {
    if (_mainOk)  _dispMain.setPowerSave(1);
    if (_stylOk)  _dispStylus.setPowerSave(1);
    _on = false;
}

void DisplayManager::turnOn() {
    if (_mainOk)  _dispMain.setPowerSave(0);
    if (_stylOk)  _dispStylus.setPowerSave(0);
    _on = true;
}

// ============================================================
// renderMainDisplay — delegates to the active Page.
// Falls back to a plain "no data" message if nothing has arrived.
// ============================================================
void DisplayManager::renderMainDisplay() {
    if (!_mainOk || !_on) return;

    const StatusData& s = _status.getStatus();

    // If we have no data and are not connected, show a connection hint.
    if (!s.valid && !_ble.isConnected()) {
        showWaitingScreen();
        return;
    }

    Page* page = _pages.activePage();
    if (page) page->render(_dispMain, s);
}

// ============================================================
// renderStylusDisplay — always shows stylus info; static layout.
// ============================================================
void DisplayManager::renderStylusDisplay() {
    if (!_stylOk || !_on) return;

    const StatusData& s = _status.getStatus();

    _dispStylus.clearBuffer();
    _dispStylus.setFont(u8g2_font_6x10_tf);

    // Top line
    char top[28];
    if (s.stylusPercent >= 0) {
        snprintf(top, sizeof(top), "Stylus Battery: %d%%", s.stylusPercent);
    } else {
        snprintf(top, sizeof(top), "Stylus: N/A");
    }
    _dispStylus.drawStr(0, 10, top);

    // Bottom section: horizontal bar graphic
    if (s.stylusPercent >= 0) {
        _drawStylusBar(s.stylusPercent);
    }

    _dispStylus.sendBuffer();
}

// ============================================================
// _drawStylusBar — stylus body + inline battery icon, 128×32.
// Uses the lower ~20 px of the display (top 12 px used by text).
// ============================================================
void DisplayManager::_drawStylusBar(int pct) {
    // Stylus body rectangle
    const int BODY_X = 2,  BODY_Y = 15, BODY_W = 96, BODY_H = 10;
    // Battery icon
    const int BAT_X  = 102, BAT_Y = 16, BAT_W = 18, BAT_H = 8;
    const int TIP_W  = 3;

    // Stylus outline
    _dispStylus.drawRFrame(BODY_X, BODY_Y, BODY_W, BODY_H, 3);

    // Battery icon outline
    _dispStylus.drawFrame(BAT_X, BAT_Y, BAT_W, BAT_H);
    // Battery positive tip
    _dispStylus.drawBox(BAT_X + BAT_W, BAT_Y + 2, TIP_W, BAT_H - 4);

    // Battery fill
    int fillW = ((BAT_W - 4) * constrain(pct, 0, 100)) / 100;
    if (fillW > 0) {
        _dispStylus.drawBox(BAT_X + 2, BAT_Y + 2, fillW, BAT_H - 4);
    }
}

// ============================================================
// Special-purpose main-display screens
// ============================================================
void DisplayManager::showPairingScreen(uint32_t passkey) {
    if (!_mainOk) return;
    _dispMain.clearBuffer();

    _dispMain.setFont(u8g2_font_7x14_tf);
    _dispMain.drawStr(4, 14, "Bluetooth Setup");

    _dispMain.setFont(u8g2_font_6x10_tf);
    _dispMain.drawStr(4, 30, "Waiting for pairing...");
    _dispMain.drawStr(4, 43, "Device: " BLE_DEVICE_NAME);

    char pk[28];
    snprintf(pk, sizeof(pk), "Passkey: %06lu", (unsigned long)passkey);
    _dispMain.drawStr(4, 57, pk);

    _dispMain.sendBuffer();
}

void DisplayManager::showPairingFailed() {
    if (!_mainOk) return;
    _dispMain.clearBuffer();

    _dispMain.setFont(u8g2_font_7x14_tf);
    _dispMain.drawStr(14, 24, "Pairing Failed");

    _dispMain.setFont(u8g2_font_6x10_tf);
    _dispMain.drawStr(32, 44, "Retrying...");

    _dispMain.sendBuffer();
}

void DisplayManager::showWaitingScreen() {
    if (!_mainOk) return;
    _dispMain.clearBuffer();

    _dispMain.setFont(u8g2_font_7x14_tf);
    _dispMain.drawStr(4, 18, "Bluetooth Setup");

    _dispMain.setFont(u8g2_font_6x10_tf);
    _dispMain.drawStr(4, 34, "Waiting for Pairing");
    _dispMain.drawStr(4, 46, "Device: " BLE_DEVICE_NAME);

    _dispMain.sendBuffer();
}
