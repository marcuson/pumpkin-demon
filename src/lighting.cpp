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

} // namespace lighting
