#include "ducky_typer.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#if defined(USB_as_HID)
#include "tusb.h"
#endif
#if defined(ARDUINO_M5STACK_COREINK)
#include <driver/gpio.h>
#include <esp_sleep.h>
#endif

#define DEF_DELAY 100

uint8_t _Ask_for_restart = 0;
int currentOutputY = 0;

#if !defined(USB_as_HID)
HardwareSerial mySerial(1);
#endif

HIDInterface *hid_usb = nullptr;
HIDInterface *hid_ble = nullptr;

enum DuckyCommandType {
    DuckyCommandType_Cmd,
    DuckyCommandType_Print,
    DuckyCommandType_Delay,
    DuckyCommandType_Comment,
    DuckyCommandType_Repeat,
    DuckyCommandType_Combination,
    DuckyCommandType_WaitForButtonPress,
    DuckyCommandType_AltChar,
    DuckyCommandType_AltString,
    DuckyCommandType_StringDelay,
    DuckyCommandType_DefaultStringDelay
};

struct DuckyCommand {
    const char *command;
    char key;
    DuckyCommandType type;
};

struct DuckyCombination {
    const char *command;
    char key1;
    char key2;
    char key3;
};
const DuckyCombination duckyComb[]{
    {"CTRL-ALT",       KEY_LEFT_CTRL, KEY_LEFT_ALT,     0             },
    {"CTRL-SHIFT",     KEY_LEFT_CTRL, KEY_LEFT_SHIFT,   0             },
    {"CTRL-GUI",       KEY_LEFT_CTRL, KEY_LEFT_GUI,     0             },
    {"CTRL-ESCAPE",    KEY_LEFT_CTRL, KEY_ESC,          0             },
    {"ALT-SHIFT",      KEY_LEFT_ALT,  KEY_LEFT_SHIFT,   0             },
    {"ALT-GUI",        KEY_LEFT_ALT,  KEY_LEFT_GUI,     0             },
    {"GUI-SHIFT",      KEY_LEFT_GUI,  KEY_LEFT_SHIFT,   0             },
    {"GUI-SPACE",      KEY_LEFT_GUI,  KEY_SPACE,        0             },
    {"CTRL-ALT-SHIFT", KEY_LEFT_CTRL, KEY_LEFT_ALT,     KEY_LEFT_SHIFT},
    {"CTRL-ALT-GUI",   KEY_LEFT_CTRL, KEY_LEFT_ALT,     KEY_LEFT_GUI  },
    {"ALT-SHIFT-GUI",  KEY_LEFT_ALT,  KEY_LEFT_SHIFT,   KEY_LEFT_GUI  },
    {"CTRL-SHIFT-GUI", KEY_LEFT_CTRL, KEY_LEFT_SHIFT,   KEY_LEFT_GUI  },
    {"SYSREQ",         KEY_LEFT_ALT,  KEY_PRINT_SCREEN, 0             }
};

