#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <functional>

// ============================================================
// BLEManager — BLE peripheral role.
// Handles advertising, passkey pairing, bonding, and data RX.
// Callbacks fire on the BT task (Core 0); wire them to
// portMUX-protected flags in main.cpp.
// ============================================================
class BLEManager {
public:
    BLEManager();

    void begin();
    void loop();

    bool isConnected() const { return _connected; }

    // ---- Register callbacks (called from setup()) ----
    void setOnDataReceived    (std::function<void(const char*)> cb) { _onData    = cb; }
    void setOnConnectionChange(std::function<void(bool)>        cb) { _onConn    = cb; }
    void setOnPasskeyDisplay  (std::function<void(uint32_t)>    cb) { _onPasskey = cb; }
    void setOnPairingResult   (std::function<void(bool)>        cb) { _onPairing = cb; }

    // ---- Called by internal callback classes — do not call directly ----
    void _handleConnect();
    void _handleDisconnect();
    void _handleData(const char* json);
    void _handlePasskey(uint32_t passkey);
    void _handlePairingResult(bool success);

private:
    void _setupSecurity();
    void _startAdvertising();

    bool               _connected;
    BLEServer*         _server;
    BLEService*        _service;
    BLECharacteristic* _characteristic;

    std::function<void(bool)>        _onConn;
    std::function<void(const char*)> _onData;
    std::function<void(uint32_t)>    _onPasskey;
    std::function<void(bool)>        _onPairing;
};
