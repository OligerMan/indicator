// Unit tests for the portable core (lib/indicator_core). Run: pio test -e native

#include <Color.h>
#include <Command.h>
#include <FramePacket.h>
#include <Power.h>
#include <Scenes.h>
#include <Taps.h>
#include <Tilt.h>
#include <unity.h>

#include <memory>
#include <string.h>
#include <vector>

using namespace ind;

void setUp() {}
void tearDown() {}

// ------------------------------------------------------------------ taps

// Feeds `ms` milliseconds of samples at 500 Hz; spikeAt >= 0 adds a 6 ms hit there.
static int feed(TapDetector& d, uint32_t& t, uint32_t ms, int spikeAt = -1, float spike = 12.0f) {
  int taps = 0;
  for (uint32_t i = 0; i < ms; i += 2, t += 2) {
    float z = 9.81f;
    if (spikeAt >= 0 && i >= static_cast<uint32_t>(spikeAt) && i < static_cast<uint32_t>(spikeAt) + 6) z += spike;
    if (d.update(0.0f, 0.0f, z, t)) taps++;
  }
  return taps;
}

void test_detector_quiet_no_taps() {
  TapDetector d;
  uint32_t t = 0;
  TEST_ASSERT_EQUAL(0, feed(d, t, 2000));
}

void test_detector_single_spike_is_one_tap() {
  TapDetector d;
  uint32_t t = 0;
  TEST_ASSERT_EQUAL(1, feed(d, t, 500, 200));
}

void test_detector_ringing_counts_once() {
  TapDetector d;
  uint32_t t = 0;
  int taps = 0;
  // Spike, settles, spikes again 50 ms after the first onset (within refractory): one tap
  taps += feed(d, t, 100, 50);
  taps += feed(d, t, 30, 0);
  TEST_ASSERT_EQUAL(1, taps);
}

void test_detector_small_bump_ignored() {
  TapDetector d;
  uint32_t t = 0;
  TEST_ASSERT_EQUAL(0, feed(d, t, 500, 200, 4.0f));
}

void test_detector_adapts_to_offset_gravity() {
  // Sensor reads 11 instead of 9.8: v1 would sit at deviation 1.2 forever; baseline must follow
  TapDetector d;
  uint32_t t = 0;
  for (int i = 0; i < 1000; i++, t += 2) TEST_ASSERT_FALSE(d.update(0, 0, 11.0f, t));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 11.0f, d.baseline());
}

void test_sequencer_gestures() {
  TapSequencer s;
  s.onTap(1000);
  TEST_ASSERT_EQUAL(Gesture::None, s.poll(1500));
  TEST_ASSERT_EQUAL(Gesture::Single, s.poll(1601));

  s.onTap(2000);
  s.onTap(2300);
  TEST_ASSERT_EQUAL(Gesture::None, s.poll(2800));
  TEST_ASSERT_EQUAL(Gesture::Double, s.poll(2901));

  s.onTap(4000);
  s.onTap(4300);
  s.onTap(4600);
  TEST_ASSERT_EQUAL(Gesture::Triple, s.poll(5300));

  for (int i = 0; i < 5; i++) s.onTap(6000 + i * 200);
  TEST_ASSERT_EQUAL(Gesture::Many, s.poll(8000));
}

void test_sequencer_bounce_ignored() {
  TapSequencer s;
  TEST_ASSERT_EQUAL(1, s.onTap(1000));
  TEST_ASSERT_EQUAL(0, s.onTap(1050));  // closer than minGap
  TEST_ASSERT_EQUAL(Gesture::Single, s.poll(2000));
}

// ------------------------------------------------------------------ tilt

void test_tilt_level_and_tilted() {
  Tilt t = computeTilt(0, 0, 9.8f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, t.x);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, t.y);

  t = computeTilt(0, 9.8f, 9.8f);  // 45 degrees around X
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 45.0f, t.y);

  t = computeTilt(0, 0, -9.8f);  // upside down folds to level, like v1
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, t.x);
}

void test_tilt_choice_clamped() {
  TEST_ASSERT_EQUAL(0, tiltChoice(-90.0f, 5));
  TEST_ASSERT_EQUAL(2, tiltChoice(0.0f, 5));
  TEST_ASSERT_EQUAL(4, tiltChoice(90.0f, 5));  // v1 would return 5 here
  TEST_ASSERT_EQUAL(4, tiltChoice(200.0f, 5));
}

// ------------------------------------------------------------------ power

void test_power_black_frame_keeps_brightness() {
  auto f = std::make_unique<Frame>();
  f->fill(colors::kBlack);
  PowerModel m;
  TEST_ASSERT_EQUAL(0u, frameLoad(*f));
  TEST_ASSERT_EQUAL(200, limitBrightness(m, frameLoad(*f), 200, 1000));
}

