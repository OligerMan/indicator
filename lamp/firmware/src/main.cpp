// Индикатор v2 - lamp firmware entry point.
// Build variants (platformio.ini): `lamp` = this file, `panel_test` = bringup/PanelTest.cpp.

#ifndef BRINGUP_PANEL_TEST

#include <Arduino.h>

#include "app/App.h"
#include "config.h"
#include "net/Net.h"

namespace {
App app;
Net net;

// Line reader for a serial port: same text commands as HTTP
class LineReader {
 public:
  explicit LineReader(Stream& s) : s_(s) {}
  void poll(App& a) {
    while (s_.available()) {
      const char ch = static_cast<char>(s_.read());
      if (ch == '\r') continue;
      if (ch == '\n') {
        buf_[len_] = 0;
        if (len_ > 0) s_.print(a.execute(buf_));
        len_ = 0;
      } else if (len_ < sizeof(buf_) - 1) {
        buf_[len_++] = ch;
      }
    }
  }

 private:
  Stream& s_;
  char buf_[96];
  size_t len_ = 0;
};

LineReader usbCommands(Serial);
LineReader linkCommands(Serial2);
}  // namespace

void setup() {
  Serial.begin(115200);
  Serial2.begin(cfg::kLinkBaud, SERIAL_8N1, cfg::kLinkRx, cfg::kLinkTx);
  delay(200);
  Serial.println("\n[indicator] lamp firmware");

  app.begin();
  net.begin(app);
  Serial.println("[indicator] ready, type 'help'");
}

void loop() {
  usbCommands.poll(app);
  linkCommands.poll(app);
  net.poll();
  app.tick();
  delay(1);
}

#endif  // BRINGUP_PANEL_TEST
