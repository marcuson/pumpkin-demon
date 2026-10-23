#include "lighting.h"
#include <assert.h>
#include <stdio.h>

using namespace lighting;

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
  puts("lighting tests passed");
}
