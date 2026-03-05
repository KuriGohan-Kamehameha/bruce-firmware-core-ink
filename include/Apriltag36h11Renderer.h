#pragma once

#include <stdint.h>
#include <string.h>

#include "apriltag36h11_codes.h"

namespace bruce_apriltag {

constexpr uint8_t kGridSize = 10U;
constexpr uint8_t kBitCount = 36U;
constexpr uint16_t kTagCount = sizeof(kTag36h11Codes) / sizeof(kTag36h11Codes[0]);

static_assert(kGridSize > 0U, "kGridSize must be positive");
static_assert(kBitCount > 0U, "kBitCount must be positive");
static_assert(kTagCount > 0U, "kTagCount must be positive");

inline uint64_t readCode(uint16_t tagId) {
    if (tagId >= kTagCount) {
        return UINT64_C(0);
    }

    uint64_t code = UINT64_C(0);
    memcpy_P(&code, &kTag36h11Codes[tagId], sizeof(code));
    return code;
}

inline uint8_t readBitX(uint8_t bitIndex) {
    if (bitIndex >= kBitCount) {
        return 0U;
    }

    uint32_t value = 0U;
    memcpy_P(&value, &kTag36h11BitX[bitIndex], sizeof(value));
    return static_cast<uint8_t>(value);
}

inline uint8_t readBitY(uint8_t bitIndex) {
    if (bitIndex >= kBitCount) {
        return 0U;
    }

    uint32_t value = 0U;
    memcpy_P(&value, &kTag36h11BitY[bitIndex], sizeof(value));
    return static_cast<uint8_t>(value);
}

// Builds a 10x10 tag grid where 0 = black and 1 = white.
inline bool buildGrid(uint16_t tagId, uint8_t out[kGridSize][kGridSize]) {
    if (tagId >= kTagCount) {
        return false;
    }

    for (uint8_t y = 0U; y < kGridSize; ++y) {
        for (uint8_t x = 0U; x < kGridSize; ++x) {
            out[y][x] = 1U; // White quiet zone.
        }
    }

    // Inner black border ring.
    for (uint8_t y = 1U; y <= 8U; ++y) {
        for (uint8_t x = 1U; x <= 8U; ++x) {
            if ((x == 1U) || (x == 8U) || (y == 1U) || (y == 8U)) {
                out[y][x] = 0U;
            }
        }
    }

    // Data region starts black and white bits are drawn over it.
    for (uint8_t y = 2U; y <= 7U; ++y) {
        for (uint8_t x = 2U; x <= 7U; ++x) {
            out[y][x] = 0U;
        }
    }

    const uint64_t code = readCode(tagId);
    for (uint8_t i = 0U; i < kBitCount; ++i) {
        const uint8_t bit = static_cast<uint8_t>((code >> (kBitCount - i - 1U)) & UINT64_C(0x1));
        if (bit == 0U) {
            continue;
        }

        const uint8_t x = static_cast<uint8_t>(readBitX(i) + 1U);
        const uint8_t y = static_cast<uint8_t>(readBitY(i) + 1U);

        if ((x < kGridSize) && (y < kGridSize)) {
            out[y][x] = 1U;
        }
    }

    return true;
}

// drawCell(x, y, isWhite)
template <typename DrawCellFn>
inline bool drawTag(uint16_t tagId, DrawCellFn drawCell) {
    uint8_t grid[kGridSize][kGridSize];
    if (!buildGrid(tagId, grid)) {
        return false;
    }

    for (uint8_t y = 0U; y < kGridSize; ++y) {
        for (uint8_t x = 0U; x < kGridSize; ++x) {
            drawCell(x, y, grid[y][x] == 1U);
        }
    }

    return true;
}

} // namespace bruce_apriltag
