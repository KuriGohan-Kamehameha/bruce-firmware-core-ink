#include "april_shower.h"
#include "Apriltag36h11Renderer.h"
#include "core/display.h"
#include <globals.h>

#include <cstdio>

namespace {

constexpr uint16_t kTagWhite = TFT_WHITE;
constexpr uint16_t kTagBlack = TFT_BLACK;
constexpr uint16_t kLayoutPadding = 20;
constexpr uint32_t kPollDelayMs = 20;

uint16_t g_tagId = 0;
bool g_showLabel = true;
int16_t g_tagX = 0;
int16_t g_tagY = 0;
uint16_t g_cellPx = 1;

void computeLayout() {
    int16_t minDimension = tftWidth < tftHeight ? tftWidth : tftHeight;
    int16_t usable = minDimension - static_cast<int16_t>(kLayoutPadding);
    if (usable < bruce_apriltag::kGridSize) usable = bruce_apriltag::kGridSize;

    g_cellPx = usable / bruce_apriltag::kGridSize;
    if (g_cellPx == 0) g_cellPx = 1;

    const int16_t tagSize = g_cellPx * bruce_apriltag::kGridSize;
    g_tagX = (tftWidth - tagSize) / 2;
    g_tagY = (tftHeight - tagSize) / 2;
}

void drawLabel() {
    if (!g_showLabel) return;

    char label[24];
    snprintf(label, sizeof(label), "tag36h11_%u", static_cast<unsigned>(g_tagId));

    tft.fillRect(0, 0, tftWidth, 12, kTagWhite);
    tft.setTextColor(kTagBlack, kTagWhite);
    tft.setTextSize(1);
    tft.drawCentreString(label, tftWidth / 2, 2, 1);
}

void renderCurrentTag() {
    if (g_tagId >= bruce_apriltag::kTagCount) g_tagId = 0;
    computeLayout();

    tft.fillScreen(kTagWhite);
    bruce_apriltag::drawTag(g_tagId, [](uint8_t cellX, uint8_t cellY, bool isWhite) {
        const uint16_t color = isWhite ? kTagWhite : kTagBlack;
        tft.fillRect(
            g_tagX + static_cast<int16_t>(cellX * g_cellPx),
            g_tagY + static_cast<int16_t>(cellY * g_cellPx),
            g_cellPx,
            g_cellPx,
            color
        );
    });
    drawLabel();
    einkFlushIfDirty(0);
}

} // namespace

void april_shower_setup() {
    if (tft.getLogging()) {
        tft.log_drawString("Not Supported", DRAWCENTRESTRING, tftWidth / 2, tftHeight / 2);
        return;
    }

    renderCurrentTag();

    for (;;) {
        bool shouldRender = false;

        if (check(EscPress)) break;
        else if (check(PrevPress) || check(UpPress)) {
            g_tagId = g_tagId == 0 ? bruce_apriltag::kTagCount - 1 : g_tagId - 1;
            shouldRender = true;
        } else if (check(NextPress) || check(DownPress)) {
            g_tagId = static_cast<uint16_t>((g_tagId + 1) % bruce_apriltag::kTagCount);
            shouldRender = true;
        } else if (check(SelPress)) {
            g_showLabel = !g_showLabel;
            shouldRender = true;
        }

        if (shouldRender) renderCurrentTag();
        delay(kPollDelayMs);
    }

    returnToMenu = true;
}
