#include "powerSave.h"
#include "display.h"
#include "settings.h"
#include <interface.h>

/* Check if it's time to put the device to sleep */
#define SCREEN_OFF_DELAY 5000

void fadeOutScreen(int startValue) {
    for (int brightValue = startValue; brightValue >= 0; brightValue -= 1) {
        setBrightness(max(brightValue, 0), false);
        delay(5);
    }
    turnOffDisplay();
}

void checkPowerSaveTime() {
    const unsigned long elapsed = millis() - previousMillis;
#if defined(HAS_EINK)
    // CoreInk has no backlight and no screen timeout handling.
    dimmer = false;
    isScreenOff = false;
#else
    if (bruceConfig.dimmerSet > 0) {
        int startDimmerBright = bruceConfig.bright / 3;
        int dimmerSetMs = bruceConfig.dimmerSet * 1000;

        if (elapsed >= dimmerSetMs && !dimmer && !isSleeping) {
            dimmer = true;
            setBrightness(startDimmerBright, false);
        } else if (elapsed >= (dimmerSetMs + SCREEN_OFF_DELAY) && !isScreenOff && !isSleeping) {
            isScreenOff = true;
            fadeOutScreen(startDimmerBright);
        }
    }
#endif

#if defined(HAS_CONTROLLED_POWEROFF)
    if (bruceConfig.autoPowerOffMinutes > 0 && !isSleeping) {
        const uint32_t autoPowerOffMs = static_cast<uint32_t>(bruceConfig.autoPowerOffMinutes) * 60000UL;
        if (elapsed >= autoPowerOffMs) {
#if defined(ARDUINO_M5STACK_COREINK)
            goToDeepSleep();
#else
            powerOff();
#endif
        }
    }
#endif
}

void sleepModeOn() {
    isSleeping = true;
    setCpuFrequencyMhz(80);

#if !defined(HAS_EINK)
    int startDimmerBright = bruceConfig.bright / 3;
    fadeOutScreen(startDimmerBright);
    panelSleep(true); //  power down screen
#endif

    disableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    disableCore1WDT();
#endif
    disableLoopWDT();
    delay(200);
}

void sleepModeOff() {
    isSleeping = false;
    setCpuFrequencyMhz(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);

#if !defined(HAS_EINK)
    panelSleep(false); // wake the screen back up
    getBrightness();
#endif

    enableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    enableCore1WDT();
#endif
    enableLoopWDT();
    feedLoopWDT();
    delay(200);
}
