#include <avr/sleep.h>

void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(2))
  {
    delay(1000);
    digitalWrite(4, HIGH);
    delay(5000);
    digitalWrite(4, LOW);
  }
  //sleep_mode();
  while (1);
}
