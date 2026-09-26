// Panel bring-up test (env:panel_test). Walks through patterns that reveal wiring and
// configuration problems one at a time. What to look for: docs/lamp/bringup.md
// Serial: 'n' next step, 'p' pause/resume, '+'/'-' brightness.

#ifdef BRINGUP_PANEL_TEST

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_heap_caps.h>

#include "config.h"

namespace {

MatrixPanel_I2S_DMA* dma = nullptr;
constexpr int W = cfg::kPanelWidth * cfg::kChainLength;
constexpr int H = cfg::kPanelHeight;
constexpr int PW = cfg::kPanelWidth;

uint8_t brightness = 40;  // low on purpose: first power-up, unknown supply
int step = 0;
bool paused = false;
bool drawn = false;
uint32_t stepStart = 0;
constexpr uint32_t kStepMs = 4000;

void fillPanel(int panel, uint8_t r, uint8_t g, uint8_t b) {
  for (int y = 0; y < H; y++)
    for (int x = 0; x < PW; x++) dma->drawPixelRGB888(panel * PW + x, y, r, g, b);
}

void rect(int x0, int y0, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
  for (int y = y0; y < y0 + h; y++)
    for (int x = x0; x < x0 + w; x++) dma->drawPixelRGB888(x, y, r, g, b);
}

struct Step {
  const char* name;
  bool animated;  // static patterns are drawn once, redrawing them would flicker
  void (*draw)(uint32_t t);
};

const Step kSteps[] = {
    {"panel 0 red", false, [](uint32_t) { dma->clearScreen(); fillPanel(0, 255, 0, 0); }},
    {"panel 0 green", false, [](uint32_t) { dma->clearScreen(); fillPanel(0, 0, 255, 0); }},
    {"panel 0 blue", false, [](uint32_t) { dma->clearScreen(); fillPanel(0, 0, 0, 255); }},
    {"panel 1 red", false, [](uint32_t) { dma->clearScreen(); fillPanel(1, 255, 0, 0); }},
    {"panel 1 green", false, [](uint32_t) { dma->clearScreen(); fillPanel(1, 0, 255, 0); }},
    {"panel 1 blue", false, [](uint32_t) { dma->clearScreen(); fillPanel(1, 0, 0, 255); }},
    {"both white (check supply sag)", false, [](uint32_t) { dma->fillScreenRGB888(255, 255, 255); }},
    // One lit row moving down: every row must light exactly once, top to bottom.
    // Rows lighting in pairs / skipping = A..E wiring or E pin not set.
    {"row scan", true, [](uint32_t t) {
       const int y = (t / 50) % H;
       dma->clearScreen();
       rect(0, y, W, 1, 255, 255, 255);
     }},
    {"column scan", true, [](uint32_t t) {
       const int x = (t / 15) % W;
       dma->clearScreen();
       rect(x, 0, 1, H, 255, 255, 255);
     }},
    // Smooth ramps: visible steps show the chosen colour depth
    {"gradients", false, [](uint32_t) {
       for (int x = 0; x < W; x++) {
         const uint8_t v = static_cast<uint8_t>(x * 255 / (W - 1));
         rect(x, 0, 1, 16, v, 0, 0);
         rect(x, 16, 1, 16, 0, v, 0);
         rect(x, 32, 1, 16, 0, 0, v);
         rect(x, 48, 1, 16, v, v, v);
       }
     }},
    // Orientation markers per panel: red = top-left, green = top-right, blue = bottom-left
    {"orientation markers", false, [](uint32_t) {
       dma->clearScreen();
       for (int p = 0; p < cfg::kChainLength; p++) {
         rect(p * PW, 0, 12, 12, 255, 0, 0);
         rect(p * PW + PW - 12, 0, 12, 12, 0, 255, 0);
         rect(p * PW, H - 12, 12, 12, 0, 0, 255);
         // panel index as a row of dots
         for (int i = 0; i <= p; i++) rect(p * PW + 40 + i * 10, 28, 6, 6, 255, 255, 255);
       }
     }},
};
constexpr int kStepCount = sizeof(kSteps) / sizeof(kSteps[0]);

void announce() {
  drawn = false;
  Serial.printf("[test] step %d/%d: %s (brightness %u)\n", step + 1, kStepCount, kSteps[step].name,
                brightness);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[panel_test] start");

  HUB75_I2S_CFG::i2s_pins pins = {cfg::kPinR1, cfg::kPinG1, cfg::kPinB1, cfg::kPinR2, cfg::kPinG2,
                                  cfg::kPinB2, cfg::kPinA,  cfg::kPinB,  cfg::kPinC,  cfg::kPinD,
                                  cfg::kPinE,  cfg::kPinLat, cfg::kPinOe, cfg::kPinClk};
  HUB75_I2S_CFG mx(cfg::kPanelWidth, cfg::kPanelHeight, cfg::kChainLength, pins);
  mx.driver = static_cast<HUB75_I2S_CFG::shift_driver>(cfg::kShiftDriver);
  mx.clkphase = cfg::kClockPhase;
  mx.min_refresh_rate = cfg::kMinRefreshHz;
  mx.setPixelColorDepthBits(cfg::kColorDepthBits);

  const size_t before = heap_caps_get_free_size(MALLOC_CAP_DMA);
  dma = new MatrixPanel_I2S_DMA(mx);
  if (!dma->begin()) {
    Serial.println("[panel_test] begin() failed: not enough DMA memory? try fewer colour bits");
    for (;;) delay(1000);
  }
  Serial.printf("[panel_test] %dx%d, %u bits, refresh %d Hz, DMA used %u, DMA free %u\n", W, H,
                cfg::kColorDepthBits, dma->calculated_refresh_rate,
                before - heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_free_size(MALLOC_CAP_DMA));
  dma->setBrightness8(brightness);
  stepStart = millis();
  announce();
}

void loop() {
  while (Serial.available()) {
    switch (Serial.read()) {
      case 'n': step = (step + 1) % kStepCount; stepStart = millis(); announce(); break;
      case 'p': paused = !paused; Serial.println(paused ? "[test] paused" : "[test] running"); break;
      case '+': brightness = brightness > 235 ? 255 : brightness + 20; dma->setBrightness8(brightness); announce(); break;
      case '-': brightness = brightness < 20 ? 0 : brightness - 20; dma->setBrightness8(brightness); announce(); break;
    }
  }
  const uint32_t now = millis();
  if (!paused && now - stepStart > kStepMs) {
    step = (step + 1) % kStepCount;
    stepStart = now;
    announce();
  }
  if (kSteps[step].animated || !drawn) {
    kSteps[step].draw(now - stepStart);
    drawn = true;
  }
  delay(20);
}

#endif  // BRINGUP_PANEL_TEST
