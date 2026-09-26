#include "Display.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "config.h"

using namespace ind;

bool Display::begin() {
  HUB75_I2S_CFG::i2s_pins pins = {cfg::kPinR1, cfg::kPinG1, cfg::kPinB1, cfg::kPinR2, cfg::kPinG2,
                                  cfg::kPinB2, cfg::kPinA,  cfg::kPinB,  cfg::kPinC,  cfg::kPinD,
                                  cfg::kPinE,  cfg::kPinLat, cfg::kPinOe, cfg::kPinClk};
  HUB75_I2S_CFG mx(cfg::kPanelWidth, cfg::kPanelHeight, cfg::kChainLength, pins);
  mx.driver = static_cast<HUB75_I2S_CFG::shift_driver>(cfg::kShiftDriver);
  mx.clkphase = cfg::kClockPhase;
  mx.min_refresh_rate = cfg::kMinRefreshHz;
  mx.double_buff = false;
  mx.setPixelColorDepthBits(cfg::kColorDepthBits);

  dma_ = new MatrixPanel_I2S_DMA(mx);
  if (!dma_->begin()) {
    delete dma_;
    dma_ = nullptr;
    return false;
  }

  model_.idleMa = cfg::kIdleCurrentMa;
  model_.maPerChannelFull = cfg::kMaPerChannelFull;
  requested_ = cfg::kDefaultBrightness;
  limitMa_ = cfg::kDefaultCurrentLimitMa;
  applied_ = requested_;
  dma_->setBrightness8(applied_);
  dma_->clearScreen();
  return true;
}

int Display::refreshRate() const { return dma_ ? dma_->calculated_refresh_rate : 0; }

uint8_t Display::present(const Frame& f) {
  if (!dma_) return 0;

  const uint32_t load = frameLoad(f);
  applied_ = limitBrightness(model_, load, requested_, limitMa_);
  estimateMa_ = estimateCurrentMa(model_, load, applied_);
  dma_->setBrightness8(applied_);

  for (int face = 0; face < kFaceCount; face++) {
    const int xOff = cfg::kFaceChainPos[face] * cfg::kPanelWidth;
    const bool fx = cfg::kFaceFlipX[face], fy = cfg::kFaceFlipY[face];
    for (int y = 0; y < kFaceHeight; y++) {
      const int py = fy ? kFaceHeight - 1 - y : y;
      for (int x = 0; x < kFaceWidth; x++) {
        const Rgb c = f.get(face, x, y);
        const int px = xOff + (fx ? kFaceWidth - 1 - x : x);
        dma_->drawPixelRGB888(px, py, c.r, c.g, c.b);
      }
    }
  }
  return applied_;
}
