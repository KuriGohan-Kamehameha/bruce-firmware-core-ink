#include "settings.h"
#include "core/led_control.h"
#include "core/wifi/wifi_common.h"
#include "current_year.h"
#include "display.h"
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "modules/bjs_interpreter/interpreter.h"
#endif
#include "modules/ble_api/ble_api.hpp"
#include "modules/others/qrcode_menu.h"
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
#include "modules/others/audio.h"
#endif
#include "modules/others/clock_alert_tones.h"
#include "modules/rf/rf_utils.h" // for initRfModule
#include "mykeyboard.h"
#include "powerSave.h"
#include "sd_functions.h"
#include "settingsColor.h"
#include "utils.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <globals.h>
#include <math.h>

int currentScreenBrightness = -1;

namespace {
std::vector<std::pair<const char *, int>> buildSelectablePins() {
    std::vector<std::pair<const char *, int>> pins = GPIO_PIN_LIST;
    pins.insert(pins.begin(), {"NC", -1});
    return pins;
}

int findPinIndex(const std::vector<std::pair<const char *, int>> &pins, gpio_num_t pinValue) {
    for (size_t i = 0; i < pins.size(); ++i) {
        if (pins[i].second == static_cast<int>(pinValue)) {
            return static_cast<int>(i);
        }
    }
    return 0;
}

gpio_num_t selectPinFromMenu(const std::vector<std::pair<const char *, int>> &pins, gpio_num_t currentPin) {
    gpio_num_t selected = currentPin;
    options.clear();
    for (const auto &pin : pins) {
        options.push_back({pin.first, [&selected, pin]() { selected = static_cast<gpio_num_t>(pin.second); }});
    }

    loopOptions(options, findPinIndex(pins, currentPin));
    options.clear();
    return selected;
}

int clampSoundVolumeValue(int volume) {
    if (volume < 0) return 0;
    if (volume > 100) return 100;
    return volume;
}

void previewSoundVolumeLevel(int volume) {
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
    if (!bruceConfig.soundEnabled) return;

    const int previousVolume = bruceConfig.soundVolume;
    bruceConfig.soundVolume = clampSoundVolumeValue(volume);
    _tone(2400 + (bruceConfig.soundVolume * 12), 70);
    bruceConfig.soundVolume = previousVolume;
#else
    (void)volume;
#endif
}

void commitSoundVolumeLevel(int volume) {
    const int clampedVolume = clampSoundVolumeValue(volume);
    bruceConfig.setSoundVolume(clampedVolume);
    previewSoundVolumeLevel(clampedVolume);
}

bool parseSoundVolumeInput(const String &input, int &parsedVolume) {
    if (input.isEmpty()) return false;

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] < '0' || input[i] > '9') return false;
    }

    parsedVolume = input.toInt();
    return parsedVolume >= 0 && parsedVolume <= 100;
}

void drawSoundVolumeAdjustScreen(int volume, int step) {
    drawMainBorderWithTitle("Sound Volume");
    printSubtitle(String("Adjust +/-") + String(step) + "%", false);

    const int barX = BORDER_PAD_X + 8;
    const int barY = BORDER_PAD_Y + FM * LH + 22;
    const int barWidth = tftWidth - (barX * 2);
    const int barHeight = 16;

    tft.drawRoundRect(barX, barY, barWidth, barHeight, 4, bruceConfig.priColor);
    tft.fillRoundRect(barX + 2, barY + 2, barWidth - 4, barHeight - 4, 3, TFT_DARKGREY);

    const int fillWidth = ((barWidth - 4) * volume) / 100;
    const uint16_t fillColor = volume >= 85 ? TFT_RED : bruceConfig.priColor;
    if (fillWidth > 0) {
        tft.fillRoundRect(barX + 2, barY + 2, fillWidth, barHeight - 4, 3, fillColor);
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FM);
    tft.drawCentreString(String(volume) + "%", tftWidth / 2, barY + barHeight + 8, 1);

    tft.setTextSize(FP);
    tft.drawCentreString("Up/Next +  Down/Prev -", tftWidth / 2, barY + barHeight + 24, 1);
    tft.drawCentreString("OK: Save   ESC: Cancel", tftWidth / 2, barY + barHeight + 34, 1);

    if (!bruceConfig.soundEnabled) {
        printCenterFootnote("Sound is OFF (preview muted)");
    }
}

void adjustSoundVolumeInteractive(int step) {
    if (step <= 0) step = 1;

    const int originalVolume = clampSoundVolumeValue(bruceConfig.soundVolume);
    int workingVolume = originalVolume;
    bool redraw = true;
    const unsigned long ignoreInputUntilMs = millis() + 180;

    while (true) {
        InputHandler();
        wakeUpScreen();

        if (redraw) {
            drawSoundVolumeAdjustScreen(workingVolume, step);
            redraw = false;
        }

        if (millis() < ignoreInputUntilMs) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        bool changed = false;
        if (check(NextPress) || check(UpPress)) {
            const int updatedVolume = clampSoundVolumeValue(workingVolume + step);
            if (updatedVolume != workingVolume) {
                workingVolume = updatedVolume;
                changed = true;
            }
        }

        if (check(PrevPress) || check(DownPress)) {
            const int updatedVolume = clampSoundVolumeValue(workingVolume - step);
            if (updatedVolume != workingVolume) {
                workingVolume = updatedVolume;
                changed = true;
            }
        }

        if (changed) {
            previewSoundVolumeLevel(workingVolume);
            redraw = true;
        }

        if (check(SelPress)) {
            if (workingVolume != originalVolume) bruceConfig.setSoundVolume(workingVolume);
            break;
        }

        if (check(EscPress)) break;

        einkFlushIfDirty(120);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void setSoundVolumeFromKeyboard() {
    String input = keyboard(String(bruceConfig.soundVolume), 3, "Volume 0-100");
    if (input == "\x1B") return;

    int enteredVolume = 0;
    if (!parseSoundVolumeInput(input, enteredVolume)) {
        displayError("Use a value from 0 to 100", true);
        return;
    }

    commitSoundVolumeLevel(enteredVolume);
}
} // namespace

// This function comes from interface.h
void _setBrightness(uint8_t brightval) {}

/*********************************************************************
**  Function: setBrightness
**  set brightness value
**********************************************************************/
void setBrightness(uint8_t brightval, bool save) {
    if (bruceConfig.bright > 100) bruceConfig.setBright(100);
    _setBrightness(brightval);
    delay(10);

    currentScreenBrightness = brightval;
    if (save) { bruceConfig.setBright(brightval); }
}

/*********************************************************************
**  Function: getBrightness
**  get brightness value
**********************************************************************/
void getBrightness() {
    if (bruceConfig.bright > 100) {
        bruceConfig.setBright(100);
        _setBrightness(bruceConfig.bright);
        delay(10);
        setBrightness(100);
    }

    _setBrightness(bruceConfig.bright);
    delay(10);

    currentScreenBrightness = bruceConfig.bright;
}

/*********************************************************************
**  Function: gsetRotation
**  get/set rotation value
**********************************************************************/
int gsetRotation(bool set) {
    const int currentRot = bruceConfigPins.rotation;
    int result = currentRot;

    if (set) {
        options = {
            {"Rotation 0 (0 deg)",   [&]() { result = 0; }, currentRot == 0},
            {"Rotation 1 (90 deg)",  [&]() { result = 1; }, currentRot == 1},
            {"Rotation 2 (180 deg)", [&]() { result = 2; }, currentRot == 2},
            {"Rotation 3 (270 deg)", [&]() { result = 3; }, currentRot == 3},
        };
        addOptionToMainMenu();
        int selected = loopOptions(options, MENU_TYPE_REGULAR, "Orientation", currentRot);
        if (selected >= 0 && selected <= 3) { result = selected; }
    }

    if (result > 3 || result < 0) { result = ROTATION; }
    if (set && result != currentRot) {
        bruceConfigPins.setRotation(result);
        tft.setRotation(result);
        tft.setRotation(result); // must repeat, sometimes ESP32S3 miss one SPI command and it just
                                 // jumps this step and don't rotate
    }
    if (set) returnToMenu = true;

    if (result & 0b01) { // if 1 or 3
        tftWidth = TFT_HEIGHT;
#if defined(HAS_TOUCH)
        tftHeight = TFT_WIDTH - 20;
#else
        tftHeight = TFT_WIDTH;
#endif
    } else { // if 2 or 0
        tftWidth = TFT_WIDTH;
#if defined(HAS_TOUCH)
        tftHeight = TFT_HEIGHT - 20;
#else
        tftHeight = TFT_HEIGHT;
#endif
    }
    return result;
}

/*********************************************************************
**  Function: setBrightnessMenu
**  Handles Menu to set brightness
**********************************************************************/
void setBrightnessMenu() {
    int idx = 0;
    if (bruceConfig.bright == 100) idx = 0;
    else if (bruceConfig.bright == 75) idx = 1;
    else if (bruceConfig.bright == 50) idx = 2;
    else if (bruceConfig.bright == 25) idx = 3;
    else if (bruceConfig.bright == 1) idx = 4;

    options = {
        {"100%",
         [=]() { setBrightness((uint8_t)100); },
         bruceConfig.bright == 100,
         [](void *pointer, bool shouldRender) {
             setBrightness((uint8_t)100, false);
             return false;
         }},
        {"75 %",
         [=]() { setBrightness((uint8_t)75); },
         bruceConfig.bright == 75,
         [](void *pointer, bool shouldRender) {
             setBrightness((uint8_t)75, false);
             return false;
         }},
        {"50 %",
         [=]() { setBrightness((uint8_t)50); },
         bruceConfig.bright == 50,
         [](void *pointer, bool shouldRender) {
             setBrightness((uint8_t)50, false);
             return false;
         }},
        {"25 %",
         [=]() { setBrightness((uint8_t)25); },
         bruceConfig.bright == 25,
         [](void *pointer, bool shouldRender) {
             setBrightness((uint8_t)25, false);
             return false;
         }},
        {" 1 %",
         [=]() { setBrightness((uint8_t)1); },
         bruceConfig.bright == 1,
         [](void *pointer, bool shouldRender) {
             setBrightness((uint8_t)1, false);
             return false;
         }}
    };
    addOptionToMainMenu(); // this one bugs the brightness selection
    loopOptions(options, MENU_TYPE_REGULAR, "", idx);
    setBrightness(bruceConfig.bright, false);
}

void setEinkRefreshMenu() {
    int idx = 0;
    if (bruceConfig.einkRefreshMs == 0) idx = 0;
    else if (bruceConfig.einkRefreshMs == 15000) idx = 1;
    else if (bruceConfig.einkRefreshMs == 30000) idx = 2;
    else if (bruceConfig.einkRefreshMs == 60000) idx = 3;
    else if (bruceConfig.einkRefreshMs == 300000) idx = 4;

    options = {
        {"Manual", [=]() { bruceConfig.setEinkRefreshMs(0); }     },
        {"15s",    [=]() { bruceConfig.setEinkRefreshMs(15000); } },
        {"30s",    [=]() { bruceConfig.setEinkRefreshMs(30000); } },
        {"60s",    [=]() { bruceConfig.setEinkRefreshMs(60000); } },
        {"5m",     [=]() { bruceConfig.setEinkRefreshMs(300000); }},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_REGULAR, "", idx);
}

void setEinkRefreshDrawsMenu() {
    int idx = 4;
    if (bruceConfig.einkRefreshDraws <= 0) idx = 0;
    else if (bruceConfig.einkRefreshDraws == 1) idx = 1;
    else if (bruceConfig.einkRefreshDraws == 3) idx = 2;
    else if (bruceConfig.einkRefreshDraws == 5) idx = 3;
    else if (bruceConfig.einkRefreshDraws == 10) idx = 4;
    else if (bruceConfig.einkRefreshDraws == 20) idx = 5;
    else if (bruceConfig.einkRefreshDraws == 40) idx = 6;
    else idx = 4;

    options = {
        {"Off",      [=]() { bruceConfig.setEinkRefreshDraws(0); } },
        {"1 draw",   [=]() { bruceConfig.setEinkRefreshDraws(1); } },
        {"3 draws",  [=]() { bruceConfig.setEinkRefreshDraws(3); } },
        {"5 draws",  [=]() { bruceConfig.setEinkRefreshDraws(5); } },
        {"10 draws", [=]() { bruceConfig.setEinkRefreshDraws(10); }},
        {"20 draws", [=]() { bruceConfig.setEinkRefreshDraws(20); }},
        {"40 draws", [=]() { bruceConfig.setEinkRefreshDraws(40); }},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_REGULAR, "Full Refresh Every", idx);
}

void setAutoPowerOffMenu() {
    int idx = 6;
    if (bruceConfig.autoPowerOffMinutes == 5) idx = 0;
    else if (bruceConfig.autoPowerOffMinutes == 10) idx = 1;
    else if (bruceConfig.autoPowerOffMinutes == 30) idx = 2;
    else if (bruceConfig.autoPowerOffMinutes == 60) idx = 3;
    else if (bruceConfig.autoPowerOffMinutes == 120) idx = 4;
    else if (bruceConfig.autoPowerOffMinutes == 240) idx = 5;

    options = {
        {"5 min",   [=]() { bruceConfig.setAutoPowerOffMinutes(5); }  },
        {"10 min",  [=]() { bruceConfig.setAutoPowerOffMinutes(10); } },
        {"30 min",  [=]() { bruceConfig.setAutoPowerOffMinutes(30); } },
        {"60 min",  [=]() { bruceConfig.setAutoPowerOffMinutes(60); } },
        {"120 min", [=]() { bruceConfig.setAutoPowerOffMinutes(120); }},
        {"240 min", [=]() { bruceConfig.setAutoPowerOffMinutes(240); }},
        {"Never",   [=]() { bruceConfig.setAutoPowerOffMinutes(0); }  },
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_REGULAR, "Auto PowerOff", idx);
}

void setPowerButtonShortPressMenu() {
#if defined(ARDUINO_M5STACK_COREINK)
    int idx = (bruceConfig.powerButtonShortPressAction == POWER_BUTTON_SHORT_PRESS_DEEP_SLEEP_MESSAGE) ? 1 : 0;

    options = {
        {"Refresh Screen",
         [=]() { bruceConfig.setPowerButtonShortPressAction(POWER_BUTTON_SHORT_PRESS_REFRESH_SCREEN); },
         bruceConfig.powerButtonShortPressAction == POWER_BUTTON_SHORT_PRESS_REFRESH_SCREEN},
        {"Deep Sleep + Msg",
         [=]() { bruceConfig.setPowerButtonShortPressAction(POWER_BUTTON_SHORT_PRESS_DEEP_SLEEP_MESSAGE); },
         bruceConfig.powerButtonShortPressAction == POWER_BUTTON_SHORT_PRESS_DEEP_SLEEP_MESSAGE},
    };

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_REGULAR, "Power Btn Short", idx);
#endif
}

/*********************************************************************
**  Function: setSleepMode
**  Turn screen off and reduces cpu clock
**********************************************************************/
void setSleepMode() {
    sleepModeOn();
    while (1) {
        if (check(AnyKeyPress)) {
            sleepModeOff();
            returnToMenu = true;
            break;
        }
    }
}

