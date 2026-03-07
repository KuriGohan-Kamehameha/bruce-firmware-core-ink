#include "ClockMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "modules/others/clock_alert_tones.h"
#include "modules/others/timer.h"
#include <time.h>

namespace {
struct WorldCity {
    const char *label;
    int16_t offsetMinutes;
};

constexpr int16_t kTorontoOffsetMarker = -32768;

constexpr WorldCity kWorldCities[] = {
    {"Toronto", kTorontoOffsetMarker},
    {"UTC",     0                   },
    {"London",  0                   },
    {"Tokyo",   9 * 60              },
    {"Dubai",   4 * 60              },
    {"Sydney",  10 * 60             },
    {"L.A.",    -8 * 60             },
};

String formatWorldTime(const struct tm &timeInfo) {
    char out[20] = {0};
    if (bruceConfig.clock24hr) {
        snprintf(out, sizeof(out), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);
        return String(out);
    }

    int hour12 = (timeInfo.tm_hour == 0)   ? 12
                 : (timeInfo.tm_hour > 12) ? timeInfo.tm_hour - 12
                                           : timeInfo.tm_hour;
    const char *ampm = (timeInfo.tm_hour < 12) ? "AM" : "PM";
    snprintf(out, sizeof(out), "%02d:%02d %s", hour12, timeInfo.tm_min, ampm);
    return String(out);
}

String formatCityTime(time_t utcEpoch, int16_t offsetMinutes) {
    struct tm cityTime = {};

    if (offsetMinutes == kTorontoOffsetMarker) {
        struct tm localTime = {};
        localtime_r(&utcEpoch, &localTime);
        cityTime = localTime;
    } else {
        const time_t cityEpoch = utcEpoch + (static_cast<time_t>(offsetMinutes) * 60);
        gmtime_r(&cityEpoch, &cityTime);
    }

    return formatWorldTime(cityTime);
}

String formatAlarmTimeLabel() {
    char out[16] = {0};
    snprintf(out, sizeof(out), "%02d:%02d", bruceConfig.clockAlarmHour, bruceConfig.clockAlarmMinute);
    return String(out);
}

void showAlarmTimeEditor() {
    std::vector<Option> hourOptions;
    hourOptions.reserve(24);
    for (int hour = 0; hour < 24; ++hour) {
        char label[4] = {0};
        snprintf(label, sizeof(label), "%02d", hour);
        hourOptions.push_back({String(label), [hour]() { bruceConfig.setClockAlarmHour(hour); }});
    }
    int selectedHour = loopOptions(hourOptions, MENU_TYPE_SUBMENU, "Alarm Hour", bruceConfig.clockAlarmHour);
    if (selectedHour < 0) return;

    std::vector<Option> minuteOptions;
    minuteOptions.reserve(60);
    for (int minute = 0; minute < 60; ++minute) {
        char label[4] = {0};
        snprintf(label, sizeof(label), "%02d", minute);
        minuteOptions.push_back({String(label), [minute]() { bruceConfig.setClockAlarmMinute(minute); }});
    }
    loopOptions(minuteOptions, MENU_TYPE_SUBMENU, "Alarm Minute", bruceConfig.clockAlarmMinute);
}

void showAlarmToneSelector() {
    std::vector<Option> toneOptions;
    toneOptions.reserve(CLOCK_ALERT_TONE_COUNT + 1);
    for (int tone = CLOCK_ALERT_TONE_MIN; tone <= CLOCK_ALERT_TONE_MAX; ++tone) {
        toneOptions.push_back(
            {clockAlertToneLabel(tone), [tone]() { bruceConfig.setClockAlarmTone(tone); }, bruceConfig.clockAlarmTone == tone}
        );
    }
    toneOptions.push_back({"Back", []() {}});

    while (true) {
        int selected = loopOptions(toneOptions, MENU_TYPE_SUBMENU, "Alarm Tone", bruceConfig.clockAlarmTone);
        if (selected < 0 || selected == static_cast<int>(toneOptions.size()) - 1) return;
        for (int tone = CLOCK_ALERT_TONE_MIN; tone <= CLOCK_ALERT_TONE_MAX; ++tone) {
            toneOptions[tone].selected = (bruceConfig.clockAlarmTone == tone);
        }
    }
}

String formatChimeHourLabel(int hour) {
    char out[8] = {0};
    snprintf(out, sizeof(out), "%02d:00", hour);
    return String(out);
}

String clockChimeModeLabel(int mode) {
    switch (mode) {
    case CLOCK_CHIME_MODE_HOUR:
        return "Hour";
    case CLOCK_CHIME_MODE_HOUR_AND_HALF:
        return "Hour+Half";
    case CLOCK_CHIME_MODE_FULL:
    default:
        return "Full";
    }
}

String analogFaceStyleLabel(int style) {
    switch (style) {
    case 0:
        return "Classic";
    case 1:
        return "Sport";
    case 2:
        return "Minimal";
    case 3:
        return "Field";
    case 4:
        return "Tactical";
    default:
        return "Sport";
    }
}

String analogHandStyleLabel(int style) {
    switch (style) {
    case 0:
        return "Balanced";
    case 1:
        return "Slim";
    case 2:
        return "Bold";
    default:
        return "Balanced";
    }
}

void showAnalogFaceStyleSelector() {
    while (true) {
        std::vector<Option> styleOptions = {
            {"Classic",  []() { bruceConfig.setAnalogClockFaceStyle(0); }, bruceConfig.analogClockFaceStyle == 0},
            {"Sport",    []() { bruceConfig.setAnalogClockFaceStyle(1); }, bruceConfig.analogClockFaceStyle == 1},
            {"Minimal",  []() { bruceConfig.setAnalogClockFaceStyle(2); }, bruceConfig.analogClockFaceStyle == 2},
            {"Field",    []() { bruceConfig.setAnalogClockFaceStyle(3); }, bruceConfig.analogClockFaceStyle == 3},
            {"Tactical", []() { bruceConfig.setAnalogClockFaceStyle(4); }, bruceConfig.analogClockFaceStyle == 4},
            {"Back",     []() {}                                                                     },
        };

        int selected = loopOptions(styleOptions, MENU_TYPE_SUBMENU, "Face Style", bruceConfig.analogClockFaceStyle);
        if (selected == -1 || selected == static_cast<int>(styleOptions.size()) - 1) return;
    }
}

void showAnalogHandStyleSelector() {
    while (true) {
        std::vector<Option> styleOptions = {
            {"Balanced", []() { bruceConfig.setAnalogClockHandStyle(0); }, bruceConfig.analogClockHandStyle == 0},
            {"Slim",     []() { bruceConfig.setAnalogClockHandStyle(1); }, bruceConfig.analogClockHandStyle == 1},
            {"Bold",     []() { bruceConfig.setAnalogClockHandStyle(2); }, bruceConfig.analogClockHandStyle == 2},
            {"Back",     []() {}                                                                     },
        };

        int selected = loopOptions(styleOptions, MENU_TYPE_SUBMENU, "Hand Style", bruceConfig.analogClockHandStyle);
        if (selected == -1 || selected == static_cast<int>(styleOptions.size()) - 1) return;
    }
}

void showChimeSpeedSelector() {
    std::vector<Option> speedOptions;
    speedOptions.reserve(6);
    for (int speed = 1; speed <= 5; ++speed) {
        speedOptions.push_back(
            {String(speed) + "x", [speed]() { bruceConfig.setClockChimeSpeed(speed); }, bruceConfig.clockChimeSpeed == speed}
        );
    }
    speedOptions.push_back({"Back", []() {}});

    while (true) {
        int selected = loopOptions(speedOptions, MENU_TYPE_SUBMENU, "Chime Speed", bruceConfig.clockChimeSpeed - 1);
        if (selected < 0 || selected == static_cast<int>(speedOptions.size()) - 1) return;
        for (int speed = 1; speed <= 5; ++speed) {
            speedOptions[speed - 1].selected = (bruceConfig.clockChimeSpeed == speed);
        }
    }
}

void showChimeModeSelector() {
    std::vector<Option> modeOptions = {
        {"Hour Only",
         []() { bruceConfig.setClockChimeMode(CLOCK_CHIME_MODE_HOUR); },
         bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_HOUR},
        {"Hour + Half",
         []() { bruceConfig.setClockChimeMode(CLOCK_CHIME_MODE_HOUR_AND_HALF); },
         bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_HOUR_AND_HALF},
        {"Full",
         []() { bruceConfig.setClockChimeMode(CLOCK_CHIME_MODE_FULL); },
         bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_FULL},
        {"Back", []() {}},
    };

    while (true) {
        int selected = loopOptions(modeOptions, MENU_TYPE_SUBMENU, "Chime Mode", bruceConfig.clockChimeMode);
        if (selected < 0 || selected == static_cast<int>(modeOptions.size()) - 1) return;
        modeOptions[0].selected = (bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_HOUR);
        modeOptions[1].selected = (bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_HOUR_AND_HALF);
        modeOptions[2].selected = (bruceConfig.clockChimeMode == CLOCK_CHIME_MODE_FULL);
    }
}

void showSilentStartHourSelector() {
    std::vector<Option> hourOptions;
    hourOptions.reserve(24);
    for (int hour = 0; hour < 24; ++hour) {
        hourOptions.push_back({formatChimeHourLabel(hour), [hour]() { bruceConfig.setClockChimeSilentStartHour(hour); }});
    }
    loopOptions(hourOptions, MENU_TYPE_SUBMENU, "Silent Start", bruceConfig.clockChimeSilentStartHour);
}

void showSilentEndHourSelector() {
    std::vector<Option> hourOptions;
    hourOptions.reserve(24);
    for (int hour = 0; hour < 24; ++hour) {
        hourOptions.push_back({formatChimeHourLabel(hour), [hour]() { bruceConfig.setClockChimeSilentEndHour(hour); }});
    }
    loopOptions(hourOptions, MENU_TYPE_SUBMENU, "Silent End", bruceConfig.clockChimeSilentEndHour);
}
} // namespace

