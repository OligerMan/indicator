#include "Power.h"

#include <math.h>

namespace ind {

namespace {

struct DutyTable {
  uint8_t v[256];
  DutyTable() {
    for (int i = 0; i < 256; i++) {
      // CIE 1931 lightness -> luminance
      const float l = i * 100.0f / 255.0f;
      float y = l <= 8.0f ? l / 903.3f : powf((l + 16.0f) / 116.0f, 3.0f);
      v[i] = static_cast<uint8_t>(y * 255.0f + 0.5f);
    }
  }
};

const DutyTable& dutyTable() {
  static const DutyTable t;
  return t;
}

}  // namespace

uint8_t linearDuty(uint8_t v) { return dutyTable().v[v]; }

uint32_t frameLoad(const Frame& f) {
  const DutyTable& t = dutyTable();
  uint32_t sum = 0;
  for (int face = 0; face < kFaceCount; face++)
    for (int y = 0; y < kFaceHeight; y++)
      for (int x = 0; x < kFaceWidth; x++) {
        const Rgb c = f.get(face, x, y);
        sum += t.v[c.r] + t.v[c.g] + t.v[c.b];
      }
  return sum;
}

float estimateCurrentMa(const PowerModel& m, uint32_t load, uint8_t brightness) {
  return m.idleMa + m.maPerChannelFull * (load / 255.0f) * (brightness / 255.0f);
}

uint8_t limitBrightness(const PowerModel& m, uint32_t load, uint8_t requested, float limitMa) {
  const float full = estimateCurrentMa(m, load, 255) - m.idleMa;  // LED current at brightness 255
  const float budget = limitMa - m.idleMa;
  if (full <= 0.0f) return requested;
  if (budget <= 0.0f) return 0;
  const float maxB = 255.0f * budget / full;
  if (maxB >= requested) return requested;
  return static_cast<uint8_t>(maxB);
}

}  // namespace ind
