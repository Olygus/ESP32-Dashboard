#pragma once
#include <Arduino.h>

// ============================================================
// StatusData — populated from JSON sent by esp32_status.py.
// Persisted to NVS so cached values survive reboots.
// ============================================================
struct StatusData {
    bool valid              = false;

    char date[16]           = {};   // e.g. "2024-01-15"
    char time[10]           = {};   // e.g. "14:32"
    char uptime[32]         = {};   // e.g. "3d 4h 22m"
    int  batteryPercent     = 0;
    char batteryRemaining[20] = {}; // e.g. "2:30"
    int  stylusPercent      = -1;   // -1 = unknown / N/A

    unsigned long receivedAtMs = 0; // millis() when last packet arrived
};

class StatusManager {
public:
    bool              updateFromJson(const char* json);
    bool              loadFromNVS();
    void              saveToNVS();
    const StatusData& getStatus() const { return _data; }

private:
    StatusData _data;
};
