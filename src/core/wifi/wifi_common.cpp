#include "core/wifi/wifi_common.h"
#include "core/display.h"    // using displayRedStripe  and loop options
#include "core/mykeyboard.h" // usinf keyboard when calling rename
#include "core/powerSave.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/wifi/wifi_mac.h" // Set Mac Address - @IncursioHack
#include <esp_event.h>
#include <esp_netif.h>
#include <globals.h>
#include <time.h>

static TaskHandle_t timezoneTaskHandle = NULL;

namespace {
constexpr uint32_t kNtpPollIntervalMs = 10000;      // Monitor connectivity without busy polling.
constexpr uint32_t kNtpResyncIntervalMs = 1800000;  // Periodic drift correction while online.
constexpr uint32_t kNtpConnectDelayMs = 5000;       // Let WiFi settle before first NTP request.
portMUX_TYPE timezoneTaskMux = portMUX_INITIALIZER_UNLOCKED;
bool timezoneTaskStarting = false;

void logWifiTime(const char *prefix) {
    const time_t nowEpoch = time(nullptr);
    if (nowEpoch <= 0) {
        Serial.printf("%s time unavailable (epoch=%lld)\n", prefix, static_cast<long long>(nowEpoch));
        return;
    }

    struct tm localNow = {};
    localtime_r(&nowEpoch, &localNow);

    char timeBuf[40] = {0};
    if (strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S %Z", &localNow) == 0) {
        snprintf(
            timeBuf,
            sizeof(timeBuf),
            "%04d-%02d-%02d %02d:%02d:%02d",
            localNow.tm_year + 1900,
            localNow.tm_mon + 1,
            localNow.tm_mday,
            localNow.tm_hour,
            localNow.tm_min,
            localNow.tm_sec
        );
    }

    Serial.printf("%s %s\n", prefix, timeBuf);
}

void ensureTimezoneTaskRunning() {
    portENTER_CRITICAL(&timezoneTaskMux);
    const bool alreadyRunning = (timezoneTaskHandle != NULL) || timezoneTaskStarting;
    if (!alreadyRunning) { timezoneTaskStarting = true; }
    portEXIT_CRITICAL(&timezoneTaskMux);

    if (alreadyRunning) return;

    TaskHandle_t createdHandle = NULL;
    BaseType_t taskCreated = xTaskCreate(updateTimezoneTask, "updateTimezone", 4096, NULL, 1, &createdHandle);

    portENTER_CRITICAL(&timezoneTaskMux);
    timezoneTaskStarting = false;
    if (taskCreated == pdPASS) { timezoneTaskHandle = createdHandle; }
    portEXIT_CRITICAL(&timezoneTaskMux);
}
} // namespace

void ensureWifiPlatform() {
    static bool netifInitialized = false;
    static bool eventLoopCreated = false;
    static portMUX_TYPE platformMux = portMUX_INITIALIZER_UNLOCKED;

    portENTER_CRITICAL(&platformMux);
    bool needNetif = !netifInitialized;
    bool needLoop = !eventLoopCreated;
    portEXIT_CRITICAL(&platformMux);

    if (needNetif) {
        ESP_ERROR_CHECK(esp_netif_init());
        portENTER_CRITICAL(&platformMux);
        netifInitialized = true;
        portEXIT_CRITICAL(&platformMux);
    }

    if (needLoop) {
        esp_err_t err = esp_event_loop_create_default();
        if (err != ESP_ERR_INVALID_STATE) { ESP_ERROR_CHECK(err); }
        portENTER_CRITICAL(&platformMux);
        eventLoopCreated = true;
        portEXIT_CRITICAL(&platformMux);
    }
}

