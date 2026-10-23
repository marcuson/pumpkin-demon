#include "lighting.h"
#include <assert.h>
#include <stdio.h>
#include <initializer_list>

using namespace lighting;

static void testEffects() {
  Controller controller;
  EffectSettings breathing;
  breathing.effect = Effect::Breathing;
  breathing.minimum = 26;
  breathing.maximum = 153;
  breathing.riseMs = breathing.fallMs = 2000;
  const uint32_t times[] = {0, 1000, 2000, 3000, 4000};
  const uint8_t expected[] = {26, 89, 153, 90, 26};
  const uint32_t starts[] = {0, UINT32_MAX - 500};
  for (uint32_t startTime : starts) {
    assert(setEffect(controller.channels, Group::Embers, breathing, startTime));
    for (uint8_t i = 0; i < 5; ++i)
      assert(evaluate(controller.channels[4], startTime + times[i]) == expected[i]);
  }

  EffectSettings flashes;
  flashes.effect = Effect::Flashes;
  flashes.maximum = 255;
  flashes.onMs = 100;
  flashes.offMs = 400;
  for (uint32_t startTime : starts) {
    assert(setEffect(controller.channels, Group::WhiteFlashes, flashes, startTime));
    assert(evaluate(controller.channels[5], startTime) == 255);
    assert(evaluate(controller.channels[5], startTime + 99) == 255);
    assert(evaluate(controller.channels[5], startTime + 100) == 0);
    assert(evaluate(controller.channels[5], startTime + 499) == 0);
    assert(evaluate(controller.channels[5], startTime + 500) == 255);
    flashes.initialDelayMs = 1500;
    flashes.offMs = 200;
    assert(setEffect(controller.channels, Group::WhiteFlashes, flashes, startTime));
    assert(evaluate(controller.channels[5], startTime + 1499) == 0);
    assert(evaluate(controller.channels[5], startTime + 1500) == 255);
    assert(evaluate(controller.channels[5], startTime + 1599) == 255);
    assert(evaluate(controller.channels[5], startTime + 1600) == 0);
    assert(evaluate(controller.channels[5], startTime + 1800) == 255);
    flashes.initialDelayMs = 0;
    flashes.offMs = 400;
  }

  for (uint8_t i = 0; i < 5; ++i) {
    breathing.minimum = i;
    breathing.maximum = 100 + i;
    assert(setEffect(controller.channels, static_cast<Group>(i), breathing, i * 100));
  }
  assert(setEffect(controller.channels, Group::WhiteFlashes, flashes, 0));
  uint8_t levels[GroupCount];
  render(controller, 1000, levels);
  for (uint8_t i = 0; i < 5; ++i) assert(levels[i] == i + (1000 - i * 100) / 20);
  assert(levels[5] == 255);
  EffectSettings steady;
  steady.maximum = 42;
  assert(setEffect(controller.channels, Group::LeftEye, steady, 1000));
  uint8_t after[GroupCount];
  render(controller, 1000, after);
  assert(after[0] == 42);
  for (uint8_t i = 1; i < GroupCount; ++i) assert(after[i] == levels[i]);
  assert(setEffect(controller.channels, Group::LeftMouth, flashes, 1000));
  render(controller, 1100, after);
  assert(after[0] == 42 && after[2] == 0);
  assert(after[1] == 51 && after[3] == 43 && after[4] == 39 && after[5] == 0);
  for (uint8_t i = 0; i < GroupCount; ++i)
    assert(setEffect(controller.channels, static_cast<Group>(i), flashes, 0));
  render(controller, 100, after);
  for (uint8_t level : after) assert(level == 0);
}

struct Draw {
  uint32_t minimum;
  uint32_t maximum;
  uint32_t value;
};

static Draw draws[16];
static uint8_t drawCount;
static uint8_t drawIndex;

static void script(std::initializer_list<Draw> values) {
  assert(values.size() <= 16);
  drawCount = 0;
  drawIndex = 0;
  for (const Draw &value : values) draws[drawCount++] = value;
}

static uint32_t scriptedRandom(uint32_t minimum, uint32_t maximum) {
  assert(drawIndex < drawCount);
  const Draw &draw = draws[drawIndex++];
  assert(draw.minimum == minimum && draw.maximum == maximum);
  return draw.value;
}

static void assertDrawsUsed() { assert(drawIndex == drawCount); }

