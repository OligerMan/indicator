#pragma once
// Logical picture of the lamp: two faces (front/back), each one physical 128x64 panel.
// Scenes draw here in face coordinates; hw/Display maps faces to the physical chain
// (mirroring, panel order) so scene code never cares how the panels are mounted.

#include <stddef.h>
#include <stdint.h>

#include "Color.h"

namespace ind {

constexpr int kFaceWidth = 128;
constexpr int kFaceHeight = 64;
constexpr int kFaceCount = 2;

enum Face : uint8_t { kFront = 0, kBack = 1 };

class Frame {
 public:
  void set(int face, int x, int y, Rgb c) {
    if (inBounds(face, x, y)) px_[face][y][x] = c;
  }
  Rgb get(int face, int x, int y) const {
    return inBounds(face, x, y) ? px_[face][y][x] : Rgb();
  }

  void fill(Rgb c);
  void fillFace(int face, Rgb c);
  // Filled rectangle, clipped to the face
  void fillRect(int face, int x, int y, int w, int h, Rgb c);
  // Filled circle, clipped to the face
  void fillCircle(int face, int cx, int cy, int r, Rgb c);
  // Copy one face to the other: both sides show the same picture
  void copyFace(int from, int to);

  static bool inBounds(int face, int x, int y) {
    return face >= 0 && face < kFaceCount && x >= 0 && x < kFaceWidth && y >= 0 &&
           y < kFaceHeight;
  }

  static constexpr size_t kBytes = sizeof(Rgb) * kFaceCount * kFaceWidth * kFaceHeight;

 private:
  Rgb px_[kFaceCount][kFaceHeight][kFaceWidth];
};

}  // namespace ind
