#pragma once
// Scene = one lamp mode (v1 called them "states"). SceneManager owns the active
// scene and applies the global rule from v1: a triple tap returns to the menu
// from any scene.

#include <stdint.h>

#include "Frame.h"
#include "Rng.h"
#include "Taps.h"
#include "Tilt.h"

namespace ind {

struct InputState {
  Tilt tilt;            // from low-passed gravity
  float motion = 0.0f;  // | |a| - baseline |, m/s^2, instantaneous
  bool imuOk = false;
};

class SceneManager;

struct SceneContext {
  uint32_t nowMs = 0;
  float dt = 0.0f;  // seconds since previous render
  InputState input;
  Rng* rng = nullptr;
  SceneManager* mgr = nullptr;
};

class Scene {
 public:
  virtual ~Scene() = default;
  virtual const char* name() const = 0;
  // Colour the menu shows while this scene is highlighted
  virtual Rgb previewColor(uint32_t nowMs) const = 0;
  // Menu and service scenes are not offered in the tilt menu
  virtual bool selectable() const { return true; }

  virtual void enter(SceneContext&) {}
  virtual void exit(SceneContext&) {}
  // Raw tap onset, for immediate visual feedback
  virtual void onTap(SceneContext&) {}
  virtual void onGesture(Gesture, SceneContext&) {}
  // Draw the whole frame
  virtual void render(Frame&, SceneContext&) = 0;
};

class SceneManager {
 public:
  static constexpr int kMaxScenes = 16;

  // First added scene is the menu
  bool add(Scene* s);
  int count() const { return count_; }
  Scene* at(int i) const { return (i >= 0 && i < count_) ? scenes_[i] : nullptr; }
  int find(const char* name) const;

  int currentIndex() const { return current_; }
  Scene* current() const { return at(current_); }

  // Selectable scenes in menu order
  int selectableCount() const;
  int selectableIndex(int n) const;  // n-th selectable -> scene index, -1 if none

  // Switches immediately (exit old, enter new) using the context of the last update
  bool setScene(int index);
  void goMenu() { setScene(0); }

  void tap(uint32_t nowMs);
  void gesture(Gesture g, uint32_t nowMs);
  void render(Frame& f, const InputState& in, uint32_t nowMs);

  Rng& rng() { return rng_; }

 private:
  SceneContext makeContext(uint32_t nowMs);

  Scene* scenes_[kMaxScenes] = {};
  int count_ = 0;
  int current_ = -1;
  uint32_t lastRenderMs_ = 0;
  bool rendered_ = false;
  InputState lastInput_;
  uint32_t lastNowMs_ = 0;
  Rng rng_;
};

}  // namespace ind