const DuckyCommand duckyCmds[]{
    {"REM",                   0,                DuckyCommandType_Comment           },
    {"//",                    0,                DuckyCommandType_Comment           },
    {"STRING",                0,                DuckyCommandType_Print             },
    {"STRINGLN",              0,                DuckyCommandType_Print             },
    {"DELAY",                 0,                DuckyCommandType_Delay             },
    {"DEFAULTDELAY",          DEF_DELAY,        DuckyCommandType_Delay             },
    {"DEFAULT_DELAY",         DEF_DELAY,        DuckyCommandType_Delay             },
    {"STRING_DELAY",          0,                DuckyCommandType_StringDelay       },
    {"STRINGDELAY",           0,                DuckyCommandType_StringDelay       },
    {"DEFAULT_STRING_DELAY",  0,                DuckyCommandType_DefaultStringDelay},
    {"DEFAULTSTRINGDELAY",    0,                DuckyCommandType_DefaultStringDelay},
    {"REPEAT",                0,                DuckyCommandType_Repeat            },
    {"WAIT_FOR_BUTTON_PRESS", 0,                DuckyCommandType_WaitForButtonPress},
    {"ALTCHAR",               0,                DuckyCommandType_AltChar           },
    {"ALTSTRING",             0,                DuckyCommandType_AltString         },
    {"ALTCODE",               0,                DuckyCommandType_AltString         },
    {"CTRL-ALT",              0,                DuckyCommandType_Combination       },
    {"CTRL-SHIFT",            0,                DuckyCommandType_Combination       },
    {"CTRL-GUI",              0,                DuckyCommandType_Combination       },
    {"CTRL-ESCAPE",           0,                DuckyCommandType_Combination       },
    {"ALT-SHIFT",             0,                DuckyCommandType_Combination       },
    {"ALT-GUI",               0,                DuckyCommandType_Combination       },
    {"GUI-SHIFT",             0,                DuckyCommandType_Combination       },
    {"GUI-SPACE",             0,                DuckyCommandType_Combination       },
    {"CTRL-ALT-SHIFT",        0,                DuckyCommandType_Combination       },
    {"CTRL-ALT-GUI",          0,                DuckyCommandType_Combination       },
    {"ALT-SHIFT-GUI",         0,                DuckyCommandType_Combination       },
    {"CTRL-SHIFT-GUI",        0,                DuckyCommandType_Combination       },
    {"SYSREQ",                0,                DuckyCommandType_Combination       },
    {"BACKSPACE",             KEYBACKSPACE,     DuckyCommandType_Cmd               },
    {"DELETE",                KEY_DELETE,       DuckyCommandType_Cmd               },
    {"ALT",                   KEY_LEFT_ALT,     DuckyCommandType_Cmd               },
    {"CTRL",                  KEY_LEFT_CTRL,    DuckyCommandType_Cmd               },
    {"CONTROL",               KEY_LEFT_CTRL,    DuckyCommandType_Cmd               },
    {"GUI",                   KEY_LEFT_GUI,     DuckyCommandType_Cmd               },
    {"WINDOWS",               KEY_LEFT_GUI,     DuckyCommandType_Cmd               },
    {"SHIFT",                 KEY_LEFT_SHIFT,   DuckyCommandType_Cmd               },
    {"ESCAPE",                KEY_ESC,          DuckyCommandType_Cmd               },
    {"ESC",                   KEY_ESC,          DuckyCommandType_Cmd               },
    {"TAB",                   KEYTAB,           DuckyCommandType_Cmd               },
    {"ENTER",                 KEY_RETURN,       DuckyCommandType_Cmd               },
    {"DOWNARROW",             KEY_DOWN_ARROW,   DuckyCommandType_Cmd               },
    {"DOWN",                  KEY_DOWN_ARROW,   DuckyCommandType_Cmd               },
    {"LEFTARROW",             KEY_LEFT_ARROW,   DuckyCommandType_Cmd               },
    {"LEFT",                  KEY_LEFT_ARROW,   DuckyCommandType_Cmd               },
    {"RIGHTARROW",            KEY_RIGHT_ARROW,  DuckyCommandType_Cmd               },
    {"RIGHT",                 KEY_RIGHT_ARROW,  DuckyCommandType_Cmd               },
    {"UPARROW",               KEY_UP_ARROW,     DuckyCommandType_Cmd               },
    {"UP",                    KEY_UP_ARROW,     DuckyCommandType_Cmd               },
    {"BREAK",                 KEY_PAUSE,        DuckyCommandType_Cmd               },
    {"PAUSE",                 KEY_PAUSE,        DuckyCommandType_Cmd               },
    {"CAPSLOCK",              KEY_CAPS_LOCK,    DuckyCommandType_Cmd               },
    {"END",                   KEY_END,          DuckyCommandType_Cmd               },
    {"HOME",                  KEY_HOME,         DuckyCommandType_Cmd               },
    {"INSERT",                KEY_INSERT,       DuckyCommandType_Cmd               },
    {"NUMLOCK",               LED_NUMLOCK,      DuckyCommandType_Cmd               },
    {"PAGEUP",                KEY_PAGE_UP,      DuckyCommandType_Cmd               },
    {"PAGEDOWN",              KEY_PAGE_DOWN,    DuckyCommandType_Cmd               },
    {"PRINTSCREEN",           KEY_PRINT_SCREEN, DuckyCommandType_Cmd               },
    {"SCROLLOCK",             KEY_SCROLL_LOCK,  DuckyCommandType_Cmd               },
    {"MENU",                  KEY_MENU,         DuckyCommandType_Cmd               },
    {"APP",                   KEY_MENU,         DuckyCommandType_Cmd               },
    {"F1",                    KEY_F1,           DuckyCommandType_Cmd               },
    {"F2",                    KEY_F2,           DuckyCommandType_Cmd               },
    {"F3",                    KEY_F3,           DuckyCommandType_Cmd               },
    {"F4",                    KEY_F4,           DuckyCommandType_Cmd               },
    {"F5",                    KEY_F5,           DuckyCommandType_Cmd               },
    {"F6",                    KEY_F6,           DuckyCommandType_Cmd               },
    {"F7",                    KEY_F7,           DuckyCommandType_Cmd               },
    {"F8",                    KEY_F8,           DuckyCommandType_Cmd               },
    {"F9",                    KEY_F9,           DuckyCommandType_Cmd               },
    {"F10",                   KEY_F10,          DuckyCommandType_Cmd               },
    {"F11",                   KEY_F11,          DuckyCommandType_Cmd               },
    {"F12",                   KEY_F12,          DuckyCommandType_Cmd               },
    {"F13",                   KEY_F13,          DuckyCommandType_Cmd               },
    {"F14",                   KEY_F14,          DuckyCommandType_Cmd               },
    {"F15",                   KEY_F15,          DuckyCommandType_Cmd               },
    {"F16",                   KEY_F16,          DuckyCommandType_Cmd               },
    {"F17",                   KEY_F17,          DuckyCommandType_Cmd               },
    {"F18",                   KEY_F18,          DuckyCommandType_Cmd               },
    {"F19",                   KEY_F19,          DuckyCommandType_Cmd               },
    {"F20",                   KEY_F20,          DuckyCommandType_Cmd               },
    {"F21",                   KEY_F21,          DuckyCommandType_Cmd               },
    {"F22",                   KEY_F22,          DuckyCommandType_Cmd               },
    {"F23",                   KEY_F23,          DuckyCommandType_Cmd               },
    {"F24",                   KEY_F24,          DuckyCommandType_Cmd               },
    {"SPACE",                 KEY_SPACE,        DuckyCommandType_Cmd               },
    {"FN",                    KEYFN,            DuckyCommandType_Cmd               },
    {"GLOBE",                 KEYFN,            DuckyCommandType_Cmd               },
};

const uint8_t *keyboardLayouts[] = {
    KeyboardLayout_en_US, // 0
    KeyboardLayout_da_DK, // 1
    KeyboardLayout_en_UK, // 2
    KeyboardLayout_fr_FR, // 3
    KeyboardLayout_de_DE, // 4
    KeyboardLayout_hu_HU, // 5
    KeyboardLayout_it_IT, // 6
    KeyboardLayout_en_US, // 7
    KeyboardLayout_pt_BR, // 8
    KeyboardLayout_pt_PT, // 9
    KeyboardLayout_si_SI, // 10
    KeyboardLayout_es_ES, // 11
    KeyboardLayout_sv_SE, // 12
    KeyboardLayout_tr_TR  // 13
};

void ducky_startKb(HIDInterface *&hid, bool ble) {
    Serial.printf("\nducky_startKb before hid==null: BLE: %d\n", ble);
    if (hid == nullptr) {
        Serial.printf("ducky_startKb after hid==null: BLE: %d\n", ble);
        if (ble) {
            // _Ask_for_restart change to 2 when use Disconnect option in BLE menu
            if (_Ask_for_restart == 2) {
                displayError("Restart your Device");
                returnToMenu = true;
            }
            hid = new BleKeyboard(bruceConfigPins.bleName, "BruceFW", 100);
        } else {
#if defined(USB_as_HID)
            hid = new USBHIDKeyboard();
            USB.begin();

            // Wait for USB subsystem to be ready
            while (!tud_mounted()) {
                printStatusBadUSBBLE("Waiting USB Host...");
                delay(500);
            }

            printStatusBadUSBBLE("USB Host Connected");
#else
            mySerial.begin(CH9329_DEFAULT_BAUDRATE, SERIAL_8N1, BAD_RX, BAD_TX);
            delay(100);
            hid = new CH9329_Keyboard_();
#endif
        }
    }
    if (ble) {
        if (hid->isConnected()) {
            // If connected as media controller and switch to BadBLE, changes the layout
            hid->setLayout(keyboardLayouts[bruceConfig.badUSBBLEKeyboardLayout]);
            hid->setDelay(bruceConfig.badUSBBLEKeyDelay);
            return;
        }
        if (!_Ask_for_restart) _Ask_for_restart = 1; // arm the flag
        hid->begin(keyboardLayouts[bruceConfig.badUSBBLEKeyboardLayout]);
        hid->setDelay(bruceConfig.badUSBBLEKeyDelay);
    } else {
#if defined(USB_as_HID)
        hid->begin(keyboardLayouts[bruceConfig.badUSBBLEKeyboardLayout]);
        hid->setDelay(bruceConfig.badUSBBLEKeyDelay);
#else
        mySerial.begin(CH9329_DEFAULT_BAUDRATE, SERIAL_8N1, BAD_RX, BAD_TX);
        delay(100);
        hid->begin(mySerial, keyboardLayouts[bruceConfig.badUSBBLEKeyboardLayout]);
        hid->setDelay(bruceConfig.badUSBBLEKeyDelay);
#endif
    }
}

