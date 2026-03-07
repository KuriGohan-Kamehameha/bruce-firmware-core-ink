#ifndef __BRUCE_CONFIG_H__
#define __BRUCE_CONFIG_H__

#include "theme.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <precompiler_flags.h>
#include <set>
#include <vector>

enum EvilPortalPasswordMode { FULL_PASSWORD = 0, FIRST_LAST_CHAR = 1, HIDE_PASSWORD = 2, SAVE_LENGTH = 3 };
enum PowerButtonShortPressAction {
    POWER_BUTTON_SHORT_PRESS_REFRESH_SCREEN = 0,
    POWER_BUTTON_SHORT_PRESS_DEEP_SLEEP_MESSAGE = 1
};
enum NavigationSoundMode {
    NAVIGATION_SOUND_OFF = 0,
    NAVIGATION_SOUND_BEEPS = 1,
    NAVIGATION_SOUND_CLICKS = 2
};
enum ClockChimeMode {
    CLOCK_CHIME_MODE_HOUR = 0,
    CLOCK_CHIME_MODE_HOUR_AND_HALF = 1,
    CLOCK_CHIME_MODE_FULL = 2
};

class BruceConfig : public BruceTheme {
public:
    struct WiFiCredential {
        String ssid;
        String pwd;
    };
    struct Credential {
        String user;
        String pwd;
    };
    struct QrCodeEntry {
        String menuName;
        String content;
    };
    struct EvilPortalEndpoints {
        String getCredsEndpoint;
        String setSsidEndpoint;
        bool showEndpoints;
        bool allowSetSsid;
        bool allowGetCreds;
    };

    const char *filepath = "/bruce.conf";

    //  Settings
    int dimmerSet = 10;
    int autoPowerOffMinutes = 0;
    int bright = 100;
    int einkRefreshMs = 15000;
    int einkRefreshDraws = 10;
    bool automaticTimeUpdateViaNTP = true;
    float tmz = -5;
    bool dst = false;
    bool clock24hr = true;
    int analogClockFaceStyle = 1;
    int analogClockHandStyle = 0;
    bool analogClockShowSecondHand = false;
    bool analogClockShowSecondTail = false;
    bool analogClockShowDigitalTime = true;
    bool analogClockDigitalShowSeconds = false;
    bool analogClockShowDate = true;
    bool analogClockShowWeekday = true;
    bool analogClockShowBattery = true;
    bool analogClockShowWifi = true;
    bool analogClockShowBle = false;
    bool analogClockShowCharging = true;
    bool analogClockShowMinuteTicks = true;
    bool analogClockSmoothSecondHand = false;
    bool analogClockFaceInverted = false;
    #if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
    int soundEnabled = 1;
    int menuBeepEnabled = NAVIGATION_SOUND_BEEPS;
    #else
    int soundEnabled = 0;
    int menuBeepEnabled = NAVIGATION_SOUND_OFF;
    #endif
    int soundVolume = 100;
    int startupChimeStyle = 0; // 0 = classic, 1 = four-tone
    int clockChimeStyle = 0;   // 0 = off, 1 = Westminster quarters
    int clockChimeSpeed = 1;   // 1x..5x
    int clockChimeMode = CLOCK_CHIME_MODE_FULL;
    int clockChimeSilentEnabled = 1;
    int clockChimeSilentStartHour = 20;
    int clockChimeSilentEndHour = 11;
    int clockAlarmEnabled = 0;
    int clockAlarmHour = 7;
    int clockAlarmMinute = 0;
    int clockAlarmTone = 0;
    int timerAlertTone = 0;
    int wifiAtStartup = 0;
    int instantBoot = 0;

#ifdef HAS_RGB_LED
    // Led
    int ledBright = 50;
    uint32_t ledColor = 0x960064;
    int ledBlinkEnabled = 1;
    int ledEffect = 0;
    int ledEffectSpeed = 5;
    int ledEffectDirection = 1;
#endif

    // Wifi
    Credential webUI = {"admin", "bruce"};
    std::vector<String> webUISessions = {}; // FIFO queue of session tokens
    WiFiCredential wifiAp = {"BruceNet", "brucenet"};
    std::map<String, String> wifi = {};
    std::set<String> evilWifiNames = {};
    String wifiMAC = ""; //@IncursioHack

    // EvilPortal
    EvilPortalEndpoints evilPortalEndpoints = {"/creds", "/ssid", true, true, true};
    EvilPortalPasswordMode evilPortalPasswordMode = FULL_PASSWORD;

    void setWifiMAC(const String &mac) {
        wifiMAC = mac;
        saveFile(); // opcional, para salvar imediatamente
    }

    // RFID
    std::set<String> mifareKeys = {};

    // Misc
    String startupApp = "";
    String startupAppJSInterpreterFile = "";
    String wigleBasicToken = "";
    int devMode = 0;
#if defined(HAS_EINK)
    int colorInverted = 0;
#else
    int colorInverted = 1;
#endif
    int rockerInverted = 0;
    int powerButtonShortPressAction = POWER_BUTTON_SHORT_PRESS_REFRESH_SCREEN;
    int badUSBBLEKeyboardLayout = 0;
    uint16_t badUSBBLEKeyDelay = 10;
    bool badUSBBLEShowOutput = true;

    std::vector<String> disabledMenus = {};

