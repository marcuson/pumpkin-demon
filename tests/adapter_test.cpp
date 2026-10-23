#include <assert.h>
#include <algorithm>
#include <limits>
#include <stdio.h>
#include <string>
#include <vector>
#include "../src/main.cpp"

namespace {
struct Call { char kind; uint8_t pin; int value; };
std::vector<Call> calls;
struct Choice { long minimum; long maximum; long value; };
std::vector<Choice> choices;
size_t choiceIndex = 0;
uint32_t clockMs = 0;
uint32_t clockUs = 0;
unsigned int millisCalls = 0;
unsigned int microsCalls = 0;
unsigned int analogReads = 0;
unsigned int seedCalls = 0;
unsigned long lastSeed = 0;
unsigned long serialBaud = 0;
int txCapacity = 0;
size_t shortWriteLimit = std::numeric_limits<size_t>::max();
std::string serialOutput;
#ifndef LIGHTING_INTEGRATION
uint8_t fakeLevels[lighting::GroupCount] = {51, 51, 51, 51, 26, 0};
bool failStart = false;
bool changeMode = false;
lighting::Mode nextMode = lighting::Mode::Awake;
unsigned int updateCalls = 0;
unsigned int renderCalls = 0;
uint32_t updateTime = 0;
#endif

void resetBoard() {
  controller = lighting::Controller();
  std::fill(lastLevels, lastLevels + lighting::GroupCount, 0);
  outputsApplied = initialized = false;
  pendingLength = pendingOffset = 0;
  calls.clear();
  choices.clear();
  choiceIndex = 0;
  clockMs = clockUs = 0;
  millisCalls = microsCalls = analogReads = seedCalls = 0;
  lastSeed = serialBaud = 0;
  txCapacity = 0;
  shortWriteLimit = std::numeric_limits<size_t>::max();
  serialOutput.clear();
#ifndef LIGHTING_INTEGRATION
  const uint8_t initial[] = {51, 51, 51, 51, 26, 0};
  std::copy(initial, initial + lighting::GroupCount, fakeLevels);
  failStart = changeMode = false;
  nextMode = lighting::Mode::Awake;
  updateCalls = renderCalls = 0;
  updateTime = 0;
#endif
}

void drainLogs() {
  shortWriteLimit = 3;
  for (unsigned int i = 0; i < 100 && pendingOffset < pendingLength; ++i) {
    txCapacity = 7;
    loop();
  }
  assert(pendingOffset == pendingLength);
}

void expectLevel(uint8_t pin, uint8_t level) {
  for (auto it = calls.rbegin(); it != calls.rend(); ++it) {
    if (it->pin == pin && (it->kind == 'D' || it->kind == 'A')) {
      assert(it->kind == ((level == 0 || level == 255) ? 'D' : 'A'));
      assert(it->value == (level == 255 ? HIGH : level == 0 ? LOW : level));
      return;
    }
  }
  assert(false && "expected GPIO output was not applied");
}

void testBoundedEventFormatting() {
  resetBoard();
  controller.mode = lighting::Mode::Chaos;
  controller.durationMs = 5000;
  for (uint8_t i = 0; i < lighting::GroupCount; ++i)
    controller.channels[i].settings.initialDelayMs = 1500;
  queueModeLog(UINT32_MAX, false);
  const std::string expected = "t=4294967295 event=Mode mode=Chaos duration_ms=5000 delays_ms=eyes:1500 mouth:1500 embers:1500 whites:1500\n";
  assert(expected.size() < sizeof(pendingLog));
  assert(std::string(pendingLog, pendingLength) == expected);
  txCapacity = 11;
  serviceLog();
  assert(pendingOffset == 11);
  controller.mode = lighting::Mode::Sleep;
  controller.durationMs = 600000;
  queueModeLog(0, false);
  queueError(0);
  assert(std::string(pendingLog, pendingLength) == expected);
  while (pendingOffset < pendingLength) { txCapacity = 5; serviceLog(); }
  assert(serialOutput == expected);
  queueModeLog(0, false);
  while (pendingOffset < pendingLength) { txCapacity = 5; serviceLog(); }
  assert(serialOutput == expected + "t=0 event=Mode mode=Sleep duration_ms=600000\n");
}

void testStartupAndSerialBackpressure() {
  resetBoard();
#ifdef LIGHTING_INTEGRATION
  choices.push_back({180000, 600001, 180000});
#endif
  setup();
  assert(initialized);
  assert(serialBaud == 115200);
  assert(analogReads == 16 && microsCalls == 17);
  assert(seedCalls == 1 && lastSeed != 0);
  assert(millisCalls == 1);
  assert(calls.size() >= 18);
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) {
    assert(calls[i].kind == 'D' && calls[i].pin == lighting::Pins[i] && calls[i].value == LOW);
    assert(calls[i + 6].kind == 'P' && calls[i + 6].pin == lighting::Pins[i]);
  }
  const uint8_t expected[] = {51, 51, 51, 51, 26, 0};
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) expectLevel(lighting::Pins[i], expected[i]);
  assert(serialOutput.empty() && pendingOffset == 0);
  const size_t before = calls.size();
  loop();
  assert(millisCalls == 2 && calls.size() == before);
  drainLogs();
  assert(serialOutput == "t=0 event=Startup mode=Sleep duration_ms=180000\n");
  const size_t logged = serialOutput.size();
  loop();
  assert(serialOutput.size() == logged && seedCalls == 1);
}

