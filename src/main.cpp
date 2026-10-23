#include <Arduino.h>

constexpr uint8_t LEFT_PIN = 3;
constexpr uint8_t RIGHT_PIN = 5;

void setup() {
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("[diag] Avvio test LED: gruppo 1=D3, gruppo 2=D5"));
  Serial.println(F("[diag] Uscite inizializzate LOW; ogni fase dura 1 secondo"));
}

void loop() {
  analogWrite(LEFT_PIN, 255);
  analogWrite(RIGHT_PIN, 0);
  Serial.println(F("[diag] Fase 1/5: gruppo 1 acceso | PWM D3=255 D5=0"));
  delay(1000);

  analogWrite(LEFT_PIN, 0);
  analogWrite(RIGHT_PIN, 255);
  Serial.println(F("[diag] Fase 2/5: gruppo 2 acceso | PWM D3=0 D5=255"));
  delay(1000);

  analogWrite(LEFT_PIN, 128);
  analogWrite(RIGHT_PIN, 128);
  Serial.println(F("[diag] Fase 3/5: entrambi a meta | PWM D3=128 D5=128"));
  delay(1000);

  analogWrite(LEFT_PIN, 255);
  analogWrite(RIGHT_PIN, 255);
  Serial.println(F("[diag] Fase 4/5: entrambi accesi | PWM D3=255 D5=255"));
  delay(1000);

  analogWrite(LEFT_PIN, 0);
  analogWrite(RIGHT_PIN, 0);
  Serial.println(F("[diag] Fase 5/5: entrambi spenti | PWM D3=0 D5=0"));
  delay(1000);
}
