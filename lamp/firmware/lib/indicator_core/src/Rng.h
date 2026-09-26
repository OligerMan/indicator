#pragma once
// Small deterministic PRNG (xorshift32) so scenes behave the same on ESP32 and in tests.

#include <stdint.h>

namespace ind {

class Rng {
 public:
  explicit Rng(uint32_t seed = 0x1234567u) { reseed(seed); }
  void reseed(uint32_t seed) { s_ = seed ? seed : 0x1234567u; }

  uint32_t next() {
    s_ ^= s_ << 13;
    s_ ^= s_ >> 17;
    s_ ^= s_ << 5;
    return s_;
  }
  // [0, n)
  uint32_t below(uint32_t n) { return n ? next() % n : 0; }
  uint8_t byte() { return static_cast<uint8_t>(next() >> 24); }

 private:
  uint32_t s_;
};

}  // namespace ind