void ClockMenu::optionsMenu() {
    while (!returnToMenu) {
        if (viewMode == ViewMode::Analog) {
            runAnalogClockLoop(true);
        } else if (viewMode == ViewMode::Digital) {
            runClockLoop(true);
        } else {
            runStopwatchLoop(true);
        }

        // If ESC is pressed on the watch, it exits
        if (returnToMenu) break;

        // OK pressed, show submenu
        showSubMenu();

        // If "Exit" is pressed in the submenu, it exits
        if (returnToMenu) break;
    }
}

bool ClockMenu::showClockViewSubMenu() {
    while (true) {
        bool selectedView = false;
        std::vector<Option> localOptions = {
            {"Analog Clock",
             [this, &selectedView]() {
                 viewMode = ViewMode::Analog;
                 selectedView = true;
             },
             viewMode == ViewMode::Analog},
            {"Digital Clock",
             [this, &selectedView]() {
                 viewMode = ViewMode::Digital;
                 selectedView = true;
             },
             viewMode == ViewMode::Digital},
            {"Stopwatch",
             [this, &selectedView]() {
                 viewMode = ViewMode::Stopwatch;
                 selectedView = true;
             },
             viewMode == ViewMode::Stopwatch},
            {"Back", []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Clock Views");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return false;
        if (selectedView) return true;
    }
}

void ClockMenu::showClockToolsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"Timer",       [=]() { Timer(); }                },
            {"World Clock", [this]() { runWorldClockLoop(true); }},
            {"Back",        []() {}                           },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Clock Tools");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showAnalogSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"Face Settings",  [this]() { showAnalogFaceSettingsSubMenu(); }   },
            {"Hand Settings",  [this]() { showAnalogHandSettingsSubMenu(); }   },
            {"Complications",  [this]() { showAnalogComplicationsSubMenu(); }  },
            {"Back",           []() {}                                         },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Analog Settings");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showClockSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"Alarm Settings",  [this]() { showAlarmSettingsSubMenu(); }  },
            {"Chime Settings",  [this]() { showChimeSettingsSubMenu(); }  },
            {"Analog Settings", [this]() { showAnalogSettingsSubMenu(); } },
            {"Back",            []() {}                                   },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Clock Settings");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showSubMenu() {
    while (true) {
        bool returnToClock = false;
        std::vector<Option> localOptions = {
            {"Views",         [this, &returnToClock]() { returnToClock = showClockViewSubMenu(); }},
            {"Tools",         [this]() { showClockToolsSubMenu(); }                               },
            {"Settings",      [this]() { showClockSettingsSubMenu(); }                            },
            {"Back to Clock", [&returnToClock]() { returnToClock = true; }                        },
            {"Exit",          [=]() { returnToMenu = true; }                                       },
        };

        delay(200);
        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Clock Menu");
        if (selected == -1 || returnToClock || returnToMenu) return;
    }
}