// Start badUSBBLE or badBLE ducky runner
void ducky_setup(HIDInterface *&hid, bool ble) {
    Serial.println("Ducky typer begin");

    if (ble && bruceConfig.badUSBBLEKeyDelay < 50) {
        displayWarning("Key delay is below 50ms. You may experience issues with missing keys.", true);
    }

    tft.fillScreen(bruceConfig.bgColor);

    if (ble && _Ask_for_restart == 2) {
        displayError("Restart your Device");
        returnToMenu = true;
        return;
    }
    FS *fs = nullptr;
    bool first_time = true;

    tft.fillScreen(bruceConfig.bgColor);
    String bad_script = "";
    options = {};

    if (setupSdCard()) {
        options.push_back({"SD Card", [&]() { fs = &SD; }});
    }
    options.push_back({"LittleFS", [&]() { fs = &LittleFS; }});
    options.push_back({"Main Menu", [&]() { fs = nullptr; }});

    loopOptions(options);

    if (fs != nullptr) {
        bad_script = loopSD(*fs, true);
        if (bad_script == "") {
            displayWarning("Canceled", true);
            returnToMenu = true;
            goto EXIT;
        }
    StartRunningScript:
        printHeaderBadUSBBLE(bad_script);
        printStatusBadUSBBLE("Preparing");

        if (first_time) {
            printStatusBadUSBBLE("Preparing USB");
            ducky_startKb(hid, ble);
            if (returnToMenu) goto EXIT; // make sure to free the hid object before exiting
            first_time = false;
            if (!ble) {
#if !defined(USB_as_HID)
                mySerial.write(0x00);
                while (mySerial.available() <= 0) {
                    if (mySerial.available() <= 0) {
                        displayTextLine("CH9329 -> USB");
                        delay(200);
                        mySerial.write(0x00);
                    } else break;
                    if (check(EscPress)) {
                        displayError("CH9329 not found"); // Cancel run
                        delay(500);
                        goto EXIT;
                    }
                }
#endif
                printStatusBadUSBBLE("Preparing USB");
                delay(2000); // Time to Computer or device recognize the USB HID
            } else {
                printStatusBadUSBBLE("Waiting Victim");
                while (!hid->isConnected() && !check(EscPress));
                if (hid->isConnected()) {
                    BLEConnected = true;
                    printStatusBadUSBBLE("Preparing BLE");
                    delay(1000);
                } else {
                    displayWarning("Canceled", true);
                    goto EXIT;
                }
            }
        }
        printStatusBadUSBBLE(String(BTN_ALIAS) + " to start");
        if (!waitForButtonPress()) { goto EXIT; }
        delay(200);
        key_input(*fs, bad_script, hid);

        printStatusBadUSBBLE("Finished - " + String(BTN_ALIAS) + " to restart");
        if (!waitForButtonPress()) { goto EXIT; }

        goto StartRunningScript;
    }
EXIT:
    if (!ble) {
        delete hid; // Keep the hid object alive for BLE
        hid = nullptr;
#if !defined(USB_as_HID)
        mySerial.end();       // Stops UART Serial as HID
        Serial.begin(115200); // Force restart of Serial, just in case....
#endif
    }
    returnToMenu = true;
}

