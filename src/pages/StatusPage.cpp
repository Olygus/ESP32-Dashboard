#include "StatusPage.h"
#include "AssetManager.h"

void StatusPage::render(U8G2& d, const StatusData& s) {
    d.clearBuffer();

    // Chi-Rho — top-left, 16×16
    d.drawXBMP(0, 0, CHI_RHO_W, CHI_RHO_H, CHI_RHO_BITS);

    // Date
    d.setFont(u8g2_font_6x10_tf);
    d.drawStr(20, 10, s.date);

    // Time (larger font)
    d.setFont(u8g2_font_7x14_tf);
    d.drawStr(20, 26, s.time);

    // Battery
    d.setFont(u8g2_font_6x10_tf);
    char bat[32];
    if (s.batteryRemaining[0] != '\0') {
        snprintf(bat, sizeof(bat), "Bat: %d%% (%s)", s.batteryPercent, s.batteryRemaining);
    } else {
        snprintf(bat, sizeof(bat), "Bat: %d%%", s.batteryPercent);
    }
    d.drawStr(0, 44, bat);

    // Uptime
    char up[36];
    snprintf(up, sizeof(up), "Up: %s", s.uptime);
    d.drawStr(0, 56, up);

    d.sendBuffer();
}
