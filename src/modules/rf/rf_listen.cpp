#include "rf_listen.h"

#include "../others/audio.h"

volatile unsigned long lastMicros = 0;
volatile unsigned long pulseMicros = 0;
volatile float ___frequency = 0;
volatile unsigned long pulseDuration = 0;
volatile bool newPulse = false;

void IRAM_ATTR onPulse() {
    static bool wasHigh = false;
    unsigned long now = micros();

    if (digitalRead(bruceConfigPins.CC1101_bus.io0)) {
        pulseDuration = now - lastMicros;
        ___frequency = 1000000.0 / pulseDuration;
        newPulse = true;
        wasHigh = true;
    } else if (wasHigh) {
        lastMicros = now;
        wasHigh = false;
    }
}

void rf_listen() {
    float freq = 433.92;
    float last_freq = -1;
    bool redraw = false;
    while (!check(SelPress) && !check(EscPress)) {
        if (check(PrevPress)) { freq -= 0.1f; }
        if (check(NextPress)) { freq += 0.1f; }

        freq = constrain(freq, 300.0f, 928.0f);
        if (freq != last_freq) {
            redraw = true;
            last_freq = freq;
        } else {
            redraw = false;
        }

        if (redraw) {
            String text = String("Frequency: ") + String(freq, 2) + String("MHz");
            displayRedStripe(text, getComplementaryColor2(bruceConfig.priColor), bruceConfig.priColor);
        }

        if (check(EscPress)) break;
        if (check(SelPress)) break;
    }

    if (bruceConfigPins.rfModule != CC1101_SPI_MODULE) {
        displayError("Listener needs a CC1101!", true);
        return;
    }
    if (!initRfModule("rx", freq)) {
        displayError("CC1101 not found!", true);
        return;
    }

    ELECHOUSE_cc1101.setRxBW(58);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setDcFilterOff(true);
    attachInterrupt(digitalPinToInterrupt(bruceConfigPins.CC1101_bus.io0), onPulse, CHANGE);
    displayRedStripe("Listening...", getComplementaryColor2(bruceConfig.priColor), bruceConfig.priColor);

    unsigned long lastPulseTime = millis();
    bool pulseActive = false;
#if defined(BUZZ_PIN)
    bool buzzerAttached = false;
    unsigned long buzzerOffAtMs = 0;
#endif

    while (check(EscPress)) { delay(10); }

    while (!check(EscPress)) {
        displayRedStripe(
            "Waiting for a pulse", getComplementaryColor2(bruceConfig.priColor), bruceConfig.priColor
        );
#if defined(BUZZ_PIN)
        if (buzzerOffAtMs > 0 && millis() >= buzzerOffAtMs) {
            ledcWrite(BUZZ_PIN, 0);
            ledcWriteTone(BUZZ_PIN, 0);
            buzzerOffAtMs = 0;
        }
#endif
        if (newPulse) {
            newPulse = false;
            lastPulseTime = millis();
            pulseActive = true;
            String pulseText = String("Freq: ") + String(___frequency, 2) + String(" Hz");
            displayRedStripe(pulseText, getComplementaryColor2(bruceConfig.priColor), bruceConfig.priColor);
#if defined(BUZZ_PIN)
            if (bruceConfig.soundEnabled) {
                const unsigned long durationMs = max(1UL, pulseDuration / 1000UL);
                if (!buzzerAttached) { buzzerAttached = ledcAttach(BUZZ_PIN, static_cast<uint32_t>(___frequency), 10); }
                if (buzzerAttached) {
                    int volume = bruceConfig.soundVolume;
                    if (volume < 0) volume = 0;
                    if (volume > 100) volume = 100;
                    uint32_t duty = 0;
                    if (volume > 0) {
                        const uint32_t minDuty = 1023U / 14U;
                        duty = minDuty + ((1023U - minDuty) * static_cast<uint32_t>(volume) / 100U);
                    }
                    ledcWriteTone(BUZZ_PIN, static_cast<uint32_t>(___frequency));
                    ledcWrite(BUZZ_PIN, duty);
                    buzzerOffAtMs = millis() + durationMs;
                }
            }
#elif defined(HAS_NS4168_SPKR)
            playTone(___frequency, pulseDuration, 0);
#endif
        }

        if (pulseActive && millis() - lastPulseTime > 3000) {
            pulseActive = false;
            displayRedStripe("No signal", getComplementaryColor2(bruceConfig.priColor), bruceConfig.priColor);
        }

        if (check(EscPress)) break;
        if (check(SelPress)) break;
    }

    detachInterrupt(digitalPinToInterrupt(bruceConfigPins.CC1101_bus.io0));
#if defined(BUZZ_PIN)
    ledcWrite(BUZZ_PIN, 0);
    ledcWriteTone(BUZZ_PIN, 0);
#endif
}