// Parses a file to run in the badUSBBLE
void key_input(FS fs, String bad_script, HIDInterface *_hid) {
    if (!fs.exists(bad_script) || bad_script == "") return;
    File payloadFile = fs.open(bad_script, "r");
    if (!payloadFile) return;
    String lineContent = "";
    String Command = "";
    char Cmd[25];
    String Argument = "";
    String RepeatTmp = "";

    // String delay variables
    static int nextStringDelay = -1; // One-time delay for next STRING command (-1 = use default)
    static int defaultStringDelay = bruceConfig.badUSBBLEKeyDelay; // Default delay for all STRING commands
    currentOutputY = 0;

    _hid->releaseAll();

    printHeaderBadUSBBLE(bad_script);

    printStatusBadUSBBLE("Running");

    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.priColor);
    tft.setCursor(BORDER_OFFSET_FROM_SCREEN_EDGE * 2, FP * 8 * 3 + 2 + STATUS_BAR_HEIGHT);
    tft.print("Run Time:");

    printDecimalTime(0);

    tft.drawLine(
        BORDER_OFFSET_FROM_SCREEN_EDGE,
        tftHeight / 2 - FP * 4 - 2,
        tftWidth - BORDER_OFFSET_FROM_SCREEN_EDGE,
        tftHeight / 2 - FP * 4 - 2,
        bruceConfig.priColor
    );
    if (!bruceConfig.badUSBBLEShowOutput) {
        tft.setTextSize(FP);
        tft.setTextColor(TFT_RED);
        tft.setCursor(BORDER_OFFSET_FROM_SCREEN_EDGE * 2, tftHeight / 2);
        tft.print("Script output disabled");
    }

    uint32_t startMillisBADUSBBLE = millis();

    while (payloadFile.available()) {

        previousMillis = millis(); // resets DimScreen
        if (check(SelPress)) {
            if (!handlePauseResume()) { goto EXIT; }
        }
        // CRLF is a combination of two control characters: the "Carriage Return" represented by
        // the character "\r" and the "Line Feed" represented by the character "\n".
        lineContent = payloadFile.readStringUntil('\n');
        if (lineContent.endsWith("\r")) lineContent.remove(lineContent.length() - 1);

        if (lineContent.length() == 0) continue; // skip empty lines

        int spaceIndex = lineContent.indexOf(' ');

        // Check if this is a REPEAT command
        if (spaceIndex > 0 && lineContent.substring(0, spaceIndex) == "REPEAT") {
            RepeatTmp = lineContent.substring(spaceIndex + 1);
            if (RepeatTmp.toInt() <= 0) {
                RepeatTmp = "1";
                printTFTBadUSBBLE("REPEAT argument NaN, repeating once", ALCOLOR, true);
            }
        } else if (spaceIndex == -1 && lineContent == "REPEAT") {
            RepeatTmp = "1";
            printTFTBadUSBBLE("REPEAT without argument, repeating once", ALCOLOR, true);
        } else {
            if (spaceIndex > 0) {
                Command = lineContent.substring(0, spaceIndex);
                Argument = lineContent.substring(spaceIndex + 1);
            } else {
                Command = lineContent;
                Argument = "";
            }
            strcpy(Cmd, Command.c_str());
            RepeatTmp = "1";
        }

        uint16_t i;
        uint16_t repeatCount = RepeatTmp.toInt();
        for (i = 0; i < repeatCount; i++) {
            DuckyCommand *PriCmd = findDuckyCommand(Cmd);
            DuckyCommand *ArgCmd = findDuckyCommand(Argument.c_str());

            if (PriCmd != nullptr) {
                // REM comment lines are processed here
                if (PriCmd->type == DuckyCommandType_Comment) {
                    // Do nothing for comments
                }
                // STRING and STRINGLN are processed here
                else if (PriCmd->type == DuckyCommandType_Print) {
                    // Set appropriate delay for this STRING command
                    int currentDelay = (nextStringDelay >= 0) ? nextStringDelay : defaultStringDelay;
                    _hid->setDelay(currentDelay);

                    _hid->print(Argument);
                    if (strcmp(PriCmd->command, "STRINGLN") == 0) _hid->println();

                    // Reset one-time delay after use
                    if (nextStringDelay >= 0) { nextStringDelay = -1; }
                }
                // WAIT_FOR_BUTTON_PRESS is processed here
                else if (PriCmd->type == DuckyCommandType_WaitForButtonPress) {
                    printStatusBadUSBBLE("Waiting for button press");
                    bool waitSelect = false;
                    while (!waitSelect) {
                        waitSelect = check(SelPress);
                        delay(50); // Small delay to prevent excessive CPU usage
                    }
                    printStatusBadUSBBLE("Running");
                    tft.setTextSize(1);
                }
                // DELAY and DEFAULTDELAY are processed here
                else if (PriCmd->type == DuckyCommandType_Delay) {
                    if ((int)PriCmd->key > 0) delay(DEF_DELAY); // Default delay is 10ms
                    else {
                        int delayTime = Argument.toInt();
                        if (delayTime > 0) delay(delayTime);
                        else delay(DEF_DELAY);
                    }
                }
                // ALTCHAR command is processed here
                else if (PriCmd->type == DuckyCommandType_AltChar) {
                    int charCode = Argument.toInt();
                    if (charCode > 0 && charCode <= 255) { sendAltChar(_hid, (uint8_t)charCode); }
                }
                // ALTSTRING and ALTCODE commands are processed here
                else if (PriCmd->type == DuckyCommandType_AltString) {
                    sendAltString(_hid, Argument);
                }
                // STRING_DELAY and STRINGDELAY commands are processed here
                else if (PriCmd->type == DuckyCommandType_StringDelay) {
                    int delayValue = Argument.toInt();
                    if (delayValue >= 0) { nextStringDelay = delayValue; }
                }
                // DEFAULT_STRING_DELAY and DEFAULTSTRINGDELAY commands are processed here
                else if (PriCmd->type == DuckyCommandType_DefaultStringDelay) {
                    int delayValue = Argument.toInt();
                    if (delayValue >= 0) { defaultStringDelay = delayValue; }
                }
                // Normal commands are processed here
                else if (PriCmd->type == DuckyCommandType_Cmd) {
                    _hid->press(PriCmd->key);
                }
                // Combinations are processed here
                else if (PriCmd->type == DuckyCommandType_Combination) {
                    DuckyCombination *comb = findDuckyCombination(Cmd);
                    if (comb != nullptr) {
                        _hid->press(comb->key1);
                        _hid->press(comb->key2);
                        if (comb->key3 != 0) _hid->press(comb->key3);
                    }
                }

                // Send keys
                if (PriCmd->type != DuckyCommandType_Comment) {
                    if (ArgCmd != nullptr && PriCmd != nullptr && ArgCmd->type == DuckyCommandType_Cmd &&
                        PriCmd->type == DuckyCommandType_Cmd) {
                        _hid->press(ArgCmd->key);
                    } else if (PriCmd != nullptr && PriCmd->type == DuckyCommandType_Cmd &&
                               Argument.length() > 0) {
                        for (int idx = 0; idx < Argument.length(); idx++) {
                            _hid->press(Argument.charAt(idx));
                        }
                    }
                    _hid->releaseAll();
                }
            }

            // Output to screen
            if (PriCmd == nullptr) {
                printTFTBadUSBBLE(Command + " - UNKNOWN COMMAND", ALCOLOR, true);
            } else if (PriCmd->type != DuckyCommandType_Comment) {
                printTFTBadUSBBLE(Command, bruceConfig.priColor);
                if (Argument.length() > 0) {
                    printTFTBadUSBBLE(" " + Argument, (ArgCmd == nullptr ? TFT_WHITE : NULL), true);
                } else printTFTBadUSBBLE("", NULL, true);
            } else if (PriCmd->type == DuckyCommandType_Comment) {
                printTFTBadUSBBLE(Argument, TFT_DARKGREEN, true);
            }
        }

        printDecimalTime(millis() - startMillisBADUSBBLE);
    }

    printStatusBadUSBBLE("Finished");

EXIT:
    tft.setTextSize(FP);
    payloadFile.close();
    _hid->releaseAll();
}