void ClockMenu::showAnalogFaceSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {String("Style: ") + analogFaceStyleLabel(bruceConfig.analogClockFaceStyle), []() { showAnalogFaceStyleSelector(); }},
            {String("Minute Ticks: ") + (bruceConfig.analogClockShowMinuteTicks ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowMinuteTicks(!bruceConfig.analogClockShowMinuteTicks);
             }},
            {String("Invert Face: ") + (bruceConfig.analogClockFaceInverted ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockFaceInverted(!bruceConfig.analogClockFaceInverted);
             }},
            {"Back", []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Face Settings");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showAnalogHandSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {String("Style: ") + analogHandStyleLabel(bruceConfig.analogClockHandStyle), []() { showAnalogHandStyleSelector(); }},
            {String("Second Hand: ") + (bruceConfig.analogClockShowSecondHand ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowSecondHand(!bruceConfig.analogClockShowSecondHand);
             }},
            {String("Smooth Sweep: ") + (bruceConfig.analogClockSmoothSecondHand ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockSmoothSecondHand(!bruceConfig.analogClockSmoothSecondHand);
             }},
            {String("Second Tail: ") + (bruceConfig.analogClockShowSecondTail ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowSecondTail(!bruceConfig.analogClockShowSecondTail);
             }},
            {"Back", []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Hand Settings");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showAnalogComplicationsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {String("Digital Time: ") + (bruceConfig.analogClockShowDigitalTime ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowDigitalTime(!bruceConfig.analogClockShowDigitalTime);
             }},
            {String("Digital Sec: ") + (bruceConfig.analogClockDigitalShowSeconds ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockDigitalShowSeconds(!bruceConfig.analogClockDigitalShowSeconds);
             }},
            {String("Date: ") + (bruceConfig.analogClockShowDate ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowDate(!bruceConfig.analogClockShowDate);
             }},
            {String("Weekday: ") + (bruceConfig.analogClockShowWeekday ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowWeekday(!bruceConfig.analogClockShowWeekday);
             }},
            {String("Battery: ") + (bruceConfig.analogClockShowBattery ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowBattery(!bruceConfig.analogClockShowBattery);
             }},
            {String("WiFi: ") + (bruceConfig.analogClockShowWifi ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowWifi(!bruceConfig.analogClockShowWifi);
             }},
            {String("BLE: ") + (bruceConfig.analogClockShowBle ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowBle(!bruceConfig.analogClockShowBle);
             }},
            {String("Charging: ") + (bruceConfig.analogClockShowCharging ? "ON" : "OFF"),
             [this]() {
                 bruceConfig.setAnalogClockShowCharging(!bruceConfig.analogClockShowCharging);
             }},
            {"Back", []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Complications");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showChimeSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"Off", [this]() { bruceConfig.setClockChimeStyle(0); }, bruceConfig.clockChimeStyle == 0},
            {"Westminster Quarters",
             [this]() { bruceConfig.setClockChimeStyle(1); },
             bruceConfig.clockChimeStyle == 1},
            {String("Speed: ") + String(bruceConfig.clockChimeSpeed) + "x", []() { showChimeSpeedSelector(); }},
            {String("Mode: ") + clockChimeModeLabel(bruceConfig.clockChimeMode), []() { showChimeModeSelector(); }},
            {String("Silent: ") + (bruceConfig.clockChimeSilentEnabled ? "ON" : "OFF"),
             []() { bruceConfig.setClockChimeSilentEnabled(bruceConfig.clockChimeSilentEnabled ? 0 : 1); }},
            {String("Silent On: ") + formatChimeHourLabel(bruceConfig.clockChimeSilentStartHour),
             []() { showSilentStartHourSelector(); }},
            {String("Silent Off: ") + formatChimeHourLabel(bruceConfig.clockChimeSilentEndHour),
             []() { showSilentEndHourSelector(); }},
            {"Back", []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Clock Chimes");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::showAlarmSettingsSubMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {String("Enabled: ") + (bruceConfig.clockAlarmEnabled ? "ON" : "OFF"),
             []() { bruceConfig.setClockAlarmEnabled(bruceConfig.clockAlarmEnabled ? 0 : 1); }},
            {String("Time: ") + formatAlarmTimeLabel(), []() { showAlarmTimeEditor(); }},
            {String("Tone: ") + clockAlertToneLabel(bruceConfig.clockAlarmTone), []() { showAlarmToneSelector(); }},
            {"Preview Tone", []() { playClockAlertTonePreview(bruceConfig.clockAlarmTone); }},
            {"Back",         []() {}                                                     },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Alarm Settings");
        if (selected == -1 || selected == static_cast<int>(localOptions.size()) - 1) return;
    }
}

