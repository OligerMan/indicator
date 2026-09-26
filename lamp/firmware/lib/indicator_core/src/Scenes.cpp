#include "Scenes.h"

#include <math.h>

namespace ind {

namespace {

Rgb randomColor(Rng& rng) { return Rgb(rng.byte(), rng.byte(), rng.byte()); }

// Row of dots at the bottom of a face: where we are in the menu
void drawMenuDots(Frame& f, int face, int count, int selected) {
  const int spacing = 12;
  const int x0 = kFaceWidth / 2 - (count - 1) * spacing / 2;
  const int y = kFaceHeight - 8;
  for (int i = 0; i < count; i++) {
    const int x = x0 + i * spacing;
    if (i == selected) {
      f.fillCircle(face, x, y, 3, colors::kWhite);
    } else {
      f.fillCircle(face, x, y, 2, colors::kBlack);
      f.fillCircle(face, x, y, 1, Rgb(128, 128, 128));
    }
  }
}

}  // namespace

// ---------------------------------------------------------------- MenuScene

void MenuScene::onGesture(Gesture g, SceneContext& ctx) {
  if (!selecting_) {
    if (g == Gesture::Double) selecting_ = true;
    return;
  }
  if (g == Gesture::Single) {
    const int idx = ctx.mgr->selectableIndex(highlighted_);
    if (idx >= 0) ctx.mgr->setScene(idx);  // this object is exited here, return immediately
    return;
  }
  if (g == Gesture::Double) selecting_ = false;
}

void MenuScene::render(Frame& f, SceneContext& ctx) {
  if (!selecting_) {
    // Slow breathing blue: 60%..100% over ~4 s
    const float phase = (ctx.nowMs % 4000) / 4000.0f * 6.2831853f;
    const uint8_t level = static_cast<uint8_t>(204 + 51 * sinf(phase));
    f.fill(scale(colors::kBlue, level));
    return;
  }
  const int n = ctx.mgr->selectableCount();
  highlighted_ = tiltChoice(ctx.input.tilt.y, n);
  const Scene* s = ctx.mgr->at(ctx.mgr->selectableIndex(highlighted_));
  f.fillFace(kFront, s ? s->previewColor(ctx.nowMs) : colors::kBlack);
  drawMenuDots(f, kFront, n, highlighted_);
  f.copyFace(kFront, kBack);
}

// -------------------------------------------------------------- SmoothScene

void SmoothScene::enter(SceneContext& ctx) {
  from_ = randomColor(*ctx.rng);
  to_ = randomColor(*ctx.rng);
  startMs_ = ctx.nowMs;
}

void SmoothScene::render(Frame& f, SceneContext& ctx) {
  uint32_t t = ctx.nowMs - startMs_;
  if (t >= kPeriodMs) {
    from_ = to_;
    to_ = randomColor(*ctx.rng);
    startMs_ = ctx.nowMs;
    t = 0;
  }
  f.fill(lerpf(from_, to_, static_cast<float>(t) / kPeriodMs));
}

// --------------------------------------------------------------- SolidScene

void SolidScene::enter(SceneContext& ctx) {
  if (!keepColor_) color_ = randomColor(*ctx.rng);
  keepColor_ = false;
}

void SolidScene::onGesture(Gesture g, SceneContext& ctx) {
  if (g == Gesture::Single) color_ = randomColor(*ctx.rng);
}

void SolidScene::render(Frame& f, SceneContext&) { f.fill(color_); }

// ----------------------------------------------------------- TiltColorScene

Rgb TiltColorScene::previewColor(uint32_t nowMs) const {
  return hsv(static_cast<uint8_t>(nowMs / 20), 255, 255);
}

void TiltColorScene::enter(SceneContext&) {
  fixed_ = false;
  blinkStartMs_ = 0;
}

void TiltColorScene::onGesture(Gesture g, SceneContext& ctx) {
  if (g != Gesture::Double) return;
  fixed_ = !fixed_;
  if (fixed_) blinkStartMs_ = ctx.nowMs;
}

void TiltColorScene::render(Frame& f, SceneContext& ctx) {
  if (!fixed_) {
    const float tilt[3] = {ctx.input.tilt.subX, ctx.input.tilt.subYPlus, ctx.input.tilt.subYMinus};
    for (int ch = 0; ch < 3; ch++) {
      if (tilt[ch] <= kThresholdDeg) continue;
      if (c_[ch] >= 254.0f) {
        // Channel saturated: tilting further pushes the other two down
        for (int o = 0; o < 3; o++)
          if (o != ch) c_[o] = fmaxf(0.0f, c_[o] - kFallPerSec * ctx.dt);
      } else {
        c_[ch] = fminf(255.0f, c_[ch] + kRisePerSec * ctx.dt);
      }
    }
  }
  const Rgb c(static_cast<uint8_t>(c_[0]), static_cast<uint8_t>(c_[1]), static_cast<uint8_t>(c_[2]));

  // Confirmation after fixing, as in v1: on 300, off 500, on 300, off 500, on
  if (fixed_ && blinkStartMs_) {
    const uint32_t t = ctx.nowMs - blinkStartMs_;
    const bool off = (t >= 300 && t < 800) || (t >= 1100 && t < 1600);
    if (t >= 1600) blinkStartMs_ = 0;
    f.fill(off ? colors::kBlack : c);
    return;
  }
  f.fill(c);
}

// ------------------------------------------------------------ SpecklesScene

void SpecklesScene::shuffle(SceneContext& ctx) {
  for (Speck& s : specks_) {
    s.x = static_cast<int16_t>(ctx.rng->below(kFaceWidth));
    s.y = static_cast<int16_t>(ctx.rng->below(kFaceHeight));
    s.r = static_cast<uint8_t>(1 + ctx.rng->below(3));
    s.phase = ctx.rng->byte();
  }
}

void SpecklesScene::onGesture(Gesture g, SceneContext& ctx) {
  if (g == Gesture::Single) shuffle(ctx);
}

void SpecklesScene::render(Frame& f, SceneContext& ctx) {
  f.fillFace(kFront, background);
  for (const Speck& s : specks_) {
    // Gentle twinkle so the speckles look alive
    const float ph = (ctx.nowMs / 1500.0f + s.phase / 255.0f) * 6.2831853f;
    const float k = 0.65f + 0.35f * sinf(ph);
    f.fillCircle(kFront, s.x, s.y, s.r, lerpf(background, speck, k));
  }
  f.copyFace(kFront, kBack);
}

// -------------------------------------------------------------- MotionScene

void MotionScene::render(Frame& f, SceneContext& ctx) {
  const float dev = ctx.input.motion;
  const float target = fminf(100.0f, 100.0f * 100.0f / (100.0f + dev * dev * 1000.0f));
  // v1 filter: 0.1 new + 0.9 old per ~30 ms frame, made frame-rate independent
  const float keep = powf(0.9f, ctx.dt * 33.0f);
  calm_ = target * (1.0f - keep) + calm_ * keep;
  f.fill(lerpf(colors::kRed, colors::kCyan, calm_ / 100.0f));
}

// -------------------------------------------------------------- StreamScene

bool StreamScene::notifyPacket(SceneManager& mgr, uint32_t nowMs) {
  lastPacketMs_ = nowMs;
  if (mgr.current() != this) {
    returnTo_ = mgr.currentIndex() < 0 ? 0 : mgr.currentIndex();
    const int self = mgr.find(name());
    if (self < 0) return false;
    mgr.setScene(self);
  }
  return true;
}

void StreamScene::render(Frame&, SceneContext& ctx) {
  // The frame is written by the network code; only watch the timeout here.
  if (ctx.nowMs - lastPacketMs_ > kTimeoutMs) ctx.mgr->setScene(returnTo_);
}

}  // namespace ind