// Sends a simple command
void key_input_from_string(String text) {
    ducky_startKb(hid_usb, false);

    hid_usb->print(text.c_str()); // buggy with some special chars

    delete hid_usb;
    hid_usb = nullptr;
#if !defined(USB_as_HID)
    mySerial.end();
#endif
}
#ifndef KB_HID_EXIT_MSG
#define KB_HID_EXIT_MSG "Exit"
#endif
// Use device as a keyboard (USB or BLE)
void ducky_keyboard(HIDInterface *&hid, bool ble) {
    String _mymsg = "";
    keyStroke key;
    long debounce = millis();
    ducky_startKb(hid, ble);
    if (returnToMenu) return;

    if (ble) {
        displayTextLine("Waiting Victim");
        while (!hid->isConnected() && !check(EscPress));
        if (hid->isConnected()) {
            BLEConnected = true;
        } else {
            displayWarning("Canceled", true);
            goto EXIT;
        }
    } else {
        // send a key to start communication
        hid->press(KEY_LEFT_ALT);
        hid->releaseAll();
    }

    drawMainBorder();
    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.priColor);
    tft.drawString("Keyboard Started", tftWidth / 2, tftHeight / 2);

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FP);
    drawMainBorder();
    tft.setCursor(10, 28);
    if (ble) tft.println("BLE Keyboard:");
    else tft.println("USB Keyboard:");
    tft.drawCentreString("> " + String(KB_HID_EXIT_MSG) + " <", tftWidth / 2, tftHeight - 20, 1);
    tft.setTextSize(FP);

    while (1) {
#if defined(HAS_KEYBOARD)
        key = _getKeyPress();
        if (key.pressed && (millis() - debounce > 200)) {
            if (key.alt) hid->press(KEY_LEFT_ALT);
            if (key.ctrl) hid->press(KEY_LEFT_CTRL);
            if (key.gui) hid->press(KEY_LEFT_GUI);
            if (key.enter) hid->println();
            else if (key.del) hid->press(KEYBACKSPACE);
            else {
                for (char k : key.word) { hid->press(k); }
                for (auto k : key.modifier_keys) { hid->press(k); }
            }
            if (key.fn && key.exit_key) break;

            hid->releaseAll();

            // only text for tft
            String keyStr = "";
            for (auto i : key.word) {
                if (keyStr != "") {
                    keyStr = keyStr + "+" + i;
                } else {
                    keyStr += i;
                }
            }

            if (keyStr.length() > 0) {
                drawMainBorder(false);
                if (_mymsg.length() > keyStr.length())
                    tft.drawCentreString(
                        "                                  ", tftWidth / 2, tftHeight / 2, 1
                    ); // clears screen
                tft.drawCentreString("Pressed: " + keyStr, tftWidth / 2, tftHeight / 2, 1);
                _mymsg = keyStr;
            }
            debounce = millis();
        }
#else
        hid->releaseAll();
        static int inx = 0;
        String str = "";
        const DuckyCommand *cmd = nullptr;
        options = {};
        for (auto &cmds : duckyCmds) {
            auto &cmds_cpy = cmds;
            if (cmds_cpy.type == DuckyCommandType_Combination || cmds_cpy.type == DuckyCommandType_Cmd) {
                options.push_back({cmds.command, [&]() { cmd = &cmds_cpy; }});
            }
        }
        addOptionToMainMenu();
        inx = loopOptions(options, inx);
        options.clear();
        if (returnToMenu || cmd == nullptr) break;
        if (cmd->type == DuckyCommandType_Print) {
            str = keyboard("", 76, "Type your message:");
            if (str.length() > 0) {
                hid->print(str.c_str());
                if (strcmp(cmd->command, "STRINGLN") == 0) hid->println();
            }
        } else if (cmd->type == DuckyCommandType_Cmd) {
            str = keyboard("", 1, "Type a character:");
            hid->press(cmd->key);
            if (str.length() > 0) { hid->press(str.c_str()[0]); }
        } else if (cmd->type == DuckyCommandType_Combination) {
            for (auto comb : duckyComb) {
                if (strcmp(cmd->command, comb.command) == 0) {
                    str = keyboard("", 1, "Type a character:");
                    hid->press(comb.key1);
                    hid->press(comb.key2);
                    if (comb.key3 != 0) hid->press(comb.key3);
                    if (str.length() > 0) { hid->press(str.c_str()[0]); }
                }
            }
        }
        hid->releaseAll();
#endif
    }
EXIT:
    if (!ble) {
        delete hid; // Keep the hid object alive for BLE
        hid = nullptr;
#if !defined(USB_as_HID)
        mySerial.end();       // Stops UART Serial as HID
        Serial.begin(115200); // Force restart of Serial, just in case....
#endif
    }
}

// Send media commands through BLE or USB HID
void MediaCommands(HIDInterface *hid, bool ble) {
    if (_Ask_for_restart == 2) return;
    _Ask_for_restart = 1; // arm the flag

    ducky_startKb(hid, true);

    displayTextLine("Pairing...");

    while (!hid->isConnected() && !check(EscPress));

    if (hid->isConnected()) {
        BLEConnected = true;
        drawMainBorder();
        int index = 0;

    reMenu:
        options = {
            {"ScreenShot", [=]() { hid->press(KEY_PRINT_SCREEN); }, false, true        },
            {"Play/Pause", [=]() { hid->press(KEY_MEDIA_PLAY_PAUSE); }, false, true    },
            {"Stop",       [=]() { hid->press(KEY_MEDIA_STOP); }, false, true          },
            {"Next Track", [=]() { hid->press(KEY_MEDIA_NEXT_TRACK); }, false, true    },
            {"Prev Track", [=]() { hid->press(KEY_MEDIA_PREVIOUS_TRACK); }, false, true},
            {"Volume +",   [=]() { hid->press(KEY_MEDIA_VOLUME_UP); }, false, true     },
            {"Volume -",   [=]() { hid->press(KEY_MEDIA_VOLUME_DOWN); }, false, true   },
            {"Hold Vol +",
             [=]() {
                 hid->press(KEY_MEDIA_VOLUME_UP);
                 delay(1000);
                 hid->releaseAll();
             },
             false,
             true                                                            },
            {"Mute",       [=]() { hid->press(KEY_MEDIA_MUTE); }, false, true          },
        };
        addOptionToMainMenu();
        index = loopOptions(options, index);
        hid->releaseAll();
        if (!returnToMenu) goto reMenu;
    }
    returnToMenu = true;
}

