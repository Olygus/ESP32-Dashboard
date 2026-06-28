#include "StatusManager.h"
#include <ArduinoJson.h>
#include <Preferences.h>

static const char* NVS_NS  = "status";
static const char* NVS_KEY = "cached";

// ============================================================
// updateFromJson — parses the packet sent by esp32_status.py.
// JSON keys must match serializer.py field names exactly.
// NOTE: incoming payload must be ≤ 511 bytes (gDataBuf limit).
// ============================================================
bool StatusManager::updateFromJson(const char* json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.printf("[Status] JSON parse error: %s\n", err.c_str());
        return false;
    }

    strlcpy(_data.date,             doc["date"]             | "", sizeof(_data.date));
    strlcpy(_data.time,             doc["time"]             | "", sizeof(_data.time));
    strlcpy(_data.uptime,           doc["uptime"]           | "", sizeof(_data.uptime));
    strlcpy(_data.batteryRemaining, doc["batteryRemaining"] | "", sizeof(_data.batteryRemaining));

    _data.batteryPercent = doc["batteryPercent"] | 0;
    _data.stylusPercent  = doc["stylusPercent"]  | -1;
    _data.receivedAtMs   = millis();
    _data.valid          = true;
    return true;
}

// ============================================================
// NVS persistence — survives reboots and power loss.
// ============================================================
bool StatusManager::loadFromNVS() {
    Preferences prefs;
    if (!prefs.begin(NVS_NS, /*readOnly=*/true)) return false;
    String cached = prefs.getString(NVS_KEY, "");
    prefs.end();
    if (cached.isEmpty()) return false;
    return updateFromJson(cached.c_str());
}

void StatusManager::saveToNVS() {
    if (!_data.valid) return;

    JsonDocument doc;
    doc["date"]             = _data.date;
    doc["time"]             = _data.time;
    doc["uptime"]           = _data.uptime;
    doc["batteryPercent"]   = _data.batteryPercent;
    doc["batteryRemaining"] = _data.batteryRemaining;
    doc["stylusPercent"]    = _data.stylusPercent;

    String out;
    serializeJson(doc, out);

    Preferences prefs;
    if (prefs.begin(NVS_NS, /*readOnly=*/false)) {
        prefs.putString(NVS_KEY, out);
        prefs.end();
        Serial.println("[Status] Saved to NVS.");
    }
}
