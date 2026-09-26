#pragma once
// Portable colour helpers. No Arduino dependencies: compiled for ESP32 and for native tests.

#include <stdint.h>

namespace ind {

struct Rgb {
  uint8_t r = 0, g = 0, b = 0;

  constexpr Rgb() = default;
  constexpr Rgb(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}

  bool operator==(const Rgb& o) const { return r == o.r && g == o.g && b == o.b; }
  bool operator!=(const Rgb& o) const { return !(*this == o); }
};

namespace colors {
constexpr Rgb kBlack{0, 0, 0};
constexpr Rgb kWhite{255, 255, 255};
constexpr Rgb kRed{255, 0, 0};
constexpr Rgb kBlue{0, 0, 255};
constexpr Rgb kCyan{0, 255, 255};
constexpr Rgb kGreen{0, 255, 0};
// "фиолетовый" from the v1 lamp
constexpr Rgb kViolet{109, 0, 204};
}  // namespace colors

// t = 0 -> a, t = 255 -> b
Rgb lerp(Rgb a, Rgb b, uint8_t t);
// Same, with float t in [0, 1]
Rgb lerpf(Rgb a, Rgb b, float t);
// Multiply every channel by s/255
Rgb scale(Rgb c, uint8_t s);
// Hue 0..255 wraps around the colour wheel; sat and val are 0..255
Rgb hsv(uint8_t hue, uint8_t sat, uint8_t val);

}  // namespace ind
