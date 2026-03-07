#include "clock_alert_tones.h"
#include "globals.h"
#include "modules/others/audio.h"

namespace {
struct ToneStep {
    uint16_t frequency;
    uint16_t durationMs;
    uint16_t gapMs;
};

constexpr ToneStep kClassicPattern[] = {
    {2000, 220, 70},
    {6000, 220, 70},
    {6000, 320, 180}
};

constexpr ToneStep kAscendingPattern[] = {
    {880,  140, 40},
    {1047, 140, 40},
    {1319, 170, 40},
    {1760, 260, 180}
};

constexpr ToneStep kDigitalPattern[] = {
    {2400, 80,  40},
    {2400, 80,  40},
    {2400, 80,  80},
    {2400, 80,  40},
    {2400, 80,  40},
    {2400, 120, 140}
};

constexpr ToneStep kBellPattern[] = {
    {659, 190, 50},
    {523, 170, 50},
    {587, 190, 50},
    {392, 320, 180}
};

constexpr ToneStep kMelodyPattern[] = {
    {523, 170, 45},
    {659, 170, 45},
    {784, 170, 45},
    {1047, 260, 45},
    {784, 170, 45},
    {659, 170, 45},
    {523, 260, 180}
};

void playToneSteps(const ToneStep *pattern, size_t steps) {
    if (bruceConfig.soundEnabled == 0) {
        delay(200);
        return;
    }

    for (size_t i = 0; i < steps; ++i) {
        _tone(pattern[i].frequency, pattern[i].durationMs);
        if (pattern[i].gapMs > 0) delay(pattern[i].gapMs);
    }
}
} // namespace

const char *clockAlertToneLabel(int toneStyle) {
    switch (toneStyle) {
    case CLOCK_ALERT_TONE_ASCENDING:
        return "Ascending";
    case CLOCK_ALERT_TONE_DIGITAL:
        return "Digital";
    case CLOCK_ALERT_TONE_BELL:
        return "Bell";
    case CLOCK_ALERT_TONE_MELODY:
        return "Melody";
    case CLOCK_ALERT_TONE_CLASSIC:
    default:
        return "Classic";
    }
}

void playClockAlertToneOnce(int toneStyle) {
    switch (toneStyle) {
    case CLOCK_ALERT_TONE_ASCENDING:
        playToneSteps(kAscendingPattern, sizeof(kAscendingPattern) / sizeof(kAscendingPattern[0]));
        break;
    case CLOCK_ALERT_TONE_DIGITAL:
        playToneSteps(kDigitalPattern, sizeof(kDigitalPattern) / sizeof(kDigitalPattern[0]));
        break;
    case CLOCK_ALERT_TONE_BELL:
        playToneSteps(kBellPattern, sizeof(kBellPattern) / sizeof(kBellPattern[0]));
        break;
    case CLOCK_ALERT_TONE_MELODY:
        playToneSteps(kMelodyPattern, sizeof(kMelodyPattern) / sizeof(kMelodyPattern[0]));
        break;
    case CLOCK_ALERT_TONE_CLASSIC:
    default:
        playToneSteps(kClassicPattern, sizeof(kClassicPattern) / sizeof(kClassicPattern[0]));
        break;
    }
}

void playClockAlertTonePreview(int toneStyle) { playClockAlertToneOnce(toneStyle); }
