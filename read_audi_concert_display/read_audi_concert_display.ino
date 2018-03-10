#define RADIO_STATUS 2 // int0
#define RADIO_CLOCK 3  // int1
#define RADIO_DATA 4
#define DISPLAY_STATUS 18 // int2
#define DISPLAY_CLOCK 19  // int3
#define DISPLAY_DATA 20
#define RADIO_TXBUFFER 12
#define DISPLAY_TXBUFFER 12

volatile byte DISPLAY_myByte = 0;
volatile byte DISPLAY_myBit = 0;
volatile int DISPLAY_done = 0;
volatile byte RADIO_myByte = 0;
volatile byte RADIO_myBit = 0;
volatile int RADIO_done = 0;
uint8_t RADIO_txbuffer[RADIO_TXBUFFER];
uint8_t DISPLAY_txbuffer[DISPLAY_TXBUFFER];
uint8_t RADIO_txinptr;
uint8_t RADIO_txoutptr;
uint8_t DISPLAY_txinptr;
uint8_t DISPLAY_txoutptr;
volatile RADIO_capturing; //flag that  we are receiving, so no sending muchachos
volatile DISPLAY_capturing;

void DISPLAY_STATUS_line_rising() {
  DISPLAY_myByte = 0;
  DISPLAY_myBit = 0;
  attachInterrupt(digitalPinToInterrupt(DISPLAY_CLOCK), DISPLAY_read_data_line, RISING); //data are valid on rising edge of CLK
  attachInterrupt(digitalPinToInterrupt(DISPLAY_STATUS), DISPLAY_STATUS_line_falling, FALLING); //if STATUS changed to low, data on DATA line are no more valid. this will fire ISR which disable CLK ISR
}

void DISPLAY_STATUS_line_falling() {
  detachInterrupt(digitalPinToInterrupt(DISPLAY_CLOCK));
  attachInterrupt(digitalPinToInterrupt(DISPLAY_STATUS), DISPLAY_STATUS_line_rising, RISING);
}

void DISPLAY_read_data_line() {
  if (digitalRead(DISPLAY_DATA)) {
    DISPLAY_txbuffer[RADIO_txinptr] = DISPLAY_txbuffer[RADIO_txinptr] << 1 | 0x0001;
    DISPLAY_myBit++;
  }
  else {
    DISPLAY_txbuffer[RADIO_txinptr] = DISPLAY_txbuffer[RADIO_txinptr] << 1;
    DISPLAY_myBit++;
  }
  if (DISPLAY_myBit == 8) {
    DISPLAY_txinptr++;
  }
}

void RADIO_STATUS_line_rising() {
  RADIO_myByte = 0;
  RADIO_myBit = 0;
  attachInterrupt(digitalPinToInterrupt(RADIO_CLOCK), RADIO_read_data_line, RISING); //data are valid on rising edge of CLK
  attachInterrupt(digitalPinToInterrupt(RADIO_STATUS), RADIO_STATUS_line_falling, FALLING); //if STATUS changed to low, data on DATA line are no more valid. this will fire ISR which disable CLK ISR
}

void RADIO_STATUS_line_falling() {
  detachInterrupt(digitalPinToInterrupt(RADIO_CLOCK));
  attachInterrupt(digitalPinToInterrupt(RADIO_STATUS), RADIO_STATUS_line_rising, RISING);
}

void RADIO_read_data_line() {
  if (digitalRead(RADIO_DATA)) {
    RADIO_txbuffer[RADIO_txinptr] = RADIO_txbuffer[RADIO_txinptr] << 1 | 0x0001;
    RADIO_myBit++;
  }
  else {
    RADIO_txbuffer[RADIO_txinptr] = RADIO_txbuffer[RADIO_txinptr] << 1;
    RADIO_myBit++;
  }
  if (RADIO_myBit == 8) {
    RADIO_txinptr++;
  }
}

void RADIO_send_cmd(int cmd) { //cheking if we can send somesing is done in main loop
  pinMode(RADIO_STATUS, OUTPUT);
  pinMode(RADIO_CLOCK, OUTPUT);
  pinMode(RADIO_DATA, OUTPUT);

  digitalWrite(RADIO_DATA, LOW);
  digitalWrite(RADIO_CLOCK, LOW);

  digitalWrite(RADIO_STATUS, LOW);
  delayMicroseconds(200);
  digitalWrite(RADIO_STATUS, HIGH);
  // shiftOut(RADIO_DATA, RADIO_DATA, MSBFIRST, cmd);


  static int iResult[8];
  for (int i = 0; i <= 7; i++)
  {
    iResult[i] = cmd % 2;
    cmd = cmd / 2;
  }
  for (int i = 7; i >= 0; i--) {
    switch (iResult[i]) {
      case 1: digitalWrite(RADIO_DATA, HIGH);
        break;
      case 0: digitalWrite(RADIO_DATA, LOW);
        break;
    }
    delayMicroseconds(5);
    digitalWrite(RADIO_CLOCK, HIGH);
    delayMicroseconds(5);
    digitalWrite(RADIO_CLOCK, LOW);
    delayMicroseconds(5);
  }

  delayMicroseconds(50);
  digitalWrite(RADIO_STATUS, LOW);
  delayMicroseconds(200);

  digitalWrite(RADIO_STATUS, HIGH);
}
}




void setup() {
  Serial.begin(9600);
  //Serial.print("hello");

  RADIO_txinptr = 0;
  RADIO_txoutptr = 0;
  DISPLAY_txinptr = 0;
  DISPLAY_txoutptr = 0;
  RADIO_capturing = 0;
  DISPLAY_capturing = 0;

  pinMode(SS, INPUT);
  pinMode(SCK, INPUT);
  pinMode(MISO, INPUT);
  attachInterrupt(intSS_IN, SS_line_rising, RISING);
}

void loop() {
  if (done) {
    Serial.println(myByte, HEX);;
    myByte = 0;
    myBit = 0;
    done = 0;
    //    t = millis();
  }
}

