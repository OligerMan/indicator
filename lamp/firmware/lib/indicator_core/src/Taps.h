#pragma once
// Tap ("стук") recognition, split in two stages so each can be tested on its own:
//   TapDetector  - raw accelerometer samples (hundreds of Hz) -> tap onsets
//   TapSequencer - tap onsets -> gestures (single / double / triple ...)
// Thresholds come from the v1 lamp (legacy/indicator-v1): 5 m/s^2 to trigger,
// 3 m/s^2 to re-arm. v1 compared against a fixed 9.8; here the baseline adapts,
// so a slightly miscalibrated sensor does not produce phantom taps.

#include <stdint.h>

namespace ind {

struct TapDetectorConfig {
  float highThreshold = 5.0f;  // m/s^2 deviation from baseline that counts as a hit
  float lowThreshold = 3.0f;   // deviation below which the detector re-arms
  uint32_t refractoryMs = 60;  // ignore ringing right after a hit
  float baselineAlpha = 0.01f; // low-pass factor for |a| baseline, per sample
};

class TapDetector {
 public:
  explicit TapDetector(const TapDetectorConfig& cfg = TapDetectorConfig()) : cfg_(cfg) {}

  // Feed one sample (m/s^2). Returns true exactly once per tap, at its onset.
  bool update(float ax, float ay, float az, uint32_t nowMs);

  // |a| - baseline of the last sample (signed), useful as a "motion" measure
  float deviation() const { return lastDeviation_; }
  float baseline() const { return baseline_; }
  const TapDetectorConfig& config() const { return cfg_; }
  void setConfig(const TapDetectorConfig& cfg) { cfg_ = cfg; }

 private:
  TapDetectorConfig cfg_;
  bool initialized_ = false;
  bool armed_ = true;
  bool tapped_ = false;
  uint32_t lastTapMs_ = 0;
  float baseline_ = 9.8f;
  float lastDeviation_ = 0.0f;
};

enum class Gesture : uint8_t {
  None = 0,
  Single = 1,
  Double = 2,
  Triple = 3,
  Many = 4,  // four or more taps
};

const char* gestureName(Gesture g);

struct TapSequencerConfig {
  uint32_t minGapMs = 150;  // closer taps are treated as bounce of the same tap
  uint32_t maxGapMs = 600;  // a longer pause ends the sequence
};

class TapSequencer {
 public:
  explicit TapSequencer(const TapSequencerConfig& cfg = TapSequencerConfig()) : cfg_(cfg) {}

  // Register a tap onset. Returns the number of taps in the current sequence
  // (0 if the tap was discarded as bounce).
  uint8_t onTap(uint32_t nowMs);

  // Call regularly. Returns a gesture once the sequence has ended, otherwise None.
  // A single tap is therefore reported maxGapMs after it happened, like
  // single_hit_not_double in v1.
  Gesture poll(uint32_t nowMs);

  uint8_t pendingTaps() const { return count_; }
  void reset() { count_ = 0; }

 private:
  TapSequencerConfig cfg_;
  uint8_t count_ = 0;
  uint32_t lastTapMs_ = 0;
};

}  // namespace ind