static void assertUnchanged(const Controller &actual, const Controller &expected) {
  assert(actual.mode == expected.mode && actual.startedAt == expected.startedAt &&
         actual.durationMs == expected.durationMs);
  for (uint8_t i = 0; i < GroupCount; ++i) {
    const Channel &a = actual.channels[i];
    const Channel &b = expected.channels[i];
    assert(a.startedAt == b.startedAt);
    assert(a.settings.effect == b.settings.effect);
    assert(a.settings.minimum == b.settings.minimum && a.settings.maximum == b.settings.maximum);
    assert(a.settings.riseMs == b.settings.riseMs && a.settings.fallMs == b.settings.fallMs);
    assert(a.settings.onMs == b.settings.onMs && a.settings.offMs == b.settings.offMs);
    assert(a.settings.initialDelayMs == b.settings.initialDelayMs);
  }
}

static void testScenes() {
  Controller controller;
  uint8_t levels[GroupCount];
  script({{180000, 600001, 180000}});
  assert(start(controller, 123, scriptedRandom));
  assertDrawsUsed();
  assert(controller.mode == Mode::Sleep && controller.startedAt == 123 &&
         controller.durationMs == 180000);
  render(controller, 123, levels);
  for (uint8_t i = 0; i < 4; ++i) assert(levels[i] == 51);
  assert(levels[4] == 26 && levels[5] == 0);
  render(controller, 2123, levels);
  assert(levels[4] == 153);

  script({{5000, 10001, 10000}});
  assert(enterMode(controller, Mode::Awake, 2123, scriptedRandom));
  assertDrawsUsed();
  assert(controller.durationMs == 10000);
  render(controller, 2123, levels);
  for (uint8_t level : levels) assert(level == 255);
  render(controller, 2223, levels);
  assert(levels[5] == 0);
  for (uint8_t i = 0; i < 5; ++i) assert(levels[i] == 255);

  script({{2000, 5001, 2000}, {0, 1501, 0}, {0, 1501, 100},
          {0, 1501, 1500}, {0, 1501, 1500}});
  const uint32_t sceneStart = UINT32_MAX - 10;
  assert(enterMode(controller, Mode::Chaos, sceneStart, scriptedRandom));
  assertDrawsUsed();
  const uint32_t expectedDelays[] = {0, 0, 100, 100, 1500, 1500};
  for (uint8_t i = 0; i < GroupCount; ++i) {
    assert(controller.channels[i].startedAt == sceneStart);
    assert(controller.channels[i].settings.initialDelayMs == expectedDelays[i]);
    assert(controller.channels[i].settings.effect == Effect::Flashes);
  }
  for (uint32_t t = 0; t < 2000; ++t) {
    render(controller, sceneStart + t, levels);
    assert(levels[0] == levels[1] && levels[2] == levels[3]);
    assert(levels[4] == levels[5]);
  }
  render(controller, sceneStart, levels);
  assert(levels[0] == 255 && levels[2] == 0 && levels[4] == 0);
  render(controller, sceneStart + 100, levels);
  assert(levels[0] == 0 && levels[2] == 255 && levels[4] == 0);
  render(controller, sceneStart + 1499, levels);
  assert(levels[4] == 0);
  render(controller, sceneStart + 1500, levels);
  assert(levels[4] == 255 && levels[5] == 255);
  render(controller, sceneStart + 1600, levels);
  assert(levels[4] == 0 && levels[5] == 0);
  assertDrawsUsed();

  script({{180000, 600001, 600000}});
  assert(enterMode(controller, Mode::Sleep, sceneStart + 1700, scriptedRandom));
  assertDrawsUsed();
  assert(controller.durationMs == 600000);
  render(controller, sceneStart + 1700, levels);
  assert(levels[0] == 51 && levels[2] == 51 && levels[4] == 26 && levels[5] == 0);

  const Controller before = controller;
  assert(!enterMode(controller, static_cast<Mode>(255), 99, scriptedRandom));
  assert(!enterMode(controller, Mode::Awake, 99, nullptr));
  assert(!start(controller, 99, nullptr));
  assertUnchanged(controller, before);
  script({{5000, 10001, 10001}});
  assert(!enterMode(controller, Mode::Awake, 99, scriptedRandom));
  assertUnchanged(controller, before);
  script({{2000, 5001, 2000}, {0, 1501, 1501}});
  assert(!enterMode(controller, Mode::Chaos, 99, scriptedRandom));
  assertUnchanged(controller, before);
  assertDrawsUsed();
}

