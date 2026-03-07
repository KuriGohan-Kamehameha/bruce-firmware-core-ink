#include "april_shower.h"

#include "Apriltag36h11Renderer.h"
#include "core/display.h"

#include <esp_system.h>
#include <globals.h>

#include <cstdio>

namespace {

constexpr uint16_t kTagWhite = TFT_WHITE;
constexpr uint16_t kTagBlack = TFT_BLACK;
constexpr uint16_t kFastStep = 10U;
constexpr uint32_t kPollDelayMs = 20U;
constexpr int16_t kLabelMinMarginPx = 10;

struct AppState {
    uint16_t currentTagId = 0U;
    bool showLabel = true;
};

struct TagLayout {
    int16_t originX = 0;
    int16_t originY = 0;
    int16_t tagPx = 0;
    uint16_t cellPx = 1U;
    bool valid = false;
};

AppState g_state;

void normalizeState() {
    if (bruce_apriltag::kTagCount == 0U) {
        g_state.currentTagId = 0U;
        return;
    }

    if (g_state.currentTagId >= bruce_apriltag::kTagCount) {
        g_state.currentTagId = static_cast<uint16_t>(bruce_apriltag::kTagCount - 1U);
    }
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
        return layout;
    }

    const uint16_t cellPx = static_cast<uint16_t>(usable / bruce_apriltag::kGridSize);
    if (cellPx == 0U) {
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
    normalizeState();
    const TagLayout layout = computeTagLayout();
    if (!layout.valid) {
        displayError("Invalid layout", true);
        return false;
    }

    tft.fillScreen(kTagWhite);

    const bool drawOk = bruce_apriltag::drawTag(g_state.currentTagId, [&](uint8_t cellX, uint8_t cellY, bool isWhite) {
        tft.fillRect(
            static_cast<int16_t>(layout.originX + static_cast<int16_t>(cellX * layout.cellPx)),
            static_cast<int16_t>(layout.originY + static_cast<int16_t>(cellY * layout.cellPx)),
            layout.cellPx,
            layout.cellPx,
            isWhite ? kTagWhite : kTagBlack
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

void randomizeTag() {
    if (bruce_apriltag::kTagCount == 0U) {
        g_state.currentTagId = 0U;
        return;
    }

    g_state.currentTagId = static_cast<uint16_t>(esp_random() % bruce_apriltag::kTagCount);
}

void showAbout() {
    char message[200];
    snprintf(
        message,
        sizeof(message),
        "April Shower\n"
        "Family: tag36h11 (%u tags)\n"
        "Viewer controls:\n"
        "L/R: -/+ 1\n"
        "U/D: -/+ %u\n"
        "C: label on/off\n"
        "PWR: back",
        static_cast<unsigned>(bruce_apriltag::kTagCount),
        static_cast<unsigned>(kFastStep)
    );

    (void)displayMessage(message, nullptr, "OK", nullptr, bruceConfig.priColor);
}

void runViewer() {
    if (!renderCurrentTag()) {
        return;
    }

    while (true) {
        bool shouldRender = false;

        if (check(EscPress)) {
            break;
        }

        if (check(PrevPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, -1);
            shouldRender = true;
        }

        if (check(NextPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, 1);
            shouldRender = true;
        }

        if (check(UpPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, -static_cast<int32_t>(kFastStep));
            shouldRender = true;
        }

        if (check(DownPress)) {
            g_state.currentTagId = wrapTagDelta(g_state.currentTagId, static_cast<int32_t>(kFastStep));
            shouldRender = true;
        }

        if (check(SelPress)) {
            g_state.showLabel = !g_state.showLabel;
            shouldRender = true;
        }

        if (shouldRender) {
            (void)renderCurrentTag();
        }

        delay(kPollDelayMs);
    }
}

void randomizeTagAndRunViewer() {
    randomizeTag();
    runViewer();
}

void toggleLabel() {
    g_state.showLabel = !g_state.showLabel;
}

} // namespace

void april_shower_setup() {
    if (tft.getLogging()) {
        tft.log_drawString("Not Supported", DRAWCENTRESTRING, tftWidth / 2, tftHeight / 2);
        return;
    }

    normalizeState();
    int selectedIndex = 0;

    while (true) {
        const String viewerLabel = String("Viewer (ID ") + String(g_state.currentTagId) + String(")");
        const String labelToggle = String("Show Label: ") + (g_state.showLabel ? "on" : "off");

        options = {
            {viewerLabel, runViewer},
            {"Random Tag", randomizeTagAndRunViewer},
            {labelToggle, toggleLabel},
            {"About", showAbout},
            {"Back", []() {}},
        };

        selectedIndex = loopOptions(options, MENU_TYPE_SUBMENU, "April Shower", selectedIndex);

        if (selectedIndex < 0 || returnToMenu) {
            break;
        }

        if (selectedIndex == static_cast<int>(options.size() - 1)) {
            break;
        }
    }

    returnToMenu = true;
}