void MediaCommandsCoreInk(HIDInterface *&hid, bool ble) {
#if !defined(ARDUINO_M5STACK_COREINK)
    MediaCommands(hid, ble);
#else
    if (!ble) {
        MediaCommands(hid, ble);
        return;
    }

    if (_Ask_for_restart == 2) {
        displayError("Restart your Device");
        delay(1000);
        return;
    }

    ducky_startKb(hid, ble);
    displayTextLine("Pairing...");

    while (!hid->isConnected() && !check(EscPress)) { delay(20); }
    if (!hid->isConnected()) {
        displayWarning("Canceled", true);
        returnToMenu = true;
        return;
    }

    BLEConnected = true;

    constexpr uint32_t kDoublePressWindowMs = 320;
    constexpr uint32_t kInputSettleMs = 220;
    constexpr uint32_t kWakeSettleMs = 350;
    constexpr uint32_t kMediaCmdDebounceMs = 170;

    bool trackMode = false;
    bool rockerLocked = false;
    bool pendingSel = false;
    uint32_t pendingSelMs = 0;
    uint32_t ignoreInputUntilMs = millis() + kInputSettleMs;
    uint32_t lastMediaCmdMs = 0;

    auto tapMedia = [&](const MediaKeyReport key) -> bool {
        uint32_t now = millis();
        if ((now - lastMediaCmdMs) < kMediaCmdDebounceMs) return false;
        hid->press(key);
        delay(35);
        hid->releaseAll();
        lastMediaCmdMs = millis();
        return true;
    };

    auto drainInputQueue = [&]() {
        check(AuxPress);
        check(SelPress);
        check(EscPress);
        check(NextPress);
        check(PrevPress);
    };

    auto drawWrappedCentered = [&](const String &text, int &y, int textSize, int maxLines = 2) {
        int maxChars = (tftWidth - 2 * (BORDER_PAD_X + 8)) / (textSize * LW);
        if (maxChars < 4) maxChars = 4;

        String remaining = text;
        int lines = 0;
        while (!remaining.isEmpty() && lines < maxLines) {
            String line;
            if (remaining.length() <= static_cast<size_t>(maxChars)) {
                line = remaining;
                remaining = "";
            } else {
                int split = maxChars;
                while (split > 0 && remaining[split] != ' ') { --split; }
                if (split == 0) split = maxChars;

                line = remaining.substring(0, split);
                remaining.remove(0, split);
                line.trim();
                remaining.trim();

                if (line.isEmpty()) {
                    line = remaining.substring(0, maxChars);
                    remaining.remove(0, maxChars);
                    remaining.trim();
                }
            }

            tft.drawCentreString(line, tftWidth / 2, y, SMOOTH_FONT);
            y += textSize * LH + 2;
            ++lines;
        }

        if (!remaining.isEmpty()) {
            String ellipsis = remaining;
            if (ellipsis.length() > static_cast<size_t>(maxChars - 3)) {
                ellipsis = ellipsis.substring(0, maxChars - 3) + "...";
            }
            tft.drawCentreString(ellipsis, tftWidth / 2, y, SMOOTH_FONT);
            y += textSize * LH + 2;
        }
    };

    auto sleepAndWakeOnG5 = [&]() -> bool {
        constexpr gpio_num_t kWakePin = GPIO_NUM_5;
#if defined(SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP) && SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP
        // Chips that support deep-sleep GPIO wake can use true deep sleep on G5.
        esp_deep_sleep_enable_gpio_wakeup(1ULL << kWakePin, ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_deep_sleep_start();
        return false;
#else
        // Classic ESP32 can't deep-sleep wake on GPIO5 (non-RTC), so use light sleep here.
        gpio_wakeup_enable(kWakePin, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_light_sleep_start();
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
        gpio_wakeup_disable(kWakePin);
        M5.update();
        return true;
#endif
    };

    const int panelX = BORDER_PAD_X;
    const int panelY = BORDER_PAD_Y + FM * LH + 4;
    const int panelW = tftWidth - 2 * BORDER_PAD_X;
    const int panelH = tftHeight - panelY - BORDER_PAD_X;
    const int statusX = panelX + 6;
    const int statusY = panelY + 8;
    const int statusW = panelW - 12;
    const int statusH = 2 * FP * LH + 8;
    const int hintsY = statusY + statusH + 10;

    bool statusDrawn = false;
    bool lastDrawnTrackMode = trackMode;
    bool lastDrawnRockerLocked = rockerLocked;

    auto drawStatus = [&](bool force = false) {
        if (!force && statusDrawn && lastDrawnTrackMode == trackMode && lastDrawnRockerLocked == rockerLocked) {
            return;
        }

        tft.fillRoundRect(statusX, statusY, statusW, statusH, 6, bruceConfig.bgColor);
        tft.drawRoundRect(statusX, statusY, statusW, statusH, 6, bruceConfig.priColor);
        tft.setTextSize(FP);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

        String modeLine = String("Mode: ") + (trackMode ? "Track" : "Volume") + "  Lock: " +
                          (rockerLocked ? "On" : "Off");
        int y = statusY + 3;
        drawWrappedCentered(modeLine, y, FP, 1);
        String mapLine = trackMode ? "Left/Right: Prev/Next" : "Left/Right: Vol-/+";
        drawWrappedCentered(mapLine, y, FP, 1);

        lastDrawnTrackMode = trackMode;
        lastDrawnRockerLocked = rockerLocked;
        statusDrawn = true;
        einkFlushIfDirty(120);
    };

    auto drawStaticUI = [&]() {
        drawMainBorderWithTitle("Media Remote");
        tft.drawRoundRect(panelX, panelY, panelW, panelH, 8, bruceConfig.priColor);
        tft.drawFastHLine(panelX + 8, hintsY - 6, panelW - 16, bruceConfig.priColor);

        tft.setTextSize(FP);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        const char *lines[] = {"G5: Play/Pause", "Tap Ctr: Mode", "2x Ctr: Lock Rocker", "Hold Ctr: Sleep"};
        constexpr int lineCount = sizeof(lines) / sizeof(lines[0]);
        int y = hintsY;

        for (int i = 0; i < lineCount; ++i) {
            drawWrappedCentered(String(lines[i]), y, FP, 1);
        }

        drawWrappedCentered("Rocker follows selected mode.", y, FP, 2);
        einkFlushIfDirty(0);
    };

    drainInputQueue();
    drawStaticUI();
    drawStatus(true);

    while (hid->isConnected()) {
        if (millis() < ignoreInputUntilMs) {
            drainInputQueue();
            delay(10);
            continue;
        }

        bool uiChanged = false;

        if (check(AuxPress)) {
            (void)tapMedia(KEY_MEDIA_PLAY_PAUSE);
            check(EscPress); // G5 is globally mapped to Esc; consume it in this app.
        }

        if (check(SelPress)) {
            uint32_t now = millis();
            if (pendingSel && (now - pendingSelMs) <= kDoublePressWindowMs) {
                pendingSel = false;
                rockerLocked = !rockerLocked;
                uiChanged = true;
            } else {
                pendingSel = true;
                pendingSelMs = now;
            }
        }

        if (pendingSel && (millis() - pendingSelMs) > kDoublePressWindowMs) {
            pendingSel = false;
            trackMode = !trackMode;
            uiChanged = true;
        }

        if (check(EscPress)) {
            hid->releaseAll();
            displayTextLine("Sleeping...");
            einkRequestFullRefresh();
            einkFlushIfDirty(0);
            if (!sleepAndWakeOnG5()) { break; }
            pendingSel = false;
            drainInputQueue();
            ignoreInputUntilMs = millis() + kWakeSettleMs;
            drawStaticUI();
            drawStatus(true);
            continue;
        }

        if (check(NextPress)) {
            if (!rockerLocked) {
                if (trackMode) (void)tapMedia(KEY_MEDIA_NEXT_TRACK);
                else (void)tapMedia(KEY_MEDIA_VOLUME_UP);
            }
        }

        if (check(PrevPress)) {
            if (!rockerLocked) {
                if (trackMode) (void)tapMedia(KEY_MEDIA_PREVIOUS_TRACK);
                else (void)tapMedia(KEY_MEDIA_VOLUME_DOWN);
            }
        }

        if (uiChanged) drawStatus();

        delay(10);
    }

    hid->releaseAll();
    check(SelPress);
    check(EscPress);
    check(AuxPress);
    returnToMenu = true;
#endif
}

DuckyCommand *findDuckyCommand(const char *cmd) {
    for (auto &cmds : duckyCmds) {
        if (strcmp(cmd, cmds.command) == 0) { return const_cast<DuckyCommand *>(&cmds); }
    }
    return nullptr;
}

DuckyCombination *findDuckyCombination(const char *cmd) {
    for (auto &comb : duckyComb) {
        if (strcmp(cmd, comb.command) == 0) { return const_cast<DuckyCombination *>(&comb); }
    }
    return nullptr;
}

void sendAltChar(HIDInterface *hid, uint8_t charCode) {
    // Hold ALT key
    hid->press(KEY_LEFT_ALT);
    delay(bruceConfig.badUSBBLEKeyDelay);

    // Convert char code to 3-digit padded string (standard ALT code format)
    String codeStr = String(charCode);
    if (codeStr.length() < 3) {
        // Pad with leading zeros for proper ALT codes (e.g., 065 instead of 65)
        while (codeStr.length() < 3) { codeStr = "0" + codeStr; }
    }

    // Send each digit using numpad keys
    for (int i = 0; i < codeStr.length(); i++) {
        char digit = codeStr[i];
        uint8_t numpadKey = 0;

        switch (digit) {
            case '0': numpadKey = KEY_KP_0; break;
            case '1': numpadKey = KEY_KP_1; break;
            case '2': numpadKey = KEY_KP_2; break;
            case '3': numpadKey = KEY_KP_3; break;
            case '4': numpadKey = KEY_KP_4; break;
            case '5': numpadKey = KEY_KP_5; break;
            case '6': numpadKey = KEY_KP_6; break;
            case '7': numpadKey = KEY_KP_7; break;
            case '8': numpadKey = KEY_KP_8; break;
            case '9': numpadKey = KEY_KP_9; break;
            default: continue; // Skip invalid characters
        }

        hid->press(numpadKey);
        delay(bruceConfig.badUSBBLEKeyDelay);
        hid->release(numpadKey);
        delay(bruceConfig.badUSBBLEKeyDelay);
    }

    // Release ALT key (this triggers the character input)
    hid->release(KEY_LEFT_ALT);
    delay(bruceConfig.badUSBBLEKeyDelay);
}

void sendAltString(HIDInterface *hid, const String &text) {
    for (int i = 0; i < text.length(); i++) {
        uint8_t charCode = (uint8_t)text[i];
        sendAltChar(hid, charCode);
        delay(bruceConfig.badUSBBLEKeyDelay);
    }
}

void printTextAtPosition(uint16_t xOffset, uint16_t yOffset, const String &text) {
    uint16_t currentTextCursorX = tft.getCursorX();
    uint16_t currentTextCursorY = tft.getCursorY();

    uint16_t x = FP * 6 * xOffset + 2 + BORDER_OFFSET_FROM_SCREEN_EDGE;
    uint16_t y = FP * 8 * yOffset + 2 + STATUS_BAR_HEIGHT;

    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.secColor);
    tft.setCursor(x, y);
    tft.fillRect(x, y, tftWidth - x - BORDER_OFFSET_FROM_SCREEN_EDGE * 2, FP * 8, bruceConfig.bgColor);
    tft.print(text);
    tft.setCursor(currentTextCursorX, currentTextCursorY);
}

void printStatusBadUSBBLE(String text) { printTextAtPosition(8, 2, text); }

void printDecimalTime(uint32_t timeElapsed) { printTextAtPosition(10, 3, formatTimeDecimal(timeElapsed)); }

void printHeaderBadUSBBLE(String bad_script) {
    tft.fillScreen(bruceConfig.bgColor);
    drawMainBorder();

    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.priColor);
    tft.drawCentreString("BadUSB/BLE", tftWidth / 2, FP + STATUS_BAR_HEIGHT);

    tft.setCursor(BORDER_OFFSET_FROM_SCREEN_EDGE * 2, FP * 8 * 1 + 2 + STATUS_BAR_HEIGHT);
    tft.print("Script: ");
    tft.setTextColor(bruceConfig.secColor);
    tft.print(bad_script.substring(bad_script.lastIndexOf("/") + 1));

    tft.setCursor(BORDER_OFFSET_FROM_SCREEN_EDGE * 2, FP * 8 * 2 + 2 + STATUS_BAR_HEIGHT);
    tft.setTextColor(bruceConfig.priColor);
    tft.println("Status:");
}