#ifndef LIGHTING_INTEGRATION
void testOutputAdapterAndCache() {
  resetBoard();
  setup();
  drainLogs();
  calls.clear();
  const uint8_t levels[] = {0, 255, 1, 254, 128, 255};
  std::copy(levels, levels + lighting::GroupCount, fakeLevels);
  clockMs = 77;
  const unsigned int timeReads = millisCalls;
  loop();
  assert(millisCalls == timeReads + 1 && updateTime == 77);
  for (uint8_t i = 0; i < lighting::GroupCount; ++i) expectLevel(lighting::Pins[i], levels[i]);
  size_t count = calls.size();
  loop();
  assert(calls.size() == count);
  fakeLevels[0] = 100;
  loop();
  assert(calls.size() == count + 1);
  expectLevel(3, 100);
  fakeLevels[0] = 255;
  loop();
  expectLevel(3, 255);
  fakeLevels[0] = 100;
  loop();
  expectLevel(3, 100);
  const uint8_t previousWhite = lastLevels[5];
  fakeLevels[5] = 128;
  loop();
  assert(lastLevels[5] == previousWhite);
  for (const auto &call : calls) assert(call.kind != 'A' || call.pin != 11);
  drainLogs();
  assert(serialOutput.find("error=Invalid effect assignment") != std::string::npos);
}

void testModeLogAndDisconnectedProgress() {
  resetBoard();
  setup();
  drainLogs();
  serialOutput.clear();
  calls.clear();
  txCapacity = 0;
  clockMs = 180000;
  changeMode = true;
  nextMode = lighting::Mode::Chaos;
  fakeLevels[0] = 0;
  const unsigned int before = updateCalls;
  loop();
  assert(updateCalls == before + 1 && serialOutput.empty());
  const std::string expected = "t=180000 event=Mode mode=Chaos duration_ms=2000 delays_ms=eyes:0 mouth:500 embers:1000 whites:1500\n";
  assert(std::string(pendingLog, pendingLength) == expected);
  assert(!calls.empty() && calls.front().kind == 'D');
  for (unsigned int i = 0; i < 100; ++i) { ++clockMs; loop(); }
  assert(updateCalls == before + 101 && pendingOffset == 0);
  // A blocked transmitter retains the complete earlier event rather than corrupting its line.
  changeMode = true;
  nextMode = lighting::Mode::Awake;
  loop();
  assert(std::string(pendingLog, pendingLength) == expected);
  drainLogs();
  assert(serialOutput == expected);
}

