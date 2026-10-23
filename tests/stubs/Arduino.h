#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <stddef.h>
#include <stdint.h>

constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;
constexpr uint8_t OUTPUT = 1;
constexpr uint8_t A0 = 14;

void digitalWrite(uint8_t pin, uint8_t value);
void analogWrite(uint8_t pin, int value);
void pinMode(uint8_t pin, uint8_t mode);
unsigned long millis();
unsigned long micros();
int analogRead(uint8_t pin);
long random(long minimum, long maximum);
void randomSeed(unsigned long seed);

struct TestSerial {
  void begin(unsigned long baud);
  int availableForWrite();
  size_t write(const uint8_t *data, size_t count);
};
extern TestSerial Serial;

#endif
