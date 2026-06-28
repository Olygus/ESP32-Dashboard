#pragma once
#include <U8g2lib.h>
#include "PageManager.h"
#include "StatusManager.h"

class BLEManager;  // forward declaration — avoids circular include

// ============================================================
// DisplayManager — drives two independent I2C OLED panels.
//
//   Main    (128×64 SH1106)  — page-based content
//   Stylus  (128×32 SSD1306) — static stylus battery bar
//
// If your 128×64 panel is SSD1306 (not SH1106), change
// _dispMain's type to U8G2_SSD1306_128X64_NONAME_F_SW_I2C.
// ============================================================
class DisplayManager {
public:
    DisplayManager(PageManager& pages, StatusManager& status, BLEManager& ble);

    bool begin();
    bool isOn() const { return _on; }

    void turnOn();
    void turnOff();

    void renderMainDisplay();
    void renderStylusDisplay();

    void showPairingScreen(uint32_t passkey);
    void showPairingFailed();
    void showWaitingScreen();

private:
    void _drawStylusBar(int pct);

    PageManager&   _pages;
    StatusManager& _status;
    BLEManager&    _ble;

    U8G2_SH1106_128X64_NONAME_F_SW_I2C    _dispMain;
    U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C _dispStylus;

    bool _mainOk = false;
    bool _stylOk = false;
    bool _on     = false;
};
