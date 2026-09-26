#include "App.h"

#include <FramePacket.h>
#include <esp_heap_caps.h>

#include "config.h"

using namespace ind;

bool App::begin() {
  // DMA buffers first: they need contiguous internal RAM, the frame can go anywhere.
  const bool displayOk = display_.begin();
  Serial.printf("[display] %s, refresh %d Hz, free DMA heap %u\n", displayOk ? "ok" : "FAILED",
                display_.refreshRate(), heap_caps_get_free_size(MALLOC_CAP_DMA));

  frame_ = new (std::nothrow) Frame();
  if (!frame_) {
    Serial.println("[app] cannot allocate frame");
    return false;
  }
  frame_->fill(colors::kBlack);

  const bool imuOk = imu_.begin();
  Serial.printf("[imu] %s\n", imuOk ? "ok" : "not found, use 'tap'/'gesture' commands");

  // Menu order = v1 tilt order: white, red, rainbow, violet, cyan
  mgr_.add(&menu_);
  mgr_.add(&smooth_);
  mgr_.add(&solid_);
  mgr_.add(&tiltColor_);
  mgr_.add(&speckles_);
  mgr_.add(&motion_);
  mgr_.add(&stream_);
  mgr_.rng().reseed(esp_random());
  mgr_.goMenu();
  return displayOk;
}

void App::injectTap(uint32_t now) {
  if (sequencer_.onTap(now)) mgr_.tap(now);
}

void App::tick() {
  const uint32_t now = millis();

  uint32_t ts;
  while (imu_.popTap(ts)) injectTap(ts);
  const Gesture g = sequencer_.poll(now);
  if (g != Gesture::None) {
    Serial.printf("[tap] %s\n", gestureName(g));
    mgr_.gesture(g, now);
  }

  if (now - lastFrameMs_ < cfg::kFramePeriodMs) return;
  lastFrameMs_ = now;

  mgr_.render(*frame_, imu_.snapshot(), now);
  display_.present(*frame_);

  fpsFrames_++;
  if (now - fpsWindowStart_ >= 1000) {
    fps_ = fpsFrames_ * 1000.0f / (now - fpsWindowStart_);
    fpsFrames_ = 0;
    fpsWindowStart_ = now;
  }
}

void App::streamPacket(const uint8_t* data, size_t len) {
  if (!frame_) return;
  if (!stream_.notifyPacket(mgr_, millis())) return;
  const FramePacketResult r = applyFramePacket(data, len, *frame_);
  if (r == FramePacketResult::Ok) streamPackets_++;
  else streamErrors_++;
}

String App::listScenes() {
  String s;
  for (int i = 0; i < mgr_.count(); i++) {
    s += String(i) + " " + mgr_.at(i)->name();
    if (i == mgr_.currentIndex()) s += " *";
    s += "\n";
  }
  return s;
}

String App::status() {
  String s;
  s += "scene " + String(mgr_.current() ? mgr_.current()->name() : "-") + "\n";
  s += "bright " + String(display_.brightness()) + " applied " + String(display_.appliedBrightness()) + "\n";
  s += "limit_ma " + String(display_.currentLimit(), 0) + " estimate_ma " +
       String(display_.estimatedCurrentMa(), 0) + "\n";
  s += "fps " + String(fps_, 1) + " refresh_hz " + String(display_.refreshRate()) + "\n";
  s += "heap " + String(ESP.getFreeHeap()) + " dma_heap " +
       String(heap_caps_get_free_size(MALLOC_CAP_DMA)) + "\n";
  s += "stream_packets " + String(streamPackets_) + " errors " + String(streamErrors_) + "\n";
  return s;
}

String App::execute(const char* line) {
  const Command c = parseCommand(line);
  const uint32_t now = millis();
  switch (c.type) {
    case CmdType::Invalid: return String("error: ") + (c.error ? c.error : "?") + "\n";
    case CmdType::Help: return commandHelp();
    case CmdType::Status: return status();
    case CmdType::Scenes: return listScenes();
    case CmdType::Scene: {
      const int idx = c.args[0] >= 0 ? c.args[0] : mgr_.find(c.text);
      if (!mgr_.setScene(idx)) return "error: no such scene\n";
      return String("ok ") + mgr_.current()->name() + "\n";
    }
    case CmdType::Bright:
      display_.setBrightness(static_cast<uint8_t>(c.args[0]));
      return "ok\n";
    case CmdType::Limit:
      display_.setCurrentLimit(static_cast<float>(c.args[0]));
      return "ok\n";
    case CmdType::Color:
      solid_.setColor(Rgb(c.args[0], c.args[1], c.args[2]));
      mgr_.setScene(mgr_.find(solid_.name()));
      return "ok\n";
    case CmdType::Gesture:
      mgr_.gesture(static_cast<Gesture>(c.args[0]), now);
      return "ok\n";
    case CmdType::Tap:
      injectTap(now);
      return "ok\n";
  }
  return "error\n";
}
