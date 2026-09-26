#pragma once
// Hardware configuration of the lamp. Everything that depends on how the device is
// wired or assembled lives here. Pin table and rationale: docs/lamp/hardware.md

#include <stdint.h>

namespace cfg {

// ---------------------------------------------------------------- HUB75 panels
// Two P2.5 320x160 mm panels, 128x64 px, 1/32 scan (HUB75E), chained with one ribbon:
// ESP32 -> panel 0 (front) -> panel 1 (back).
constexpr uint16_t kPanelWidth = 128;
constexpr uint16_t kPanelHeight = 64;
constexpr uint16_t kChainLength = 2;

// Library defaults for the original ESP32 (esp32-default-pins.hpp) + E on GPIO18.
// GPIO12 is a boot strapping pin (flash voltage): if the board fails to boot with the
// panel attached, move B2 elsewhere (see docs/lamp/hardware.md).
// WROVER modules use GPIO16/17 for PSRAM: then CLK and D must be moved.
constexpr int8_t kPinR1 = 25, kPinG1 = 26, kPinB1 = 27;
constexpr int8_t kPinR2 = 14, kPinG2 = 12, kPinB2 = 13;
constexpr int8_t kPinA = 23, kPinB = 19, kPinC = 5, kPinD = 17, kPinE = 18;
constexpr int8_t kPinLat = 4, kPinOe = 15, kPinClk = 16;

// Bits per colour channel in the DMA buffer. 256x64 chain costs 16 KB per bit:
// 6 -> 96 KB, 8 -> 128 KB (too tight next to Wi-Fi on the original ESP32).
constexpr uint8_t kColorDepthBits = 6;
// Panel driver chip: 0 = plain shift register (ICN2037 & co), 1 = FM6124, 2 = FM6126A,
// 3 = ICN2038S, 4 = MBI5124, 5 = DP3246 (values of HUB75_I2S_CFG::shift_driver)
constexpr uint8_t kShiftDriver = 0;
constexpr bool kClockPhase = true;   // flip if the picture is shifted by one pixel
constexpr uint8_t kMinRefreshHz = 60;

// Physical orientation of each face relative to the logical picture.
// Adjust after assembly so text reads correctly on both sides.
constexpr bool kFaceFlipX[2] = {false, true};
constexpr bool kFaceFlipY[2] = {false, false};
// Which chain position shows which face (0 = first panel after the ESP32)
constexpr uint8_t kFaceChainPos[2] = {0, 1};

// ---------------------------------------------------------------- power
constexpr uint8_t kDefaultBrightness = 96;      // 0..255, global (OE) brightness
constexpr float kDefaultCurrentLimitMa = 6000;  // both panels together, 5 V rail
constexpr float kIdleCurrentMa = 600;           // placeholder until measured
constexpr float kMaPerChannelFull = 0.25f;      // placeholder until measured

// ---------------------------------------------------------------- IMU (MPU6050)
constexpr int8_t kPinSda = 21, kPinScl = 22;
constexpr uint32_t kI2cHz = 400000;
constexpr uint32_t kImuPeriodMs = 2;  // 500 Hz sampling for tap detection
// Map sensor axes to lamp axes if the board is mounted rotated (sign or swap)
constexpr int8_t kImuAxisMap[3] = {0, 1, 2};    // lamp x,y,z <- sensor axis index
constexpr int8_t kImuAxisSign[3] = {1, 1, 1};

// ---------------------------------------------------------------- UART link
// Future connection to the walker's brain (Raspberry Pi). Same text protocol as USB.
constexpr int8_t kLinkRx = 32, kLinkTx = 33;
constexpr uint32_t kLinkBaud = 115200;

// ---------------------------------------------------------------- network
constexpr const char* kHostname = "indicator";
constexpr const char* kFallbackApSsid = "indicator-setup";
constexpr const char* kFallbackApPass = "indicator";  // >= 8 chars for WPA2
constexpr uint32_t kWifiConnectTimeoutMs = 15000;

// ---------------------------------------------------------------- render loop
constexpr uint32_t kFramePeriodMs = 33;  // ~30 fps

}  // namespace cfg
