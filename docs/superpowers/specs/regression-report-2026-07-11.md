# CoreInk Regression Report — dump vs proposed release

Date: 2026-07-11 · Author: CPCS

## Method
- **Baseline (known-good):** `175b9bd8` — the pre-upstream-sync CoreInk line, an ancestor of `main`; the connected device's feature-set (pwnagotchi, april_shower, RFID/Tag-O-Matic) matches it.
- **Proposed release:** `c4b7f518` (`main`) — the July "Unify fork history after upstream sync".
- **Ground truth:** the last-known-working device dump (`strings` fingerprint) + `boards/m5stack-coreink/readme.md` adaptation spec.
- 6-subsystem parallel audit (display/e-ink, build-flags, partition, power-input, settings-clock, startup-menu), each candidate finding **adversarially verified** (9 agents total). Plus manual byte-level analysis of the device dump.

## Finding: 1 real issue — partition migration wipes WiFi creds on flash  → **FIXED**
- **What:** the release relocated the shared `custom_4Mb_full.csv` filesystem partition: `spiffs 0x3A0000/0x60000 → 0x3D0000/0x30000` (app0 `0x390000 → 0x3C0000`). The **device runs the old layout** — proven by a valid LittleFS superblock (magic `littlefs`) at byte 8 of `0x3A0000` and the live `/bruce.conf` at ~`0x3BC000`.
- **Impact:** flashing the release's table makes `LittleFS.begin(true)` (`main.cpp:179`) look at `0x3D0000`, find no superblock, **auto-format**, and wipe `/bruce.conf` + saved WiFi networks. Independently confirmed by manual analysis **and all three workflow verifiers**.
- **Taxonomy note:** the audit tagged it "REJECTED / not CoreInk-specific" because the csv is shared by ~30 board envs and the app0 growth is upstream-driven (the fork actually chose a *smaller* app0 than upstream, giving CoreInk +64KB spiffs). True — but board-wide ≠ harmless. It is exactly the creds loss to prevent.
- **Fix:** dedicated `custom_4Mb_coreink.csv` matching the device layout (`app0 0x390000`, `spiffs 0x3A0000/0x60000`); `m5stack-coreink.ini` now points at it. The ~2.6MB build fits `app0` at 69.3%. An app-only flash writes ≤`~0x287000`, never touching spiffs — **creds preserved, no re-injection needed**. Shared csv and other boards untouched.

## Preserved — every documented CoreInk behavior is intact (or improved)
| Subsystem | Verdict | Note |
|-----------|---------|------|
| E-ink refresh / flush / 300ms menu interval | intact | `display.cpp:10–60` helper byte-identical; `EINK_MENU_FLUSH_INTERVAL_MS=300` still invoked in `loopOptions` |
| Forced B&W (no grayscale) | intact | `mapToBinaryEinkColor()` unchanged |
| Rocker L/R + inversion, short=select, long-press 800ms=back | intact | `interface.cpp:213/241–245` byte-identical; dump confirms `rockerInverted` |
| 35ms LED button feedback, power mgmt (timeout disabled, sleep bypassed, 80MHz) | intact | `powerSave.cpp`/`interface.cpp` behavior unchanged; `_setBrightness` no-op on e-ink |
| Simplified HH:MM clock | intact | `ClockMenu.cpp` byte-identical; **+ hardening**: `setClock` `am=0` init fixes an uninitialized read |
| Board build flags (LITE_VERSION, HAS_EINK, ROTATION=3, RTC, GPIO lists) | intact | values identical; MusicMenu exclusion newly + correctly implemented |
| Settings/e-ink persistence (dimmer/refresh validators) | intact | `config.cpp` serialization/validators byte-identical |
| Startup init order, LittleFS mount | intact | `setup()` order identical; only a no-op `setBrightness` added |

## Conclusion
The July upstream sync **did not regress any CoreInk behavior** — it preserved everything byte-for-byte and added two hardening fixes. The one real hazard (partition-migration creds wipe) is now fixed via the CoreInk-specific partition table. Proposed release + this fix is safe to flash with creds intact.