void printTFTBadUSBBLE(String text, uint16_t color, bool newline) {
    if (!bruceConfig.badUSBBLEShowOutput) return;

    static int bottomHalfStartY = tftHeight / 2;
    const int leftX = BORDER_OFFSET_FROM_SCREEN_EDGE * 2;
    const int rightLimit = tftWidth - BORDER_OFFSET_FROM_SCREEN_EDGE * 2;
    const int lineHeight = 9;

    // Use current cursor X if already set
    int cursorX = tft.getCursorX();
    if (cursorX < leftX || cursorX > rightLimit) {
        cursorX = leftX; // reset if out of bounds
    }

    // Scroll if we reach the bottom
    if (currentOutputY == 0 || currentOutputY > tftHeight - BORDER_OFFSET_FROM_SCREEN_EDGE * 2 - lineHeight) {
        tft.fillRect(
            leftX,
            bottomHalfStartY,
            rightLimit - leftX,
            tftHeight - bottomHalfStartY - BORDER_OFFSET_FROM_SCREEN_EDGE * 2,
            bruceConfig.bgColor
        );
        currentOutputY = bottomHalfStartY;
        cursorX = leftX;
    }

    tft.setCursor(cursorX, currentOutputY);
    tft.setTextColor(color);
    tft.setTextSize(FP);

    // Calculate max characters that fit from current cursor to right edge
    int charWidth = 6 * FP;
    int availableWidth = rightLimit - cursorX;
    int maxChars = availableWidth / charWidth;

    // Crop the string if necessary
    String textToPrint = text;
    if (text.length() > maxChars) { textToPrint = text.substring(0, maxChars); }

    // Print text
    if (newline) {
        tft.println(textToPrint);
        currentOutputY += lineHeight;
    } else {
        tft.print(textToPrint);
    }
}