/*********************************************************************
**  Function: setBWInvertMenu
**  Handles Menu to set B/W color inversion
**********************************************************************/
void setBWInvertMenu() {
    int idx = bruceConfig.colorInverted ? 0 : 1;
    options = {
        {"Enable",
         [=]() {
             bruceConfig.setColorInverted(1);
#if !defined(HAS_EINK)
             tft.invertDisplay(bruceConfig.colorInverted);
#else
             // Force a full repaint after polarity changes to avoid stale dots/ghosting.
             einkRequestFullRefresh();
             tft.fillScreen(bruceConfig.bgColor);
#endif
             einkFlushIfDirty(0);
         }, bruceConfig.colorInverted == 1},
        {"Disable",
         [=]() {
             bruceConfig.setColorInverted(0);
#if !defined(HAS_EINK)
             tft.invertDisplay(bruceConfig.colorInverted);
#else
             // Force a full repaint after polarity changes to avoid stale dots/ghosting.
             einkRequestFullRefresh();
             tft.fillScreen(bruceConfig.bgColor);
#endif
             einkFlushIfDirty(0);
         }, bruceConfig.colorInverted == 0},
    };
    loopOptions(options, idx);
}

/*********************************************************************
**  Function: setUIColor
**  Set and store main UI color
**********************************************************************/
void setUIColor() {
#if defined(HAS_EINK)
    uint16_t secColor = 0x0000;
    uint16_t bgColor = 0xFFFF;
    bruceConfig.setUiColor(0x0000, &secColor, &bgColor);
    return;
#endif

    while (1) {
        options.clear();
        int idx = UI_COLOR_COUNT;
        int i = 0;
        for (const auto &mapping : UI_COLORS) {
            if (bruceConfig.priColor == mapping.priColor && bruceConfig.secColor == mapping.secColor &&
                bruceConfig.bgColor == mapping.bgColor) {
                idx = i;
            }

            options.emplace_back(
                mapping.name,
                [mapping]() {
                    uint16_t secColor = mapping.secColor;
                    uint16_t bgColor = mapping.bgColor;
                    bruceConfig.setUiColor(mapping.priColor, &secColor, &bgColor);
                },
                idx == i
            );
            ++i;
        }

        options.push_back(
            {"Custom Color",
             [=]() {
                 uint16_t oldPriColor = bruceConfig.priColor;
                 uint16_t oldSecColor = bruceConfig.secColor;
                 uint16_t oldBgColor = bruceConfig.bgColor;

                 if (setCustomUIColorMenu()) {
                     bruceConfig.setUiColor(
                         bruceConfig.priColor, &bruceConfig.secColor, &bruceConfig.bgColor
                     );
                 } else {
                     bruceConfig.priColor = oldPriColor;
                     bruceConfig.secColor = oldSecColor;
                     bruceConfig.bgColor = oldBgColor;
                 }
                 tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
             },
             idx == UI_COLOR_COUNT}
        );

        options.push_back(
            {"Invert Color",
             [=]() {
                 bruceConfig.setColorInverted(!bruceConfig.colorInverted);
                 tft.invertDisplay(bruceConfig.colorInverted);
             },
             bruceConfig.colorInverted > 0}
        );

        addOptionToMainMenu();

        int selectedOption = loopOptions(options, idx);
        if (selectedOption == -1 || selectedOption == options.size() - 1) return;
    }
}

uint16_t alterOneColorChannel565(uint16_t color, int newR, int newG, int newB) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    if (newR != 256) r = newR & 0x1F;
    if (newG != 256) g = newG & 0x3F;
    if (newB != 256) b = newB & 0x1F;

    return (r << 11) | (g << 5) | b;
}

bool setCustomUIColorMenu() {
    while (1) {
        options = {
            {"Primary",    [=]() { setCustomUIColorChoiceMenu(1); }},
            {"Secondary",  [=]() { setCustomUIColorChoiceMenu(2); }},
            {"Background", [=]() { setCustomUIColorChoiceMenu(3); }},
            {"Save",       [=]() {}                                },
            {"Cancel",     [=]() {}                                }
        };

        int selectedOption = loopOptions(options);
        if (selectedOption == -1 || selectedOption == options.size() - 1) {
            return false;
        } else if (selectedOption == 3) {
            return true;
        }
    }
}

void setCustomUIColorChoiceMenu(int colorType) {
    while (1) {
        options = {
            {"Red Channel",   [=]() { setCustomUIColorSettingMenuR(colorType); }},
            {"Green Channel", [=]() { setCustomUIColorSettingMenuG(colorType); }},
            {"Blue Channel",  [=]() { setCustomUIColorSettingMenuB(colorType); }},
            {"Back",          [=]() {}                                          }
        };

        int selectedOption = loopOptions(options);
        if (selectedOption == -1 || selectedOption == options.size() - 1) return;
    }
}

void setCustomUIColorSettingMenuR(int colorType) {
    setCustomUIColorSettingMenu(colorType, 1, [](uint16_t baseColor, int i) {
        return alterOneColorChannel565(baseColor, i, 256, 256);
    });
}

void setCustomUIColorSettingMenuG(int colorType) {
    setCustomUIColorSettingMenu(colorType, 2, [](uint16_t baseColor, int i) {
        return alterOneColorChannel565(baseColor, 256, i, 256);
    });
}

void setCustomUIColorSettingMenuB(int colorType) {
    setCustomUIColorSettingMenu(colorType, 3, [](uint16_t baseColor, int i) {
        return alterOneColorChannel565(baseColor, 256, 256, i);
    });
}

constexpr const char *colorTypes[] = {
    "Background", // 0
    "Primary",    // 1
    "Secondary"   // 2
};

constexpr const char *rgbNames[] = {
    "Blue", // 0
    "Red",  // 1
    "Green" // 2
};

void setCustomUIColorSettingMenu(
    int colorType, int rgb, std::function<uint16_t(uint16_t, int)> colorGenerator
) {
    uint16_t color = (colorType == 1)   ? bruceConfig.priColor
                     : (colorType == 2) ? bruceConfig.secColor
                                        : bruceConfig.bgColor;

    options.clear();

    static auto hoverFunctionPriColor = [](void *pointer, bool shouldRender) -> bool {
        uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
        // Serial.printf("Setting primary color to: %04X\n", colorToSet);
        bruceConfig.priColor = colorToSet;
        return false;
    };
    static auto hoverFunctionSecColor = [](void *pointer, bool shouldRender) -> bool {
        uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
        // Serial.printf("Setting secondary color to: %04X\n", colorToSet);
        bruceConfig.secColor = colorToSet;
        return false;
    };

    static auto hoverFunctionBgColor = [](void *pointer, bool shouldRender) -> bool {
        uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
        // Serial.printf("Setting bg color to: %04X\n", colorToSet);
        bruceConfig.bgColor = colorToSet;
        tft.fillScreen(bruceConfig.bgColor);
        return false;
    };

    static uint16_t colorStorage[32];
    int selectedIndex = 0;
    int i = 0;
    int index = 0;

    if (rgb == 1) {
        selectedIndex = (color >> 11) & 0x1F;
    } else if (rgb == 2) {
        selectedIndex = ((color >> 5) & 0x3F);
    } else {
        selectedIndex = color & 0x1F;
    }

    while (i <= (rgb == 2 ? 63 : 31)) {
        if (i == 0 || (rgb == 2 && (i + 1) % 2 == 0) || (rgb != 2)) {
            uint16_t updatedColor = colorGenerator(color, i);
            colorStorage[index] = updatedColor;

            options.emplace_back(
                String(i),
                [colorType, updatedColor]() {
                    if (colorType == 1) bruceConfig.priColor = updatedColor;
                    else if (colorType == 2) bruceConfig.secColor = updatedColor;
                    else bruceConfig.bgColor = updatedColor;
                },
                selectedIndex == i,
                (colorType == 1 ? hoverFunctionPriColor
                                : (colorType == 2 ? hoverFunctionSecColor : hoverFunctionBgColor)),
                &colorStorage[index]
            );
            ++index;
        }
        ++i;
    }

    addOptionToMainMenu();

    int selectedOption = loopOptions(
        options,
        MENU_TYPE_SUBMENU,
        (String(colorType == 1 ? "Primary" : (colorType == 2 ? "Secondary" : "Background")) + " - " +
         (rgb == 1 ? "Red" : (rgb == 2 ? "Green" : "Blue")))
            .c_str(),
        (rgb != 2) ? selectedIndex : (selectedIndex > 0 ? (selectedIndex + 1) / 2 : 0)
    );
    if (selectedOption == -1 || selectedOption == options.size() - 1) {
        if (colorType == 1) {
            bruceConfig.priColor = color;
        } else if (colorType == 2) {
            bruceConfig.secColor = color;
        } else {
            bruceConfig.bgColor = color;
        }
        return;
    }
}

/*********************************************************************
**  Function: setSoundConfig - 01/2026 - Refactored "ConfigMenu" (this function manteined for
* retrocompatibility)
**  Enable or disable sound
**********************************************************************/
void setSoundConfig() {
    options = {
        {"Sound off", [=]() { bruceConfig.setSoundEnabled(0); }, bruceConfig.soundEnabled == 0},
        {"Sound on",  [=]() { bruceConfig.setSoundEnabled(1); }, bruceConfig.soundEnabled == 1},
    };
    loopOptions(options, bruceConfig.soundEnabled);
}

/*********************************************************************
**  Function: setSoundVolume
**  Set sound volume
**********************************************************************/
void setSoundVolume() {
    while (true) {
        const int currentVolume = clampSoundVolumeValue(bruceConfig.soundVolume);
        int selectedIndex = 0;

        options = {
            {"Adjust (+/-5%)", [=]() { adjustSoundVolumeInteractive(5); }},
            {"Adjust (+/-1%)", [=]() { adjustSoundVolumeInteractive(1); }},
            {"Exact Value...", [=]() { setSoundVolumeFromKeyboard(); }},
            {"Mute (0%)",      [=]() { commitSoundVolumeLevel(0); }, currentVolume == 0},
            {"25%",            [=]() { commitSoundVolumeLevel(25); }, currentVolume == 25},
            {"50%",            [=]() { commitSoundVolumeLevel(50); }, currentVolume == 50},
            {"75%",            [=]() { commitSoundVolumeLevel(75); }, currentVolume == 75},
            {"Max (100%)",     [=]() { commitSoundVolumeLevel(100); }, currentVolume == 100},
            {"Back",           []() {}                                                        },
        };

        if (currentVolume == 0) selectedIndex = 3;
        else if (currentVolume == 25) selectedIndex = 4;
        else if (currentVolume == 50) selectedIndex = 5;
        else if (currentVolume == 75) selectedIndex = 6;
        else if (currentVolume == 100) selectedIndex = 7;

        String submenuTitle = String("Sound Volume (") + String(currentVolume) + "%)";
        const int selectedOption = loopOptions(options, MENU_TYPE_SUBMENU, submenuTitle.c_str(), selectedIndex);
        if (selectedOption == -1 || selectedOption == static_cast<int>(options.size()) - 1) return;
    }
}

#ifdef HAS_RGB_LED
/*********************************************************************
**  Function: setLedBlinkConfig - 01/2026 - Refactored "ConfigMenu" (this function manteined for
* retrocompatibility)
**  Enable or disable led blink
**********************************************************************/
void setLedBlinkConfig() {
    options = {
        {"Led Blink off", [=]() { bruceConfig.setLedBlinkEnabled(0); }, bruceConfig.ledBlinkEnabled == 0},
        {"Led Blink on",  [=]() { bruceConfig.setLedBlinkEnabled(1); }, bruceConfig.ledBlinkEnabled == 1},
    };
    loopOptions(options, bruceConfig.ledBlinkEnabled);
}
#endif

/*********************************************************************
**  Function: setWifiStartupConfig
**  Enable or disable wifi connection at startup
**********************************************************************/
void setWifiStartupConfig() {
    options = {
        {"Disable", [=]() { bruceConfig.setWifiAtStartup(0); }, bruceConfig.wifiAtStartup == 0},
        {"Enable",  [=]() { bruceConfig.setWifiAtStartup(1); }, bruceConfig.wifiAtStartup == 1},
    };
    loopOptions(options, bruceConfig.wifiAtStartup);
}

/*********************************************************************
**  Function: addEvilWifiMenu
**  Handles Menu to add evil wifi names into config list
**********************************************************************/
void addEvilWifiMenu() {
    String apName = keyboard("", 30, "Evil Portal SSID");
    if (apName != "\x1B") bruceConfig.addEvilWifiName(apName);
}

/*********************************************************************
**  Function: removeEvilWifiMenu
**  Handles Menu to remove evil wifi names from config list
**********************************************************************/
void removeEvilWifiMenu() {
    options = {};

    for (const auto &wifi_name : bruceConfig.evilWifiNames) {
        options.push_back({wifi_name.c_str(), [wifi_name]() { bruceConfig.removeEvilWifiName(wifi_name); }});
    }

    options.push_back({"Cancel", [=]() { backToMenu(); }});

    loopOptions(options);
}

/*********************************************************************
**  Function: setEvilEndpointCreds
**  Handles menu for changing the endpoint to access captured creds
**********************************************************************/
void setEvilEndpointCreds() {
    String userInput = keyboard(bruceConfig.evilPortalEndpoints.getCredsEndpoint, 30, "Evil creds endpoint");
    if (userInput != "\x1B") bruceConfig.setEvilEndpointCreds(userInput);
}

/*********************************************************************
**  Function: setEvilEndpointSsid
**  Handles menu for changing the endpoint to change evilSsid
**********************************************************************/
void setEvilEndpointSsid() {
    String userInput = keyboard(bruceConfig.evilPortalEndpoints.setSsidEndpoint, 30, "Evil creds endpoint");
    if (userInput != "\x1B") bruceConfig.setEvilEndpointSsid(userInput);
}

/*********************************************************************
**  Function: setEvilAllowGetCredentials
**  Handles menu for toggling access to the credential list endpoint
**********************************************************************/

void setEvilAllowGetCreds() {
    options = {
        {"Disallow",
         [=]() { bruceConfig.setEvilAllowGetCreds(false); },
         bruceConfig.evilPortalEndpoints.allowGetCreds == false},
        {"Allow",
         [=]() { bruceConfig.setEvilAllowGetCreds(true); },
         bruceConfig.evilPortalEndpoints.allowGetCreds == true },
    };
    loopOptions(options, bruceConfig.evilPortalEndpoints.allowGetCreds);
}

/*********************************************************************
**  Function: setEvilAllowGetCredentials
**  Handles menu for toggling access to the change SSID endpoint
**********************************************************************/

void setEvilAllowSetSsid() {
    options = {
        {"Disallow",
         [=]() { bruceConfig.setEvilAllowSetSsid(false); },
         bruceConfig.evilPortalEndpoints.allowSetSsid == false},
        {"Allow",
         [=]() { bruceConfig.setEvilAllowSetSsid(true); },
         bruceConfig.evilPortalEndpoints.allowSetSsid == true },
    };
    loopOptions(options, bruceConfig.evilPortalEndpoints.allowSetSsid);
}