bool _wifiConnect(const String &ssid, int encryption) {
    String password = bruceConfig.getWifiPassword(ssid);
    if (password == "" && encryption > 0) { password = keyboard(password, 63, "Network Password:", true); }
    bool connected = _connectToWifiNetwork(ssid, password);
    bool retry = false;

    while (!connected) {
        wakeUpScreen();

        options = {
            {"Retry",  [&]() { retry = true; } },
            {"Cancel", [&]() { retry = false; }},
        };
        loopOptions(options);

        if (!retry) {
            wifiDisconnect();
            return false;
        }

        password = keyboard(password, 63, "Network Password:", true);
        connected = _connectToWifiNetwork(ssid, password);
    }

    if (connected) {
        wifiConnected = true;
        wifiIP = WiFi.localIP().toString();
        bruceConfig.addWifiCredential(ssid, password);

        if (bruceConfig.automaticTimeUpdateViaNTP) { updateClockTimezone(); }

        // Keep a lightweight background sync task alive while the device is running.
        ensureTimezoneTaskRunning();
    }

    delay(200);
    return connected;
}

bool _connectToWifiNetwork(const String &ssid, const String &pwd) {
    drawMainBorderWithTitle("WiFi Connect");
    padprintln("");
    padprint("Connecting to: " + ssid + ".");
    WiFi.mode(WIFI_MODE_STA);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    WiFi.begin(ssid, pwd);

    int i = 1;
    while (WiFi.status() != WL_CONNECTED) {
        if (tft.getCursorX() >= tftWidth - 12) {
            padprintln("");
            padprint("");
        }
#ifdef HAS_SCREEN
        tft.print(".");
#else
        Serial.print(".");
#endif

        if (i > 20) {
            displayError("Wifi Offline");
            vTaskDelay(500 / portTICK_RATE_MS);
            break;
        }

        vTaskDelay(500 / portTICK_RATE_MS);
        i++;
    }

    return WiFi.status() == WL_CONNECTED;
}

bool _setupAP() {
    IPAddress AP_GATEWAY(172, 0, 0, 1);
    WiFi.softAPConfig(AP_GATEWAY, AP_GATEWAY, IPAddress(255, 255, 255, 0));
    WiFi.softAP(bruceConfig.wifiAp.ssid, bruceConfig.wifiAp.pwd, 6, 0, 4, false);
    wifiIP = WiFi.softAPIP().toString(); // update global var
    Serial.println("IP: " + wifiIP);
    wifiConnected = true;
    return true;
}

void wifiDisconnect() {
    WiFi.softAPdisconnect(true); // turn off AP mode
    WiFi.disconnect(true, true); // turn off STA mode
    WiFi.mode(WIFI_OFF);         // enforces WIFI_OFF mode
    wifiConnected = false;
    returnToMenu = true;
}

bool wifiConnectMenu(wifi_mode_t mode) {
    if (WiFi.isConnected()) return false; // safeguard

    switch (mode) {
        case WIFI_AP: // access point
            WiFi.mode(WIFI_AP);
            return _setupAP();
            break;

        case WIFI_STA: { // station mode
            int nets;
            WiFi.mode(WIFI_MODE_STA);

            // wifiMACMenu();
            applyConfiguredMAC();

            bool refresh_scan = false;
            do {
                displayTextLine("Scanning..");
                nets = WiFi.scanNetworks();
                options = {};
                for (int i = 0; i < nets; i++) {
                    if (options.size() < 250) {
                        String ssid = WiFi.SSID(i);
                        int encryptionType = WiFi.encryptionType(i);
                        int32_t rssi = WiFi.RSSI(i);
                        int32_t ch = WiFi.channel(i);
                        // Check if the network is secured
                        String encryptionPrefix = (encryptionType == WIFI_AUTH_OPEN) ? "" : "#";
                        String encryptionTypeStr;
                        switch (encryptionType) {
                            case WIFI_AUTH_OPEN: encryptionTypeStr = "Open"; break;
                            case WIFI_AUTH_WEP: encryptionTypeStr = "WEP"; break;
                            case WIFI_AUTH_WPA_PSK: encryptionTypeStr = "WPA/PSK"; break;
                            case WIFI_AUTH_WPA2_PSK: encryptionTypeStr = "WPA2/PSK"; break;
                            case WIFI_AUTH_WPA_WPA2_PSK: encryptionTypeStr = "WPA/WPA2/PSK"; break;
                            case WIFI_AUTH_WPA2_ENTERPRISE: encryptionTypeStr = "WPA2/Enterprise"; break;
                            default: encryptionTypeStr = "Unknown"; break;
                        }

                        String optionText = encryptionPrefix + ssid + "(" + String(rssi) + "|" +
                                            encryptionTypeStr + "|ch." + String(ch) + ")";

                        options.push_back({optionText.c_str(), [=]() {
                                               _wifiConnect(ssid, encryptionType);
                                           }});
                    }
                }
                options.push_back({"Hidden SSID", [=]() {
                                       String __ssid = keyboard("", 32, "Your SSID");
                                       _wifiConnect(__ssid.c_str(), 8);
                                   }});
                addOptionToMainMenu();

                loopOptions(options);
                options.clear();

                if (check(EscPress)) {
                    refresh_scan = true;
                } else {
                    refresh_scan = false;
                }
            } while (refresh_scan);
        } break;

        case WIFI_AP_STA: // repeater mode
                          // _setupRepeater();
            break;

        default: // error handling
            Serial.println("Unknown wifi mode: " + String(mode));
            break;
    }

    if (returnToMenu) {
        wifiDisconnect(); // Forced turning off the wifi module if exiting back to the menu
        return false;
    }
    return wifiConnected;
}

