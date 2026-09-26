#pragma once
// UDP frame streaming: a PC (tools/stream_frames.py) or, later, the walker's brain
// sends picture rows; each datagram carries whole rows of one face.
// Layout (little endian), spec in docs/lamp/protocol.md:
//   0  'I'
//   1  'F'
//   2  version   = 1
//   3  format    0 = RGB565, 1 = RGB888
//   4  face      0 = front, 1 = back, 2 = both
//   5  y0        first row
//   6  rows      number of rows in this packet
//   7  flags     bit0 = last packet of the frame
//   8  pixels    rows * 128 * (2 or 3) bytes, row-major

#include <stddef.h>
#include <stdint.h>

#include "Frame.h"

namespace ind {

constexpr size_t kFramePacketHeader = 8;
constexpr uint16_t kFramePort = 7777;

enum class FramePacketResult : uint8_t { Ok, TooShort, BadMagic, BadVersion, BadFormat, BadGeometry };

struct FramePacketInfo {
  uint8_t face = 0, y0 = 0, rows = 0, flags = 0, format = 0;
  bool endOfFrame() const { return flags & 1; }
};

// Validates the datagram and, if valid, writes its rows into `frame`.
FramePacketResult applyFramePacket(const uint8_t* data, size_t len, Frame& frame,
                                   FramePacketInfo* info = nullptr);

const char* framePacketResultName(FramePacketResult r);

}  // namespace ind
