#pragma once
// Text command protocol shared by every control channel (USB serial, UART link,
// HTTP /api/cmd). One command per line, space-separated. Spec: docs/lamp/protocol.md

#include <stdint.h>

namespace ind {

enum class CmdType : uint8_t {
  Invalid,
  Help,      // help
  Status,    // status
  Scenes,    // scenes
  Scene,     // scene <name|index>
  Bright,    // bright <0..255>
  Limit,     // limit <mA>
  Color,     // color <r> <g> <b>
  Gesture,   // gesture <single|double|triple|many>
  Tap,       // tap            (simulated raw tap, goes through the sequencer)
};

struct Command {
  static constexpr int kMaxText = 24;

  CmdType type = CmdType::Invalid;
  int32_t args[3] = {0, 0, 0};
  char text[kMaxText] = {0};  // scene name / gesture name
  const char* error = nullptr;
};

// Never fails hard: on error returns type Invalid with `error` set.
Command parseCommand(const char* line);

const char* commandHelp();

}  // namespace ind
