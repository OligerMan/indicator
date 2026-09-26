#pragma once
// Tilt angles from the gravity vector. Ported from angle_check() of the v1 lamp.
// Axes follow the accelerometer as mounted in the lamp (see docs/lamp/hardware.md).

namespace ind {

struct Tilt {
  float x = 0;          // degrees, -90..90, rotation around the Y axis
  float y = 0;          // degrees, -90..90, rotation around the X axis
  // Non-negative tilts along three directions 120 degrees apart
  // (used by the RGB colour-mixing scene: one direction per channel)
  float subX = 0;
  float subYMinus = 0;
  float subYPlus = 0;
};

// Folds (-180, 180) into (-90, 90) so that "upside down" reads as level, like v1.
float foldAngle(float deg);

// ax, ay, az in any consistent unit (only their ratios matter)
Tilt computeTilt(float ax, float ay, float az);

// Splits the -90..90 range of `angle` into `count` equal sectors and returns 0..count-1.
int tiltChoice(float angle, int count);

}  // namespace ind
