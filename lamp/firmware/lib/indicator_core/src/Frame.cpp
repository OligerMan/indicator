#include "Frame.h"

#include <string.h>

namespace ind {

void Frame::fill(Rgb c) {
  for (int f = 0; f < kFaceCount; f++) fillFace(f, c);
}

void Frame::fillFace(int face, Rgb c) {
  if (face < 0 || face >= kFaceCount) return;
  for (int y = 0; y < kFaceHeight; y++)
    for (int x = 0; x < kFaceWidth; x++) px_[face][y][x] = c;
}

void Frame::fillRect(int face, int x, int y, int w, int h, Rgb c) {
  for (int yy = y; yy < y + h; yy++)
    for (int xx = x; xx < x + w; xx++) set(face, xx, yy, c);
}

void Frame::fillCircle(int face, int cx, int cy, int r, Rgb c) {
  const int r2 = r * r;
  for (int dy = -r; dy <= r; dy++)
    for (int dx = -r; dx <= r; dx++)
      if (dx * dx + dy * dy <= r2) set(face, cx + dx, cy + dy, c);
}

void Frame::copyFace(int from, int to) {
  if (from == to || from < 0 || to < 0 || from >= kFaceCount || to >= kFaceCount) return;
  memcpy(px_[to], px_[from], sizeof(px_[0]));
}

}  // namespace ind