void test_power_white_frame_is_limited() {
  auto f = std::make_unique<Frame>();
  f->fill(colors::kWhite);
  PowerModel m;
  m.idleMa = 500;
  m.maPerChannelFull = 0.25f;
  const uint32_t load = frameLoad(*f);
  // 2 faces * 8192 px * 3 ch * 0.25 mA = 12288 mA at full brightness
  TEST_ASSERT_FLOAT_WITHIN(1.0f, 500 + 12288, estimateCurrentMa(m, load, 255));
  const uint8_t b = limitBrightness(m, load, 255, 6000);
  TEST_ASSERT_TRUE(estimateCurrentMa(m, load, b) <= 6000.0f);
  TEST_ASSERT_TRUE(estimateCurrentMa(m, load, b + 1) > 6000.0f);
}

void test_power_budget_below_idle_gives_zero() {
  auto f = std::make_unique<Frame>();
  f->fill(colors::kWhite);
  PowerModel m;
  TEST_ASSERT_EQUAL(0, limitBrightness(m, frameLoad(*f), 255, 100));
}

// ------------------------------------------------------------------ commands

void test_command_parse() {
  Command c = parseCommand("  Bright 120 ");
  TEST_ASSERT_EQUAL(CmdType::Bright, c.type);
  TEST_ASSERT_EQUAL(120, c.args[0]);

  c = parseCommand("color 109 0 204");
  TEST_ASSERT_EQUAL(CmdType::Color, c.type);
  TEST_ASSERT_EQUAL(204, c.args[2]);

  c = parseCommand("scene speckles");
  TEST_ASSERT_EQUAL(CmdType::Scene, c.type);
  TEST_ASSERT_EQUAL(-1, c.args[0]);
  TEST_ASSERT_EQUAL_STRING("speckles", c.text);

  c = parseCommand("scene 3");
  TEST_ASSERT_EQUAL(3, c.args[0]);

  c = parseCommand("gesture double");
  TEST_ASSERT_EQUAL(2, c.args[0]);
}

void test_command_errors() {
  TEST_ASSERT_EQUAL(CmdType::Invalid, parseCommand("").type);
  TEST_ASSERT_EQUAL(CmdType::Invalid, parseCommand("bright 300").type);
  TEST_ASSERT_EQUAL(CmdType::Invalid, parseCommand("bright x").type);
  TEST_ASSERT_EQUAL(CmdType::Invalid, parseCommand("color 1 2").type);
  TEST_ASSERT_EQUAL(CmdType::Invalid, parseCommand("fly").type);
  TEST_ASSERT_NOT_NULL(parseCommand("fly").error);
}

// ------------------------------------------------------------------ frame packets

void test_frame_packet_rgb888_both_faces() {
  auto f = std::make_unique<Frame>();
  f->fill(colors::kBlack);
  std::vector<uint8_t> p(kFramePacketHeader + 2 * kFaceWidth * 3, 0);
  const uint8_t hdr[] = {'I', 'F', 1, 1, 2, 10, 2, 1};
  memcpy(p.data(), hdr, sizeof(hdr));
  p[kFramePacketHeader + 0] = 10;  // (0,10) = rgb(10,20,30)
  p[kFramePacketHeader + 1] = 20;
  p[kFramePacketHeader + 2] = 30;
  FramePacketInfo info;
  TEST_ASSERT_EQUAL(FramePacketResult::Ok, applyFramePacket(p.data(), p.size(), *f, &info));
  TEST_ASSERT_TRUE(info.endOfFrame());
  TEST_ASSERT_TRUE(f->get(kFront, 0, 10) == Rgb(10, 20, 30));
  TEST_ASSERT_TRUE(f->get(kBack, 0, 10) == Rgb(10, 20, 30));
  TEST_ASSERT_TRUE(f->get(kFront, 0, 9) == colors::kBlack);
}

void test_frame_packet_rgb565_white() {
  auto f = std::make_unique<Frame>();
  std::vector<uint8_t> p(kFramePacketHeader + kFaceWidth * 2, 0xFF);
  const uint8_t hdr[] = {'I', 'F', 1, 0, 0, 63, 1, 0};
  memcpy(p.data(), hdr, sizeof(hdr));
  TEST_ASSERT_EQUAL(FramePacketResult::Ok, applyFramePacket(p.data(), p.size(), *f));
  TEST_ASSERT_TRUE(f->get(kFront, 127, 63) == colors::kWhite);
}