void testStartupFailureAndZeroSeed() {
  resetBoard();
  failStart = true;
  setup();
  assert(!initialized && lastSeed == 1);
  const unsigned int before = updateCalls;
  loop();
  assert(updateCalls == before);
  for (const auto &call : calls) assert(call.kind != 'A' && (call.kind != 'D' || call.value == LOW));
  drainLogs();
  assert(serialOutput == "t=0 error=Invalid effect assignment\n");
}
#else
void testRealCoreTransitions() {
  resetBoard();
  choices = {{180000, 600001, 180000}, {0, 100, 0}, {5000, 10001, 5000},
             {180000, 600001, 180000}, {0, 100, 90}, {2000, 5001, 2000},
             {0, 1501, 0}, {0, 1501, 500}, {0, 1501, 1000}, {0, 1501, 1500},
             {180000, 600001, 600000}};
  setup();
  drainLogs();
  serialOutput.clear();
  txCapacity = 0;
  clockMs = 180000;
  loop();
  assert(controller.mode == lighting::Mode::Awake);
  for (uint8_t pin : lighting::Pins) expectLevel(pin, 255);
  clockMs += 100;
  loop();
  expectLevel(11, 0);
  drainLogs();
  assert(serialOutput == "t=180000 event=Mode mode=Awake duration_ms=5000\n");
  clockMs = 185000;
  loop();
  assert(controller.mode == lighting::Mode::Sleep);
  expectLevel(10, 26);
  drainLogs();
  serialOutput.clear();
  txCapacity = 0;
  clockMs = 365000;
  loop();
  assert(controller.mode == lighting::Mode::Chaos);
  expectLevel(3, 255); expectLevel(5, 255);
  expectLevel(6, 0); expectLevel(9, 0); expectLevel(10, 0); expectLevel(11, 0);
  clockMs += 500;
  loop();
  expectLevel(6, 255); expectLevel(9, 255);
  clockMs += 1000;
  loop();
  expectLevel(11, 255);
  drainLogs();
  assert(serialOutput == "t=365000 event=Mode mode=Chaos duration_ms=2000 delays_ms=eyes:0 mouth:500 embers:1000 whites:1500\n");
  clockMs = 367000;
  loop();
  assert(controller.mode == lighting::Mode::Sleep && controller.durationMs == 600000);
  expectLevel(10, 26); expectLevel(11, 0);
  assert(choiceIndex == choices.size());
}
#endif
} // namespace

TestSerial Serial;
void digitalWrite(uint8_t pin, uint8_t value) { calls.push_back({'D', pin, value}); }
void analogWrite(uint8_t pin, int value) { calls.push_back({'A', pin, value}); }
void pinMode(uint8_t pin, uint8_t value) { calls.push_back({'P', pin, value}); }
unsigned long millis() { ++millisCalls; return clockMs; }
unsigned long micros() { ++microsCalls; return clockUs; }
int analogRead(uint8_t pin) { assert(pin == A0); ++analogReads; return 0; }
void randomSeed(unsigned long seed) { lastSeed = seed; ++seedCalls; }
long random(long minimum, long maximum) {
  assert(minimum < maximum);
  if (choiceIndex == choices.size()) return minimum;
  const Choice choice = choices[choiceIndex++];
  assert(choice.minimum == minimum && choice.maximum == maximum);
  assert(choice.value >= minimum && choice.value < maximum);
  return choice.value;
}
void TestSerial::begin(unsigned long baud) { serialBaud = baud; }
int TestSerial::availableForWrite() { return txCapacity; }
size_t TestSerial::write(const uint8_t *data, size_t count) {
  assert(txCapacity >= 0 && count <= static_cast<size_t>(txCapacity));
  const size_t written = std::min(count, shortWriteLimit);
  serialOutput.append(reinterpret_cast<const char *>(data), written);
  txCapacity -= static_cast<int>(written);
  return written;
}

#ifndef LIGHTING_INTEGRATION
namespace lighting {
bool start(Controller &state, uint32_t now, RandomRange randomRange) {
  if (failStart) return false;
  state.mode = Mode::Sleep;
  state.startedAt = now;
  state.durationMs = randomRange(180000, 600001);
  return true;
}
bool update(Controller &state, uint32_t now, RandomRange) {
  ++updateCalls;
  updateTime = now;
  if (!changeMode) return false;
  changeMode = false;
  state.mode = nextMode;
  state.startedAt = now;
  state.durationMs = nextMode == Mode::Chaos ? 2000 : 5000;
  state.channels[0].settings.initialDelayMs = 0;
  state.channels[2].settings.initialDelayMs = 500;
  state.channels[4].settings.initialDelayMs = 1000;
  state.channels[5].settings.initialDelayMs = 1500;
  return true;
}
void render(const Controller &, uint32_t now, uint8_t (&levels)[GroupCount]) {
  assert(now == clockMs);
  ++renderCalls;
  std::copy(fakeLevels, fakeLevels + GroupCount, levels);
}
} // namespace lighting
#endif

int main() {
  testBoundedEventFormatting();
  testStartupAndSerialBackpressure();
#ifndef LIGHTING_INTEGRATION
  testOutputAdapterAndCache();
  testModeLogAndDisconnectedProgress();
  testStartupFailureAndZeroSeed();
  puts("hardware adapter tests passed");
#else
  testRealCoreTransitions();
  puts("integrated firmware tests passed");
#endif
}
