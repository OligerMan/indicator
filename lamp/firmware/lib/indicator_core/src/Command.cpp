#include "Command.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace ind {

namespace {

constexpr int kMaxTokens = 5;
constexpr int kMaxToken = Command::kMaxText;

// Splits `line` into lower-case tokens. Returns token count (capped at kMaxTokens).
int tokenize(const char* line, char out[kMaxTokens][kMaxToken]) {
  int n = 0;
  const char* p = line;
  while (*p && n < kMaxTokens) {
    while (*p && isspace(static_cast<unsigned char>(*p))) p++;
    if (!*p) break;
    int len = 0;
    while (*p && !isspace(static_cast<unsigned char>(*p))) {
      if (len < kMaxToken - 1) out[n][len++] = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
      p++;
    }
    out[n][len] = 0;
    n++;
  }
  return n;
}

bool parseInt(const char* s, int32_t lo, int32_t hi, int32_t& out) {
  char* end = nullptr;
  const long v = strtol(s, &end, 10);
  if (end == s || *end != 0 || v < lo || v > hi) return false;
  out = static_cast<int32_t>(v);
  return true;
}

Command fail(const char* msg) {
  Command c;
  c.error = msg;
  return c;
}

}  // namespace

Command parseCommand(const char* line) {
  char tok[kMaxTokens][kMaxToken];
  const int n = tokenize(line ? line : "", tok);
  if (n == 0) return fail("empty");

  Command c;
  const char* name = tok[0];

  if (!strcmp(name, "help") || !strcmp(name, "?")) {
    c.type = CmdType::Help;
  } else if (!strcmp(name, "status")) {
    c.type = CmdType::Status;
  } else if (!strcmp(name, "scenes")) {
    c.type = CmdType::Scenes;
  } else if (!strcmp(name, "scene")) {
    if (n < 2) return fail("usage: scene <name|index>");
    c.type = CmdType::Scene;
    strncpy(c.text, tok[1], sizeof(c.text) - 1);
    if (!parseInt(tok[1], 0, 255, c.args[0])) c.args[0] = -1;
  } else if (!strcmp(name, "bright")) {
    if (n < 2 || !parseInt(tok[1], 0, 255, c.args[0])) return fail("usage: bright <0..255>");
    c.type = CmdType::Bright;
  } else if (!strcmp(name, "limit")) {
    if (n < 2 || !parseInt(tok[1], 0, 40000, c.args[0])) return fail("usage: limit <mA 0..40000>");
    c.type = CmdType::Limit;
  } else if (!strcmp(name, "color")) {
    if (n < 4) return fail("usage: color <r> <g> <b>");
    for (int i = 0; i < 3; i++)
      if (!parseInt(tok[i + 1], 0, 255, c.args[i])) return fail("color components are 0..255");
    c.type = CmdType::Color;
  } else if (!strcmp(name, "gesture")) {
    if (n < 2) return fail("usage: gesture <single|double|triple|many>");
    const char* g = tok[1];
    if (!strcmp(g, "single") || !strcmp(g, "1")) c.args[0] = 1;
    else if (!strcmp(g, "double") || !strcmp(g, "2")) c.args[0] = 2;
    else if (!strcmp(g, "triple") || !strcmp(g, "3")) c.args[0] = 3;
    else if (!strcmp(g, "many") || !strcmp(g, "4")) c.args[0] = 4;
    else return fail("usage: gesture <single|double|triple|many>");
    c.type = CmdType::Gesture;
  } else if (!strcmp(name, "tap")) {
    c.type = CmdType::Tap;
  } else {
    return fail("unknown command, try 'help'");
  }
  return c;
}

const char* commandHelp() {
  return "help                      this text\n"
         "status                    scene, brightness, current estimate, fps, heap\n"
         "scenes                    list scenes\n"
         "scene <name|index>        switch scene\n"
         "bright <0..255>           requested global brightness\n"
         "limit <mA>                panel current budget\n"
         "color <r> <g> <b>         solid colour (switches to 'solid')\n"
         "gesture <single|double|triple|many>  inject a gesture\n"
         "tap                       inject one raw tap\n";
}

}  // namespace ind
