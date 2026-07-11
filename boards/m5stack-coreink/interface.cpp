#include "core/powerSave.h"
#include "core/display.h"
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
#include "modules/others/audio.h"
#endif
#include <M5Unified.h>
#include <Wire.h>
#include <globals.h>
#include <interface.h>

namespace {
constexpr uint8_t ROCKER_LEFT_PIN = 39;
constexpr uint8_t ROCKER_CENTER_PIN = 38;
constexpr uint8_t ROCKER_RIGHT_PIN = 37;
constexpr bool ROCKER_ACTIVE_LOW = true;
constexpr uint8_t INPUT_LED_ON = 255;
constexpr uint8_t INPUT_LED_OFF = 0;
constexpr uint8_t CHARGING_LED_ON = 48;
constexpr uint32_t INPUT_LED_PULSE_MS = 35;
constexpr uint32_t POWER_EVENT_LED_PULSE_MS = 180;
constexpr uint32_t POWER_STATE_POLL_MS = 250;
constexpr uint16_t POWER_CONNECTED_TONE_LOW_HZ = 2600;
constexpr uint16_t POWER_CONNECTED_TONE_HIGH_HZ = 3600;
constexpr uint16_t POWER_DISCONNECTED_TONE_HIGH_HZ = 3200;
constexpr uint16_t POWER_DISCONNECTED_TONE_LOW_HZ = 1900;
constexpr uint16_t POWER_EVENT_TONE_SHORT_MS = 45;
constexpr uint16_t POWER_EVENT_TONE_LONG_MS = 70;

uint32_t g_inputLedPulseUntilMs = 0;
uint32_t g_lastPowerSampleMs = 0;
uint8_t g_ledIdleBrightness = INPUT_LED_OFF;
bool g_powerStateInitialized = false;
bool g_lastExternalPowerPresent = false;
bool g_lastChargingState = false;

bool readRockerPin(uint8_t pin) {
    int level = digitalRead(pin);
    return ROCKER_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}

void applyIdleLedState() {
    if (g_inputLedPulseUntilMs == 0U) { M5.Power.setLed(g_ledIdleBrightness); }
}

void setIdleLedForCharging(bool charging) {
    g_ledIdleBrightness = charging ? CHARGING_LED_ON : INPUT_LED_OFF;
    applyIdleLedState();
}

void triggerLedPulse(uint32_t pulseMs, uint8_t brightness) {
    M5.Power.setLed(brightness);
    g_inputLedPulseUntilMs = millis() + pulseMs;
}

void serviceInputLedPulse() {
    if (g_inputLedPulseUntilMs != 0U && millis() >= g_inputLedPulseUntilMs) {
        g_inputLedPulseUntilMs = 0;
        applyIdleLedState();
    }
}

void triggerInputLedPulse() {
    triggerLedPulse(INPUT_LED_PULSE_MS, INPUT_LED_ON);
}

void handlePowerButtonShortPressAction() {
    if (bruceConfig.powerButtonShortPressAction == POWER_BUTTON_SHORT_PRESS_DEEP_SLEEP_MESSAGE) {
        displayInfo("Entering deep sleep");
        delay(120);
        goToDeepSleep();
        return;
    }

    einkRequestFullRefresh();
    einkFlushIfDirty(0);
}

bool isExternalPowerPresent() {
    if (M5.Power.getType() == m5::Power_Class::pmic_t::pmic_axp192) {
        return M5.Power.Axp192.isACIN() || M5.Power.Axp192.isVBUS();
    }
    int16_t vbusMv = M5.Power.getVBUSVoltage();
    if (vbusMv < 0) {
        return false;
    }
    return vbusMv > 4300;
}

bool readChargingState() {
    return M5.Power.isCharging() == m5::Power_Class::is_charging_t::is_charging;
}

void playPowerTransitionTone(bool powerConnected) {
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
    if (!bruceConfig.soundEnabled) {
        return;
    }

    if (powerConnected) {
        _tone(POWER_CONNECTED_TONE_LOW_HZ, POWER_EVENT_TONE_SHORT_MS);
        _tone(POWER_CONNECTED_TONE_HIGH_HZ, POWER_EVENT_TONE_LONG_MS);
    } else {
        _tone(POWER_DISCONNECTED_TONE_HIGH_HZ, POWER_EVENT_TONE_SHORT_MS);
        _tone(POWER_DISCONNECTED_TONE_LOW_HZ, POWER_EVENT_TONE_LONG_MS);
    }
#else
    (void)powerConnected;
#endif
}

void servicePowerIndicators() {
    const uint32_t nowMs = millis();
    if ((nowMs - g_lastPowerSampleMs) < POWER_STATE_POLL_MS) {
        return;
    }
    g_lastPowerSampleMs = nowMs;

    const bool powerPresent = isExternalPowerPresent();
    const bool charging = readChargingState();

    if (!g_powerStateInitialized) {
        g_lastExternalPowerPresent = powerPresent;
        g_lastChargingState = charging;
        g_powerStateInitialized = true;
        setIdleLedForCharging(charging);
        return;
    }

    if (charging != g_lastChargingState) {
        setIdleLedForCharging(charging);
        g_lastChargingState = charging;
    }

    if (powerPresent != g_lastExternalPowerPresent) {
        g_lastExternalPowerPresent = powerPresent;
        triggerLedPulse(POWER_EVENT_LED_PULSE_MS, INPUT_LED_ON);
        playPowerTransitionTone(powerPresent);
    }
}
} // namespace

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    M5.begin();
#if defined(HAS_RTC)
    _rtc.setWire(&Wire);
