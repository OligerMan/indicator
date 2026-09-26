#pragma once
// Software current limiter for the LED panels.
//
// The indicator mostly shows solid fills, which is close to the worst case for
// current. Before every frame we estimate the panel current from pixel values and
// lower the global (OE) brightness if the estimate exceeds the budget. Global
// brightness is used instead of scaling pixel values so no colour bits are lost.
//
// Model: I = idle + perChannel * sum(linear(channel)) * brightness / 255
//   linear() approximates the CIE correction the HUB75 library applies, so the
//   sum reflects how long LEDs are actually on.
// Coefficients are placeholders until measured with the INA226
// (docs/lamp/bringup.md, step "калибровка тока").

#include <stdint.h>

#include "Frame.h"

namespace ind {

struct PowerModel {
  float idleMa = 600.0f;            // both panels, all black
  float maPerChannelFull = 0.25f;   // one LED channel at value 255, brightness 255
};

// Perceptual 0..255 -> linear duty 0..255 (approx. CIE 1931, same curve as the driver)
uint8_t linearDuty(uint8_t v);

// Sum of linearDuty() over every channel of every pixel of the frame
uint32_t frameLoad(const Frame& f);

float estimateCurrentMa(const PowerModel& m, uint32_t load, uint8_t brightness);

// Highest brightness <= requested that keeps the estimate within limitMa.
uint8_t limitBrightness(const PowerModel& m, uint32_t load, uint8_t requested, float limitMa);

}  // namespace ind
