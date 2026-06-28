#pragma once
#include <Arduino.h>
#include <functional>

// ============================================================
// InputManager — two-button debouncer.
// Buttons are active-LOW (INPUT_PULLUP).
// A falling edge fires the registered callback.
// ============================================================
class InputManager {
public:
    void begin();
    void loop();

    void setButton1Callback(std::function<void()> cb) { _cb1 = cb; }
    void setButton2Callback(std::function<void()> cb) { _cb2 = cb; }

private:
    std::function<void()> _cb1;
    std::function<void()> _cb2;

    bool          _last1       = HIGH;
    bool          _last2       = HIGH;
    unsigned long _debounce1Ms = 0;
    unsigned long _debounce2Ms = 0;

    static constexpr unsigned long DEBOUNCE_MS = 50;
};