void ClockMenu::runWorldClockLoop(bool showMenuHint) {
    unsigned long lastTickMs = 0;
    unsigned long hintStartTime = millis();
    bool hintVisible = showMenuHint;
    bool hintDrawn = false;
    bool frameDrawn = false;

    const int frameX = BORDER_PAD_X;
    const int frameY = BORDER_PAD_X;
    const int frameW = tftWidth - (2 * BORDER_PAD_X);
    const int frameH = tftHeight - (2 * BORDER_PAD_X);
    const int firstRowY = BORDER_PAD_X + 18;
    const int lineStep = 12;
    const int timeFieldX = tftWidth / 2;
    const int timeFieldW = tftWidth - BORDER_PAD_X - 4 - timeFieldX;
    const int hintBoxX = BORDER_PAD_X + 1;
    const int hintBoxY = tftHeight - BORDER_PAD_X - 11;
    const int hintBoxW = tftWidth - (2 * BORDER_PAD_X) - 2;
    const int hintBoxH = 10;

    tft.fillScreen(bruceConfig.bgColor);
    delay(120);

    for (;;) {
        if (millis() - lastTickMs >= 1000) {
            const time_t utcEpoch = time(nullptr);
            tft.setTextSize(1);
            tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
            bool redraw = false;
            const bool drawStatic = !frameDrawn;

            if (drawStatic) {
                tft.drawRect(frameX, frameY, frameW, frameH, bruceConfig.priColor);
                tft.drawCentreString("World Clock", tftWidth / 2, BORDER_PAD_X + 3, 1);
                redraw = true;
            }

            int y = firstRowY;
            for (const auto &city : kWorldCities) {
                if (drawStatic) tft.drawString(city.label, BORDER_PAD_X + 4, y, 1);

                const String cityTime = formatCityTime(utcEpoch, city.offsetMinutes);
                tft.fillRect(timeFieldX, y, timeFieldW, 10, bruceConfig.bgColor);
                const int timeWidth = tft.textWidth(cityTime);
                tft.drawString(cityTime, tftWidth - BORDER_PAD_X - 4 - timeWidth, y, 1);
                y += lineStep;
                if (y > tftHeight - 18) break;
            }
            if (drawStatic) frameDrawn = true;
            redraw = true;

            if (hintVisible && (millis() - hintStartTime < 5000)) {
                if (!hintDrawn) {
                    tft.fillRect(hintBoxX, hintBoxY, hintBoxW, hintBoxH, bruceConfig.bgColor);
                    tft.drawCentreString("OK for menu", tftWidth / 2, tftHeight - BORDER_PAD_X - 10, 1);
                    hintDrawn = true;
                    redraw = true;
                }
            } else if (hintVisible && hintDrawn) {
                tft.fillRect(hintBoxX, hintBoxY, hintBoxW, hintBoxH, bruceConfig.bgColor);
                hintVisible = false;
                hintDrawn = false;
                redraw = true;
            }

#if defined(HAS_EINK)
            if (redraw) einkFlushIfDirtyPartial(0);
#endif
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

void ClockMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 30;
    int pointerSize = scale * 15;

    // Case
    tft.drawArc(
        iconCenterX, iconCenterY, 1.1 * radius, radius, 0, 360, bruceConfig.priColor, bruceConfig.bgColor
    );

    // Pivot center
    tft.fillCircle(iconCenterX, iconCenterY, radius / 10, bruceConfig.priColor);

    // Hours & minutes
    tft.drawLine(
        iconCenterX,
        iconCenterY,
        iconCenterX - 2 * pointerSize / 3,
        iconCenterY - 2 * pointerSize / 3,
        bruceConfig.priColor
    );
    tft.drawLine(
        iconCenterX, iconCenterY, iconCenterX + pointerSize, iconCenterY - pointerSize, bruceConfig.priColor
    );
}
