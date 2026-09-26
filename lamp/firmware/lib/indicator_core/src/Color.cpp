#include "Color.h"

namespace ind {

static uint8_t mix(uint8_t a, uint8_t b, uint8_t t) {
  return static_cast<uint8_t>((a * (255 - t) + b * t + 127) / 255);
}

Rgb lerp(Rgb a, Rgb b, uint8_t t) {
  return Rgb(mix(a.r, b.r, t), mix(a.g, b.g, t), mix(a.b, b.b, t));
}

Rgb lerpf(Rgb a, Rgb b, float t) {
  if (t <= 0.0f) return a;
  if (t >= 1.0f) return b;
  return lerp(a, b, static_cast<uint8_t>(t * 255.0f + 0.5f));
}

Rgb scale(Rgb c, uint8_t s) {
  return Rgb(static_cast<uint8_t>((c.r * s + 127) / 255),
             static_cast<uint8_t>((c.g * s + 127) / 255),
             static_cast<uint8_t>((c.b * s + 127) / 255));
}

Rgb hsv(uint8_t hue, uint8_t sat, uint8_t val) {
  // Six 43-step sectors (256 / 6)
  const uint8_t region = hue / 43;
  const uint8_t rem = static_cast<uint8_t>((hue - region * 43) * 6);

  const uint8_t p = static_cast<uint8_t>((val * (255 - sat)) >> 8);
  const uint8_t q = static_cast<uint8_t>((val * (255 - ((sat * rem) >> 8))) >> 8);
  const uint8_t t = static_cast<uint8_t>((val * (255 - ((sat * (255 - rem)) >> 8))) >> 8);

  switch (region) {
    case 0: return Rgb(val, t, p);
    case 1: return Rgb(q, val, p);
    case 2: return Rgb(p, val, t);
    case 3: return Rgb(p, q, val);
    case 4: return Rgb(t, p, val);
    default: return Rgb(val, p, q);
  }
}

}  // namespace ind
