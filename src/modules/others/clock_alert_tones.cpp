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
    {2000, 220, 70 },
    {6000, 220, 70 },
    {6000, 320, 180}
};

constexpr ToneStep kAscendingPattern[] = {
    {880,  140, 40 },
    {1047, 140, 40 },
    {1319, 170, 40 },
    {1760, 260, 180}
};

constexpr ToneStep kDigitalPattern[] = {
    {2400, 80,  40 },
    {2400, 80,  40 },
    {2400, 80,  80 },
    {2400, 80,  40 },
    {2400, 80,  40 },
    {2400, 120, 140}
};

constexpr ToneStep kBellPattern[] = {
    {659, 190, 50 },
    {587, 190, 50 },
    {523, 170, 50 },
    {392, 320, 180}
};

constexpr ToneStep kMelodyPattern[] = {
    {523,  170, 45 },
    {659,  170, 45 },
    {784,  170, 45 },
    {1047, 260, 45 },
    {784,  170, 45 },
    {659,  170, 45 },
    {523,  260, 180}
};

// Chicken Dance
constexpr ToneStep kChickenDancePattern[] = {
    {523, 100, 25 }, // C
    {523, 100, 25 }, // C
    {523, 100, 25 }, // C
    {659, 200, 40 }, // E
    {587, 100, 25 }, // D
    {587, 100, 25 }, // D
    {587, 100, 25 }, // D
    {698, 200, 40 }, // F
    {659, 130, 25 }, // E
    {659, 130, 25 }, // E
    {587, 130, 25 }, // D
    {587, 130, 25 }, // D
    {523, 280, 180}  // C
};

// Oh Canada
constexpr ToneStep kOhCanadaPattern[] = {
    {392, 300, 50 }, // G
    {523, 450, 50 }, // C
    {523, 200, 50 }, // C
    {587, 200, 50 }, // D
    {523, 200, 50 }, // C
    {587, 200, 50 }, // D
    {659, 400, 50 }, // E
    {698, 200, 50 }, // F
    {784, 600, 180}  // G
};

// USSR Anthem
constexpr ToneStep kUSSRAnthemPattern[] = {
    {392, 400, 50 }, // G
    {523, 400, 50 }, // C
    {587, 200, 50 }, // D
    {523, 200, 50 }, // C
    {466, 200, 50 }, // Bb
    {392, 200, 50 }, // G
    {523, 400, 50 }, // C
    {587, 400, 50 }, // D
    {523, 600, 180}  // C
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
        case CLOCK_ALERT_TONE_ASCENDING: return "Ascending";
        case CLOCK_ALERT_TONE_DIGITAL: return "Digital";
        case CLOCK_ALERT_TONE_BELL: return "Bell";
        case CLOCK_ALERT_TONE_MELODY: return "Melody";
        case CLOCK_ALERT_TONE_CHICKEN_DANCE: return "Chicken Dance";
        case CLOCK_ALERT_TONE_OH_CANADA: return "Oh Canada";
        case CLOCK_ALERT_TONE_USSR_ANTHEM: return "USSR Anthem";
        case CLOCK_ALERT_TONE_CLASSIC:
        default: return "Classic";
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
        case CLOCK_ALERT_TONE_CHICKEN_DANCE:
            playToneSteps(
                kChickenDancePattern, sizeof(kChickenDancePattern) / sizeof(kChickenDancePattern[0])
            );
            break;
        case CLOCK_ALERT_TONE_OH_CANADA:
            playToneSteps(kOhCanadaPattern, sizeof(kOhCanadaPattern) / sizeof(kOhCanadaPattern[0]));
            break;
        case CLOCK_ALERT_TONE_USSR_ANTHEM:
            playToneSteps(kUSSRAnthemPattern, sizeof(kUSSRAnthemPattern) / sizeof(kUSSRAnthemPattern[0]));
            break;
        case CLOCK_ALERT_TONE_CLASSIC:
        default: playToneSteps(kClassicPattern, sizeof(kClassicPattern) / sizeof(kClassicPattern[0])); break;
    }
}

void playClockAlertTonePreview(int toneStyle) { playClockAlertToneOnce(toneStyle); }
