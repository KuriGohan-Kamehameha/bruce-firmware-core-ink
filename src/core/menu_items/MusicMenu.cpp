#include "MusicMenu.h"

#include "core/display.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "modules/others/audio.h"
#include "modules/others/audio_player.h"

// Global instance used by main_menu
MusicMenu musicMenu;

void MusicMenu::optionsMenu() {
    options.clear();
    // allow selection of audio files from available filesystems
#if defined(HAS_NS4168_SPKR)
    if (setupSdCard()) {
        options.push_back({"SD Card", [=]() {
                               String file = loopSD(SD, true, "*.wav;*.mp3;*.ogg;*.flac;*.aac", "/");
                               if (file.length() > 0) { musicPlayerUI(&SD, file); }
                           }});
    }
    options.push_back({"LittleFS", [=]() {
                           String file = loopSD(LittleFS, true, "*.wav;*.mp3;*.ogg;*.flac;*.aac", "/");
                           if (file.length() > 0) { musicPlayerUI(&LittleFS, file); }
                       }});

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Music");
#else
    displayError("No audio hardware", true);
#endif
}

void MusicMenu::drawIcon(float scale) {
    clearIconArea();
    // simple music note icon
    int size = scale * 24;
    int x = iconCenterX - size / 2;
    int y = iconCenterY - size / 2;

    // draw stem
    tft.fillRect(x + size / 3, y, size / 6, size, bruceConfig.priColor);
    // draw head (circle)
    tft.fillCircle(x + size / 2, y + size - (size / 6), size / 4, bruceConfig.priColor);
    // draw flag
    tft.fillTriangle(
        x + size / 2 + (size / 6),
        y,
        x + size / 2 + size / 2,
        y + (size / 6),
        x + size / 2 + (size / 6),
        y + (size / 3),
        bruceConfig.priColor
    );
}