static void testScheduler() {
  uint8_t awakeCount = 0;
  uint8_t chaosCount = 0;
  for (uint32_t selection = 0; selection < 100; ++selection) {
    Controller controller;
    const uint32_t startTime = UINT32_MAX - 1000;
    script({{180000, 600001, 180000}});
    assert(start(controller, startTime, scriptedRandom));
    assertDrawsUsed();
    script({});
    assert(!update(controller, startTime + 179999, scriptedRandom));
    assertDrawsUsed();
    const uint32_t episodeStart = startTime + 180000;
    if (selection < 90) {
      script({{0, 100, selection}, {5000, 10001, 5000}});
      assert(update(controller, episodeStart, scriptedRandom));
      assert(controller.mode == Mode::Awake && controller.durationMs == 5000);
      ++awakeCount;
    } else {
      script({{0, 100, selection}, {2000, 5001, 2000}, {0, 1501, 1500},
              {0, 1501, 1500}, {0, 1501, 1500}, {0, 1501, 1500}});
      assert(update(controller, episodeStart, scriptedRandom));
      assert(controller.mode == Mode::Chaos && controller.durationMs == 2000);
      ++chaosCount;
    }
    assert(controller.startedAt == episodeStart);
    assertDrawsUsed();
    const uint32_t episodeDuration = controller.durationMs;
    script({});
    assert(!update(controller, episodeStart + episodeDuration - 1, scriptedRandom));
    assertDrawsUsed();
    script({{180000, 600001, 600000}});
    assert(update(controller, episodeStart + episodeDuration, scriptedRandom));
    assertDrawsUsed();
    assert(controller.mode == Mode::Sleep && controller.durationMs == 600000);
    uint8_t levels[GroupCount];
    render(controller, episodeStart + episodeDuration, levels);
    assert(levels[4] == 26 && levels[5] == 0);
  }
  assert(awakeCount == 90 && chaosCount == 10);

  Controller controller;
  script({{180000, 600001, 180000}});
  assert(start(controller, 10, scriptedRandom));
  for (uint8_t episode = 0; episode < 2; ++episode) {
    const uint32_t now = controller.startedAt + controller.durationMs;
    script({{0, 100, 99}, {2000, 5001, 5000}, {0, 1501, 0},
            {0, 1501, 0}, {0, 1501, 0}, {0, 1501, 0}});
    assert(update(controller, now, scriptedRandom));
    assert(controller.mode == Mode::Chaos && controller.durationMs == 5000);
    assertDrawsUsed();
    script({{180000, 600001, 180000}});
    assert(update(controller, now + 5000, scriptedRandom));
    assert(controller.mode == Mode::Sleep);
    assertDrawsUsed();
  }

  const uint32_t overdue = controller.startedAt + controller.durationMs + 999999;
  script({{0, 100, 0}, {5000, 10001, 10000}});
  assert(update(controller, overdue, scriptedRandom));
  assert(controller.startedAt == overdue && controller.mode == Mode::Awake);
  assertDrawsUsed();
  script({});
  assert(!update(controller, overdue, scriptedRandom));
  assertDrawsUsed();
  const Controller before = controller;
  assert(!update(controller, overdue + 10000, nullptr));
  assertUnchanged(controller, before);

  script({{180000, 600001, 180000}});
  assert(start(controller, 12345, scriptedRandom));
  assertDrawsUsed();
  assert(controller.mode == Mode::Sleep && controller.startedAt == 12345);
  const Controller sleep = controller;
  script({{0, 100, 100}});
  assert(!update(controller, 12345 + 180000, scriptedRandom));
  assertUnchanged(controller, sleep);
  assertDrawsUsed();
}

