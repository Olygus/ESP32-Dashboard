#pragma once
#include <Arduino.h>
#include <functional>

// ============================================================
// PowerManager — display sleep/wake timeout.
// After DISPLAY_TIMEOUT_MS of inactivity, calls onSleep.
// resetTimeout() wakes the display and restarts the timer.
// ============================================================
class PowerManager {
public:
    void begin();
    void loop();
    void resetTimeout();

    void setOnSleep(std::function<void()> cb) { _onSleep = cb; }
    void setOnWake (std::function<void()> cb) { _onWake  = cb; }

    bool isAsleep() const { return _asleep; }

private:
    std::function<void()> _onSleep;
    std::function<void()> _onWake;

    bool          _asleep     = false;
    unsigned long _lastWakeMs = 0;
};
