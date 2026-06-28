#include "ConnectionPage.h"
#include "Config.h"

void ConnectionPage::render(U8G2& d, const StatusData& s) {
    d.clearBuffer();

    d.setFont(u8g2_font_7x14_tf);
    d.drawStr(0, 13, "Connection");

    d.setFont(u8g2_font_6x10_tf);

    char conn[28];
    snprintf(conn, sizeof(conn), "BLE: %s",
             _ble.isConnected() ? "Connected" : "Disconnected");
    d.drawStr(0, 28, conn);

    char upd[28];
    if (s.receivedAtMs > 0) {
        unsigned long sec = (millis() - s.receivedAtMs) / 1000UL;
        if      (sec < 60)   snprintf(upd, sizeof(upd), "Last: %lus ago", sec);
        else if (sec < 3600) snprintf(upd, sizeof(upd), "Last: %lum ago", sec / 60);
        else                 snprintf(upd, sizeof(upd), "Last: %luh ago", sec / 3600);
    } else {
        snprintf(upd, sizeof(upd), "Last: never");
    }
    d.drawStr(0, 42, upd);

    d.drawStr(0, 56, "Dev: " BLE_DEVICE_NAME);

    d.sendBuffer();
}
