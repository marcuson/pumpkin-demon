#include <Arduino.h>
#include <stdio.h>
#include "lighting.h"

namespace {

lighting::Controller controller;
uint8_t lastLevels[lighting::GroupCount] = {};
bool outputsApplied = false;
bool initialized = false;
char pendingLog[160];
size_t pendingLength = 0;
size_t pendingOffset = 0;

uint32_t randomRange(uint32_t minimum, uint32_t maximum) {
  return static_cast<uint32_t>(random(static_cast<long>(minimum), static_cast<long>(maximum)));
}

void seedRandom() {
  uint32_t seed = micros();
  for (uint8_t i = 0; i < 16; ++i) {
    seed = (seed << 5) ^ (seed >> 27) ^ static_cast<uint32_t>(analogRead(A0)) ^ micros();
  }
  randomSeed(seed == 0 ? 1 : seed);
}

void finishLog(int length) {
  pendingOffset = 0;
  pendingLength = length > 0 ? static_cast<size_t>(length) : 0;
  if (pendingLength >= sizeof(pendingLog)) pendingLength = sizeof(pendingLog) - 1;
}

void queueError(uint32_t now) {
  if (pendingOffset < pendingLength) return;
  finishLog(snprintf(pendingLog, sizeof(pendingLog),
                     "t=%lu error=Invalid effect assignment\n", static_cast<unsigned long>(now)));
}

void queueModeLog(uint32_t now, bool startup) {
  // shortcut: retain one pending event during stalled TX; add a queue if lossless history is required.
  if (pendingOffset < pendingLength) return;
  const char *mode = controller.mode == lighting::Mode::Sleep ? "Sleep" :
                     controller.mode == lighting::Mode::Awake ? "Awake" : "Chaos";
  if (controller.mode == lighting::Mode::Chaos) {
    finishLog(snprintf(pendingLog, sizeof(pendingLog),
        "t=%lu event=Mode mode=Chaos duration_ms=%lu delays_ms=eyes:%lu mouth:%lu embers:%lu whites:%lu\n",
        static_cast<unsigned long>(now), static_cast<unsigned long>(controller.durationMs),
        static_cast<unsigned long>(controller.channels[0].settings.initialDelayMs),
        static_cast<unsigned long>(controller.channels[2].settings.initialDelayMs),
        static_cast<unsigned long>(controller.channels[4].settings.initialDelayMs),
        static_cast<unsigned long>(controller.channels[5].settings.initialDelayMs)));
  } else {
    finishLog(snprintf(pendingLog, sizeof(pendingLog),
        "t=%lu event=%s mode=%s duration_ms=%lu\n", static_cast<unsigned long>(now),
        startup ? "Startup" : "Mode", mode, static_cast<unsigned long>(controller.durationMs)));
  }
}

void serviceLog() {
  if (pendingOffset >= pendingLength) return;
  const int available = Serial.availableForWrite();
  if (available <= 0) return;
  size_t count = pendingLength - pendingOffset;
  if (count > static_cast<size_t>(available)) count = static_cast<size_t>(available);
  pendingOffset += Serial.write(reinterpret_cast<const uint8_t *>(pendingLog + pendingOffset), count);
}

void applyOutputs(uint32_t now) {
  uint8_t levels[lighting::GroupCount];
  lighting::render(controller, now, levels);
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) {
    const uint8_t level = levels[i];
    if (i == static_cast<uint8_t>(lighting::Group::WhiteFlashes) && level != 0 && level != 255) {
      queueError(now);
      continue;
    }
    if (outputsApplied && level == lastLevels[i]) continue;
    if (level == 0 || level == 255) {
      digitalWrite(lighting::Pins[i], level == 0 ? LOW : HIGH);
    } else {
      analogWrite(lighting::Pins[i], level);
    }
    lastLevels[i] = level;
  }
  outputsApplied = true;
}

} // namespace

void setup() {
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) digitalWrite(lighting::Pins[i], LOW);
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) pinMode(lighting::Pins[i], OUTPUT);
  Serial.begin(115200);
  seedRandom();
  const uint32_t now = millis();
  initialized = lighting::start(controller, now, randomRange);
  if (initialized) {
    applyOutputs(now);
    queueModeLog(now, true);
  } else {
    queueError(now);
  }
  serviceLog();
}

void loop() {
  const uint32_t now = millis();
  if (initialized) {
    const bool changed = lighting::update(controller, now, randomRange);
    applyOutputs(now);
    if (changed) queueModeLog(now, false);
  }
  serviceLog();
}