// Helper function to wait for button press (Select or Escape)
// Returns true if Select was pressed, false if Escape was pressed
bool waitForButtonPress() {
    bool exitPressed = false;
    bool selectPressed = false;
    while (!selectPressed && !exitPressed) {
        selectPressed = check(SelPress);
        exitPressed = check(EscPress);
        delay(50); // Small delay to prevent excessive CPU usage
    }
    return selectPressed; // Return true for Select, false for Escape
}

// Helper function to handle pause/resume logic during script execution
// Returns true to continue, false to exit
bool handlePauseResume() {
    while (check(SelPress)); // hold the code in this position until release the btn
    printStatusBadUSBBLE("Paused - " + String(BTN_ALIAS) + " to resume");
    if (!waitForButtonPress()) {
        printStatusBadUSBBLE("Canceled");
        return false; // Signal to exit
    }
    printStatusBadUSBBLE("Running");
    return true; // Signal to continue
}

// Presenter mode - simple button press to advance slides
void PresenterMode(HIDInterface *&hid, bool ble) {
    if (_Ask_for_restart == 2) {
        displayError("Restart your Device");
        delay(1000);
        return;
    }

    ducky_startKb(hid, ble);

    displayTextLine("Pairing...");

    while (!hid->isConnected() && !check(EscPress));

    if (!hid->isConnected()) {
        displayWarning("Canceled", true);
        returnToMenu = true;
        return;
    }

    BLEConnected = true;

    // Initialize presenter state
    int currentSlide = 1;
    int lastDisplayedSlide = 0;
    unsigned long startTime = 0; // Will be set on first interaction
    unsigned long lastDisplayedSeconds = 0;
    bool timerStarted = false;
    bool firstDraw = true;

    // Helper function to draw static UI elements (only once)
    auto drawStaticUI = [&]() {
        tft.fillScreen(bruceConfig.bgColor);

        // Draw title "PRESENTER" at the top
        tft.setTextSize(FM);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawCentreString("PRESENTER", tftWidth / 2, 10, 1);

        // Draw a separator line
        tft.drawFastHLine(10, 35, tftWidth - 20, bruceConfig.priColor);

        // Draw time label
        tft.setTextSize(FM);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawCentreString("Time", tftWidth / 2, tftHeight / 2 + 15, 1);

        // Draw controls hint at bottom
        tft.setTextSize(1);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawCentreString("<< PREV | SEL | NEXT >>", tftWidth / 2, tftHeight - 15, 1);
    };

    // Helper function to update slide number (only when changed)
    auto updateSlideDisplay = [&]() {
        // Clear previous slide area
        tft.fillRect(0, tftHeight / 2 - 35, tftWidth, 40, bruceConfig.bgColor);

        // Draw current slide number - large and centered
        tft.setTextSize(4);
        tft.setTextColor(TFT_WHITE, bruceConfig.bgColor);
        String slideStr = "Slide " + String(currentSlide);
        tft.drawCentreString(slideStr, tftWidth / 2, tftHeight / 2 - 30, 1);
        lastDisplayedSlide = currentSlide;
    };

    // Helper function to update timer (only when seconds change)
    auto updateTimerDisplay = [&]() {
        unsigned long elapsed = 0;
        if (timerStarted) { elapsed = (millis() - startTime) / 1000; }

        int hours = elapsed / 3600;
        int minutes = (elapsed % 3600) / 60;
        int seconds = elapsed % 60;

        // Format time string
        char timeBuffer[16];
        if (hours > 0) {
            snprintf(timeBuffer, sizeof(timeBuffer), "%d:%02d:%02d", hours, minutes, seconds);
        } else {
            snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", minutes, seconds);
        }

        // Clear timer area and redraw
        tft.fillRect(0, tftHeight / 2 + 30, tftWidth, 30, bruceConfig.bgColor);
        tft.setTextSize(3);
        tft.setTextColor(timerStarted ? TFT_GREEN : TFT_DARKGREY, bruceConfig.bgColor);
        tft.drawCentreString(timeBuffer, tftWidth / 2, tftHeight / 2 + 35, 1);

        lastDisplayedSeconds = elapsed;
    };

    // Initial UI draw
    drawStaticUI();
    updateSlideDisplay();
    updateTimerDisplay();

    while (1) {
        bool slideChanged = false;

        // Middle button = Next slide (Right Arrow for presentations)
        if (check(SelPress)) {
            delay(50); // Allow system to stabilize after check()
            // First press only starts timer, doesn't send key
            if (!timerStarted) {
                startTime = millis();
                timerStarted = true;
                updateTimerDisplay();
                // Prime the HID connection with an empty report
                hid->releaseAll();
                delay(50);
            } else {
                hid->press(KEY_RIGHT_ARROW);
                delay(80);
                hid->releaseAll();
                currentSlide++;
                slideChanged = true;
            }
            delay(150); // debounce
        }
        // Wheel right = Next slide (Right arrow)
        else if (check(NextPress)) {
            delay(50); // Allow system to stabilize after check()
            // First press only starts timer, doesn't send key
            if (!timerStarted) {
                startTime = millis();
                timerStarted = true;
                updateTimerDisplay();
                // Prime the HID connection with an empty report
                hid->releaseAll();
                delay(50);
            } else {
                hid->press(KEY_RIGHT_ARROW);
                delay(80);
                hid->releaseAll();
                currentSlide++;
                slideChanged = true;
            }
            delay(150); // debounce
        }
        // Wheel left = Previous slide (Left arrow)
        else if (check(PrevPress)) {
            delay(50); // Allow system to stabilize after check()
            // First press only starts timer, doesn't send key
            if (!timerStarted) {
                startTime = millis();
                timerStarted = true;
                updateTimerDisplay();
                // Prime the HID connection with an empty report
                hid->releaseAll();
                delay(50);
            } else {
                hid->press(KEY_LEFT_ARROW);
                delay(80);
                hid->releaseAll();
                if (currentSlide > 1) currentSlide--;
                slideChanged = true;
            }
            delay(150); // debounce
        }

        // Update slide display only if changed
        if (slideChanged) {
            updateSlideDisplay();
            updateTimerDisplay();
        }

        // Update timer display every second (only if timer is running)
        if (timerStarted) {
            unsigned long currentSeconds = (millis() - startTime) / 1000;
            if (currentSeconds != lastDisplayedSeconds) { updateTimerDisplay(); }
        }

        // Escape to exit
        if (check(EscPress)) break;

        delay(10);
    }

    hid->releaseAll();
    returnToMenu = true;
}
