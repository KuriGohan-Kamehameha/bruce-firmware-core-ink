#ifndef __CLOCK_ALERT_TONES_H__
#define __CLOCK_ALERT_TONES_H__

#include <stdint.h>

constexpr int CLOCK_ALERT_TONE_CLASSIC = 0;
constexpr int CLOCK_ALERT_TONE_ASCENDING = 1;
constexpr int CLOCK_ALERT_TONE_DIGITAL = 2;
constexpr int CLOCK_ALERT_TONE_BELL = 3;
constexpr int CLOCK_ALERT_TONE_MELODY = 4;

constexpr int CLOCK_ALERT_TONE_MIN = CLOCK_ALERT_TONE_CLASSIC;
constexpr int CLOCK_ALERT_TONE_MAX = CLOCK_ALERT_TONE_MELODY;
constexpr int CLOCK_ALERT_TONE_COUNT = CLOCK_ALERT_TONE_MAX - CLOCK_ALERT_TONE_MIN + 1;

const char *clockAlertToneLabel(int toneStyle);
void playClockAlertToneOnce(int toneStyle);
void playClockAlertTonePreview(int toneStyle);

#endif
