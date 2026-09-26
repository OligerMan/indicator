#include "FramePacket.h"

namespace ind {

FramePacketResult applyFramePacket(const uint8_t* data, size_t len, Frame& frame,
                                   FramePacketInfo* info) {
  if (!data || len < kFramePacketHeader) return FramePacketResult::TooShort;
  if (data[0] != 'I' || data[1] != 'F') return FramePacketResult::BadMagic;
  if (data[2] != 1) return FramePacketResult::BadVersion;

  FramePacketInfo p;
  p.format = data[3];
  p.face = data[4];
  p.y0 = data[5];
  p.rows = data[6];
  p.flags = data[7];

  size_t bpp;
  if (p.format == 0) bpp = 2;
  else if (p.format == 1) bpp = 3;
  else return FramePacketResult::BadFormat;

  if (p.face > 2 || p.rows == 0 || p.y0 + p.rows > kFaceHeight) return FramePacketResult::BadGeometry;
  if (len < kFramePacketHeader + static_cast<size_t>(p.rows) * kFaceWidth * bpp)
    return FramePacketResult::TooShort;

  const uint8_t* px = data + kFramePacketHeader;
  for (int y = p.y0; y < p.y0 + p.rows; y++) {
    for (int x = 0; x < kFaceWidth; x++, px += bpp) {
      Rgb c;
      if (bpp == 2) {
        const uint16_t v = static_cast<uint16_t>(px[0] | (px[1] << 8));
        const uint8_t r5 = (v >> 11) & 0x1F, g6 = (v >> 5) & 0x3F, b5 = v & 0x1F;
        c = Rgb(static_cast<uint8_t>((r5 << 3) | (r5 >> 2)), static_cast<uint8_t>((g6 << 2) | (g6 >> 4)),
                static_cast<uint8_t>((b5 << 3) | (b5 >> 2)));
      } else {
        c = Rgb(px[0], px[1], px[2]);
      }
      if (p.face == 2) {
        frame.set(kFront, x, y, c);
        frame.set(kBack, x, y, c);
      } else {
        frame.set(p.face, x, y, c);
      }
    }
  }
  if (info) *info = p;
  return FramePacketResult::Ok;
}

const char* framePacketResultName(FramePacketResult r) {
  switch (r) {
    case FramePacketResult::Ok: return "ok";
    case FramePacketResult::TooShort: return "too short";
    case FramePacketResult::BadMagic: return "bad magic";
    case FramePacketResult::BadVersion: return "bad version";
    case FramePacketResult::BadFormat: return "bad format";
    case FramePacketResult::BadGeometry: return "bad geometry";
  }
  return "?";
}

}  // namespace ind
