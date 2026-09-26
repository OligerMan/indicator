#include "Tilt.h"

#include <math.h>

namespace ind {

static constexpr float kRadToDeg = 57.29577951f;

float foldAngle(float deg) {
  if (fabsf(deg) > 90.0f) {
    const float sign = deg > 0 ? 1.0f : -1.0f;
    return (fabsf(deg) - 180.0f) * sign;
  }
  return deg;
}

Tilt computeTilt(float ax, float ay, float az) {
  static const float kSqrt3 = sqrtf(3.0f);
  Tilt t;
  t.x = foldAngle(atan2f(ax, az) * kRadToDeg);
  t.y = foldAngle(atan2f(ay, az) * kRadToDeg);
  const float yMinus = foldAngle(atan2f(-ay * kSqrt3 - ax, az) * kRadToDeg);
  const float yPlus = foldAngle(atan2f(ay * kSqrt3 - ax, az) * kRadToDeg);
  t.subX = t.x > 0 ? t.x : 0;
  t.subYMinus = yMinus > 0 ? yMinus : 0;
  t.subYPlus = yPlus > 0 ? yPlus : 0;
  return t;
}

int tiltChoice(float angle, int count) {
  if (count <= 1) return 0;
  int i = static_cast<int>((angle + 90.0f) / (180.0f / count));
  if (i < 0) i = 0;
  if (i >= count) i = count - 1;
  return i;
}

}  // namespace ind
