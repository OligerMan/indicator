#include "Scene.h"

#include <string.h>

namespace ind {

bool SceneManager::add(Scene* s) {
  if (!s || count_ >= kMaxScenes) return false;
  scenes_[count_++] = s;
  return true;
}

int SceneManager::find(const char* name) const {
  for (int i = 0; i < count_; i++)
    if (!strcmp(scenes_[i]->name(), name)) return i;
  return -1;
}

int SceneManager::selectableCount() const {
  int n = 0;
  for (int i = 0; i < count_; i++)
    if (scenes_[i]->selectable()) n++;
  return n;
}

int SceneManager::selectableIndex(int n) const {
  for (int i = 0; i < count_; i++)
    if (scenes_[i]->selectable() && n-- == 0) return i;
  return -1;
}

SceneContext SceneManager::makeContext(uint32_t nowMs) {
  SceneContext ctx;
  ctx.nowMs = nowMs;
  ctx.input = lastInput_;
  ctx.rng = &rng_;
  ctx.mgr = this;
  return ctx;
}

bool SceneManager::setScene(int index) {
  if (index < 0 || index >= count_) return false;
  SceneContext ctx = makeContext(lastNowMs_);
  if (Scene* old = current()) old->exit(ctx);
  current_ = index;
  scenes_[current_]->enter(ctx);
  return true;
}

void SceneManager::tap(uint32_t nowMs) {
  lastNowMs_ = nowMs;
  SceneContext ctx = makeContext(nowMs);
  if (Scene* s = current()) s->onTap(ctx);
}

void SceneManager::gesture(Gesture g, uint32_t nowMs) {
  lastNowMs_ = nowMs;
  if (g == Gesture::None) return;
  // Global "back to default" from v1
  if (g == Gesture::Triple && current_ != 0) {
    goMenu();
    return;
  }
  SceneContext ctx = makeContext(nowMs);
  if (Scene* s = current()) s->onGesture(g, ctx);
}

void SceneManager::render(Frame& f, const InputState& in, uint32_t nowMs) {
  lastInput_ = in;
  lastNowMs_ = nowMs;
  if (current_ < 0 && count_ > 0) setScene(0);
  Scene* s = current();
  if (!s) return;
  SceneContext ctx = makeContext(nowMs);
  ctx.dt = rendered_ ? (nowMs - lastRenderMs_) / 1000.0f : 0.0f;
  rendered_ = true;
  lastRenderMs_ = nowMs;
  s->render(f, ctx);
}

}  // namespace ind