static void testLimitsAndDeadlines() {
  Controller controller;
  EffectSettings settings;
  settings.effect = Effect::Breathing;
  settings.maximum = 255;
  settings.riseMs = settings.fallMs = MaxEffectDurationMs;
  const uint32_t startTime = UINT32_MAX - 10;
  assert(setEffect(controller.channels, Group::LeftEye, settings, startTime));
  assert(evaluate(controller.channels[0], startTime + 599999) == 254);
  assert(evaluate(controller.channels[0], startTime + 600000) == 255);
  assert(evaluate(controller.channels[0], startTime + 1199999) == 1);
  assert(evaluate(controller.channels[0], startTime + 1200000) == 0);
  settings.riseMs = MaxEffectDurationMs + 1;
  assert(!setEffect(controller.channels, Group::LeftEye, settings, 99));
  assert(controller.channels[0].startedAt == startTime);
  settings.riseMs = 1;
  settings.fallMs = UINT32_MAX;
  assert(!setEffect(controller.channels, Group::LeftEye, settings, 99));

  settings.effect = Effect::Flashes;
  settings.onMs = settings.offMs = settings.initialDelayMs = MaxEffectDurationMs;
  assert(setEffect(controller.channels, Group::WhiteFlashes, settings, startTime));
  assert(evaluate(controller.channels[5], startTime + 599999) == 0);
  assert(evaluate(controller.channels[5], startTime + 600000) == 255);
  assert(evaluate(controller.channels[5], startTime + 1199999) == 255);
  assert(evaluate(controller.channels[5], startTime + 1200000) == 0);
  assert(evaluate(controller.channels[5], startTime + 1800000) == 255);
  settings.initialDelayMs = MaxEffectDurationMs + 1;
  assert(!setEffect(controller.channels, Group::WhiteFlashes, settings, 99));
  settings.initialDelayMs = 0;
  settings.onMs = UINT32_MAX;
  settings.offMs = 1;
  assert(!setEffect(controller.channels, Group::WhiteFlashes, settings, 99));
  settings.onMs = 1;
  settings.offMs = 0;
  assert(!setEffect(controller.channels, Group::WhiteFlashes, settings, 99));

  script({{2000, 5001, 2000}, {0, 1501, 1400}, {0, 1501, 1400},
          {0, 1501, 1400}, {0, 1501, 1400}});
  assert(enterMode(controller, Mode::Chaos, startTime, scriptedRandom));
  assertDrawsUsed();
  uint8_t levels[GroupCount];
  render(controller, startTime + 2000, levels);
  for (uint8_t level : levels) assert(level == 255);
  script({{180000, 600001, 180000}});
  assert(update(controller, startTime + 2000, scriptedRandom));
  render(controller, startTime + 2000, levels);
  assert(levels[0] == 51 && levels[4] == 26 && levels[5] == 0);
  assertDrawsUsed();
}

int main() {
  const uint8_t expectedPins[] = {3, 5, 6, 9, 10, 11};
  for (uint8_t i = 0; i < GroupCount; ++i) assert(Pins[i] == expectedPins[i]);
  assert(brightnessPercent(0) == 0);
  assert(brightnessPercent(10) == 26);
  assert(brightnessPercent(20) == 51);
  assert(brightnessPercent(60) == 153);
  assert(brightnessPercent(100) == 255);
  assert(brightnessPercent(255) == 255);

  Channel channels[GroupCount];
  EffectSettings settings;
  settings.maximum = 255;
  assert(setEffect(channels, Group::WhiteFlashes, settings, 42));
  settings.maximum = 128;
  assert(!setEffect(channels, Group::WhiteFlashes, settings, 99));
  assert(channels[5].settings.maximum == 255 && channels[5].startedAt == 42);
  assert(!setEffect(channels, static_cast<Group>(6), settings, 99));
  settings.effect = Effect::Breathing;
  settings.riseMs = settings.fallMs = 2000;
  assert(!setEffect(channels, Group::WhiteFlashes, settings, 99));
  assert(setEffect(channels, Group::LeftEye, settings, 100));
  settings.riseMs = 0;
  assert(!setEffect(channels, Group::LeftEye, settings, 200));
  settings.riseMs = 2000;
  settings.minimum = 200;
  assert(!setEffect(channels, Group::LeftEye, settings, 200));
  settings.minimum = 0;
  settings.riseMs = UINT32_MAX;
  assert(!setEffect(channels, Group::LeftEye, settings, 200));
  settings.effect = Effect::Flashes;
  settings.onMs = 100;
  settings.offMs = 200;
  assert(setEffect(channels, Group::LeftMouth, settings, 300));
  settings.offMs = UINT32_MAX;
  assert(!setEffect(channels, Group::LeftMouth, settings, 400));
  settings.effect = static_cast<Effect>(255);
  assert(!setEffect(channels, Group::LeftMouth, settings, 400));
  assert(channels[2].startedAt == 300);
  testEffects();
  testScenes();
  testScheduler();
  testLimitsAndDeadlines();
  puts("lighting tests passed");
}
