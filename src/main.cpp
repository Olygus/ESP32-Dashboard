#include <Arduino.h>
#include <memory>

#include "Config.h"
#include "AssetManager.h"
#include "StatusManager.h"
#include "BLEManager.h"
#include "InputManager.h"
#include "PowerManager.h"
#include "PageManager.h"
#include "DisplayManager.h"
#include "pages/StatusPage.h"
#include "pages/SystemInfoPage.h"
#include "pages/ConnectionPage.h"

// ============================================================
// Module instances (initialised in declaration order)
// ============================================================
static StatusManager  statusMgr;
static BLEManager     bleMgr;
static InputManager   inputMgr;
static PowerManager   powerMgr;
static PageManager    pageMgr;
static DisplayManager displayMgr(pageMgr, statusMgr, bleMgr);

// ============================================================
// Cross-task event flags
//
// BLE callbacks run on Core 0 (BT task); loop() runs on Core 1.
// We use a portMUX critical section to safely hand data across.
// ============================================================
static portMUX_TYPE  gMux          = portMUX_INITIALIZER_UNLOCKED;

// --- Incoming JSON packet ---
static volatile bool gDataPending  = false;
static char          gDataBuf[512] = {};

// --- Connection state ---
static volatile bool gConnChanged  = false;
static volatile bool gNewConnState = false;

// --- Pairing ---
static volatile bool     gPasskeyPending = false;
static volatile uint32_t gPasskey        = 0;
static volatile bool     gPairingDone    = false;
static volatile bool     gPairingOk      = false;

// ---- UI state ----
static bool  uiPairing      = false;
static bool  uiPairingFailed = false;
static uint32_t uiPasskey   = 0;

// Render at 4 fps (250 ms) to reduce display wear.
static constexpr unsigned long RENDER_MS = 250;
static unsigned long lastRenderMs = 0;

// ============================================================
// Helpers — post events from BLE task into shared flags
// ============================================================
static void postData(const char* json) {
    portENTER_CRITICAL(&gMux);
    strncpy(gDataBuf, json, sizeof(gDataBuf) - 1);
    gDataBuf[sizeof(gDataBuf) - 1] = '\0';
    gDataPending = true;
    portEXIT_CRITICAL(&gMux);
}

static void postConnChange(bool connected) {
    portENTER_CRITICAL(&gMux);
    gNewConnState = connected;
    gConnChanged  = true;
    portEXIT_CRITICAL(&gMux);
}

static void postPasskey(uint32_t pk) {
    portENTER_CRITICAL(&gMux);
    gPasskey        = pk;
    gPasskeyPending = true;
    portEXIT_CRITICAL(&gMux);
}

static void postPairingResult(bool ok) {
    portENTER_CRITICAL(&gMux);
    gPairingOk   = ok;
    gPairingDone = true;
    portEXIT_CRITICAL(&gMux);
}

// ============================================================
// processEvents — called from loop() to drain the flag queue
// ============================================================
static void processEvents() {
    // --- New JSON packet ---
    if (gDataPending) {
        char tmp[512];
        portENTER_CRITICAL(&gMux);
        strncpy(tmp, gDataBuf, sizeof(tmp));
        gDataPending = false;
        portEXIT_CRITICAL(&gMux);

        if (statusMgr.updateFromJson(tmp)) {
            statusMgr.saveToNVS();
            powerMgr.resetTimeout();   // Incoming data wakes the displays
            uiPairing      = false;
            uiPairingFailed = false;
        }
    }

    // --- Connection state change ---
    if (gConnChanged) {
        bool conn;
        portENTER_CRITICAL(&gMux);
        conn         = gNewConnState;
        gConnChanged = false;
        portEXIT_CRITICAL(&gMux);

        Serial.printf("[Main] BLE %s\n", conn ? "connected" : "disconnected");
        powerMgr.resetTimeout();
    }

    // --- Passkey to display ---
    if (gPasskeyPending) {
        portENTER_CRITICAL(&gMux);
        uiPasskey        = gPasskey;
        gPasskeyPending  = false;
        portEXIT_CRITICAL(&gMux);

        uiPairing       = true;
        uiPairingFailed = false;
        powerMgr.resetTimeout();
        displayMgr.showPairingScreen(uiPasskey);
    }

    // --- Pairing result ---
    if (gPairingDone) {
        bool ok;
        portENTER_CRITICAL(&gMux);
        ok           = gPairingOk;
        gPairingDone = false;
        portEXIT_CRITICAL(&gMux);

        uiPairing = false;
        if (!ok) {
            uiPairingFailed = true;
            displayMgr.showPairingFailed();
        } else {
            uiPairingFailed = false;
        }
        powerMgr.resetTimeout();
    }
}

// ============================================================
// renderCycle — decides which content to show on each display
// ============================================================
static void renderCycle() {
    if (!displayMgr.isOn()) return;

    if (uiPairing) {
        displayMgr.showPairingScreen(uiPasskey);
        // Stylus display still shows stylus info during pairing.
        displayMgr.renderStylusDisplay();
        return;
    }
    if (uiPairingFailed) {
        displayMgr.showPairingFailed();
        displayMgr.renderStylusDisplay();
        return;
    }

    displayMgr.renderMainDisplay();
    displayMgr.renderStylusDisplay();
}

// ============================================================
// setup
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[Main] ESP32 Status Display booting...");

    // ---- 1. Load cached status from NVS ----
    if (statusMgr.loadFromNVS()) {
        Serial.println("[Main] Cached status loaded.");
    }

    // ---- 2. Initialise displays ----
    if (!displayMgr.begin()) {
        Serial.println("[Main] WARNING: No displays found. Continuing headlessly.");
    }

    // ---- 3. Register pages (add new pages here; nothing else changes) ----
    pageMgr.registerPage(std::make_unique<StatusPage>());
    pageMgr.registerPage(std::make_unique<SystemInfoPage>());
    pageMgr.registerPage(std::make_unique<ConnectionPage>(bleMgr));

    // ---- 4. Input ----
    inputMgr.begin();

    inputMgr.setButton1Callback([]() {
        // Button 1: wake displays and reset the timeout.
        powerMgr.resetTimeout();
        Serial.println("[Input] Button 1 — wake.");
    });

    inputMgr.setButton2Callback([]() {
        // Button 2: cycle pages and reset the timeout.
        powerMgr.resetTimeout();
        pageMgr.nextPage();
        Serial.println("[Input] Button 2 — next page.");
    });

    // ---- 5. Power management ----
    powerMgr.begin();
    powerMgr.setOnSleep([]() { displayMgr.turnOff(); });
    powerMgr.setOnWake ([]() { displayMgr.turnOn();  });

    // ---- 6. BLE ----
    bleMgr.setOnDataReceived    (postData);
    bleMgr.setOnConnectionChange(postConnChange);
    bleMgr.setOnPasskeyDisplay  (postPasskey);
    bleMgr.setOnPairingResult   (postPairingResult);
    bleMgr.begin();

    // ---- 7. Show initial screen ----
    displayMgr.showWaitingScreen();

    Serial.println("[Main] Boot complete.\n");
}

// ============================================================
// loop
// ============================================================
void loop() {
    bleMgr.loop();
    inputMgr.loop();
    powerMgr.loop();

    processEvents();

    unsigned long now = millis();
    if (now - lastRenderMs >= RENDER_MS) {
        lastRenderMs = now;
        renderCycle();
    }

    delay(10);
}