#endif
    pinMode(ROCKER_LEFT_PIN, INPUT_PULLUP);
    pinMode(ROCKER_CENTER_PIN, INPUT_PULLUP);
    pinMode(ROCKER_RIGHT_PIN, INPUT_PULLUP);
#if defined(BUZZ_PIN)
    pinMode(BUZZ_PIN, OUTPUT);
    digitalWrite(BUZZ_PIN, LOW);
#endif
    g_lastExternalPowerPresent = isExternalPowerPresent();
    g_lastChargingState = readChargingState();
    g_powerStateInitialized = true;
    setIdleLedForCharging(g_lastChargingState);
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
    int percent = M5.Power.getBatteryLevel();
    return (percent < 0) ? 1 : (percent >= 100) ? 100 : percent;
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { (void)brightval; }

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;

    M5.update();
    serviceInputLedPulse();
    servicePowerIndicators();
    if (millis() - tm < 200 && !LongPress) return;

    // Rocker button: left/right = next/prev, center = select/back (hold), G5 = back.
    // PWR short click behavior is configurable via settings.
    static bool lastLeft = false;
    static bool lastRight = false;
    static bool lastCenter = false;
    static unsigned long centerPressTime = 0;

    bool leftNow = readRockerPin(ROCKER_LEFT_PIN);
    bool rightNow = readRockerPin(ROCKER_RIGHT_PIN);
    bool centerNow = readRockerPin(ROCKER_CENTER_PIN);

    bool leftPressed = leftNow && !lastLeft;
    bool rightPressed = rightNow && !lastRight;
    bool centerPressed = centerNow && !lastCenter;
    bool auxPressed = M5.BtnEXT.wasPressed();       // G5 top button
    bool pwrShortPressed = M5.BtnPWR.wasClicked();  // PMIC power button short click

    // Track center button hold time for long press detection
    if (centerNow && !lastCenter) { centerPressTime = millis(); }
    bool centerLongPress = centerNow && (millis() - centerPressTime > 800);

    lastLeft = leftNow;
    lastRight = rightNow;
    lastCenter = centerNow;

    if (centerPressed) {
        leftPressed = false;
        rightPressed = false;
    }

    if (pwrShortPressed) {
        tm = millis();
        triggerInputLedPulse();
        handlePowerButtonShortPressAction();
        return;
    }

    bool any = leftPressed || rightPressed || centerPressed || auxPressed;
    if (any) tm = millis();
    if (any && wakeUpScreen()) return;

    if (any) { triggerInputLedPulse(); }

    AnyKeyPress = any;
    AuxPress = auxPressed;
    UpPress = false;
    DownPress = false;
    const bool invertRocker = bruceConfig.rockerInverted;
    SelPress = centerPressed && !centerLongPress;           // Select on short press
    NextPress = invertRocker ? rightPressed : leftPressed;  // Next item
    PrevPress = invertRocker ? leftPressed : rightPressed;  // Previous item
    EscPress = centerLongPress || auxPressed;     // Back on center hold or G5 press
    LongPress = false;
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {
    drawPowerOffFrame();
    delay(120);
    M5.Power.powerOff();
}
void goToDeepSleep() { M5.Power.deepSleep(); }

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to turn off the device (name is odd btw)
**********************************************************************/
void checkReboot() {}

/***************************************************************************************
** Function name: isCharging()
** Description:   Determines if the device is charging
***************************************************************************************/
bool isCharging() {
    return readChargingState();
}
