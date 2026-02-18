#pragma once

#include <stdint.h>
#include <string.h>

#include "apriltag36h11_codes.h"

namespace bruce_apriltag {

constexpr uint8_t kGridSize = 10;
constexpr uint8_t kBitCount = 36;
constexpr uint16_t kTagCount = sizeof(kTag36h11Codes) / sizeof(kTag36h11Codes[0]);

inline uint64_t readCode(uint16_t tagId) {
  uint64_t code = 0;
  memcpy_P(&code, &kTag36h11Codes[tagId], sizeof(code));
  return code;
}

inline uint8_t readBitX(uint8_t bitIndex) {
  uint32_t value = 0;
  memcpy_P(&value, &kTag36h11BitX[bitIndex], sizeof(value));
  return static_cast<uint8_t>(value);
}

inline uint8_t readBitY(uint8_t bitIndex) {
  uint32_t value = 0;
  memcpy_P(&value, &kTag36h11BitY[bitIndex], sizeof(value));
  return static_cast<uint8_t>(value);
}

// Builds a 10x10 tag grid where 0 = black, 1 = white.
inline bool buildGrid(uint16_t tagId, uint8_t out[kGridSize][kGridSize]) {
  if (tagId >= kTagCount) {
    return false;
  }

  for (uint8_t y = 0; y < kGridSize; ++y) {
    for (uint8_t x = 0; x < kGridSize; ++x) {
      out[y][x] = 1;  // white background
    }
  }

  for (uint8_t y = 1; y <= 8; ++y) {
    for (uint8_t x = 1; x <= 8; ++x) {
      if (x == 1 || x == 8 || y == 1 || y == 8) {
        out[y][x] = 0;
      }
    }
  }

  for (uint8_t y = 2; y <= 7; ++y) {
    for (uint8_t x = 2; x <= 7; ++x) {
      out[y][x] = 0;
    }
  }

  const uint64_t code = readCode(tagId);
  for (uint8_t i = 0; i < kBitCount; ++i) {
    const uint8_t bit = (code >> (kBitCount - i - 1)) & 0x1;
    if (bit == 0) {
      continue;
    }

    const uint8_t x = static_cast<uint8_t>(readBitX(i) + 1);
    const uint8_t y = static_cast<uint8_t>(readBitY(i) + 1);
    out[y][x] = 1;
  }

  return true;
}

// Helper for Bruce integration. Pass a lambda that maps a cell to a drawn square.
// drawCell(x, y, isWhite)

template <typename DrawCellFn>
inline bool drawTag(uint16_t tagId, DrawCellFn drawCell) {
  uint8_t grid[kGridSize][kGridSize];
  if (!buildGrid(tagId, grid)) {
    return false;
  }

  for (uint8_t y = 0; y < kGridSize; ++y) {
    for (uint8_t x = 0; x < kGridSize; ++x) {
      drawCell(x, y, grid[y][x] == 1);
    }
  }

  return true;
}

}  // namespace bruce_apriltag