    std::vector<QrCodeEntry> qrCodes = {
        {"Bruce AP",   "WIFI:T:WPA;S:BruceNet;P:brucenet;;"},
        {"Bruce Wiki", "https://github.com/pr3y/Bruce/wiki"},
        {"Bruce Site", "https://bruce.computer"            },
        {"Rickroll",   "https://youtu.be/dQw4w9WgXcQ"      }
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    BruceConfig() {};
    // ~BruceConfig();

    /////////////////////////////////////////////////////////////////////////////////////
    // Operations
    /////////////////////////////////////////////////////////////////////////////////////
    void saveFile();
    void fromFile(bool checkFS = true);
    void factoryReset();
    void validateConfig();
    JsonDocument toJson() const;

    // UI Color
    void setUiColor(uint16_t primary, uint16_t *secondary = nullptr, uint16_t *background = nullptr);

    // Settings
    void setDimmer(int value);
    void validateDimmerValue();
    void setAutoPowerOffMinutes(int value);
    void validateAutoPowerOffMinutes();
    void setBright(uint8_t value);
    void validateBrightValue();
    void setEinkRefreshMs(int value);
    void validateEinkRefreshMs();
    void setEinkRefreshDraws(int value);
    void validateEinkRefreshDraws();
    void setAutomaticTimeUpdateViaNTP(bool value);
    void setTmz(float value);
    void validateTmzValue();
    void setDST(bool value);
    void setClock24Hr(bool value);
    void setAnalogClockFaceStyle(int value);
    void validateAnalogClockFaceStyle();
    void setAnalogClockHandStyle(int value);
    void validateAnalogClockHandStyle();
    void setAnalogClockShowSecondHand(bool value);
    void setAnalogClockShowSecondTail(bool value);
    void setAnalogClockShowDigitalTime(bool value);
    void setAnalogClockDigitalShowSeconds(bool value);
    void setAnalogClockShowDate(bool value);
    void setAnalogClockShowWeekday(bool value);
    void setAnalogClockShowBattery(bool value);
    void setAnalogClockShowWifi(bool value);
    void setAnalogClockShowBle(bool value);
    void setAnalogClockShowCharging(bool value);
    void setAnalogClockShowMinuteTicks(bool value);
    void setAnalogClockSmoothSecondHand(bool value);
    void setAnalogClockFaceInverted(bool value);
    void setSoundEnabled(int value);
    void setMenuBeepEnabled(int value);
    void setSoundVolume(int value);
    void setStartupChimeStyle(int value);
    void setClockChimeStyle(int value);
    void setClockChimeSpeed(int value);
    void setClockChimeMode(int value);
    void setClockChimeSilentEnabled(int value);
    void setClockChimeSilentStartHour(int value);
    void setClockChimeSilentEndHour(int value);
    void setClockAlarmEnabled(int value);
    void setClockAlarmHour(int value);
    void setClockAlarmMinute(int value);
    void setClockAlarmTone(int value);
    void setTimerAlertTone(int value);
    void validateSoundEnabledValue();
    void validateMenuBeepEnabledValue();
    void validateSoundVolumeValue();
    void validateStartupChimeStyle();
    void validateClockChimeStyle();
    void validateClockChimeSpeed();
    void validateClockChimeMode();
    void validateClockChimeSilentEnabled();
    void validateClockChimeSilentStartHour();
    void validateClockChimeSilentEndHour();
    void validateClockAlarmEnabled();
    void validateClockAlarmHour();
    void validateClockAlarmMinute();
    void validateClockAlarmTone();
    void validateTimerAlertTone();
    void setWifiAtStartup(int value);
    void validateWifiAtStartupValue();

#ifdef HAS_RGB_LED
    // Led
    void setLedBright(int value);
    void validateLedBrightValue();
    void setLedColor(uint32_t value);
    void validateLedColorValue();
    void setLedBlinkEnabled(int value);
    void validateLedBlinkEnabledValue();
    void setLedEffect(int value);
    void validateLedEffectValue();
    void setLedEffectSpeed(int value);
    void validateLedEffectSpeedValue();
    void setLedEffectDirection(int value);
    void validateLedEffectDirectionValue();
#endif

    // Wifi
    void setWebUICreds(const String &usr, const String &pwd);
    void setWifiApCreds(const String &ssid, const String &pwd);
    void addWifiCredential(const String &ssid, const String &pwd);
    void addQrCodeEntry(const String &menuName, const String &content);
    void removeQrCodeEntry(const String &menuName);
    String getWifiPassword(const String &ssid) const;
    void addEvilWifiName(String value);
    void removeEvilWifiName(String value);
    void setEvilEndpointCreds(String value);
    void setEvilEndpointSsid(String value);
    void setEvilAllowEndpointDisplay(bool value);
    void setEvilAllowGetCreds(bool value);
    void setEvilAllowSetSsid(bool value);
    void setEvilPasswordMode(EvilPortalPasswordMode value);
    void validateEvilEndpointCreds();
    void validateEvilEndpointSsid();
    void validateEvilPasswordMode();

    // RFID
    void addMifareKey(String value);
    void validateMifareKeysItems();

    // Misc
    void setStartupApp(String value);
    void setStartupAppJSInterpreterFile(String value);
    void setWigleBasicToken(String value);
    void setDevMode(int value);
    void validateDevModeValue();
    void setColorInverted(int value);
    void validateColorInverted();
    void setRockerInverted(int value);
    void validateRockerInvertedValue();
    void setPowerButtonShortPressAction(int value);
    void validatePowerButtonShortPressAction();
    void setBadUSBBLEKeyboardLayout(int value);
    void validateBadUSBBLEKeyboardLayout();
    void setBadUSBBLEKeyDelay(uint16_t value);
    void validateBadUSBBLEKeyDelay();
    void setBadUSBBLEShowOutput(bool value);
    void addDisabledMenu(String value);
    // TODO: removeDisabledMenu(String value);

    void addWebUISession(const String &token);
    void removeWebUISession(const String &token);
    bool isValidWebUISession(const String &token);
};

#endif