void test_frame_packet_rejects_bad_input() {
  auto f = std::make_unique<Frame>();
  uint8_t p[kFramePacketHeader + kFaceWidth * 3] = {'I', 'F', 1, 1, 0, 63, 2, 0};  // rows past 64
  TEST_ASSERT_EQUAL(FramePacketResult::BadGeometry, applyFramePacket(p, sizeof(p), *f));
  p[6] = 1;
  TEST_ASSERT_EQUAL(FramePacketResult::TooShort, applyFramePacket(p, 20, *f));
  p[0] = 'X';
  TEST_ASSERT_EQUAL(FramePacketResult::BadMagic, applyFramePacket(p, sizeof(p), *f));
}

// ------------------------------------------------------------------ scenes

struct Lamp {
  SceneManager mgr;
  MenuScene menu;
  SmoothScene smooth;
  SolidScene solid;
  TiltColorScene tilt;
  SpecklesScene speckles;
  MotionScene motion;
  StreamScene stream;
  std::unique_ptr<Frame> frame = std::make_unique<Frame>();

  Lamp() {
    mgr.add(&menu);
    mgr.add(&smooth);
    mgr.add(&solid);
    mgr.add(&tilt);
    mgr.add(&speckles);
    mgr.add(&motion);
    mgr.add(&stream);
    mgr.goMenu();
  }
  void render(uint32_t now, float tiltY = 0) {
    InputState in;
    in.tilt.y = tiltY;
    mgr.render(*frame, in, now);
  }
};

void test_menu_select_by_tilt_and_tap() {
  Lamp l;
  TEST_ASSERT_EQUAL(5, l.mgr.selectableCount());
  l.mgr.gesture(Gesture::Double, 0);
  TEST_ASSERT_TRUE(l.menu.selecting());
  l.render(10, 20.0f);  // sector 3 of 5 -> speckles
  TEST_ASSERT_EQUAL(3, l.menu.highlighted());
  TEST_ASSERT_TRUE(l.frame->get(kFront, 0, 0) == colors::kViolet);
  l.mgr.gesture(Gesture::Single, 20);
  TEST_ASSERT_EQUAL_STRING("speckles", l.mgr.current()->name());
}

void test_triple_tap_returns_to_menu() {
  Lamp l;
  l.mgr.setScene(l.mgr.find("motion"));
  l.mgr.gesture(Gesture::Triple, 0);
  TEST_ASSERT_EQUAL(0, l.mgr.currentIndex());
  TEST_ASSERT_FALSE(l.menu.selecting());
}

void test_solid_color_command_path() {
  Lamp l;
  l.solid.setColor(Rgb(1, 2, 3));
  l.mgr.setScene(l.mgr.find("solid"));
  l.render(0);
  TEST_ASSERT_TRUE(l.frame->get(kBack, 64, 32) == Rgb(1, 2, 3));
}

void test_stream_times_out_to_previous_scene() {
  Lamp l;
  l.mgr.setScene(l.mgr.find("smooth"));
  l.render(0);
  TEST_ASSERT_TRUE(l.stream.notifyPacket(l.mgr, 100));
  TEST_ASSERT_EQUAL_STRING("stream", l.mgr.current()->name());
  l.render(1000);
  TEST_ASSERT_EQUAL_STRING("stream", l.mgr.current()->name());
  l.render(100 + StreamScene::kTimeoutMs + 1);
  TEST_ASSERT_EQUAL_STRING("smooth", l.mgr.current()->name());
}

void test_hsv_primaries() {
  TEST_ASSERT_TRUE(hsv(0, 255, 255) == Rgb(255, 0, 0));
  const Rgb g = hsv(86, 255, 255);
  TEST_ASSERT_TRUE(g.g > 240 && g.r < 20);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_detector_quiet_no_taps);
  RUN_TEST(test_detector_single_spike_is_one_tap);
  RUN_TEST(test_detector_ringing_counts_once);
  RUN_TEST(test_detector_small_bump_ignored);
  RUN_TEST(test_detector_adapts_to_offset_gravity);
  RUN_TEST(test_sequencer_gestures);
  RUN_TEST(test_sequencer_bounce_ignored);
  RUN_TEST(test_tilt_level_and_tilted);
  RUN_TEST(test_tilt_choice_clamped);
  RUN_TEST(test_power_black_frame_keeps_brightness);
  RUN_TEST(test_power_white_frame_is_limited);
  RUN_TEST(test_power_budget_below_idle_gives_zero);
  RUN_TEST(test_command_parse);
  RUN_TEST(test_command_errors);
  RUN_TEST(test_frame_packet_rgb888_both_faces);
  RUN_TEST(test_frame_packet_rgb565_white);
  RUN_TEST(test_frame_packet_rejects_bad_input);
  RUN_TEST(test_menu_select_by_tilt_and_tap);
  RUN_TEST(test_triple_tap_returns_to_menu);
  RUN_TEST(test_solid_color_command_path);
  RUN_TEST(test_stream_times_out_to_previous_scene);
  RUN_TEST(test_hsv_primaries);
  return UNITY_END();
}
