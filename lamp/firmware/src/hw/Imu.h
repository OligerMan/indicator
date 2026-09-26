#pragma once
// MPU6050 sampled by its own FreeRTOS task at cfg::kImuPeriodMs. The task runs the
// TapDetector (needs every sample) and publishes tap timestamps through a queue plus
// a low-passed gravity vector for tilt. v1 read the sensor once per ~30 ms loop with
// a 21 Hz filter, which smeared short hits; this is the main change.

#include <Scene.h>
#include <Taps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class Imu {
 public:
  // Returns false if the sensor is not found; the lamp keeps working without it
  // (gestures can still be injected with commands).
  bool begin();
  bool ok() const { return ok_; }

  // Next tap onset time (millis) if any
  bool popTap(uint32_t& tsMs);
  ind::InputState snapshot();

 private:
  static void taskEntry(void* self);
  void taskLoop();

  bool ok_ = false;
  QueueHandle_t taps_ = nullptr;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  float grav_[3] = {0, 0, 9.8f};
  float motion_ = 0;
  ind::TapDetector detector_;
};
