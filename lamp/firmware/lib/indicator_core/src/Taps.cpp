#include "Taps.h"

#include <math.h>

namespace ind {

bool TapDetector::update(float ax, float ay, float az, uint32_t nowMs) {
  const float mag = sqrtf(ax * ax + ay * ay + az * az);
  if (!initialized_) {
    baseline_ = mag;
    initialized_ = true;
  }

  lastDeviation_ = mag - baseline_;
  const float dev = fabsf(lastDeviation_);

  bool tap = false;
  if (armed_) {
    if (dev > cfg_.highThreshold && (!tapped_ || nowMs - lastTapMs_ >= cfg_.refractoryMs)) {
      tap = true;
      tapped_ = true;
      armed_ = false;
      lastTapMs_ = nowMs;
    } else {
      // Track slow drift (temperature, orientation-dependent calibration error) only
      // while nothing is happening, so hits do not drag the baseline.
      baseline_ += cfg_.baselineAlpha * (mag - baseline_);
    }
  } else if (dev < cfg_.lowThreshold) {
    armed_ = true;
  }
  return tap;
}

const char* gestureName(Gesture g) {
  switch (g) {
    case Gesture::None: return "none";
    case Gesture::Single: return "single";
    case Gesture::Double: return "double";
    case Gesture::Triple: return "triple";
    case Gesture::Many: return "many";
  }
  return "?";
}

uint8_t TapSequencer::onTap(uint32_t nowMs) {
  if (count_ > 0 && nowMs - lastTapMs_ < cfg_.minGapMs) return 0;
  if (count_ < 255) count_++;
  lastTapMs_ = nowMs;
  return count_;
}

Gesture TapSequencer::poll(uint32_t nowMs) {
  if (count_ == 0 || nowMs - lastTapMs_ <= cfg_.maxGapMs) return Gesture::None;
  const uint8_t n = count_;
  count_ = 0;
  if (n >= 4) return Gesture::Many;
  return static_cast<Gesture>(n);
}

}  // namespace ind
