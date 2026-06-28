#pragma once
#include "Page.h"
#include "BLEManager.h"

// Page 3 — BLE state · Last update age · Device name
class ConnectionPage : public Page {
public:
    explicit ConnectionPage(BLEManager& ble) : _ble(ble) {}
    void        render(U8G2& display, const StatusData& status) override;
    const char* name() const override { return "Connection"; }
private:
    BLEManager& _ble;
};
