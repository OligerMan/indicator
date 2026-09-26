#include "Net.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FramePacket.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "WebPage.h"
#include "app/App.h"
#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

namespace {
WebServer server(80);
WiFiUDP udp;
uint8_t udpBuf[1500];
}  // namespace

void Net::begin(App& app) {
  app_ = &app;
  WiFi.setHostname(cfg::kHostname);

  bool connected = false;
#ifdef WIFI_SSID
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("[net] connecting to %s", WIFI_SSID);
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < cfg::kWifiConnectTimeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  connected = WiFi.status() == WL_CONNECTED;
#endif
  if (connected) {
    // Modem sleep adds 100+ ms latency to UDP streaming
    WiFi.setSleep(false);
    Serial.printf("[net] ip %s\n", WiFi.localIP().toString().c_str());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(cfg::kFallbackApSsid, cfg::kFallbackApPass);
    Serial.printf("[net] access point '%s', ip %s\n", cfg::kFallbackApSsid,
                  WiFi.softAPIP().toString().c_str());
  }

  MDNS.begin(cfg::kHostname);
  MDNS.addService("http", "tcp", 80);

  ArduinoOTA.setHostname(cfg::kHostname);
#ifdef OTA_PASS
  if (strlen(OTA_PASS) > 0) ArduinoOTA.setPassword(OTA_PASS);
#endif
  ArduinoOTA.begin();

  startWeb();
  udp.begin(ind::kFramePort);
}

void Net::startWeb() {
  server.on("/", HTTP_GET, [] { server.send(200, "text/html; charset=utf-8", kWebPage); });
  server.on("/api/cmd", HTTP_POST, [this] {
    const String body = server.arg("plain");
    server.send(200, "text/plain; charset=utf-8", app_->execute(body.c_str()));
  });
  server.on("/api/status", HTTP_GET, [this] {
    server.send(200, "text/plain; charset=utf-8", app_->execute("status"));
  });
  server.begin();
}

void Net::pollUdp() {
  // Drain everything that arrived since the last loop; rows land in the frame directly
  for (int n = 0; n < 32; n++) {
    const int len = udp.parsePacket();
    if (len <= 0) break;
    const int got = udp.read(udpBuf, sizeof(udpBuf));
    if (got > 0) app_->streamPacket(udpBuf, static_cast<size_t>(got));
  }
}

void Net::poll() {
  ArduinoOTA.handle();
  server.handleClient();
  pollUdp();
}
