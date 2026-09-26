#pragma once
// Glue between hardware adapters and the portable core: owns the frame, scenes and
// tap sequencer, runs the render loop and executes text commands from any channel.

#include <Arduino.h>
#include <Command.h>
#include <Frame.h>
#include <Scenes.h>
#include <Taps.h>

#include "hw/Display.h"
#include "hw/Imu.h"

class App {
 public:
  bool begin();
  // Call from loop(): input, gestures, rendering at cfg::kFramePeriodMs
  void tick();

  // Executes one command line and returns the reply text
  String execute(const char* line);
  // One UDP datagram of the frame stream
  void streamPacket(const uint8_t* data, size_t len);

  Display& display() { return display_; }
  float fps() const { return fps_; }

 private:
  String status();
  String listScenes();
  void injectTap(uint32_t now);

  Display display_;
  Imu imu_;
  ind::Frame* frame_ = nullptr;  // heap, allocated after the DMA buffers
  ind::SceneManager mgr_;
  ind::TapSequencer sequencer_;

  ind::MenuScene menu_;
  ind::SmoothScene smooth_;
  ind::SolidScene solid_;
  ind::TiltColorScene tiltColor_;
  ind::SpecklesScene speckles_;
  ind::MotionScene motion_;
  ind::StreamScene stream_;

  uint32_t lastFrameMs_ = 0;
  uint32_t fpsWindowStart_ = 0;
  uint32_t fpsFrames_ = 0;
  float fps_ = 0;
  uint32_t streamPackets_ = 0, streamErrors_ = 0;
};
