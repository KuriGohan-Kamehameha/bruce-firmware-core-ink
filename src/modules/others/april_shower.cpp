#include "april_shower.h"

#include "Apriltag36h11Renderer.h"
#include "core/display.h"

#include <Preferences.h>
#include <esp_system.h>
#include <globals.h>

#include <cstdio>

namespace {

constexpr uint16_t kTagWhite = TFT_WHITE;
constexpr uint16_t kTagBlack = TFT_BLACK;

constexpr uint32_t kPollDelayMs = 20U;
constexpr int16_t kLabelMinMarginPx = 10;

constexpr uint8_t kFavoriteSlots = 8U;
constexpr uint16_t kDefaultFavorites[kFavoriteSlots] = {
    113U,
    0U,
    23U,
    75U,
    128U,
    256U,
    420U,
    586U,
};

constexpr uint8_t kNavStepOptionCount = 4U;
constexpr uint16_t kNavStepValues[kNavStepOptionCount] = {1U, 5U, 10U, 25U};
constexpr char const *kNavStepLabels[kNavStepOptionCount] = {"1", "5", "10", "25"};

constexpr uint8_t kAutoIntervalOptionCount = 5U;
constexpr uint32_t kAutoIntervalMs[kAutoIntervalOptionCount] = {1000U, 2000U, 5000U, 10000U, 30000U};
constexpr char const *kAutoIntervalLabels[kAutoIntervalOptionCount] = {"1s", "2s", "5s", "10s", "30s"};

constexpr char kPrefsNamespace[] = "apr_shower";
constexpr char kPrefTagId[] = "tag";
constexpr char kPrefShowLabel[] = "label";
constexpr char kPrefFavoriteSlot[] = "fav_slot";
constexpr char kPrefFavorites[] = "favs";
constexpr char kPrefNavStep[] = "step";
constexpr char kPrefAutoEnabled[] = "auto_en";
constexpr char kPrefAutoInterval[] = "auto_int";

struct AppState {
    uint16_t currentTagId = 0U;
    bool showLabel = true;
    uint8_t favoriteSlot = 0U;
    uint16_t favorites[kFavoriteSlots] = {0U};
    uint8_t navStepIndex = 0U;
    bool autoCycleEnabled = false;
    uint8_t autoIntervalIndex = 2U;
    bool dirty = false;
};

struct TagLayout {
    int16_t originX = 0;
    int16_t originY = 0;
    int16_t tagPx = 0;
    uint16_t cellPx = 1U;
    bool valid = false;
};

AppState g_state;

void applyDefaultState() {
    g_state.currentTagId = 0U;
    g_state.showLabel = true;
    g_state.favoriteSlot = 0U;
    g_state.navStepIndex = 0U;
    g_state.autoCycleEnabled = false;
    g_state.autoIntervalIndex = 2U;
    g_state.dirty = false;

    for (uint8_t i = 0U; i < kFavoriteSlots; ++i) {
        g_state.favorites[i] = kDefaultFavorites[i];
    }
}

void sanitizeState() {
    if (bruce_apriltag::kTagCount == 0U) {
        g_state.currentTagId = 0U;
        g_state.favoriteSlot = 0U;
        g_state.navStepIndex = 0U;
        g_state.autoIntervalIndex = 0U;
        return;
    }

    if (g_state.currentTagId >= bruce_apriltag::kTagCount) {
        g_state.currentTagId = static_cast<uint16_t>(bruce_apriltag::kTagCount - 1U);
    }

    if (g_state.favoriteSlot >= kFavoriteSlots) {
        g_state.favoriteSlot = 0U;
    }

    if (g_state.navStepIndex >= kNavStepOptionCount) {
        g_state.navStepIndex = 0U;
    }

    if (g_state.autoIntervalIndex >= kAutoIntervalOptionCount) {
        g_state.autoIntervalIndex = 0U;
    }

    for (uint8_t i = 0U; i < kFavoriteSlots; ++i) {
        if (g_state.favorites[i] >= bruce_apriltag::kTagCount) {
            g_state.favorites[i] = static_cast<uint16_t>(bruce_apriltag::kTagCount - 1U);
        }
    }
}

void markDirty() {
    g_state.dirty = true;
}

uint16_t currentNavStep() {
    if (g_state.navStepIndex >= kNavStepOptionCount) {
        return kNavStepValues[0];
    }
    return kNavStepValues[g_state.navStepIndex];
}

uint32_t currentAutoIntervalMs() {
    if (g_state.autoIntervalIndex >= kAutoIntervalOptionCount) {
        return kAutoIntervalMs[0];
    }
    return kAutoIntervalMs[g_state.autoIntervalIndex];
}

uint16_t wrapTagDelta(uint16_t baseValue, int32_t delta) {
    if (bruce_apriltag::kTagCount == 0U) {
        return 0U;
    }

    const int32_t range = static_cast<int32_t>(bruce_apriltag::kTagCount);
    int32_t candidate = static_cast<int32_t>(baseValue) + delta;

    while (candidate < 0) {
        candidate += range;
    }
    while (candidate >= range) {
        candidate -= range;
    }

    return static_cast<uint16_t>(candidate);
}

TagLayout computeTagLayout() {
    TagLayout layout;

    const int16_t usableWidth = static_cast<int16_t>(tftWidth);
    const int16_t usableHeight = static_cast<int16_t>(tftHeight);
    const int16_t usable = (usableWidth < usableHeight) ? usableWidth : usableHeight;

    if (usable < static_cast<int16_t>(bruce_apriltag::kGridSize)) {
        layout.valid = false;
        return layout;
    }

    const uint16_t cellPx = static_cast<uint16_t>(usable / bruce_apriltag::kGridSize);
    if (cellPx == 0U) {
        layout.valid = false;
        return layout;
    }

    const int16_t tagPx = static_cast<int16_t>(cellPx * bruce_apriltag::kGridSize);

    layout.originX = static_cast<int16_t>((tftWidth - tagPx) / 2);
    layout.originY = static_cast<int16_t>((tftHeight - tagPx) / 2);
    layout.tagPx = tagPx;
    layout.cellPx = cellPx;
    layout.valid = true;

    return layout;
}

void drawLabelIfMarginAvailable(const TagLayout &layout) {
    if (!g_state.showLabel) {
        return;
    }

    const int16_t marginTop = layout.originY;
    const int16_t marginBottom = static_cast<int16_t>(tftHeight - (layout.originY + layout.tagPx));
    const bool canDrawTop = marginTop >= kLabelMinMarginPx;
    const bool canDrawBottom = marginBottom >= kLabelMinMarginPx;

    if (!canDrawTop && !canDrawBottom) {
        // Edge-to-edge case (Core Ink): do not overdraw the tag.
        return;
    }

    int16_t labelY = 0;
    if (canDrawTop) {
        tft.fillRect(0, 0, tftWidth, marginTop, kTagWhite);
        labelY = 2;
    } else {
        const int16_t bannerY = static_cast<int16_t>(layout.originY + layout.tagPx);
        tft.fillRect(0, bannerY, tftWidth, marginBottom, kTagWhite);
        labelY = static_cast<int16_t>(bannerY + 2);
    }

    char label[32];
    snprintf(label, sizeof(label), "tag36h11_%u", static_cast<unsigned>(g_state.currentTagId));

    tft.setTextColor(kTagBlack, kTagWhite);
    tft.setTextSize(1);
    tft.drawCentreString(label, tftWidth / 2, labelY, 1);
}

bool renderCurrentTag() {
    sanitizeState();
    const TagLayout layout = computeTagLayout();
    if (!layout.valid) {
        displayError("Invalid layout", true);
        return false;
    }

    tft.fillScreen(kTagWhite);

    const bool drawOk = bruce_apriltag::drawTag(g_state.currentTagId, [&](uint8_t cellX, uint8_t cellY, bool isWhite) {
        const uint16_t color = isWhite ? kTagWhite : kTagBlack;
        tft.fillRect(
            static_cast<int16_t>(layout.originX + static_cast<int16_t>(cellX * layout.cellPx)),
            static_cast<int16_t>(layout.originY + static_cast<int16_t>(cellY * layout.cellPx)),
            layout.cellPx,
            layout.cellPx,
            color
        );
    });

    if (!drawOk) {
        displayError("Invalid tag", true);
        return false;
    }

    drawLabelIfMarginAvailable(layout);
    einkFlushIfDirty(0);
    return true;
}

void loadStateFromPrefs() {
    applyDefaultState();

    Preferences prefs;
    if (!prefs.begin(kPrefsNamespace, true)) {
        sanitizeState();
        return;
    }

    g_state.currentTagId = prefs.getUShort(kPrefTagId, g_state.currentTagId);
    g_state.showLabel = prefs.getBool(kPrefShowLabel, g_state.showLabel);
    g_state.favoriteSlot = prefs.getUChar(kPrefFavoriteSlot, g_state.favoriteSlot);
    g_state.navStepIndex = prefs.getUChar(kPrefNavStep, g_state.navStepIndex);
    g_state.autoCycleEnabled = prefs.getBool(kPrefAutoEnabled, g_state.autoCycleEnabled);
    g_state.autoIntervalIndex = prefs.getUChar(kPrefAutoInterval, g_state.autoIntervalIndex);

    const size_t storedFavoriteBytes = prefs.getBytesLength(kPrefFavorites);
    if (storedFavoriteBytes == sizeof(g_state.favorites)) {
        prefs.getBytes(kPrefFavorites, g_state.favorites, sizeof(g_state.favorites));
    }

    prefs.end();
    sanitizeState();
}

void saveStateToPrefsIfDirty() {
    if (!g_state.dirty) {
        return;
    }

    sanitizeState();

    Preferences prefs;
    if (!prefs.begin(kPrefsNamespace, false)) {
        return;
    }

    prefs.putUShort(kPrefTagId, g_state.currentTagId);
    prefs.putBool(kPrefShowLabel, g_state.showLabel);
    prefs.putUChar(kPrefFavoriteSlot, g_state.favoriteSlot);
    prefs.putUChar(kPrefNavStep, g_state.navStepIndex);
    prefs.putBool(kPrefAutoEnabled, g_state.autoCycleEnabled);
    prefs.putUChar(kPrefAutoInterval, g_state.autoIntervalIndex);
    prefs.putBytes(kPrefFavorites, g_state.favorites, sizeof(g_state.favorites));

    prefs.end();
    g_state.dirty = false;
}

void showAbout() {
    char message[160];
    snprintf(
        message,
        sizeof(message),
        "April Shower\n"
        "Family: tag36h11 (%u tags)\n"
        "L/R: +/- step\n"
        "C: label on/off\n"
        "Hold C: back",
        static_cast<unsigned>(bruce_apriltag::kTagCount)
    );

    (void)displayMessage(message, nullptr, "OK", nullptr, bruceConfig.priColor);
}

void runViewer() {
    uint32_t lastAutoCycleAt = millis();

    if (!renderCurrentTag()) {
        return;
    }

    while (true) {
        bool shouldRender = false;

        if (check(EscPress)) {
            break;
        }

        if (check(PrevPress) || check(UpPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, -static_cast<int32_t>(currentNavStep()));
            lastAutoCycleAt = millis();
            markDirty();
            shouldRender = true;
        }

        if (check(NextPress) || check(DownPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, static_cast<int32_t>(currentNavStep()));
            lastAutoCycleAt = millis();
            markDirty();
            shouldRender = true;
        }

        if (check(SelPress)) {
            g_state.showLabel = !g_state.showLabel;
            markDirty();
            shouldRender = true;
        }

        if (g_state.autoCycleEnabled) {
            const uint32_t now = millis();
            if ((now - lastAutoCycleAt) >= currentAutoIntervalMs()) {
                g_state.currentTagId = wrapTagDelta(g_state.currentTagId, static_cast<int32_t>(currentNavStep()));
                lastAutoCycleAt = now;
                markDirty();
                shouldRender = true;
            }
        }

        if (shouldRender) {
            (void)renderCurrentTag();
        }

        delay(kPollDelayMs);
    }

    saveStateToPrefsIfDirty();
}

void selectTagMenu() {
    uint16_t candidateTagId = g_state.currentTagId;
    int selectedIndex = 1;

    while (true) {
        const String currentLabel = String("Current ID: ") + String(candidateTagId);
        const String stepValue = String(currentNavStep());
        const String plusStepLabel = String("+Step (") + stepValue + ")";
        const String minusStepLabel = String("-Step (") + stepValue + ")";

        options = {
            {currentLabel,    []() {}, false, true},
            {"+1",           [&candidateTagId]() { candidateTagId = wrapTagDelta(candidateTagId, 1); }, false, true},
            {"-1",           [&candidateTagId]() { candidateTagId = wrapTagDelta(candidateTagId, -1); }, false, true},
            {"+10",          [&candidateTagId]() { candidateTagId = wrapTagDelta(candidateTagId, 10); }, false, true},
            {"-10",          [&candidateTagId]() { candidateTagId = wrapTagDelta(candidateTagId, -10); }, false, true},
            {plusStepLabel,   [&candidateTagId]() {
                                  candidateTagId = wrapTagDelta(candidateTagId, static_cast<int32_t>(currentNavStep()));
                              }, false, true},
            {minusStepLabel,  [&candidateTagId]() {
                                  candidateTagId = wrapTagDelta(candidateTagId, -static_cast<int32_t>(currentNavStep()));
                              }, false, true},
            {"Apply",        [&candidateTagId]() {
                                  g_state.currentTagId = candidateTagId;
                                  markDirty();
                              }},
            {"Back",         []() {}},
        };

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "Select Tag", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        if (selectedIndex >= static_cast<int>(options.size())) {
            selectedIndex = 0;
        }

        // Apply or Back both close this menu.
        if (selectedIndex >= 7) {
            break;
        }
    }

    saveStateToPrefsIfDirty();
}

void openFavoriteMenu() {
    int selectedIndex = g_state.favoriteSlot;

    while (true) {
        options.clear();

        for (uint8_t slot = 0U; slot < kFavoriteSlots; ++slot) {
            const String label = String("Slot ") + String(slot + 1U) + String(": ") + String(g_state.favorites[slot]);
            options.push_back({label, [slot]() {
                                   g_state.favoriteSlot = slot;
                                   g_state.currentTagId = g_state.favorites[slot];
                                   markDirty();
                               }});
        }

        options.push_back({"Back", []() {}});

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "Open Favorite", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        // Any explicit slot selection or Back closes this menu.
        break;
    }

    saveStateToPrefsIfDirty();
}

void saveFavoriteMenu() {
    int selectedIndex = g_state.favoriteSlot;

    while (true) {
        options.clear();

        for (uint8_t slot = 0U; slot < kFavoriteSlots; ++slot) {
            const String label = String("Set Slot ") + String(slot + 1U) + String(" = ") + String(g_state.currentTagId);
            options.push_back({label, [slot]() {
                                   g_state.favoriteSlot = slot;
                                   g_state.favorites[slot] = g_state.currentTagId;
                                   markDirty();
                               }});
        }

        options.push_back({"Back", []() {}});

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "Save Favorite", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        // Any explicit slot selection or Back closes this menu.
        break;
    }

    saveStateToPrefsIfDirty();
}

void favoritesMenu() {
    int selectedIndex = 0;

    while (true) {
        const String active = String("Active Slot: ") + String(g_state.favoriteSlot + 1U);

        options = {
            {"Open Favorite",           openFavoriteMenu},
            {"Save Current to Favorite", saveFavoriteMenu},
            {"Restore Default Favorites", []() {
                                             for (uint8_t i = 0U; i < kFavoriteSlots; ++i) {
                                                 g_state.favorites[i] = kDefaultFavorites[i];
                                             }
                                             g_state.favoriteSlot = 0U;
                                             markDirty();
                                         }},
            {active,                    []() {}, false, true},
            {"Back",                   []() {}},
        };

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "Favorites", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        if (selectedIndex == static_cast<int>(options.size() - 1)) {
            break;
        }
    }

    saveStateToPrefsIfDirty();
}

void settingsMenu() {
    int selectedIndex = 0;

    while (true) {
        const String labelState = String("Show Label: ") + (g_state.showLabel ? "on" : "off");
        const String stepState = String("Nav Step: ") + String(kNavStepLabels[g_state.navStepIndex]);
        const String autoState = String("Auto Cycle: ") + (g_state.autoCycleEnabled ? "on" : "off");
        const String intervalState = String("Auto Interval: ") + String(kAutoIntervalLabels[g_state.autoIntervalIndex]);

        options = {
            {labelState,    []() {
                               g_state.showLabel = !g_state.showLabel;
                               markDirty();
                           }},
            {stepState,     []() {
                               g_state.navStepIndex = static_cast<uint8_t>((g_state.navStepIndex + 1U) %
                                                                            kNavStepOptionCount);
                               markDirty();
                           }},
            {autoState,     []() {
                               g_state.autoCycleEnabled = !g_state.autoCycleEnabled;
                               markDirty();
                           }},
            {intervalState, []() {
                               g_state.autoIntervalIndex = static_cast<uint8_t>((g_state.autoIntervalIndex + 1U) %
                                                                                 kAutoIntervalOptionCount);
                               markDirty();
                           }},
            {"Back",       []() {}},
        };

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "Settings", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        if (selectedIndex == static_cast<int>(options.size() - 1)) {
            break;
        }
    }

    saveStateToPrefsIfDirty();
}

void randomizeTagAndRender() {
    if (bruce_apriltag::kTagCount == 0U) {
        g_state.currentTagId = 0U;
    } else {
        g_state.currentTagId = static_cast<uint16_t>(esp_random() % bruce_apriltag::kTagCount);
    }

    markDirty();
    runViewer();
}

} // namespace

void april_shower_setup() {
    if (tft.getLogging()) {
        tft.log_drawString("Not Supported", DRAWCENTRESTRING, tftWidth / 2, tftHeight / 2);
        return;
    }

    loadStateFromPrefs();

    int selectedIndex = 0;

    while (true) {
        const String viewerLabel = String("Viewer (ID ") + String(g_state.currentTagId) + String(")");

        options = {
            {viewerLabel,  runViewer},
            {"Select Tag", selectTagMenu},
            {"Random Tag", randomizeTagAndRender},
            {"Favorites",  favoritesMenu},
            {"Settings",   settingsMenu},
            {"About",      showAbout},
            {"Back",       []() {}},
        };

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "April Shower", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        if (selectedIndex == static_cast<int>(options.size() - 1)) {
            break;
        }
    }

    saveStateToPrefsIfDirty();
    returnToMenu = true;
}