/*********************************************************************
**  Function: setEvilAllowEndpointDisplay
**  Handles menu for toggling the display of the Evil Portal endpoints
**********************************************************************/

void setEvilAllowEndpointDisplay() {
    options = {
        {"Disallow",
         [=]() { bruceConfig.setEvilAllowEndpointDisplay(false); },
         bruceConfig.evilPortalEndpoints.showEndpoints == false},
        {"Allow",
         [=]() { bruceConfig.setEvilAllowEndpointDisplay(true); },
         bruceConfig.evilPortalEndpoints.showEndpoints == true },
    };
    loopOptions(options, bruceConfig.evilPortalEndpoints.showEndpoints);
}

/*********************************************************************
** Function: setEvilPasswordMode
** Handles menu for setting the evil portal password mode
***********************************************************************/
void setEvilPasswordMode() {
    options = {
        {"Save 'password'",
         [=]() { bruceConfig.setEvilPasswordMode(FULL_PASSWORD); },
         bruceConfig.evilPortalPasswordMode == FULL_PASSWORD  },
        {"Save 'p******d'",
         [=]() { bruceConfig.setEvilPasswordMode(FIRST_LAST_CHAR); },
         bruceConfig.evilPortalPasswordMode == FIRST_LAST_CHAR},
        {"Save '*hidden*'",
         [=]() { bruceConfig.setEvilPasswordMode(HIDE_PASSWORD); },
         bruceConfig.evilPortalPasswordMode == HIDE_PASSWORD  },
        {"Save length",
         [=]() { bruceConfig.setEvilPasswordMode(SAVE_LENGTH); },
         bruceConfig.evilPortalPasswordMode == SAVE_LENGTH    },
    };
    loopOptions(options, bruceConfig.evilPortalPasswordMode);
}

