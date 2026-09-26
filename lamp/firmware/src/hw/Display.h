#pragma once
// HUB75 output: maps the logical Frame (two faces) onto the physical panel chain and
// applies the current limiter through global brightness.

#include <Frame.h>
#include <Power.h>

class MatrixPanel_I2S_DMA;

class Display {
 public:
  bool begin();
  bool ok() const { return dma_ != nullptr; }

  // Pushes the frame to the DMA buffer. Returns the brightness actually applied.
  uint8_t present(const ind::Frame& f);

  void setBrightness(uint8_t b) { requested_ = b; }
  uint8_t brightness() const { return requested_; }
  uint8_t appliedBrightness() const { return applied_; }
  void setCurrentLimit(float ma) { limitMa_ = ma; }
  float currentLimit() const { return limitMa_; }
  float estimatedCurrentMa() const { return estimateMa_; }
  int refreshRate() const;

  ind::PowerModel& powerModel() { return model_; }

  // Direct access for bring-up tests
  MatrixPanel_I2S_DMA* raw() { return dma_; }

 private:
  MatrixPanel_I2S_DMA* dma_ = nullptr;
  ind::PowerModel model_;
  uint8_t requested_ = 0;
  uint8_t applied_ = 0;
  float limitMa_ = 0;
  float estimateMa_ = 0;
};
