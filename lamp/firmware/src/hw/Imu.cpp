#include "Imu.h"

#include <Adafruit_MPU6050.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "config.h"

namespace {
Adafruit_MPU6050 mpu;
constexpr float kGravityAlpha = 0.05f;  // ~4 Hz low-pass at 500 Hz sampling
}  // namespace

bool Imu::begin() {
  Wire.begin(cfg::kPinSda, cfg::kPinScl, cfg::kI2cHz);
  if (!mpu.begin(MPU6050_I2CADDR_DEFAULT, &Wire)) return false;
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // Wide enough to keep the sharp edge of a tap
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);

  taps_ = xQueueCreate(16, sizeof(uint32_t));
  ok_ = true;
  xTaskCreatePinnedToCore(taskEntry, "imu", 4096, this, 5, nullptr, 0);
  return true;
}

void Imu::taskEntry(void* self) { static_cast<Imu*>(self)->taskLoop(); }

void Imu::taskLoop() {
  TickType_t wake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(cfg::kImuPeriodMs) ? pdMS_TO_TICKS(cfg::kImuPeriodMs) : 1;
  sensors_event_t a, g, t;
  for (;;) {
    vTaskDelayUntil(&wake, period);
    mpu.getEvent(&a, &g, &t);

    const float raw[3] = {a.acceleration.x, a.acceleration.y, a.acceleration.z};
    float v[3];
    for (int i = 0; i < 3; i++) v[i] = raw[cfg::kImuAxisMap[i]] * cfg::kImuAxisSign[i];

    const uint32_t now = millis();
    if (detector_.update(v[0], v[1], v[2], now)) xQueueSend(taps_, &now, 0);

    portENTER_CRITICAL(&mux_);
    for (int i = 0; i < 3; i++) grav_[i] += kGravityAlpha * (v[i] - grav_[i]);
    motion_ = fabsf(detector_.deviation());
    portEXIT_CRITICAL(&mux_);
  }
}

bool Imu::popTap(uint32_t& tsMs) {
  return ok_ && xQueueReceive(taps_, &tsMs, 0) == pdTRUE;
}

ind::InputState Imu::snapshot() {
  ind::InputState s;
  if (!ok_) return s;
  float g[3];
  portENTER_CRITICAL(&mux_);
  for (int i = 0; i < 3; i++) g[i] = grav_[i];
  s.motion = motion_;
  portEXIT_CRITICAL(&mux_);
  s.tilt = ind::computeTilt(g[0], g[1], g[2]);
  s.imuOk = true;
  return s;
}
