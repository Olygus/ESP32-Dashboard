#include <Arduino.h>
#include "BLEManager.h"
#include <BLESecurity.h>
#include "Config.h"

// ============================================================
// Static back-pointer so callback classes can reach the manager.
// ============================================================
static BLEManager* gBLE = nullptr;

// ============================================================
// Server connection callbacks
// ============================================================
class _ServerCB : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        if (gBLE) gBLE->_handleConnect();
    }
    void onDisconnect(BLEServer*) override {
        if (gBLE) gBLE->_handleDisconnect();
    }
};

// ============================================================
// Characteristic write callback
// ============================================================
class _CharCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* ch) override {
        if (!gBLE) return;
        std::string v = ch->getValue();
        if (!v.empty()) gBLE->_handleData(v.c_str());
    }
};

// ============================================================
// Security / pairing callbacks
// ============================================================
class _SecCB : public BLESecurityCallbacks {
    // Called when the remote requests a passkey (Passkey Entry, we type side).
    // Since our IO cap is OUT (display), this shouldn't fire in normal pairing,
    // but we implement it defensively.
    uint32_t onPassKeyRequest() override {
        Serial.println("[BLE] onPassKeyRequest (unexpected for display-only IO cap).");
        return 0;
    }

    // Called with the passkey we must DISPLAY so the user can confirm on Linux.
    void onPassKeyNotify(uint32_t pass_key) override {
        Serial.printf("[BLE] Display passkey: %06lu\n", (unsigned long)pass_key);
        if (gBLE) gBLE->_handlePasskey(pass_key);
    }

    // Numeric Comparison — we auto-confirm (user confirms on the Linux side).
    bool onConfirmPIN(uint32_t pass_key) override {
        Serial.printf("[BLE] Numeric comparison passkey: %06lu — auto-confirming.\n",
                      (unsigned long)pass_key);
        if (gBLE) gBLE->_handlePasskey(pass_key);
        return true;
    }

    bool onSecurityRequest() override { return true; }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) override {
        bool ok = cmpl.success;
        Serial.printf("[BLE] Auth complete: %s\n", ok ? "BONDED OK" : "FAILED");
        if (!ok) {
            // Restart advertising so the user can retry.
            BLEDevice::startAdvertising();
        }
        if (gBLE) gBLE->_handlePairingResult(ok);
    }
};

// ============================================================
// BLEManager
// ============================================================
BLEManager::BLEManager()
    : _connected(false),
      _server(nullptr), _service(nullptr), _characteristic(nullptr) {
    gBLE = this;
}

void BLEManager::begin() {
    BLEDevice::init(BLE_DEVICE_NAME);
    BLEDevice::setMTU(BLE_MTU);

    _setupSecurity();

    _server = BLEDevice::createServer();
    _server->setCallbacks(new _ServerCB());

    _service = _server->createService(BLE_SERVICE_UUID);

    // The laptop writes JSON to this characteristic.
    _characteristic = _service->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _characteristic->setCallbacks(new _CharCB());

    _service->start();
    _startAdvertising();

    Serial.println("[BLE] Started — advertising as: " BLE_DEVICE_NAME);
}

void BLEManager::_setupSecurity() {
    BLEDevice::setSecurityCallbacks(new _SecCB());

    BLESecurity* sec = new BLESecurity();
    // Require Secure Connections + MITM protection + bond storage in NVS.
    sec->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    // Display Only: we show the passkey; the user confirms on the Linux side.
    sec->setCapability(ESP_IO_CAP_OUT);
    sec->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    sec->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

void BLEManager::_startAdvertising() {
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising started.");
}

void BLEManager::loop() {
    // Placeholder for future polling needs (e.g. watchdog, RSSI).
}

// ---- Internal event dispatchers ----
void BLEManager::_handleConnect() {
    _connected = true;
    Serial.println("[BLE] Device connected.");
    if (_onConn) _onConn(true);
}

void BLEManager::_handleDisconnect() {
    _connected = false;
    Serial.println("[BLE] Device disconnected — restarting advertising.");
    if (_onConn) _onConn(false);
    _startAdvertising();
}

void BLEManager::_handleData(const char* json) {
    Serial.printf("[BLE] Received %u bytes.\n", strlen(json));
    if (_onData) _onData(json);
}

void BLEManager::_handlePasskey(uint32_t passkey) {
    if (_onPasskey) _onPasskey(passkey);
}

void BLEManager::_handlePairingResult(bool success) {
    if (_onPairing) _onPairing(success);
}
