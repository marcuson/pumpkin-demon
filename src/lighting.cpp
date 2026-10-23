#include "lighting.h"

namespace lighting {

uint8_t brightnessPercent(uint8_t percent) {
  if (percent > 100) percent = 100;
  return (uint32_t(percent) * 255 + 50) / 100;
}

bool setEffect(Channel (&channels)[GroupCount], Group group,
               const EffectSettings &settings, uint32_t now) {
  const uint8_t index = static_cast<uint8_t>(group);
  if (index >= GroupCount) return false;
  const bool white = group == Group::WhiteFlashes;
  if (white && settings.maximum != 0 && settings.maximum != 255) return false;
  switch (settings.effect) {
    case Effect::Steady:
      break;
    case Effect::Breathing:
      if (white || settings.minimum > settings.maximum ||
          settings.riseMs == 0 || settings.fallMs == 0 ||
          settings.riseMs > MaxEffectDurationMs || settings.fallMs > MaxEffectDurationMs)
        return false;
      break;
    case Effect::Flashes:
      if (settings.onMs == 0 || settings.offMs == 0 ||
          settings.onMs > MaxEffectDurationMs || settings.offMs > MaxEffectDurationMs ||
          settings.initialDelayMs > MaxEffectDurationMs)
        return false;
      break;
    default:
      return false;
  }
  channels[index].settings = settings;
  channels[index].startedAt = now;
  return true;
}

uint8_t evaluate(const Channel &channel, uint32_t now) {
  const EffectSettings &settings = channel.settings;
  const uint32_t elapsed = now - channel.startedAt;
  switch (settings.effect) {
    case Effect::Steady:
      return settings.maximum;
    case Effect::Breathing: {
      const uint32_t phase = elapsed % (settings.riseMs + settings.fallMs);
      const uint32_t range = settings.maximum - settings.minimum;
      if (phase < settings.riseMs)
        return settings.minimum + range * phase / settings.riseMs;
      return settings.maximum - range * (phase - settings.riseMs) / settings.fallMs;
    }
    case Effect::Flashes:
      if (elapsed < settings.initialDelayMs) return 0;
      return (elapsed - settings.initialDelayMs) % (settings.onMs + settings.offMs) <
                     settings.onMs ? settings.maximum : 0;
    default:
      return 0;
  }
}

void render(const Controller &controller, uint32_t now, uint8_t (&levels)[GroupCount]) {
  for (uint8_t i = 0; i < GroupCount; ++i)
    levels[i] = evaluate(controller.channels[i], now);
}

bool enterMode(Controller &controller, Mode mode, uint32_t now, RandomRange randomRange) {
  if (!randomRange) return false;
  uint32_t minimumDuration;
  uint32_t maximumDuration;
  switch (mode) {
    case Mode::Sleep:
      minimumDuration = 180000;
      maximumDuration = 600000;
      break;
    case Mode::Awake:
      minimumDuration = 5000;
      maximumDuration = 10000;
      break;
    case Mode::Chaos:
      minimumDuration = 2000;
      maximumDuration = 5000;
      break;
    default:
      return false;
  }

  Controller next;
  next.mode = mode;
  next.startedAt = now;
  next.durationMs = randomRange(minimumDuration, maximumDuration + 1);
  if (next.durationMs < minimumDuration || next.durationMs > maximumDuration) return false;
  uint32_t delays[4] = {};
  if (mode == Mode::Chaos) {
    for (uint8_t i = 0; i < 4; ++i) {
      delays[i] = randomRange(0, 1501);
      if (delays[i] > 1500) return false;
    }
  }

  for (uint8_t i = 0; i < GroupCount; ++i) {
    EffectSettings settings;
    settings.maximum = 255;
    if (mode == Mode::Sleep) {
      settings.maximum = i < 4 ? brightnessPercent(20) : 0;
      if (i == static_cast<uint8_t>(Group::Embers)) {
        settings.effect = Effect::Breathing;
        settings.minimum = brightnessPercent(10);
        settings.maximum = brightnessPercent(60);
        settings.riseMs = settings.fallMs = 2000;
      }
    } else if (mode == Mode::Chaos || i == static_cast<uint8_t>(Group::WhiteFlashes)) {
      settings.effect = Effect::Flashes;
      settings.onMs = 100;
      settings.offMs = mode == Mode::Awake ? 400 : 200;
      settings.initialDelayMs = delays[i < 4 ? i / 2 : i - 2];
    }
    if (!setEffect(next.channels, static_cast<Group>(i), settings, now)) return false;
  }
  controller = next;
  return true;
}

bool start(Controller &controller, uint32_t now, RandomRange randomRange) {
  return enterMode(controller, Mode::Sleep, now, randomRange);
}

bool update(Controller &controller, uint32_t now, RandomRange randomRange) {
  if (!randomRange || uint32_t(now - controller.startedAt) < controller.durationMs) return false;
  Mode next;
  switch (controller.mode) {
    case Mode::Sleep: {
      const uint32_t selection = randomRange(0, 100);
      if (selection >= 100) return false;
      next = selection < 90 ? Mode::Awake : Mode::Chaos;
      break;
    }
    case Mode::Awake:
    case Mode::Chaos:
      next = Mode::Sleep;
      break;
    default:
      return false;
  }
  return enterMode(controller, next, now, randomRange);
}

} // namespace lighting
