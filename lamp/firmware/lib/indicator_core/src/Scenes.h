#pragma once
// Concrete scenes. Order and behaviour follow the v1 lamp
// (legacy/indicator-v1/sketch_sep16a2): menu -> smooth, solid, tilt colour,
// violet speckles, motion. See docs/lamp/firmware.md for the state diagram.

#include "Scene.h"

namespace ind {

// v1 state 0. Idle: calm blue. Double tap -> selection: tilt picks a scene
// (the lamp shows its preview colour), single tap starts it, double tap goes back.
class MenuScene : public Scene {
 public:
  const char* name() const override { return "menu"; }
  Rgb previewColor(uint32_t) const override { return colors::kBlue; }
  bool selectable() const override { return false; }

  void enter(SceneContext&) override { selecting_ = false; }
  void onGesture(Gesture g, SceneContext& ctx) override;
  void render(Frame& f, SceneContext& ctx) override;

  bool selecting() const { return selecting_; }
  int highlighted() const { return highlighted_; }

 private:
  bool selecting_ = false;
  int highlighted_ = 0;  // index among selectable scenes
};

// v1 state 4: endless cross-fade between random colours.
class SmoothScene : public Scene {
 public:
  const char* name() const override { return "smooth"; }
  Rgb previewColor(uint32_t) const override { return colors::kWhite; }
  void enter(SceneContext& ctx) override;
  void render(Frame& f, SceneContext& ctx) override;

  static constexpr uint32_t kPeriodMs = 3500;

 private:
  Rgb from_, to_;
  uint32_t startMs_ = 0;
};

// v1 state 1: one solid colour, single tap -> random new colour.
// Also the target of the "color r g b" command.
class SolidScene : public Scene {
 public:
  const char* name() const override { return "solid"; }
  Rgb previewColor(uint32_t) const override { return colors::kRed; }
  void enter(SceneContext& ctx) override;
  void onGesture(Gesture g, SceneContext& ctx) override;
  void render(Frame& f, SceneContext& ctx) override;

  void setColor(Rgb c) { color_ = c; keepColor_ = true; }
  Rgb color() const { return color_; }

 private:
  Rgb color_;
  bool keepColor_ = false;  // set by setColor() so enter() does not randomise it
};

// v1 state 3: mix RGB by tilting in three directions, double tap fixes the colour.
class TiltColorScene : public Scene {
 public:
  const char* name() const override { return "tiltcolor"; }
  Rgb previewColor(uint32_t nowMs) const override;
  void enter(SceneContext& ctx) override;
  void onGesture(Gesture g, SceneContext& ctx) override;
  void render(Frame& f, SceneContext& ctx) override;

  static constexpr float kThresholdDeg = 20.0f;
  static constexpr float kRisePerSec = 66.0f;  // v1: +2 per ~30 ms frame
  static constexpr float kFallPerSec = 33.0f;  // v1: -1 per frame

 private:
  float c_[3] = {0, 0, 0};
  bool fixed_ = false;
  uint32_t blinkStartMs_ = 0;
};

// v1 state 2: "фиолетовый в крапинку".
class SpecklesScene : public Scene {
 public:
  const char* name() const override { return "speckles"; }
  Rgb previewColor(uint32_t) const override { return colors::kViolet; }
  void enter(SceneContext& ctx) override { shuffle(ctx); }
  void onGesture(Gesture g, SceneContext& ctx) override;
  void render(Frame& f, SceneContext& ctx) override;

  static constexpr int kCount = 48;
  Rgb background = colors::kViolet;
  Rgb speck = colors::kGreen;

 private:
  void shuffle(SceneContext& ctx);
  struct Speck {
    int16_t x, y;
    uint8_t r, phase;
  } specks_[kCount];
};

// v1 state 5: calm = cyan, shaking turns it red.
class MotionScene : public Scene {
 public:
  const char* name() const override { return "motion"; }
  Rgb previewColor(uint32_t) const override { return colors::kCyan; }
  void enter(SceneContext&) override { calm_ = 100.0f; }
  void render(Frame& f, SceneContext& ctx) override;

 private:
  float calm_ = 100.0f;  // 100 = calm, 0 = shaking hard
};

// Frames pushed from outside (UDP from a PC, later the walker's brain).
// Not in the menu: activated by the first packet, returns to the previous scene
// after kTimeoutMs without packets.
class StreamScene : public Scene {
 public:
  const char* name() const override { return "stream"; }
  Rgb previewColor(uint32_t) const override { return colors::kWhite; }
  bool selectable() const override { return false; }
  void render(Frame& f, SceneContext& ctx) override;

  // Returns true if the caller may write the packet into the frame now
  bool notifyPacket(SceneManager& mgr, uint32_t nowMs);

  static constexpr uint32_t kTimeoutMs = 3000;

 private:
  int returnTo_ = 0;
  uint32_t lastPacketMs_ = 0;
};

}  // namespace ind
