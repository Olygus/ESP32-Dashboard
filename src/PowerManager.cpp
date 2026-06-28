#include "PowerManager.h"
#include "Config.h"

void PowerManager::begin() {
    _lastWakeMs = millis();
    _asleep     = false;
    Serial.printf("[Power] Timeout: %lums.\n", DISPLAY_TIMEOUT_MS);
}

void PowerManager::resetTimeout() {
    if (_asleep) {
        _asleep = false;
        Serial.println("[Power] Waking displays.");
        if (_onWake) _onWake();
    }
    _lastWakeMs = millis();
}

void PowerManager::loop() {
    if (!_asleep && (millis() - _lastWakeMs) >= DISPLAY_TIMEOUT_MS) {
        _asleep = true;
        Serial.println("[Power] Timeout — displays sleeping.");
        if (_onSleep) _onSleep();
    }
}
