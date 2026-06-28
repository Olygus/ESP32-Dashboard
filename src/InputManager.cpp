#include "InputManager.h"
#include "Config.h"

void InputManager::begin() {
    pinMode(BTN1_PIN, INPUT_PULLUP);
    pinMode(BTN2_PIN, INPUT_PULLUP);
    _last1 = digitalRead(BTN1_PIN);
    _last2 = digitalRead(BTN2_PIN);
    Serial.printf("[Input] Buttons ready on GPIO %d and %d.\n", BTN1_PIN, BTN2_PIN);
}

void InputManager::loop() {
    unsigned long now = millis();

    bool b1 = digitalRead(BTN1_PIN);
    if (b1 != _last1 && (now - _debounce1Ms) > DEBOUNCE_MS) {
        _debounce1Ms = now;
        if (b1 == LOW && _cb1) _cb1();
        _last1 = b1;
    }

    bool b2 = digitalRead(BTN2_PIN);
    if (b2 != _last2 && (now - _debounce2Ms) > DEBOUNCE_MS) {
        _debounce2Ms = now;
        if (b2 == LOW && _cb2) _cb2();
        _last2 = b2;
    }
}
