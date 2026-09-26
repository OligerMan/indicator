#pragma once
// Wi-Fi (station, or own access point as fallback), mDNS, ArduinoOTA, HTTP control
// page + /api/cmd, UDP frame stream. Everything is polled from loop(), so command
// handlers run on the same thread as rendering and need no locking.

class App;

class Net {
 public:
  void begin(App& app);
  void poll();

 private:
  void startWeb();
  void pollUdp();

  App* app_ = nullptr;
};
