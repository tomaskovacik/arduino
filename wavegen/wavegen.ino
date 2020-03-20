#define CALC_CONS 125000 //125000  = 16 000 000 / 64(prescaler) / 2 ;;

uint16_t freq = 10;

void setup() {
  TIMSK1 = 0;
  pinMode(9, OUTPUT);
  TCCR1A = _BV(COM1A0) |  _BV(COM1B0) |  _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);
  OCR1A = CALC_CONS / freq; 
  Serial.begin(115200);
  Serial.println("Output is on pin 9 (OC1A)");
  Serial.println("w/s control freq up/down");
  Serial.println("a/d directly in/decrement changes OCR1A register");
}

void loop() {
  if (Serial.available()) {
    switch (Serial.read())
    {
      case 'w':
        if (freq == 350)freq = 10;
        else freq++;
        OCR1A = CALC_CONS / freq;
        break;
      case 's':
        if (freq == 0)freq = 300;
        else freq--;
        OCR1A = CALC_CONS / freq;
        break;
      case 'a':
        OCR1A++;
        break;
      case 'd':
        OCR1A--;
        break;
    }
    Serial.println(OCR1A, HEX);
    Serial.println(freq);
  }
}
