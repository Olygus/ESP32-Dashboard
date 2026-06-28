#include "SystemInfoPage.h"

void SystemInfoPage::render(U8G2& d, const StatusData& s) {
    d.clearBuffer();

    d.setFont(u8g2_font_7x14_tf);
    d.drawStr(0, 13, "System Info");

    d.setFont(u8g2_font_6x10_tf);

    char bat[24];
    snprintf(bat, sizeof(bat), "Battery:   %d%%", s.batteryPercent);
    d.drawStr(0, 29, bat);

    char rem[28];
    snprintf(rem, sizeof(rem), "Remaining: %s", s.batteryRemaining);
    d.drawStr(0, 41, rem);

    char age[28];
    if (s.receivedAtMs > 0) {
        unsigned long sec = (millis() - s.receivedAtMs) / 1000UL;
        if      (sec < 60)   snprintf(age, sizeof(age), "Updated:   %lus ago", sec);
        else if (sec < 3600) snprintf(age, sizeof(age), "Updated:   %lum ago", sec / 60);
        else                 snprintf(age, sizeof(age), "Updated:   %luh ago", sec / 3600);
    } else {
        snprintf(age, sizeof(age), "Updated:   never");
    }
    d.drawStr(0, 53, age);

    d.sendBuffer();
}
