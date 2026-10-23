#ifndef PUMPKIN_LIGHTING_H
#define PUMPKIN_LIGHTING_H

#include <stdint.h>

namespace lighting {

constexpr uint8_t GroupCount = 6;
constexpr uint8_t Pins[GroupCount] = {3, 5, 6, 9, 10, 11};
constexpr uint32_t MaxEffectDurationMs = 600000UL;

enum class Group : uint8_t { LeftEye, RightEye, LeftMouth, RightMouth, Embers, WhiteFlashes };
enum class Mode : uint8_t { Sleep, Awake, Chaos };
enum class Effect : uint8_t { Steady, Breathing, Flashes };

struct EffectSettings {
  Effect effect = Effect::Steady;
  uint8_t minimum = 0;
  uint8_t maximum = 0;
  uint32_t riseMs = 0;
  uint32_t fallMs = 0;
  uint32_t onMs = 0;
  uint32_t offMs = 0;
  uint32_t initialDelayMs = 0;
};

struct Channel {
  EffectSettings settings;
  uint32_t startedAt = 0;
};

struct Controller {
  Mode mode = Mode::Sleep;
  uint32_t startedAt = 0;
  uint32_t durationMs = 0;
  Channel channels[GroupCount];
};

// The upper bound is exclusive, matching Arduino random().
using RandomRange = uint32_t (*)(uint32_t minimum, uint32_t maximum);

uint8_t brightnessPercent(uint8_t percent);
bool setEffect(Channel (&channels)[GroupCount], Group group,
               const EffectSettings &settings, uint32_t now);
uint8_t evaluate(const Channel &channel, uint32_t now);
void render(const Controller &controller, uint32_t now, uint8_t (&levels)[GroupCount]);
bool enterMode(Controller &controller, Mode mode, uint32_t now, RandomRange randomRange);
bool start(Controller &controller, uint32_t now, RandomRange randomRange);
bool update(Controller &controller, uint32_t now, RandomRange randomRange);

} // namespace lighting

#endif
