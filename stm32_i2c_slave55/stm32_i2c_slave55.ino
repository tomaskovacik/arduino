#include <Wire_slave.h> //wireslave for stm32, there is no single lib for slave/master

#define I2C_7BITADDR 0x55
const byte MY_ADDRESS = I2C_7BITADDR;

void decode_slave_i2c_data(int howmany, uint8_t data[16]);
uint8_t _TP = 0;
uint8_t _AS = 0;
uint8_t _MEM = 0;
String _NAME;
uint8_t _MODE = 0; //fm=0; fm1=1 fm2=2 am=3
String _FREQ;

void setup() {
  // put your setup code here, to run once:u
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:


}

void receiveEvent (int howMany)
{
  uint8_t data[16];
  //Serial.print(F("grabing i2c: wdp: ")); Serial.print(wdp);// Serial.print(F(" howmany: "); Serial.println(howMany);
  for (uint8_t i = 0; i < howMany; i++) {
    data[i] = Wire.read();
  }
  decode_slave_i2c_data(howMany, data);

}  // end of receiveEvent

void decode_slave_i2c_data(int howmany, uint8_t data[16]) {
  uint8_t dump = 1;
  switch (data[0]) {
    case 0x2:
      switch (data[1]) {
        case 0x84:
          Serial.println("CD track down.");
          break;
        case 0x85:
          Serial.println("CD track up.");
          break;
      }
      break;
    case 0x8:
      switch (data[1]) {
        case 0x81:
          dump = 0;
          Serial.println("FREQ MODE");
          _TP = 0;
          break;
        case 0x82:
          Serial.println("TEXT MODE");
          _TP = 0;
          break;
        case 0x85:
          Serial.println("CD MODE");
          _TP = 0;
          _MODE = 4;
          break;
        case 0x89:
          _TP = 1;
          Serial.println("FREQ MODE + TP");
          break;
        case 0x8A:
          _TP = 1;
          Serial.println("TEXT MODE + TP");
          break;
        case 0x8D:
          Serial.println("CD MODE + TP");
          _TP = 1;
          _MODE = 4;
          break;
        case 0xF9:
          _TP = 0;
          Serial.println("SEARCH");
          break;
      }
      break;
    case 0x82:
      {
        //freq packet
        //AM = Bxxxx01xx
        //AM (AS) = Bxxxx11xx
        //FM = Bxxxxx0xx (or xxxx00xx ?)
        //FM (AS) = Bxxxx001x
        if (data[2] & B00000100) { //AM MODE
          _MODE = 3; //AM
          _FREQ = String(531 + (data[1] * 9)) + "kHz";
          _MEM = data[2] >> 4;
          _AS = (data[2] & B00001000) ? 1 : 0;
        } else { //FM mode
          _MODE = 0;
          _MODE = (data[2] & B00000001) ? 2 : 1;
          _FREQ = String((((float)data[1] + 875) / 10), 1) + "MHz";
          _MEM = data[2] >> 4;
          _AS = (data[2] & B00000010) ? 1 : 0;
        }
      }
      break;
    case 0xA4: //text mode
      {
        dump = 0;
        _NAME = String((char)data[1]) + String((char)data[2]) + String((char)data[3]) + String((char)data[4]) + String((char)data[5]) + String((char)data[6]) + String((char)data[7]) + String((char)data[8]);
        // Serial.println(_NAME);
        break;/*
    case 0x8:
    break;
    case 0x8:
    break;*/
      }
  }
  if (dump)
  {
    for (uint8_t i = 0; i < howmany; i++) {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  if (_MODE < 4) {
    Serial.print(_NAME);
    switch (_MODE) {
      case 0:
        Serial.print(" FM ");
        break;
      case 1:
        Serial.print(" FM1 ");
        break;
      case 2:
        Serial.print(" FM2 ");
        break;
      case 3:
        Serial.print(" AM ");
        break;
    }
    Serial.println(" " + _FREQ + " Memory: " + String(_MEM) + String(_AS ? " AS" : "") + String(_TP ? " TP" : ""));
  }
}