void wifiConnectTask(void *pvParameters) {
    if (WiFi.status() == WL_CONNECTED) return;

    WiFi.mode(WIFI_MODE_STA);
    int nets = WiFi.scanNetworks();
    String ssid;
    String pwd;

    for (int i = 0; i < nets; i++) {
        ssid = WiFi.SSID(i);
        pwd = bruceConfig.getWifiPassword(ssid);
        if (pwd == "") continue;

        WiFi.begin(ssid, pwd);
        for (int i = 0; i < 50; i++) {
            if (WiFi.status() == WL_CONNECTED) {
                wifiConnected = true;
                wifiIP = WiFi.localIP().toString();

                if (bruceConfig.automaticTimeUpdateViaNTP) { updateClockTimezone(); }

                ensureTimezoneTaskRunning();
                logWifiTime("[Startup][WiFi] Connected. Device time:");
                drawStatusBar();
                break;
            }
            vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
    WiFi.scanDelete();

    vTaskDelete(NULL);
    return;
}

String checkMAC() { return String(WiFi.macAddress()); }

bool wifiConnecttoKnownNet(void) {
    if (WiFi.isConnected()) return true; // safeguard
    bool result = false;
    int nets;
    // WiFi.mode(WIFI_MODE_STA);
    displayTextLine("Scanning Networks..");
    WiFi.disconnect(true, true);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    nets = WiFi.scanNetworks();
    for (int i = 0; i < nets; i++) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        String ssid = WiFi.SSID(i);
        String password = bruceConfig.getWifiPassword(ssid);
        if (password != "") {
            Serial.println("Connecting to: " + ssid);
            result = _connectToWifiNetwork(ssid, password);
        }
        // Maybe it finds a known network and can't connect, then try the next
        // until it gets connected (or not)
        if (result) {
            Serial.println("Connected to: " + ssid);
            break;
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        wifiIP = WiFi.localIP().toString();

        if (bruceConfig.automaticTimeUpdateViaNTP) { updateClockTimezone(); }

        ensureTimezoneTaskRunning();
    }
    return result;
}

void updateTimezoneTask(void *pvParameters) {
    bool wasConnected = false;
    uint32_t lastSyncMs = 0;

    while (true) {
        const bool isConnected = WiFi.status() == WL_CONNECTED;

        if (bruceConfig.automaticTimeUpdateViaNTP && isConnected) {
            bool justSynced = false;

            if (!wasConnected) {
                vTaskDelay(kNtpConnectDelayMs / portTICK_PERIOD_MS);
                if (WiFi.status() == WL_CONNECTED) {
                    if (updateClockTimezone()) {
                        lastSyncMs = millis();
                        justSynced = true;
                    }
                }
            }

            if (!justSynced && (lastSyncMs == 0 || (millis() - lastSyncMs) >= kNtpResyncIntervalMs)) {
                if (updateClockTimezone()) { lastSyncMs = millis(); }
            }
        } else if (!isConnected) {
            // Force a fresh sync right after the next successful reconnection.
            lastSyncMs = 0;
        }

        wasConnected = isConnected;
        vTaskDelay(kNtpPollIntervalMs / portTICK_PERIOD_MS);
    }
}