/*********************************************************************
**  Function: setRFModuleMenu
**  Handles Menu to set the RF module in use
**********************************************************************/
void setRFModuleMenu() {
    int result = 0;
    int idx = 0;
    uint8_t pins_setup = 0;
    if (bruceConfigPins.rfModule == M5_RF_MODULE) idx = 0;
    else if (bruceConfigPins.rfModule == CC1101_SPI_MODULE) {
        idx = 1;
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
        if (bruceConfigPins.CC1101_bus.mosi == GPIO_NUM_26) idx = 2;
#endif
    }

    options = {
        {"M5 RF433T/R",         [&]() { result = M5_RF_MODULE; }   },
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
        {"CC1101 (legacy)",     [&pins_setup]() { pins_setup = 1; }},
        {"CC1101 (Shared SPI)", [&pins_setup]() { pins_setup = 2; }},
#else
        {"CC1101", [&]() { result = CC1101_SPI_MODULE; }},
#endif
        /* WIP:
         * #ifdef USE_CC1101_VIA_PCA9554
         * {"CC1101+PCA9554",  [&]() { result = 2; }},
         * #endif
         */
    };
    loopOptions(options, idx);
    if (result == CC1101_SPI_MODULE || pins_setup > 0) {
        // This setting is meant to StickCPlus and StickCPlus2 to setup the ports from RF Menu
        if (pins_setup == 1) {
            result = CC1101_SPI_MODULE;
            bruceConfigPins.setCC1101Pins(
                {(gpio_num_t)CC1101_SCK_PIN,
                 (gpio_num_t)CC1101_MISO_PIN,
                 (gpio_num_t)CC1101_MOSI_PIN,
                 (gpio_num_t)CC1101_SS_PIN,
                 (gpio_num_t)CC1101_GDO0_PIN,
                 GPIO_NUM_NC}
            );
            bruceConfigPins.setNrf24Pins(
                {(gpio_num_t)CC1101_SCK_PIN,
                 (gpio_num_t)CC1101_MISO_PIN,
                 (gpio_num_t)CC1101_MOSI_PIN,
                 (gpio_num_t)CC1101_SS_PIN,
                 (gpio_num_t)CC1101_GDO0_PIN,
                 GPIO_NUM_NC}
            );
        } else if (pins_setup == 2) {
#if CONFIG_SOC_GPIO_OUT_RANGE_MAX > 30
            result = CC1101_SPI_MODULE;
            bruceConfigPins.setCC1101Pins(
                {(gpio_num_t)SDCARD_SCK,
                 (gpio_num_t)SDCARD_MISO,
                 (gpio_num_t)SDCARD_MOSI,
                 GPIO_NUM_33,
                 GPIO_NUM_32,
                 GPIO_NUM_NC}
            );
            bruceConfigPins.setNrf24Pins(
                {(gpio_num_t)SDCARD_SCK,
                 (gpio_num_t)SDCARD_MISO,
                 (gpio_num_t)SDCARD_MOSI,
                 GPIO_NUM_33,
                 GPIO_NUM_32,
                 GPIO_NUM_NC}
            );
#endif
        }
        if (initRfModule()) {
            bruceConfigPins.setRfModule(CC1101_SPI_MODULE);
            deinitRfModule();
            if (pins_setup == 1) CC_NRF_SPI.end();
            return;
        }
        // else display an error
        displayError("CC1101 not found", true);
        if (pins_setup == 1)
            qrcode_display("https://github.com/pr3y/Bruce/blob/main/media/connections/cc1101_stick.jpg");
        if (pins_setup == 2)
            qrcode_display(
                "https://github.com/pr3y/Bruce/blob/main/media/connections/cc1101_stick_SDCard.jpg"
            );
        while (!check(AnyKeyPress)) vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    // fallback to "M5 RF433T/R" on errors
    bruceConfigPins.setRfModule(M5_RF_MODULE);
}

/*********************************************************************
**  Function: setRFFreqMenu
**  Handles Menu to set the default frequency for the RF module
**********************************************************************/
void setRFFreqMenu() {
    float result = 433.92;
    String freq_str = num_keyboard(String(bruceConfigPins.rfFreq), 10, "Default frequency:");
    if (freq_str == "\x1B") return;
    if (freq_str.length() > 1) {
        result = freq_str.toFloat();          // returns 0 if not valid
        if (result >= 280 && result <= 928) { // TODO: check valid freq according to current module?
            bruceConfigPins.setRfFreq(result);
            return;
        }
    }
    // else
    displayError("Invalid frequency");
    bruceConfigPins.setRfFreq(433.92); // reset to default
    delay(1000);
}

/*********************************************************************
**  Function: setRFIDModuleMenu
**  Handles Menu to set the RFID module in use
**********************************************************************/
void setRFIDModuleMenu() {
    options = {
        {"M5 RFID2",
         [=]() { bruceConfigPins.setRfidModule(M5_RFID2_MODULE); },
         bruceConfigPins.rfidModule == M5_RFID2_MODULE     },
#ifdef M5STICK
        {"PN532 I2C G33",
         [=]() { bruceConfigPins.setRfidModule(PN532_I2C_MODULE); },
         bruceConfigPins.rfidModule == PN532_I2C_MODULE    },
        {"PN532 I2C G36",
         [=]() { bruceConfigPins.setRfidModule(PN532_I2C_SPI_MODULE); },
         bruceConfigPins.rfidModule == PN532_I2C_SPI_MODULE},
#else
        {"PN532 on I2C",
         [=]() { bruceConfigPins.setRfidModule(PN532_I2C_MODULE); },
         bruceConfigPins.rfidModule == PN532_I2C_MODULE},
#endif
        {"PN532 on SPI",
         [=]() { bruceConfigPins.setRfidModule(PN532_SPI_MODULE); },
         bruceConfigPins.rfidModule == PN532_SPI_MODULE    },
        {"RC522 on SPI",
         [=]() { bruceConfigPins.setRfidModule(RC522_SPI_MODULE); },
         bruceConfigPins.rfidModule == RC522_SPI_MODULE    },
    };
    loopOptions(options, bruceConfigPins.rfidModule);
}

/*********************************************************************
**  Function: addMifareKeyMenu
**  Handles Menu to add MIFARE keys into config list
**********************************************************************/
void addMifareKeyMenu() {
    String key = keyboard("", 12, "MIFARE key");
    if (key != "\x1B") bruceConfig.addMifareKey(key);
}

/*********************************************************************
**  Function: setClock
**  Handles Menu to set timezone to NTP
**********************************************************************/

void setClock() {
#if defined(HAS_RTC)
    RTC_TimeTypeDef TimeStruct;
#if defined(HAS_RTC_BM8563)
    _rtc.GetBm8563Time();
#endif
#if defined(HAS_RTC_PCF85063A)
    _rtc.GetPcf85063Time();
#endif
#endif

    options = {
        {"Via NTP (Toronto)",                                           [&]() { bruceConfig.setAutomaticTimeUpdateViaNTP(true); } },
        {"Set Time Manually",                                           [&]() { bruceConfig.setAutomaticTimeUpdateViaNTP(false); }},
        {(bruceConfig.clock24hr ? "24-Hour Format" : "12-Hour Format"), [&]() {
             bruceConfig.setClock24Hr(!bruceConfig.clock24hr);
             returnToMenu = true;
         }                                                    }
    };

    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    if (bruceConfig.automaticTimeUpdateViaNTP) {
        if (!wifiConnected) wifiConnectMenu();
        if (!updateClockTimezone()) displayError("NTP sync failed");

    } else {
        int hr, mn, am;
        options = {};
        for (int i = 0; i < 12; i++) {
            String tmp = String(i < 10 ? "0" : "") + String(i);
            options.push_back({tmp.c_str(), [&]() { delay(1); }});
        }

        hr = loopOptions(options, MENU_TYPE_SUBMENU, "Set Hour");
        options.clear();

        for (int i = 0; i < 60; i++) {
            String tmp = String(i < 10 ? "0" : "") + String(i);
            options.push_back({tmp.c_str(), [&]() { delay(1); }});
        }

        mn = loopOptions(options, MENU_TYPE_SUBMENU, "Set Minute");
        options.clear();

        options = {
            {"AM", [&]() { am = 0; } },
            {"PM", [&]() { am = 12; }},
        };

        loopOptions(options);

#if defined(HAS_RTC)
        RTC_DateTypeDef DateStruct = {};
        _rtc.GetDate(&DateStruct);
        const bool rtcDateValid = DateStruct.Year >= CURRENT_YEAR && DateStruct.Month >= 1 &&
                                  DateStruct.Month <= 12 && DateStruct.Date >= 1 && DateStruct.Date <= 31;
        if (!rtcDateValid) {
            constexpr time_t kMinValidEpoch = 1704067200; // 2024-01-01 00:00:00 UTC
            struct tm fallbackDate = {};
            const time_t nowEpoch = time(nullptr);
            if (nowEpoch >= kMinValidEpoch) {
                localtime_r(&nowEpoch, &fallbackDate);
            } else {
                fallbackDate.tm_year = CURRENT_YEAR - 1900;
                fallbackDate.tm_mon = 0;
                fallbackDate.tm_mday = 1;
                mktime(&fallbackDate);
            }
            DateStruct.WeekDay = fallbackDate.tm_wday;
            DateStruct.Month = fallbackDate.tm_mon + 1;
            DateStruct.Date = fallbackDate.tm_mday;
            DateStruct.Year = fallbackDate.tm_year + 1900;
        }

        TimeStruct.Hours = hr + am;
        TimeStruct.Minutes = mn;
        TimeStruct.Seconds = 0;
        _rtc.SetTime(&TimeStruct);
        _rtc.SetDate(&DateStruct);
        _rtc.GetTime(&_time);
        _rtc.GetDate(&_date);

        struct tm timeinfo = {};
        timeinfo.tm_sec = _time.Seconds;
        timeinfo.tm_min = _time.Minutes;
        timeinfo.tm_hour = _time.Hours;
        timeinfo.tm_mday = _date.Date;
        timeinfo.tm_mon = _date.Month > 0 ? _date.Month - 1 : 0;
        timeinfo.tm_year = _date.Year >= 1900 ? _date.Year - 1900 : 0;
        time_t epoch = mktime(&timeinfo);
        struct timeval tv = {.tv_sec = epoch};
        settimeofday(&tv, nullptr);
#else
        rtc.setTime(0, mn, hr + am, 20, 06, CURRENT_YEAR); // send me a gift, @Pirata!
        struct tm t = rtc.getTimeStruct();
        time_t epoch = mktime(&t);
        struct timeval tv = {.tv_sec = epoch};
        settimeofday(&tv, nullptr);
#endif
        clock_set = true;
    }
}

static float analogClockRadians(float angleDeg) { return (angleDeg - 90.0f) * (PI / 180.0f); }

static void drawAnalogClockHand(
    int centerX, int centerY, float angleDeg, int length, int thickness, uint16_t color, int backLength = 0
) {
    const float radians = analogClockRadians(angleDeg);
    const float dirX = cosf(radians);
    const float dirY = sinf(radians);
    const float normalX = -dirY;
    const float normalY = dirX;
    const int startX = centerX - lroundf(dirX * backLength);
    const int startY = centerY - lroundf(dirY * backLength);
    const int endX = centerX + lroundf(dirX * length);
    const int endY = centerY + lroundf(dirY * length);
    const int halfThickness = thickness / 2;

    for (int offset = -halfThickness; offset <= halfThickness; ++offset) {
        const int offsetX = lroundf(normalX * offset);
        const int offsetY = lroundf(normalY * offset);
        tft.drawLine(startX + offsetX, startY + offsetY, endX + offsetX, endY + offsetY, color);
    }
}

static void drawAnalogClockTick(
    int centerX, int centerY, float angleDeg, int innerRadius, int outerRadius, uint16_t color, int width = 1
) {
    const float radians = analogClockRadians(angleDeg);
    const float dirX = cosf(radians);
    const float dirY = sinf(radians);
    const float normalX = -dirY;
    const float normalY = dirX;
    const int x1 = centerX + lroundf(dirX * innerRadius);
    const int y1 = centerY + lroundf(dirY * innerRadius);
    const int x2 = centerX + lroundf(dirX * outerRadius);
    const int y2 = centerY + lroundf(dirY * outerRadius);
    const int halfWidth = width / 2;

    for (int offset = -halfWidth; offset <= halfWidth; ++offset) {
        const int offsetX = lroundf(normalX * offset);
        const int offsetY = lroundf(normalY * offset);
        tft.drawLine(x1 + offsetX, y1 + offsetY, x2 + offsetX, y2 + offsetY, color);
    }
}

struct AnalogClockLayout {
    int centerX = 0;
    int centerY = 0;
    int outerRadius = 0;
    int majorInnerRadius = 0;
    int minorInnerRadius = 0;
    int minuteHandLength = 0;
    int hourHandLength = 0;
    int secondHandLength = 0;
    int secondTailLength = 0;
    int hourHandThickness = 0;
    int minuteHandThickness = 0;
    int secondHandThickness = 1;
    int centerDotRadius = 2;
};

static AnalogClockLayout getAnalogClockLayout() {
    AnalogClockLayout layout;

    layout.centerX = tftWidth / 2;
    layout.centerY = tftHeight / 2;

    const int minDimension = (tftWidth < tftHeight) ? tftWidth : tftHeight;
    layout.outerRadius = (minDimension / 2) - 4;
    if (layout.outerRadius < 18) layout.outerRadius = 18;

    int majorInset = 8;
    if (bruceConfig.analogClockFaceStyle == 1) majorInset = 10;
    else if (bruceConfig.analogClockFaceStyle == 2) majorInset = 12;
    else if (bruceConfig.analogClockFaceStyle == 4) majorInset = 11;
    layout.majorInnerRadius = layout.outerRadius - majorInset;
    if (layout.majorInnerRadius < 8) layout.majorInnerRadius = 8;

    layout.minorInnerRadius = layout.outerRadius - 4;
    if (bruceConfig.analogClockFaceStyle == 3) layout.minorInnerRadius = layout.outerRadius - 3;
    if (layout.minorInnerRadius < layout.majorInnerRadius) layout.minorInnerRadius = layout.majorInnerRadius;

    layout.minuteHandLength = layout.outerRadius - 11;
    if (layout.minuteHandLength < 8) layout.minuteHandLength = 8;

    layout.hourHandLength = (layout.minuteHandLength * 62) / 100;
    if (layout.hourHandLength < 6) layout.hourHandLength = 6;

    layout.secondHandLength = layout.outerRadius - 6;
    if (layout.secondHandLength < 10) layout.secondHandLength = 10;
    layout.secondTailLength = layout.outerRadius / 5;
    if (layout.secondTailLength < 5) layout.secondTailLength = 5;

    switch (bruceConfig.analogClockHandStyle) {
    case 1:
        layout.hourHandThickness = 2;
        layout.minuteHandThickness = 1;
        break;
    case 2:
        layout.hourHandThickness = 6;
        layout.minuteHandThickness = 4;
        break;
    default:
        layout.hourHandThickness = (bruceConfig.analogClockFaceStyle == 2) ? 3 : 5;
        layout.minuteHandThickness = (bruceConfig.analogClockFaceStyle == 2) ? 2 : 3;
        break;
    }

    if (layout.outerRadius < 40) {
        layout.hourHandThickness = max(2, layout.hourHandThickness - 1);
        layout.minuteHandThickness = max(1, layout.minuteHandThickness - 1);
    }

    layout.secondHandThickness = (bruceConfig.analogClockHandStyle == 2) ? 2 : 1;
    if (layout.outerRadius > 55) layout.secondHandThickness = min(3, layout.secondHandThickness + 1);
    layout.centerDotRadius = (layout.outerRadius > 55) ? 4 : 3;

    return layout;
}

static void drawAnalogClockNumeral(
    int centerX, int centerY, int radius, int hour, uint16_t color, int textSize, int yOffset = 0
) {
    const float radians = analogClockRadians((hour % 12) * 30.0f);
    const int x = centerX + lroundf(cosf(radians) * radius);
    const int y = centerY + lroundf(sinf(radians) * radius) + yOffset;
    const String label = String(hour == 0 ? 12 : hour);
    const int width = tft.textWidth(label);
    tft.drawString(label, x - (width / 2), y - ((8 * textSize) / 2), 1);
}

static uint16_t analogClockFaceFillColor() {
    return bruceConfig.analogClockFaceInverted ? bruceConfig.priColor : bruceConfig.bgColor;
}

static uint16_t analogClockFaceStrokeColor() {
    uint16_t stroke = bruceConfig.analogClockFaceInverted ? bruceConfig.bgColor : bruceConfig.priColor;
    const uint16_t fill = analogClockFaceFillColor();
    if (stroke == fill) stroke = getComplementaryColor2(fill);
    return stroke;
}

static void drawAnalogClockFace(const AnalogClockLayout &layout) {
    const int centerX = layout.centerX;
    const int centerY = layout.centerY;
    const int style = bruceConfig.analogClockFaceStyle;
    const uint16_t faceFillColor = analogClockFaceFillColor();
    const uint16_t faceStrokeColor = analogClockFaceStrokeColor();

    tft.fillCircle(centerX, centerY, layout.outerRadius, faceFillColor);
    tft.drawCircle(centerX, centerY, layout.outerRadius + 1, faceStrokeColor);
    if (style == 1 || style == 4) {
        tft.drawCircle(centerX, centerY, layout.outerRadius - 2, faceStrokeColor);
    }

    if (style == 3) {
        tft.fillTriangle(
            centerX,
            centerY - layout.outerRadius - 2,
            centerX - 3,
            centerY - layout.outerRadius + 3,
            centerX + 3,
            centerY - layout.outerRadius + 3,
            faceStrokeColor
        );
    }

    bool drawMinuteTicks = bruceConfig.analogClockShowMinuteTicks;
    if (style == 2 || style == 4) drawMinuteTicks = false;

    for (int marker = 0; marker < 60; marker++) {
        const bool isMajor = (marker % 5 == 0);
        if (!isMajor && !drawMinuteTicks) continue;

        int innerRadius = isMajor ? layout.majorInnerRadius : layout.minorInnerRadius;
        int width = 1;
        if (style == 4 && isMajor) {
            innerRadius = layout.majorInnerRadius - 2;
            width = 2;
        }
        drawAnalogClockTick(centerX, centerY, marker * 6.0f, innerRadius, layout.outerRadius, faceStrokeColor, width);
    }

    if (style == 4) {
        drawAnalogClockTick(centerX, centerY, 0.0f, layout.majorInnerRadius - 5, layout.outerRadius + 1, faceStrokeColor, 3);
        drawAnalogClockTick(
            centerX, centerY, 90.0f, layout.majorInnerRadius - 5, layout.outerRadius + 1, faceStrokeColor, 2
        );
        drawAnalogClockTick(
            centerX, centerY, 180.0f, layout.majorInnerRadius - 5, layout.outerRadius + 1, faceStrokeColor, 2
        );
        drawAnalogClockTick(
            centerX, centerY, 270.0f, layout.majorInnerRadius - 5, layout.outerRadius + 1, faceStrokeColor, 2
        );
    }

    if (style == 0 || style == 1 || style == 3) {
        const int numeralSize = (layout.outerRadius > 55) ? 2 : 1;
        tft.setTextSize(numeralSize);
        tft.setTextColor(faceStrokeColor, faceFillColor);

        if (style == 0 && layout.outerRadius >= 40) {
            for (int hour = 1; hour <= 12; hour++) {
                drawAnalogClockNumeral(
                    centerX, centerY, layout.majorInnerRadius - 8, hour, faceStrokeColor, numeralSize
                );
            }
        } else {
            drawAnalogClockNumeral(centerX, centerY, layout.majorInnerRadius - 8, 12, faceStrokeColor, numeralSize);
            drawAnalogClockNumeral(centerX, centerY, layout.majorInnerRadius - 8, 3, faceStrokeColor, numeralSize);
            drawAnalogClockNumeral(centerX, centerY, layout.majorInnerRadius - 8, 6, faceStrokeColor, numeralSize);
            drawAnalogClockNumeral(centerX, centerY, layout.majorInnerRadius - 8, 9, faceStrokeColor, numeralSize);
        }
    }
}

static String formatAnalogDigitalTime(const struct tm &timeInfo, bool withSeconds) {
    char out[32] = {0};
    if (bruceConfig.clock24hr) {
        if (withSeconds) snprintf(out, sizeof(out), "%02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
        else snprintf(out, sizeof(out), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);
        return String(out);
    }

    int hour12 = (timeInfo.tm_hour == 0)   ? 12
                 : (timeInfo.tm_hour > 12) ? timeInfo.tm_hour - 12
                                           : timeInfo.tm_hour;
    const char *ampm = (timeInfo.tm_hour < 12) ? "AM" : "PM";
    if (withSeconds) snprintf(out, sizeof(out), "%02d:%02d:%02d %s", hour12, timeInfo.tm_min, timeInfo.tm_sec, ampm);
    else snprintf(out, sizeof(out), "%02d:%02d %s", hour12, timeInfo.tm_min, ampm);
    return String(out);
}

static bool analogClockSecondHandEnabled() {
#if defined(HAS_EINK)
    return false;
#else
    return bruceConfig.analogClockShowSecondHand;
#endif
}

static bool analogClockDigitalSecondsEnabled() {
#if defined(HAS_EINK)
    return false;
#else
    return bruceConfig.analogClockDigitalShowSeconds;
#endif
}

static String formatStopwatchTime(uint32_t elapsedMs, bool showTenths) {
    const uint32_t totalSeconds = elapsedMs / 1000U;
    const uint32_t hours = totalSeconds / 3600U;
    const uint32_t minutes = (totalSeconds / 60U) % 60U;
    const uint32_t seconds = totalSeconds % 60U;
    const uint32_t tenths = (elapsedMs / 100U) % 10U;

    char out[24] = {0};
    if (hours > 0U) {
        if (showTenths) {
            snprintf(
                out,
                sizeof(out),
                "%02lu:%02lu:%02lu.%1lu",
                static_cast<unsigned long>(hours),
                static_cast<unsigned long>(minutes),
                static_cast<unsigned long>(seconds),
                static_cast<unsigned long>(tenths)
            );
        } else {
            snprintf(
                out,
                sizeof(out),
                "%02lu:%02lu:%02lu",
                static_cast<unsigned long>(hours),
                static_cast<unsigned long>(minutes),
                static_cast<unsigned long>(seconds)
            );
        }
        return String(out);
    }

    if (showTenths) {
        snprintf(
            out,
            sizeof(out),
            "%02lu:%02lu.%1lu",
            static_cast<unsigned long>(minutes),
            static_cast<unsigned long>(seconds),
            static_cast<unsigned long>(tenths)
        );
    } else {
        snprintf(
            out, sizeof(out), "%02lu:%02lu", static_cast<unsigned long>(minutes), static_cast<unsigned long>(seconds)
        );
    }
    return String(out);
}

struct AnalogDateLines {
    String primary;
    String secondary;
};

static AnalogDateLines formatAnalogDateLines(const struct tm &timeInfo) {
    AnalogDateLines lines;
    if (!bruceConfig.analogClockShowDate && !bruceConfig.analogClockShowWeekday) return lines;

    char primaryBuff[24] = {0};
    char monthBuff[12] = {0};
    const bool showDate = bruceConfig.analogClockShowDate;
    const bool showWeekday = bruceConfig.analogClockShowWeekday;

    if (showDate && showWeekday) {
        // Day + date on the first line, month below.
        strftime(primaryBuff, sizeof(primaryBuff), "%a %d", &timeInfo);
        strftime(monthBuff, sizeof(monthBuff), "%b", &timeInfo);
        lines.primary = String(primaryBuff);
        lines.secondary = String(monthBuff);
    } else if (showDate) {
        // Date on the first line, month below.
        strftime(primaryBuff, sizeof(primaryBuff), "%d", &timeInfo);
        strftime(monthBuff, sizeof(monthBuff), "%b", &timeInfo);
        lines.primary = String(primaryBuff);
        lines.secondary = String(monthBuff);
    } else {
        strftime(primaryBuff, sizeof(primaryBuff), "%a", &timeInfo);
        lines.primary = String(primaryBuff);
    }

    return lines;
}

static int updateBufferedBatteryLevel(int rawBatteryLevel, int &smoothedBatteryQ8, int displayedBatteryLevel) {
    if (rawBatteryLevel < 0) return -1;

    if (smoothedBatteryQ8 < 0) smoothedBatteryQ8 = rawBatteryLevel << 8;
    else smoothedBatteryQ8 = ((smoothedBatteryQ8 * 7) + (rawBatteryLevel << 8)) / 8;

    int filteredLevel = (smoothedBatteryQ8 + 128) >> 8;
    if (filteredLevel < 0) filteredLevel = 0;
    if (filteredLevel > 100) filteredLevel = 100;

    if (displayedBatteryLevel < 0) return filteredLevel;
    if (filteredLevel >= displayedBatteryLevel + 2) return displayedBatteryLevel + 1;
    if (filteredLevel <= displayedBatteryLevel - 2) return displayedBatteryLevel - 1;
    return displayedBatteryLevel;
}

static int analogClockComplicationMinuteToken(const struct tm &timeInfo) {
    return (((timeInfo.tm_yday * 24) + timeInfo.tm_hour) * 60) + timeInfo.tm_min;
}

static void drawAnalogClockComplications(
    const AnalogClockLayout &layout, const struct tm &timeInfo, int batteryLevel, bool chargingNow
) {
    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    const int edgePad = 3;
    const int topY = edgePad;
    const int bottomY = tftHeight - 11;

    const AnalogDateLines topLeftDate = formatAnalogDateLines(timeInfo);
    if (topLeftDate.primary.length() > 0) tft.drawString(topLeftDate.primary, edgePad, topY, 1);
    if (topLeftDate.secondary.length() > 0) {
        const int secondLineY = topY + tft.fontHeight(1) + 1;
        tft.drawString(topLeftDate.secondary, edgePad, secondLineY, 1);
    }

    String topRightText = "";
    if (bruceConfig.analogClockShowWifi) topRightText += (wifiConnected ? "WF+" : "WF-");
    if (bruceConfig.analogClockShowBle) {
        if (topRightText.length() > 0) topRightText += " ";
        topRightText += (BLEConnected ? "BT+" : "BT-");
    }
    if (topRightText.length() > 0) {
        const int width = tft.textWidth(topRightText);
        tft.drawString(topRightText, tftWidth - width - edgePad, topY, 1);
    }

    String bottomLeftText = "";
    if (bruceConfig.analogClockShowBattery && batteryLevel >= 0) bottomLeftText = "B" + String(batteryLevel) + "%";
    if (bruceConfig.analogClockShowCharging && chargingNow) {
        if (bottomLeftText.length() > 0) bottomLeftText += " ";
        bottomLeftText += "CHG";
    }
    if (bottomLeftText.length() > 0) tft.drawString(bottomLeftText, edgePad, bottomY, 1);

    String bottomRightText = "";
    if (bruceConfig.analogClockShowDigitalTime) {
        const bool withSeconds = analogClockDigitalSecondsEnabled() || analogClockSecondHandEnabled();
        bottomRightText = formatAnalogDigitalTime(timeInfo, withSeconds);
    }
    if (bottomRightText.length() > 0) {
        const int width = tft.textWidth(bottomRightText);
        tft.drawString(bottomRightText, tftWidth - width - edgePad, bottomY, 1);
    }

}

static void drawAnalogClockFrame(
    const AnalogClockLayout &layout, const struct tm &timeInfo, bool showMenuHint, unsigned long hintStartTime,
    bool smoothSecond, int batteryLevel, bool chargingNow, bool refreshComplications
) {
    const uint16_t faceFillColor = analogClockFaceFillColor();
    const uint16_t faceStrokeColor = analogClockFaceStrokeColor();
    uint16_t secondColor = bruceConfig.secColor;
    if (secondColor == faceFillColor) secondColor = getComplementaryColor2(faceStrokeColor);
    const float secondFraction = smoothSecond ? ((millis() % 1000) / 1000.0f) : 0.0f;

    // Avoid full-screen blanking every frame to reduce visible flicker/flash and power draw.
    // Clear only the dial disc so corner complications can stay cached between minute refreshes.
    const int facePad = 3;
    tft.fillCircle(layout.centerX, layout.centerY, layout.outerRadius + facePad, faceFillColor);

    if (refreshComplications) {
        // Clear only corner complication areas when refreshing complication text.
        const int cornerPad = 2;
        const int cornerH = 13;
        const int cornerW = max(36, (tftWidth / 2) - 10);
        tft.fillRect(cornerPad, cornerPad, cornerW, cornerH, bruceConfig.bgColor);
        tft.fillRect(tftWidth - cornerPad - cornerW, cornerPad, cornerW, cornerH, bruceConfig.bgColor);
        tft.fillRect(cornerPad, tftHeight - cornerPad - cornerH, cornerW, cornerH, bruceConfig.bgColor);
        tft.fillRect(
            tftWidth - cornerPad - cornerW, tftHeight - cornerPad - cornerH, cornerW, cornerH, bruceConfig.bgColor
        );
    }

    drawAnalogClockFace(layout);
    if (refreshComplications) {
        drawAnalogClockComplications(layout, timeInfo, batteryLevel, chargingNow);
    }

    const float minuteAngle = (timeInfo.tm_min * 6.0f) + (timeInfo.tm_sec * 0.1f);
    const float hourAngle = ((timeInfo.tm_hour % 12) * 30.0f) + (timeInfo.tm_min * 0.5f) + (timeInfo.tm_sec / 120.0f);
    drawAnalogClockHand(
        layout.centerX,
        layout.centerY,
        minuteAngle,
        layout.minuteHandLength,
        layout.minuteHandThickness,
        faceStrokeColor
    );
    drawAnalogClockHand(
        layout.centerX, layout.centerY, hourAngle, layout.hourHandLength, layout.hourHandThickness, faceStrokeColor
    );

    if (analogClockSecondHandEnabled()) {
        float secondAngle = (timeInfo.tm_sec + secondFraction) * 6.0f;
        if (secondAngle > 360.0f) secondAngle -= 360.0f;
        const int secondTailLength = bruceConfig.analogClockShowSecondTail ? layout.secondTailLength : 0;
        drawAnalogClockHand(
            layout.centerX,
            layout.centerY,
            secondAngle,
            layout.secondHandLength,
            layout.secondHandThickness,
            secondColor,
            secondTailLength
        );
        tft.fillCircle(layout.centerX, layout.centerY, layout.centerDotRadius + 1, secondColor);
    }
    tft.fillCircle(layout.centerX, layout.centerY, layout.centerDotRadius, faceStrokeColor);

    if (showMenuHint && (millis() - hintStartTime < 5000)) {
        tft.setTextSize(1);
        tft.setTextColor(faceStrokeColor, faceFillColor);
        int hintY = layout.centerY + layout.outerRadius + 2;
        if (hintY > tftHeight - 11) hintY = tftHeight - 11;
        tft.drawCentreString("OK for menu", tftWidth / 2, hintY, 1);
    }
}

#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
namespace {
struct ToneStep {
    uint16_t frequency;
    uint16_t durationMs;
};

// Westminster Quarters timing model:
// - Canonical rhythm per 4-note change: quarter, quarter, quarter, half (5 beats total).
// - On the hour, 4 changes (16 notes) should span ~25 seconds before the first hour strike.
constexpr uint16_t kWestminsterBeatMs = 1250;
constexpr uint16_t kWestminsterGapMs = 100;
constexpr uint16_t kWestminsterPhraseGapMs = 0;
constexpr uint16_t kWestminsterShortToneMs = kWestminsterBeatMs - kWestminsterGapMs;
constexpr uint16_t kWestminsterLongToneMs = 2 * kWestminsterBeatMs;

// Match the original bell register more closely (B3, E4, F#4, G#4), and an E-like hour strike.
constexpr uint16_t kWestminsterNoteB3Hz = 247;
constexpr uint16_t kWestminsterNoteE4Hz = 330;
constexpr uint16_t kWestminsterNoteFs4Hz = 370;
constexpr uint16_t kWestminsterNoteGs4Hz = 415;
constexpr uint16_t kWestminsterHourStrikeHz = 165;
constexpr uint16_t kWestminsterHourStrikeToneMs = 700;
constexpr uint16_t kWestminsterHourStrikeGapMs = 3300;
constexpr uint16_t kWestminsterPreStrikePauseMs = 0;
constexpr uint16_t kWestminsterMinNoteGapMs = 35;
constexpr uint16_t kWestminsterMinHourStrikeGapMs = 220;

// Westminster Quarters changes in E major:
// 1: G# F# E B
// 2: E G# F# B
// 3: E F# G# E
// 4: G# E F# B
// 5: B F# G# E
constexpr ToneStep kWestminsterChange1[] = {
    {kWestminsterNoteGs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteFs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteE4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteB3Hz, kWestminsterLongToneMs}
};
constexpr ToneStep kWestminsterChange2[] = {
    {kWestminsterNoteE4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteGs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteFs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteB3Hz, kWestminsterLongToneMs}
};
constexpr ToneStep kWestminsterChange3[] = {
    {kWestminsterNoteE4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteFs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteGs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteE4Hz, kWestminsterLongToneMs}
};
constexpr ToneStep kWestminsterChange4[] = {
    {kWestminsterNoteGs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteE4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteFs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteB3Hz, kWestminsterLongToneMs}
};
constexpr ToneStep kWestminsterChange5[] = {
    {kWestminsterNoteB3Hz, kWestminsterShortToneMs},
    {kWestminsterNoteFs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteGs4Hz, kWestminsterShortToneMs},
    {kWestminsterNoteE4Hz, kWestminsterLongToneMs}
};

uint16_t scaleWestminsterDurationMs(uint16_t baseDurationMs, int speedMultiplier) {
    if (baseDurationMs == 0) return 0;
    if (speedMultiplier < 1) speedMultiplier = 1;
    if (speedMultiplier > 5) speedMultiplier = 5;

    uint16_t scaled = baseDurationMs / static_cast<uint16_t>(speedMultiplier);
    if (scaled == 0) scaled = 1;
    return scaled;
}

void playToneSequence(const ToneStep *sequence, size_t length, int speedMultiplier, uint16_t gapMs = kWestminsterGapMs) {
    const uint16_t scaledGapMs = scaleWestminsterDurationMs(gapMs, speedMultiplier);
    const uint16_t effectiveGapMs =
        (gapMs == 0) ? 0 : (scaledGapMs < kWestminsterMinNoteGapMs ? kWestminsterMinNoteGapMs : scaledGapMs);
    for (size_t i = 0; i < length; ++i) {
        _tone(sequence[i].frequency, scaleWestminsterDurationMs(sequence[i].durationMs, speedMultiplier));
        if (i + 1 < length && effectiveGapMs > 0) delay(effectiveGapMs);
    }
}

void playWestminsterQuarterSequence(int quarterIndex, int speedMultiplier) {
    switch (quarterIndex) {
    case 1:
        // Quarter past: 1
        playToneSequence(kWestminsterChange1, sizeof(kWestminsterChange1) / sizeof(kWestminsterChange1[0]), speedMultiplier);
        break;
    case 2:
        // Half past: 2, 3
        playToneSequence(kWestminsterChange2, sizeof(kWestminsterChange2) / sizeof(kWestminsterChange2[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange3, sizeof(kWestminsterChange3) / sizeof(kWestminsterChange3[0]), speedMultiplier);
        break;
    case 3:
        // Quarter to: 4, 5, 1
        playToneSequence(kWestminsterChange4, sizeof(kWestminsterChange4) / sizeof(kWestminsterChange4[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange5, sizeof(kWestminsterChange5) / sizeof(kWestminsterChange5[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange1, sizeof(kWestminsterChange1) / sizeof(kWestminsterChange1[0]), speedMultiplier);
        break;
    case 4:
        // Full hour prelude: 2, 3, 4, 5
        playToneSequence(kWestminsterChange2, sizeof(kWestminsterChange2) / sizeof(kWestminsterChange2[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange3, sizeof(kWestminsterChange3) / sizeof(kWestminsterChange3[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange4, sizeof(kWestminsterChange4) / sizeof(kWestminsterChange4[0]), speedMultiplier);
        delay(scaleWestminsterDurationMs(kWestminsterPhraseGapMs, speedMultiplier));
        playToneSequence(kWestminsterChange5, sizeof(kWestminsterChange5) / sizeof(kWestminsterChange5[0]), speedMultiplier);
        break;
    default:
        break;
    }
}

void playWestminsterHourStrike(int hour24, int speedMultiplier) {
    int hourCount = hour24 % 12;
    if (hourCount == 0) hourCount = 12;
    const uint16_t strikeToneMs = scaleWestminsterDurationMs(kWestminsterHourStrikeToneMs, speedMultiplier);
    uint16_t strikeGapMs = scaleWestminsterDurationMs(kWestminsterHourStrikeGapMs, speedMultiplier);
    if (strikeGapMs < kWestminsterMinHourStrikeGapMs) strikeGapMs = kWestminsterMinHourStrikeGapMs;
    for (int i = 0; i < hourCount; ++i) {
        _tone(kWestminsterHourStrikeHz, strikeToneMs);
        if (i + 1 < hourCount) delay(strikeGapMs);
    }
}
} // namespace
#endif

namespace {
constexpr uint8_t kWestminsterTriggerMinutes[] = {14, 29, 44, 59};
constexpr uint16_t kWestminsterChangeDurationMs = 6250;
constexpr unsigned long kClockAlarmSnoozeMs = 5UL * 60UL * 1000UL;
unsigned long gClockAlarmSnoozeDeadlineMs = 0;

int clampClockChimeSpeed(int speedMultiplier) {
    if (speedMultiplier < 1) return 1;
    if (speedMultiplier > 5) return 5;
    return speedMultiplier;
}

uint8_t getWestminsterTriggerStartSecond(int quarterIndex, int speedMultiplier) {
    if (quarterIndex < 1 || quarterIndex > 4) return 59;

    const uint32_t scaledPreludeDurationMs = (static_cast<uint32_t>(quarterIndex) * kWestminsterChangeDurationMs) /
                                             static_cast<uint32_t>(clampClockChimeSpeed(speedMultiplier));
    const uint32_t durationSeconds = scaledPreludeDurationMs / 1000U;

    if (durationSeconds >= 60U) return 0;
    if (durationSeconds == 0U) return 59;
    return static_cast<uint8_t>(60U - durationSeconds);
}

int getWestminsterQuarterIndexForCurrentTime(const struct tm &timeInfo, int speedMultiplier) {
    for (size_t i = 0; i < (sizeof(kWestminsterTriggerMinutes) / sizeof(kWestminsterTriggerMinutes[0])); ++i) {
        const int quarterIndex = static_cast<int>(i) + 1;
        if (timeInfo.tm_min == kWestminsterTriggerMinutes[i] &&
            timeInfo.tm_sec >= getWestminsterTriggerStartSecond(quarterIndex, speedMultiplier)) {
            return static_cast<int>(i) + 1;
        }
    }
    return 0;
}

bool isWestminsterQuarterEnabledByMode(int quarterIndex) {
    switch (bruceConfig.clockChimeMode) {
    case CLOCK_CHIME_MODE_HOUR:
        return quarterIndex == 4;
    case CLOCK_CHIME_MODE_HOUR_AND_HALF:
        return quarterIndex == 2 || quarterIndex == 4;
    case CLOCK_CHIME_MODE_FULL:
    default:
        return quarterIndex >= 1 && quarterIndex <= 4;
    }
}

bool isClockChimeInSilentWindow(const struct tm &timeInfo) {
    if (bruceConfig.clockChimeSilentEnabled == 0) return false;

    const int startHour = bruceConfig.clockChimeSilentStartHour;
    const int endHour = bruceConfig.clockChimeSilentEndHour;
    const int currentHour = timeInfo.tm_hour;

    if (startHour == endHour) return true;
    if (startHour < endHour) return currentHour >= startHour && currentHour < endHour;
    return currentHour >= startHour || currentHour < endHour;
}

int getWestminsterTriggerToken(const struct tm &timeInfo, int quarterIndex) {
    return (((timeInfo.tm_yday * 24) + timeInfo.tm_hour) * 4) + (quarterIndex - 1);
}

int getAlarmTriggerToken(const struct tm &timeInfo) {
    return (((timeInfo.tm_yday * 24) + timeInfo.tm_hour) * 60) + timeInfo.tm_min;
}

bool checkClockAlarmStopPressed(bool &snoozed) {
    if (check(PrevPress)) {
        snoozed = true;
        return true;
    }
    return check(SelPress) || check(EscPress) || check(NextPress) || check(AuxPress);
}

void runClockAlarmAlert(int toneStyle) {
    tft.fillScreen(bruceConfig.bgColor);
    drawMainBorderWithTitle("Alarm", false);
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, bruceConfig.bgColor);
    tft.drawCentreString("WAKE UP!", tftWidth / 2, (tftHeight / 2) - LH, 1);

    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.drawCentreString(clockAlertToneLabel(toneStyle), tftWidth / 2, (tftHeight / 2) + 4, 1);
    tft.drawCentreString("SEL/BACK stop  PREV snooze", tftWidth / 2, (tftHeight / 2) + 16, 1);

    bool snoozed = false;
    while (true) {
        if (checkClockAlarmStopPressed(snoozed)) break;
        playClockAlertToneOnce(toneStyle);
        if (checkClockAlarmStopPressed(snoozed)) break;

        unsigned long waitStartMs = millis();
        while (millis() - waitStartMs < 140UL) {
            if (checkClockAlarmStopPressed(snoozed)) break;
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if (snoozed) break;
    }

    if (snoozed) gClockAlarmSnoozeDeadlineMs = millis() + kClockAlarmSnoozeMs;
    else gClockAlarmSnoozeDeadlineMs = 0;
}
} // namespace

static bool maybeTriggerClockAlarm(const struct tm &timeInfo, bool &alarmSyncReady, int &lastAlarmTriggerToken) {
    if (!alarmSyncReady) {
        const bool alignedOnAlarmMinute = bruceConfig.clockAlarmEnabled &&
                                          timeInfo.tm_hour == bruceConfig.clockAlarmHour &&
                                          timeInfo.tm_min == bruceConfig.clockAlarmMinute;
        lastAlarmTriggerToken = alignedOnAlarmMinute ? getAlarmTriggerToken(timeInfo) : -1;
        alarmSyncReady = true;
        return false;
    }

    if (!bruceConfig.clockAlarmEnabled) {
        gClockAlarmSnoozeDeadlineMs = 0;
        return false;
    }

    const bool snoozeDue = gClockAlarmSnoozeDeadlineMs != 0 &&
                           static_cast<long>(millis() - gClockAlarmSnoozeDeadlineMs) >= 0;
    if (snoozeDue) {
        gClockAlarmSnoozeDeadlineMs = 0;
        runClockAlarmAlert(bruceConfig.clockAlarmTone);
        return true;
    }

    if (timeInfo.tm_hour != bruceConfig.clockAlarmHour || timeInfo.tm_min != bruceConfig.clockAlarmMinute) return false;

    const int alarmToken = getAlarmTriggerToken(timeInfo);
    if (alarmToken == lastAlarmTriggerToken) return false;
    lastAlarmTriggerToken = alarmToken;
    gClockAlarmSnoozeDeadlineMs = 0;
    runClockAlarmAlert(bruceConfig.clockAlarmTone);
    return true;
}

static void maybePlayClockChime(const struct tm &timeInfo, bool &chimeSyncReady, int &lastChimeTriggerToken) {
    const int chimeSpeedMultiplier = clampClockChimeSpeed(bruceConfig.clockChimeSpeed);
    if (!chimeSyncReady) {
        const int initialQuarterIndex = getWestminsterQuarterIndexForCurrentTime(timeInfo, chimeSpeedMultiplier);
        lastChimeTriggerToken = (initialQuarterIndex > 0) ? getWestminsterTriggerToken(timeInfo, initialQuarterIndex) : -1;
        chimeSyncReady = true;
        return;
    }

    const int quarterIndex = getWestminsterQuarterIndexForCurrentTime(timeInfo, chimeSpeedMultiplier);
    if (quarterIndex == 0) return;

    const int triggerToken = getWestminsterTriggerToken(timeInfo, quarterIndex);
    if (triggerToken == lastChimeTriggerToken) return;
    lastChimeTriggerToken = triggerToken;

    if (bruceConfig.soundEnabled == 0 || bruceConfig.clockChimeStyle == 0) return;
    if (!isWestminsterQuarterEnabledByMode(quarterIndex)) return;
    if (isClockChimeInSilentWindow(timeInfo)) return;

#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)
    if (bruceConfig.clockChimeStyle == 1) {
        playWestminsterQuarterSequence(quarterIndex, chimeSpeedMultiplier);
        if (quarterIndex == 4) {
            delay(scaleWestminsterDurationMs(kWestminsterPreStrikePauseMs, chimeSpeedMultiplier));
            playWestminsterHourStrike((timeInfo.tm_hour + 1) % 24, chimeSpeedMultiplier);
        }
    }
#else
    (void)timeInfo;
    (void)chimeSpeedMultiplier;
#endif
}

void runClockLoop(bool showMenuHint) {
    unsigned long lastTickMs = 0;
    unsigned long lastChimePollMs = 0;
    unsigned long hintStartTime = millis();
    bool hintVisible = showMenuHint;
    bool hintDrawn = false;
    bool frameDrawn = false;
    String lastRenderedTime = "";
    bool chimeSyncReady = false;
    int lastChimeTriggerToken = -1;
    bool alarmSyncReady = false;
    int lastAlarmTriggerToken = -1;

    const int frameX = BORDER_PAD_X;
    const int frameY = BORDER_PAD_X;
    const int frameW = tftWidth - 2 * BORDER_PAD_X;
    const int frameH = tftHeight - 2 * BORDER_PAD_X;
    const int timeBoxX = BORDER_PAD_X + 1;
    const int timeBoxY = (tftHeight / 2) - 16;
    const int timeBoxW = tftWidth - 2 * BORDER_PAD_X - 2;
    const int timeBoxH = (4 * LH) + 6;
    const int hintBoxX = BORDER_PAD_X + 1;
    const int hintBoxY = tftHeight / 2 + 20;
    const int hintBoxW = tftWidth - 2 * BORDER_PAD_X - 2;
    const int hintBoxH = 20;

#if defined(HAS_RTC)
#if defined(HAS_RTC_BM8563)
    _rtc.GetBm8563Time();
#endif
#if defined(HAS_RTC_PCF85063A)
    _rtc.GetPcf85063Time();
#endif
    _rtc.GetTime(&_time);
#endif

    // Delay due to SelPress() detected on run
    tft.fillScreen(bruceConfig.bgColor);
    delay(300);

    for (;;) {
        if (millis() - lastChimePollMs >= 100) {
#if defined(HAS_RTC)
            const struct tm chimeTimeInfo = _rtc.getTimeStruct();
#else
            const struct tm chimeTimeInfo = rtc.getTimeStruct();
#endif
            maybePlayClockChime(chimeTimeInfo, chimeSyncReady, lastChimeTriggerToken);
            lastChimePollMs = millis();
        }

        if (millis() - lastTickMs > 1000) {
#if defined(HAS_RTC)
            const struct tm timeInfo = _rtc.getTimeStruct();
#else
            const struct tm timeInfo = rtc.getTimeStruct();
#endif
            updateTimeStr(timeInfo);
            const bool alarmDisplayed = maybeTriggerClockAlarm(timeInfo, alarmSyncReady, lastAlarmTriggerToken);

            bool redraw = false;
            String currentTime = String(timeStr);

            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
#if defined(HAS_EINK)
            // Avoid intermediate panel updates while composing a full clock frame.
            tft.setEinkAutoFlushEnabled(false);
#endif

            if (alarmDisplayed) {
                frameDrawn = false;
                hintVisible = showMenuHint;
                hintDrawn = false;
                hintStartTime = millis();
                lastRenderedTime = "";
            }

            if (!frameDrawn) {
                tft.drawRect(frameX, frameY, frameW, frameH, bruceConfig.priColor);
                frameDrawn = true;
                redraw = true;
            }

            uint8_t f_size = 4;
            for (uint8_t i = 4; i > 0; i--) {
                if (i * LW * strlen(timeStr) < (tftWidth - BORDER_PAD_X * 2)) {
                    f_size = i;
                    break;
                }
            }

            if (currentTime != lastRenderedTime) {
                tft.fillRect(timeBoxX, timeBoxY, timeBoxW, timeBoxH, bruceConfig.bgColor);
                tft.setTextSize(f_size);
                tft.drawCentreString(timeStr, tftWidth / 2, tftHeight / 2 - 13, 1);
                lastRenderedTime = currentTime;
                redraw = true;
            }

            // "OK to show menu" hint management
            if (hintVisible && (millis() - hintStartTime < 5000)) {
                if (!hintDrawn) {
                    tft.setTextSize(1);
                    tft.fillRect(hintBoxX, hintBoxY, hintBoxW, hintBoxH, bruceConfig.bgColor);
                    tft.drawCentreString("OK to show menu", tftWidth / 2, tftHeight / 2 + 25, 1);
                    hintDrawn = true;
                    redraw = true;
                }
            } else if (hintVisible && (millis() - hintStartTime >= 5000)) {
                // Clear hint after 5 seconds
                tft.fillRect(hintBoxX, hintBoxY, hintBoxW, hintBoxH, bruceConfig.bgColor);
                hintVisible = false;
                hintDrawn = false;
                redraw = true;
            }
            lastTickMs = millis();
#if defined(HAS_EINK)
            tft.setEinkAutoFlushEnabled(true);
            if (redraw) einkFlushIfDirtyPartial(0);
#endif
        }

        // Checks to exit the loop
        if (check(SelPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            if (showMenuHint) {
                // Exits the loop to return to the caller (ClockMenu)
                break;
            } else {
                // Original behavior
                returnToMenu = true;
                break;
            }
        }

        if (check(EscPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            returnToMenu = true;
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void runStopwatchLoop(bool showMenuHint) {
#if defined(HAS_EINK)
    const bool showTenths = false;
    const uint32_t refreshIntervalMs = 500;
#else
    const bool showTenths = true;
    const uint32_t refreshIntervalMs = 100;
#endif

    const int frameX = BORDER_PAD_X;
    const int frameY = BORDER_PAD_X;
    const int frameW = tftWidth - 2 * BORDER_PAD_X;
    const int frameH = tftHeight - 2 * BORDER_PAD_X;
    const int titleY = BORDER_PAD_X + 3;
    const int timeBoxX = BORDER_PAD_X + 1;
    const int timeBoxY = (tftHeight / 2) - 17;
    const int timeBoxW = tftWidth - 2 * BORDER_PAD_X - 2;
    const int timeBoxH = 24;
    const int statusBoxX = timeBoxX;
    const int statusBoxY = timeBoxY + timeBoxH + 2;
    const int statusBoxW = timeBoxW;
    const int statusBoxH = 12;
    const int lapBoxX = timeBoxX;
    const int lapBoxY = statusBoxY + statusBoxH + 2;
    const int lapBoxW = timeBoxW;
    const int lapBoxH = 12;
    const int controlsBoxX = timeBoxX;
    const int controlsBoxY = tftHeight - BORDER_PAD_X - 14;
    const int controlsBoxW = timeBoxW;
    const int controlsBoxH = 12;

    bool running = false;
    bool frameDrawn = false;
    bool lastRunning = false;
    bool lastHintVisible = false;
    bool lastResetHint = false;
    bool lastLapVisible = false;
    uint32_t startedAtMs = 0;
    uint32_t accumulatedMs = 0;
    uint32_t lastLapStampMs = 0;
    uint32_t lastTickMs = 0;
    uint32_t lastAlarmCheckMs = 0;
    unsigned long hintStartTime = millis();
    String lastTimeText = "";
    String lastLapText = "";
    String lastLapRenderedText = "";
    uint16_t lapCount = 0;
    bool lapChanged = false;
    bool alarmSyncReady = false;
    int lastAlarmTriggerToken = -1;
    const unsigned long inputIgnoreUntilMs = millis() + 300;

    tft.fillScreen(bruceConfig.bgColor);
    delay(140);

    for (;;) {
        uint32_t nowMs = millis();

        if (nowMs - lastAlarmCheckMs >= 250U) {
            struct tm timeInfo;
#if defined(HAS_RTC)
            timeInfo = _rtc.getTimeStruct();
#else
            timeInfo = rtc.getTimeStruct();
#endif
            if (maybeTriggerClockAlarm(timeInfo, alarmSyncReady, lastAlarmTriggerToken)) {
                frameDrawn = false;
                hintStartTime = millis();
                lastTimeText = "";
                lastLapRenderedText = "";
                lastTickMs = 0;
            }
            lastAlarmCheckMs = millis();
            nowMs = millis();
        }

        const uint32_t elapsedMs = running ? (accumulatedMs + (nowMs - startedAtMs)) : accumulatedMs;
        const bool hintVisible = showMenuHint && (nowMs - hintStartTime < 5000);
        const bool resetHintVisible = !running && elapsedMs > 0U;
        const bool lapVisible = lapCount > 0U;

        const bool timeRenderDue = !frameDrawn || (nowMs - lastTickMs >= refreshIntervalMs);
        const bool stateChanged = running != lastRunning;
        const bool hintChanged = hintVisible != lastHintVisible || resetHintVisible != lastResetHint;
        const bool lapVisibilityChanged = lapVisible != lastLapVisible;
        if (timeRenderDue || stateChanged || hintChanged || lapChanged || lapVisibilityChanged) {
            bool redraw = false;
            const bool drawStatic = !frameDrawn;

            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
#if defined(HAS_EINK)
            tft.setEinkAutoFlushEnabled(false);
#endif
            if (drawStatic) {
                tft.drawRect(frameX, frameY, frameW, frameH, bruceConfig.priColor);
                tft.setTextSize(1);
                tft.drawCentreString("Stopwatch", tftWidth / 2, titleY, 1);
                frameDrawn = true;
                redraw = true;
            }

            const String timeText = formatStopwatchTime(elapsedMs, showTenths);
            if (timeText != lastTimeText || drawStatic) {
                tft.fillRect(timeBoxX, timeBoxY, timeBoxW, timeBoxH, bruceConfig.bgColor);
                tft.setTextSize(showTenths ? 3 : 4);
                tft.drawCentreString(timeText, tftWidth / 2, timeBoxY + (showTenths ? 2 : -2), 1);
                lastTimeText = timeText;
                redraw = true;
            }

            if (stateChanged || drawStatic) {
                tft.fillRect(statusBoxX, statusBoxY, statusBoxW, statusBoxH, bruceConfig.bgColor);
                tft.setTextSize(1);
                tft.drawCentreString(running ? "RUNNING" : "PAUSED", tftWidth / 2, statusBoxY + 1, 1);
                redraw = true;
            }

            if (lapChanged || lapVisibilityChanged || drawStatic || lastLapRenderedText != lastLapText) {
                tft.fillRect(lapBoxX, lapBoxY, lapBoxW, lapBoxH, bruceConfig.bgColor);
                if (lapVisible) {
                    tft.setTextSize(1);
                    tft.drawCentreString(lastLapText, tftWidth / 2, lapBoxY + 1, 1);
                }
                lastLapRenderedText = lastLapText;
                redraw = true;
            }

            if (hintChanged || drawStatic) {
                tft.fillRect(controlsBoxX, controlsBoxY, controlsBoxW, controlsBoxH, bruceConfig.bgColor);
                tft.setTextSize(1);
                if (hintVisible) {
                    tft.drawCentreString("Top/Next: Start/Stop", tftWidth / 2, controlsBoxY + 1, 1);
                } else if (running) {
                    tft.drawCentreString("Top/Next: Stop  Prev: Lap", tftWidth / 2, controlsBoxY + 1, 1);
                } else if (resetHintVisible) {
                    tft.drawCentreString("Top/Next: Start  Prev: Reset", tftWidth / 2, controlsBoxY + 1, 1);
                } else {
                    tft.drawCentreString("Top/Next: Start", tftWidth / 2, controlsBoxY + 1, 1);
                }
                redraw = true;
            }

            lastRunning = running;
            lastHintVisible = hintVisible;
            lastResetHint = resetHintVisible;
            lastLapVisible = lapVisible;
            lastTickMs = nowMs;
            lapChanged = false;

#if defined(HAS_EINK)
            tft.setEinkAutoFlushEnabled(true);
            if (redraw) einkFlushIfDirtyPartial(0);
#endif
        }

        // Ignore stale key events from the menu transition into stopwatch.
        if (millis() < inputIgnoreUntilMs) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        auto toggleRunningState = [&]() {
            const uint32_t toggleMs = millis();
            if (running) accumulatedMs += (toggleMs - startedAtMs);
            else startedAtMs = toggleMs;
            running = !running;
        };

        if (check(AuxPress)) {
            toggleRunningState();

            // On CoreInk, top button is mapped to EscPress globally. Consume that mapping here.
            check(EscPress);
            continue;
        }

        if (check(NextPress)) {
            toggleRunningState();
            continue;
        }

        if (check(PrevPress)) {
            if (running) {
                uint32_t lapElapsedMs = elapsedMs - lastLapStampMs;
                lastLapStampMs = elapsedMs;
                lapCount++;
                lastLapText =
                    "Lap " + String(lapCount) + " +" + formatStopwatchTime(lapElapsedMs, showTenths);
                lapChanged = true;
                continue;
            }
            if (elapsedMs > 0U) {
                accumulatedMs = 0;
                lastLapStampMs = 0;
                lapCount = 0;
                lastTimeText = "";
                lastLapText = "";
                lapChanged = true;
                check(EscPress);
                continue;
            }
        }

        if (check(SelPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            if (showMenuHint) break;
            returnToMenu = true;
            break;
        }

        if (check(EscPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            returnToMenu = true;
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void runAnalogClockLoop(bool showMenuHint) {
    unsigned long lastTickMs = 0;
    unsigned long lastChimePollMs = 0;
    unsigned long hintStartTime = millis();
    bool frameDrawn = false;
    bool lastHintVisible = false;

    int lastHour = -1;
    int lastMinute = -1;
    int lastSecond = -1;
    int lastDay = -1;
    int lastMonth = -1;
    int lastYear = -1;
    int displayedBattery = -1;
    int smoothedBatteryQ8 = -1;
    int lastComplicationMinuteToken = -1;
    int lastSmoothSlice = -1;
    bool chimeSyncReady = false;
    int lastChimeTriggerToken = -1;
    bool alarmSyncReady = false;
    int lastAlarmTriggerToken = -1;

    const bool secondHandEnabled = analogClockSecondHandEnabled();
    bool smoothSecond = secondHandEnabled && bruceConfig.analogClockSmoothSecondHand;
    const unsigned long updateIntervalMs = smoothSecond ? 200 : 1000;
    const AnalogClockLayout layout = getAnalogClockLayout();

    tft.fillScreen(bruceConfig.bgColor);
    delay(120);

    for (;;) {
        if (millis() - lastChimePollMs >= 100) {
            struct tm chimeTimeInfo;
#if defined(HAS_RTC)
            chimeTimeInfo = _rtc.getTimeStruct();
#else
            chimeTimeInfo = rtc.getTimeStruct();
#endif
            maybePlayClockChime(chimeTimeInfo, chimeSyncReady, lastChimeTriggerToken);
            lastChimePollMs = millis();
        }

        if (millis() - lastTickMs >= updateIntervalMs) {
            struct tm timeInfo;
#if defined(HAS_RTC)
            timeInfo = _rtc.getTimeStruct();
#else
            timeInfo = rtc.getTimeStruct();
#endif
            const bool alarmDisplayed = maybeTriggerClockAlarm(timeInfo, alarmSyncReady, lastAlarmTriggerToken);

            const bool hintVisible = showMenuHint && (millis() - hintStartTime < 5000);
            const bool hintChanged = hintVisible != lastHintVisible;
            const int smoothSlice = smoothSecond ? static_cast<int>(millis() / updateIntervalMs) : 0;
            const bool needsBatteryLevel = bruceConfig.analogClockShowBattery;
            const int rawBatteryLevel = needsBatteryLevel ? getBattery() : -1;
            if (needsBatteryLevel) {
                displayedBattery = updateBufferedBatteryLevel(rawBatteryLevel, smoothedBatteryQ8, displayedBattery);
            } else {
                displayedBattery = -1;
                smoothedBatteryQ8 = -1;
            }
            const bool chargingState = bruceConfig.analogClockShowCharging ? isCharging() : false;
            const int complicationMinuteToken = analogClockComplicationMinuteToken(timeInfo);
            const bool refreshComplications = !frameDrawn || complicationMinuteToken != lastComplicationMinuteToken;
            const bool useSecondGranularity = secondHandEnabled ||
                                              (bruceConfig.analogClockShowDigitalTime &&
                                               analogClockDigitalSecondsEnabled());

            bool redraw = !frameDrawn || timeInfo.tm_hour != lastHour || timeInfo.tm_min != lastMinute || hintChanged;
            if (useSecondGranularity) {
                if (smoothSecond) redraw = redraw || (smoothSlice != lastSmoothSlice) || (timeInfo.tm_sec != lastSecond);
                else redraw = redraw || (timeInfo.tm_sec != lastSecond);
            }
            if (bruceConfig.analogClockShowDate || bruceConfig.analogClockShowWeekday) {
                redraw = redraw || timeInfo.tm_mday != lastDay || timeInfo.tm_mon != lastMonth || timeInfo.tm_year != lastYear;
            }
            if (alarmDisplayed) {
                frameDrawn = false;
                hintStartTime = millis();
                lastHintVisible = false;
                lastHour = -1;
                lastMinute = -1;
                lastSecond = -1;
                redraw = true;
            }

            if (redraw) {
#if defined(HAS_EINK)
                // Batch draw operations so the panel updates only once per frame.
                tft.setEinkAutoFlushEnabled(false);
#endif
                drawAnalogClockFrame(
                    layout,
                    timeInfo,
                    showMenuHint,
                    hintStartTime,
                    smoothSecond,
                    displayedBattery,
                    chargingState,
                    refreshComplications
                );
                frameDrawn = true;
                if (refreshComplications) lastComplicationMinuteToken = complicationMinuteToken;
#if defined(HAS_EINK)
                tft.setEinkAutoFlushEnabled(true);
                einkFlushIfDirtyPartial(0);
#endif
            }

            lastHour = timeInfo.tm_hour;
            lastMinute = timeInfo.tm_min;
            lastSecond = timeInfo.tm_sec;
            lastDay = timeInfo.tm_mday;
            lastMonth = timeInfo.tm_mon;
            lastYear = timeInfo.tm_year;
            lastSmoothSlice = smoothSlice;
            lastHintVisible = hintVisible;
            lastTickMs = millis();
        }

        if (check(SelPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            if (showMenuHint) break;
            returnToMenu = true;
            break;
        }

        if (check(EscPress)) {
            tft.fillScreen(bruceConfig.bgColor);
            returnToMenu = true;
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*********************************************************************
**  Function: gsetIrTxPin
**  get or set IR Tx Pin
**********************************************************************/
int gsetIrTxPin(bool set) {
    int result = bruceConfigPins.irTx;

    if (result > 50) bruceConfigPins.setIrTxPin(TXLED);
    if (set) {
        options.clear();
        std::vector<std::pair<const char *, int>> pins;
        pins = IR_TX_PINS;
        int idx = 100;
        int j = 0;
        for (auto pin : pins) {
            if (pin.second == bruceConfigPins.irTx && idx == 100) idx = j;
            j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
            int i = pin.second;
            if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI && i != TFT_BL &&
                i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI && i != SDCARD_MISO)
#endif
                options.push_back(
                    {pin.first,
                     [=]() { bruceConfigPins.setIrTxPin(pin.second); },
                     pin.second == bruceConfigPins.irTx}
                );
        }

        loopOptions(options, idx);
        options.clear();

        Serial.println("Saved pin: " + String(bruceConfigPins.irTx));
    }

    returnToMenu = true;
    return bruceConfigPins.irTx;
}

void setIrTxRepeats() {
    uint8_t chRpts = 0; // Chosen Repeats

    options = {
        {"None",             [&]() { chRpts = 0; } },
        {"5  (+ 1 initial)", [&]() { chRpts = 5; } },
        {"10 (+ 1 initial)", [&]() { chRpts = 10; }},
        {"Custom",           [&]() {
             // up to 99 repeats
             String rpt =
                 num_keyboard(String(bruceConfigPins.irTxRepeats), 2, "Nbr of Repeats (+ 1 initial)");
             chRpts = static_cast<uint8_t>(rpt.toInt());
         }                       },
    };
    addOptionToMainMenu();

    loopOptions(options);

    if (returnToMenu) return;

    bruceConfigPins.setIrTxRepeats(chRpts);
}
/*********************************************************************
**  Function: gsetIrRxPin
**  get or set IR Rx Pin
**********************************************************************/
int gsetIrRxPin(bool set) {
    int result = bruceConfigPins.irRx;

    if (result > 45) bruceConfigPins.setIrRxPin(GROVE_SCL);
    if (set) {
        options.clear();
        std::vector<std::pair<const char *, int>> pins;
        pins = IR_RX_PINS;
        int idx = -1;
        int j = 0;
        for (auto pin : pins) {
            if (pin.second == bruceConfigPins.irRx && idx < 0) idx = j;
            j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
            int i = pin.second;
            if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI && i != TFT_BL &&
                i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI && i != SDCARD_MISO)
#endif
                options.push_back(
                    {pin.first,
                     [=]() { bruceConfigPins.setIrRxPin(pin.second); },
                     pin.second == bruceConfigPins.irRx}
                );
        }

        loopOptions(options);
    }

    returnToMenu = true;
    return bruceConfigPins.irRx;
}

/*********************************************************************
**  Function: gsetRfTxPin
**  get or set RF Tx Pin
**********************************************************************/
int gsetRfTxPin(bool set) {
    int result = bruceConfigPins.rfTx;

    if (result > 45) bruceConfigPins.setRfTxPin(GROVE_SDA);
    if (set) {
        options.clear();
        std::vector<std::pair<const char *, int>> pins;
        pins = RF_TX_PINS;
        int idx = -1;
        int j = 0;
        for (auto pin : pins) {
            if (pin.second == bruceConfigPins.rfTx && idx < 0) idx = j;
            j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
            int i = pin.second;
            if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI && i != TFT_BL &&
                i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI && i != SDCARD_MISO)
#endif
                options.push_back(
                    {pin.first,
                     [=]() { bruceConfigPins.setRfTxPin(pin.second); },
                     pin.second == bruceConfigPins.rfTx}
                );
        }

        loopOptions(options);
        options.clear();
    }

    returnToMenu = true;
    return bruceConfigPins.rfTx;
}

/*********************************************************************
**  Function: gsetRfRxPin
**  get or set FR Rx Pin
**********************************************************************/
int gsetRfRxPin(bool set) {
    int result = bruceConfigPins.rfRx;

    if (result > 36) bruceConfigPins.setRfRxPin(GROVE_SCL);
    if (set) {
        options.clear();
        std::vector<std::pair<const char *, int>> pins;
        pins = RF_RX_PINS;
        int idx = -1;
        int j = 0;
        for (auto pin : pins) {
            if (pin.second == bruceConfigPins.rfRx && idx < 0) idx = j;
            j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
            int i = pin.second;
            if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI && i != TFT_BL &&
                i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI && i != SDCARD_MISO)
#endif
                options.push_back(
                    {pin.first,
                     [=]() { bruceConfigPins.setRfRxPin(pin.second); },
                     pin.second == bruceConfigPins.rfRx}
                );
        }

        loopOptions(options);
        options.clear();
    }

    returnToMenu = true;
    return bruceConfigPins.rfRx;
}

/*********************************************************************
**  Function: setStartupApp
**  Handles Menu to set startup app
**********************************************************************/
void setStartupApp() {
    int idx = 0;
    if (bruceConfig.startupApp == "") idx = 0;

    options = {
        {"None", [=]() { bruceConfig.setStartupApp(""); }, bruceConfig.startupApp == ""}
    };

    int index = 0;
    for (String appName : startupApp.getAppNames()) {
        index++;
        if (bruceConfig.startupApp == appName) idx = index;

        options.push_back({appName.c_str(), [=]() {
                               bruceConfig.setStartupApp(appName);
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
                               if (appName == "JS Interpreter") {
                                   options = getScriptsOptionsList("", true);
                                   loopOptions(options, MENU_TYPE_SUBMENU, "Startup Script");
                               }
#endif
                           }});
    }

    loopOptions(options, idx);
    options.clear();
}

/*********************************************************************
**  Function: setGpsBaudrateMenu
**  Handles Menu to set the baudrate for the GPS module
**********************************************************************/
void setGpsBaudrateMenu() {
    options = {
        {"9600 bps",   [=]() { bruceConfigPins.setGpsBaudrate(9600); },  bruceConfigPins.gpsBaudrate == 9600 },
        {"19200 bps",  [=]() { bruceConfigPins.setGpsBaudrate(19200); }, bruceConfigPins.gpsBaudrate == 19200},
        {"38400 bps",  [=]() { bruceConfigPins.setGpsBaudrate(38400); }, bruceConfigPins.gpsBaudrate == 38400},
        {"57600 bps",  [=]() { bruceConfigPins.setGpsBaudrate(57600); }, bruceConfigPins.gpsBaudrate == 57600},
        {"115200 bps",
         [=]() { bruceConfigPins.setGpsBaudrate(115200); },
         bruceConfigPins.gpsBaudrate == 115200                                                               },
    };

    loopOptions(options, bruceConfigPins.gpsBaudrate);
}

/*********************************************************************
**  Function: setWifiApSsidMenu
**  Handles Menu to set the WiFi AP SSID
**********************************************************************/
void setWifiApSsidMenu() {
    const bool isDefault = bruceConfig.wifiAp.ssid == "BruceNet";

    options = {
        {"Default (BruceNet)",
         [=]() { bruceConfig.setWifiApCreds("BruceNet", bruceConfig.wifiAp.pwd); },
         isDefault                                                                            },
        {"Custom",
         [=]() {
             String newSsid = keyboard(bruceConfig.wifiAp.ssid, 32, "WiFi AP SSID:");
             if (newSsid != "\x1B") {
                 if (!newSsid.isEmpty()) bruceConfig.setWifiApCreds(newSsid, bruceConfig.wifiAp.pwd);
                 else displayError("SSID cannot be empty", true);
             }
         },                                                                         !isDefault},
    };
    addOptionToMainMenu();

    loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApPasswordMenu
**  Handles Menu to set the WiFi AP Password
**********************************************************************/
void setWifiApPasswordMenu() {
    const bool isDefault = bruceConfig.wifiAp.pwd == "brucenet";

    options = {
        {"Default (brucenet)",
         [=]() { bruceConfig.setWifiApCreds(bruceConfig.wifiAp.ssid, "brucenet"); },
         isDefault                                                                             },
        {"Custom",
         [=]() {
             String newPassword = keyboard(bruceConfig.wifiAp.pwd, 32, "WiFi AP Password:", true);
             if (newPassword != "\x1B") {
                 if (!newPassword.isEmpty()) bruceConfig.setWifiApCreds(bruceConfig.wifiAp.ssid, newPassword);
                 else displayError("Password cannot be empty", true);
             }
         },                                                                          !isDefault},
    };
    addOptionToMainMenu();

    loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApCredsMenu
**  Handles Menu to configure WiFi AP Credentials
**********************************************************************/
void setWifiApCredsMenu() {
    options = {
        {"SSID",     setWifiApSsidMenu    },
        {"Password", setWifiApPasswordMenu},
    };
    addOptionToMainMenu();

    loopOptions(options);
}

/*********************************************************************
**  Function: setNetworkCredsMenu
**  Main Menu for setting Network credentials (BLE & WiFi)
**********************************************************************/
void setNetworkCredsMenu() {
    options = {
        {"WiFi AP Creds", setWifiApCredsMenu}
    };
    addOptionToMainMenu();

    loopOptions(options);
}

/*********************************************************************
**  Function: setBadUSBBLEMenu
**  Main Menu for setting Bad USB/BLE options
**********************************************************************/
void setBadUSBBLEMenu() {
    options = {
        {"Keyboard Layout", setBadUSBBLEKeyboardLayoutMenu},
        {"Key Delay",       setBadUSBBLEKeyDelayMenu      },
        {"Show Output",     setBadUSBBLEShowOutputMenu    },
    };
    addOptionToMainMenu();

    loopOptions(options);
}

/*********************************************************************
**  Function: setBadUSBBLEKeyboardLayoutMenu
**  Main Menu for setting Bad USB/BLE Keyboard Layout
**********************************************************************/
void setBadUSBBLEKeyboardLayoutMenu() {
    uint8_t opt = bruceConfig.badUSBBLEKeyboardLayout;

    options.clear();
    options = {
        {"US International",      [&]() { opt = 0; } },
        {"Danish",                [&]() { opt = 1; } },
        {"English (UK)",          [&]() { opt = 2; } },
        {"French (AZERTY)",       [&]() { opt = 3; } },
        {"German",                [&]() { opt = 4; } },
        {"Hungarian",             [&]() { opt = 5; } },
        {"Italian",               [&]() { opt = 6; } },
        {"Polish",                [&]() { opt = 7; } },
        {"Portuguese (Brazil)",   [&]() { opt = 8; } },
        {"Portuguese (Portugal)", [&]() { opt = 9; } },
        {"Slovenian",             [&]() { opt = 10; }},
        {"Spanish",               [&]() { opt = 11; }},
        {"Swedish",               [&]() { opt = 12; }},
        {"Turkish",               [&]() { opt = 13; }},
    };
    addOptionToMainMenu();

    loopOptions(options, opt);

    if (opt != bruceConfig.badUSBBLEKeyboardLayout) { bruceConfig.setBadUSBBLEKeyboardLayout(opt); }
}

/*********************************************************************
**  Function: setBadUSBBLEKeyDelayMenu
**  Main Menu for setting Bad USB/BLE Keyboard Key Delay
**********************************************************************/
void setBadUSBBLEKeyDelayMenu() {
    String delayStr = num_keyboard(String(bruceConfig.badUSBBLEKeyDelay), 3, "Key Delay (ms):");
    if (delayStr != "\x1B") {
        int delayVal = delayStr.toInt();
        if (delayVal >= 0 && delayVal <= 500) {
            bruceConfig.setBadUSBBLEKeyDelay(static_cast<uint16_t>(delayVal));
        } else {
            displayError("Invalid key delay value (0 to 500)", true);
        }
    }
}

/*********************************************************************
**  Function: setBadUSBBLEShowOutputMenu
**  Main Menu for setting Bad USB/BLE Show Output
**********************************************************************/
void setBadUSBBLEShowOutputMenu() {
    options.clear();
    options = {
        {"Enable",  [&]() { bruceConfig.setBadUSBBLEShowOutput(true); } },
        {"Disable", [&]() { bruceConfig.setBadUSBBLEShowOutput(false); }},
    };
    addOptionToMainMenu();

    loopOptions(options, bruceConfig.badUSBBLEShowOutput ? 0 : 1);
}

/*********************************************************************
**  Function: setMacAddressMenu - @IncursioHack
**  Handles Menu to configure WiFi MAC Address
**********************************************************************/
void setMacAddressMenu() {
    String currentMAC = bruceConfig.wifiMAC;
    if (currentMAC == "") currentMAC = WiFi.macAddress();

    options.clear();
    options = {
        {"Default MAC (" + WiFi.macAddress() + ")",
         [&]() { bruceConfig.setWifiMAC(""); },
         bruceConfig.wifiMAC == ""},
        {"Set Custom MAC",
         [&]() {
             String newMAC = keyboard(bruceConfig.wifiMAC, 17, "XX:YY:ZZ:AA:BB:CC");
             if (newMAC == "\x1B") return;
             if (newMAC.length() == 17) {
                 bruceConfig.setWifiMAC(newMAC);
             } else {
                 displayError("Invalid MAC format");
             }
         }, bruceConfig.wifiMAC != ""},
        {"Random MAC", [&]() {
             uint8_t randomMac[6];
             for (int i = 0; i < 6; i++) randomMac[i] = random(0x00, 0xFF);
             char buf[18];
             sprintf(
                 buf,
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 randomMac[0],
                 randomMac[1],
                 randomMac[2],
                 randomMac[3],
                 randomMac[4],
                 randomMac[5]
             );
             bruceConfig.setWifiMAC(String(buf));
         }}
    };

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_REGULAR, ("Current: " + currentMAC).c_str());
}

/*********************************************************************
**  Function: setSPIPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setSPIPinsMenu(BruceConfigPins::SPIPins &value) {
    uint8_t opt = 0;
    bool changed = false;
    BruceConfigPins::SPIPins points = value;

RELOAD:
    options = {
        {String("SCK =" + String(points.sck)).c_str(), [&]() { opt = 1; }},
        {String("MISO=" + String(points.miso)).c_str(), [&]() { opt = 2; }},
        {String("MOSI=" + String(points.mosi)).c_str(), [&]() { opt = 3; }},
        {String("CS  =" + String(points.cs)).c_str(), [&]() { opt = 4; }},
        {String("CE/GDO0=" + String(points.io0)).c_str(), [&]() { opt = 5; }},
        {String("NC/GDO2=" + String(points.io2)).c_str(), [&]() { opt = 6; }},
        {"Save Config", [&]() { opt = 7; }, changed},
        {"Main Menu", [&]() { opt = 0; }},
    };

    loopOptions(options);
    if (opt == 0) return;
    else if (opt == 7) {
        if (changed) {
            value = points;
            bruceConfigPins.setSpiPins(value);
        }
    } else {
        const auto pins = buildSelectablePins();
        if (opt == 1) points.sck = selectPinFromMenu(pins, points.sck);
        else if (opt == 2) points.miso = selectPinFromMenu(pins, points.miso);
        else if (opt == 3) points.mosi = selectPinFromMenu(pins, points.mosi);
        else if (opt == 4) points.cs = selectPinFromMenu(pins, points.cs);
        else if (opt == 5) points.io0 = selectPinFromMenu(pins, points.io0);
        else if (opt == 6) points.io2 = selectPinFromMenu(pins, points.io2);
        changed = true;
        goto RELOAD;
    }
}

/*********************************************************************
**  Function: setUARTPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setUARTPinsMenu(BruceConfigPins::UARTPins &value) {
    uint8_t opt = 0;
    bool changed = false;
    BruceConfigPins::UARTPins points = value;

RELOAD:
    options = {
        {String("RX = " + String(points.rx)).c_str(), [&]() { opt = 1; }},
        {String("TX = " + String(points.tx)).c_str(), [&]() { opt = 2; }},
        {"Save Config", [&]() { opt = 7; }, changed},
        {"Main Menu", [&]() { opt = 0; }},
    };

    loopOptions(options);
    if (opt == 0) return;
    else if (opt == 7) {
        if (changed) {
            value = points;
            bruceConfigPins.setUARTPins(value);
        }
    } else {
        const auto pins = buildSelectablePins();
        if (opt == 1) points.rx = selectPinFromMenu(pins, points.rx);
        else if (opt == 2) points.tx = selectPinFromMenu(pins, points.tx);
        changed = true;
        goto RELOAD;
    }
}

/*********************************************************************
**  Function: setI2CPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setI2CPinsMenu(BruceConfigPins::I2CPins &value) {
    uint8_t opt = 0;
    bool changed = false;
    BruceConfigPins::I2CPins points = value;

RELOAD:
    options = {
        {String("SDA = " + String(points.sda)).c_str(), [&]() { opt = 1; }},
        {String("SCL = " + String(points.scl)).c_str(), [&]() { opt = 2; }},
        {"Save Config", [&]() { opt = 7; }, changed},
        {"Main Menu", [&]() { opt = 0; }},
    };

    loopOptions(options);
    if (opt == 0) return;
    else if (opt == 7) {
        if (changed) {
            value = points;
            bruceConfigPins.setI2CPins(value);
        }
    } else {
        const auto pins = buildSelectablePins();
        if (opt == 1) points.sda = selectPinFromMenu(pins, points.sda);
        else if (opt == 2) points.scl = selectPinFromMenu(pins, points.scl);
        changed = true;
        goto RELOAD;
    }
}

/*********************************************************************
**  Function: setTheme
**  Menu to change Theme
**********************************************************************/
void setTheme() {
    FS *fs = &LittleFS;
    options = {
        {"Little FS", [&]() { fs = &LittleFS; }},
        {"Default",
         [&]() {
             bruceConfig.removeTheme();
             bruceConfig.themePath = "";
             bruceConfig.theme.fs = 0;
             bruceConfig.secColor = DEFAULT_SECCOLOR;
             bruceConfig.bgColor = TFT_BLACK;
             bruceConfig.setUiColor(DEFAULT_PRICOLOR);
#ifdef HAS_RGB_LED
             bruceConfig.ledBright = 50;
             bruceConfig.ledColor = 0x960064;
             bruceConfig.ledEffect = 0;
             bruceConfig.ledEffectSpeed = 5;
             bruceConfig.ledEffectDirection = 1;
             ledSetup();
#endif
             bruceConfig.saveFile();
             fs = nullptr;
         }                                     },
        {"Main Menu", [&]() { fs = nullptr; }  }
    };
    if (setupSdCard()) {
        options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});
    }
    loopOptions(options);
    if (fs == nullptr) return;

    String filepath = loopSD(*fs, true, "JSON");
    if (bruceConfig.openThemeFile(fs, filepath, true)) {
        bruceConfig.themePath = filepath;
        if (fs == &LittleFS) bruceConfig.theme.fs = 1;
        else if (fs == &SD) bruceConfig.theme.fs = 2;
        else bruceConfig.theme.fs = 0;

        bruceConfig.saveFile();
    }
}
#if !defined(LITE_VERSION)
BLE_API bleApi;
static bool ble_api_enabled = false;
#include <HTTPClient.h>

namespace {
constexpr const char *APP_STORE_PATH = "/BruceJS/Tools/App Store.js";
constexpr const char *APP_STORE_URL =
    "https://raw.githubusercontent.com/BruceDevices/App-Store/refs/heads/main/minified/App%20Store.js";

const char APP_STORE_BOOTSTRAP_SCRIPT[] PROGMEM = R"APPSTOREJS((function () {
  var wifi = require("wifi");
  var storage = require("storage");
  var dialog = require("dialog");

  var APP_STORE_URL = "https://raw.githubusercontent.com/BruceDevices/App-Store/refs/heads/main/minified/App%20Store.js";
  var APP_STORE_PATH = "/BruceJS/Tools/App Store.js";
  storage.mkdir("/BruceJS");
  storage.mkdir("/BruceJS/Tools");

  try {
    var response = wifi.httpFetch(APP_STORE_URL, {
      save: { path: APP_STORE_PATH, mode: "write" }
    });
    if (response && response.ok && response.saved) {
      dialog.success("App Store installed. Run it again.", true);
      return;
    }
  } catch (error) {}

  dialog.warning("App Store update failed. Check WiFi.", true);
})();)APPSTOREJS";

bool ensureAppStoreDirectoryTree(FS &fs) {
    if (!fs.exists("/BruceJS") && !fs.mkdir("/BruceJS")) { return false; }
    if (!fs.exists("/BruceJS/Tools") && !fs.mkdir("/BruceJS/Tools")) { return false; }
    return true;
}

bool writeAppStorePayload(FS &fs, const char *payload, size_t payloadLen) {
    if (!ensureAppStoreDirectoryTree(fs)) { return false; }
    if (fs.exists(APP_STORE_PATH) && !fs.remove(APP_STORE_PATH)) { return false; }

    File file = fs.open(APP_STORE_PATH, FILE_WRITE);
    if (!file) { return false; }

    size_t bytesWritten = file.write((const uint8_t *)payload, payloadLen);
    file.close();
    return bytesWritten == payloadLen;
}

bool sdCardHasScriptsTree() {
    return sdcardMounted && (SD.exists("/scripts") || SD.exists("/BruceScripts") || SD.exists("/BruceJS"));
}

FS *preferredAppStoreFs() {
    if (sdCardHasScriptsTree()) { return &SD; }
    return &LittleFS;
}
} // namespace

void enableBLEAPI() {
    if (!ble_api_enabled) {
        // displayWarning("BLE API require huge amount of RAM.");
        // displayWarning("Some features may stop working.");
        Serial.println(ESP.getFreeHeap());
        bleApi.setup();
        Serial.println(ESP.getFreeHeap());
    } else {
        bleApi.end();
    }

    ble_api_enabled = !ble_api_enabled;
}

bool appStoreInstalled() {
    if (LittleFS.exists(APP_STORE_PATH)) { return true; }
    if (sdcardMounted && SD.exists(APP_STORE_PATH)) { return true; }
    return false;
}

void ensureAppStorePreinstalled() {
    if (!LittleFS.exists(APP_STORE_PATH)) {
        writeAppStorePayload(
            LittleFS, APP_STORE_BOOTSTRAP_SCRIPT, sizeof(APP_STORE_BOOTSTRAP_SCRIPT) - 1
        );
    }

    if (sdCardHasScriptsTree() && !SD.exists(APP_STORE_PATH)) {
        writeAppStorePayload(
            SD, APP_STORE_BOOTSTRAP_SCRIPT, sizeof(APP_STORE_BOOTSTRAP_SCRIPT) - 1
        );
    }
}

void installAppStoreJS() {
    ensureAppStorePreinstalled();

    FS *fs = preferredAppStoreFs();
    if (!ensureAppStoreDirectoryTree(*fs)) {
        displayWarning("Failed to create /BruceJS/Tools directory", true);
        return;
    }

    if (WiFi.status() != WL_CONNECTED) { wifiConnectMenu(WIFI_STA); }
    if (WiFi.status() != WL_CONNECTED) {
        displayWarning("WiFi not connected; bundled App Store kept", true);
        return;
    }

    HTTPClient http;
    http.setReuse(false);
    http.begin(APP_STORE_URL);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        displayWarning("Failed to download App Store", true);
        return;
    }

    if (fs->exists(APP_STORE_PATH) && !fs->remove(APP_STORE_PATH)) {
        http.end();
        displayWarning("Failed to replace App Store", true);
        return;
    }

    File file = fs->open(APP_STORE_PATH, FILE_WRITE);
    if (!file) {
        http.end();
        displayWarning("Failed to save App Store", true);
        return;
    }

    int expectedBytes = http.getSize();
    int bytesWritten = http.writeToStream(&file);
    file.close();
    http.end();

    if (bytesWritten <= 0 || (expectedBytes > 0 && bytesWritten != expectedBytes)) {
        fs->remove(APP_STORE_PATH);
        writeAppStorePayload(LittleFS, APP_STORE_BOOTSTRAP_SCRIPT, sizeof(APP_STORE_BOOTSTRAP_SCRIPT) - 1);
        displayWarning("Failed to save App Store", true);
        return;
    }

    // Keep a safe offline bootstrap in flash in case an SD card script tree is removed later.
    if (!LittleFS.exists(APP_STORE_PATH)) {
        writeAppStorePayload(
            LittleFS, APP_STORE_BOOTSTRAP_SCRIPT, sizeof(APP_STORE_BOOTSTRAP_SCRIPT) - 1
        );
    }

    displaySuccess("App Store installed", true);
    displaySuccess("Goto JS Interpreter -> Tools -> App Store", true);
}
#endif
