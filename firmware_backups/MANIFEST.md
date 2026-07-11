# Firmware backups — M5Stack CoreInk (MAC 10:06:1c:0a:bd:4c)

Full-flash images are **gitignored** — they contain the NVS region with saved
WiFi credentials and must never be pushed to the public fork. This manifest and
the per-dump `.meta.txt` sidecars are the tracked, secrets-free record. Durable
binary copies live here and in `/Users/sat/coreink-bruce-backups/2026-07-11-pre-unify/`.

| Date | File | app0 sha256 (short) | Role |
|------|------|---------------------|------|
| 2026-07-05 | `m5stack-coreink_chunked_10-06-1c-0a-bd-4c_4MB.bin` | `55201185…dece` | earlier dump (superseded) |
| 2026-07-11 | `m5stack-coreink_lkw_10-06-1c-0a-bd-4c_2026-07-11_4MB.bin` | `44121744…d156` | **last-known-working on hardware** |

The 2026-07-11 image is the reference baseline for the Phase C regression
comparison against the proposed release. See its `.meta.txt` for full metadata.
