
// Written by Nick Gammon
// February 2012
#define displayCLK 3
#define displaySTATUS 2
#define displayDATA 4 //PD2
#define displayRESET 8

#define DATA_IS_HIGH (PIND & (1<<PD4))

#define SDA_PORT PORTC
#define SDA_PIN 2 // = A2
#define SCL_PORT PORTC
#define SCL_PIN 3 // = A3


//#define I2C_SLOWMODE 1
#include <SoftI2CMaster.h>

#define I2C_7BITADDR 0x44
#define howmanypackets 10
#define howmanybytesinpacket 10

#include <Wire.h>


const byte MY_ADDRESS = 0x44;

uint8_t wdp = 0;
uint8_t rdp = 0;
uint8_t reading_i2c = 0;
int8_t data[howmanypackets][howmanybytesinpacket]; //size+subaddress+4 (for seting balance and fade ...) I never see more data  so, 8 should be preaty safe... also never see more then 3 packet send one after another, so 6 packets should be ok
volatile uint8_t _bit = 0;
volatile uint8_t _byte;
volatile uint8_t _msg[howmanypackets];
volatile uint8_t dwdp = 0;
volatile uint8_t grabing_SPI = 0;
uint8_t drdp = 0;
uint8_t volume_up = 0; //2=up 1=down, 0=nothing
int volume = 0; //set start volume here ...


void sendI2C(uint8_t data[howmanybytesinpacket]);
void decode_i2c(uint8_t data[howmanybytesinpacket]);
void fix_data(uint8_t data[howmanybytesinpacket]);
void spk_atten(uint8_t c);
void enableInteruptOnCLK();
void disableInteruptOnCLK();
void readCLK();

void setup ()
{
  pinMode(displayCLK, INPUT_PULLUP);
  pinMode(displaySTATUS, INPUT_PULLUP);
  pinMode(displayDATA, INPUT_PULLUP);
  //pinMode(displayRESET, INPUT);
//  attachInterrupt(digitalPinToInterrupt(displaySTATUS), enableInteruptOnCLK, RISING);
  Serial.begin(115200);
  Serial.println("hello");
  Wire.begin (MY_ADDRESS);
  for (byte i = 2; i <= 7; i++)
    pinMode (i, OUTPUT);
  Wire.onReceive (receiveEvent);
  //      if (!i2c_init()) // Initialize everything and check for bus lockup
  //        Serial.println("I2C init failed");
}  // end of setup

void loop()
{
//  if (!grabing_SPI) { //
//    while (drdp != dwdp) {
//      uint8_t prev;
//      if (drdp > 0) prev = _msg[drdp - 1];
//      if (drdp == 0) prev = _msg[howmanypackets];
//
//      if (prev == 0x25)
//      {
//        //Serial.println(_msg[drdp],HEX);
//        if (_msg[drdp] == 0xD ) {
//          volume = 2;
//          //Serial.println("Vol up");
//        }
//        if (_msg[drdp] == 0x11 ) {
//          volume = 1;
//          //Serial.println("Vol down");
//        }
//      }
//      drdp++;
//      if (drdp == howmanypackets) drdp = 0;
//    }
//  }
  if (!reading_i2c) {// not capturing i2c, can send it out...
    while (rdp != wdp) {
      //      Serial.println("we are not capturing, and wdp is bigger then rdp");
      //      Serial.print("wdp: "); Serial.print(wdp); Serial.print(" rdp "); Serial.println(rdp);
      uint8_t _data[howmanybytesinpacket];
      for (uint8_t i = 0; i < howmanybytesinpacket; i++) {
        _data[i] = data[rdp][i];
      }
      // Serial.println("array copied, sending to TDA");
      fix_data(_data);
     // sendI2C(_data);
      decode_i2c(_data);
      rdp++;
      if (rdp == howmanypackets) rdp = 0;
      //      Serial.print("wdp: "); Serial.print(wdp); Serial.print(" rdp "); Serial.println(rdp);
    }
  }

}

//enable interupt on CLK line when STATUS line is up
void enableInteruptOnCLK()
{

  attachInterrupt(digitalPinToInterrupt(displaySTATUS), disableInteruptOnCLK, FALLING);
  _bit = 0;
  grabing_SPI = 1;
  attachInterrupt(digitalPinToInterrupt(displayCLK), readCLK, FALLING); //or based on OSCILO observations
}

//disable CLK interupt while status is low
void disableInteruptOnCLK()
{
  _msg[dwdp++] = _byte;
  if (dwdp == howmanypackets) dwdp = 0;
  grabing_SPI = 0;
  detachInterrupt(digitalPinToInterrupt(displayCLK));
  attachInterrupt(digitalPinToInterrupt(displaySTATUS), enableInteruptOnCLK, RISING);
}

void readCLK()
{
  // Serial.println(digitalRead(displayDATA),DEC);
  // if (digitalRead(displayDATA)) {
  if (DATA_IS_HIGH) {
    _byte = (_byte << 1) | 1;
  } else {
    _byte = (_byte << 1);
  }
}

// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
{
  //Serial.print("grabing i2c: wdp: "); Serial.print(wdp); Serial.print(" howmany: "); Serial.println(howMany);
  reading_i2c = 1;
  data[wdp][0] = howMany;
  for (uint8_t i = 0; i < howMany; i++) {
    data[wdp][i + 1] = Wire.read();
    //   Serial.print(c,HEX);
  }

  wdp++;
  if (wdp == howmanypackets) wdp = 0;
  reading_i2c = 0;
}  // end of receiveEvent



void sendI2C (uint8_t data[howmanybytesinpacket]) {
  int timeout_us = 5000;
  while (!i2c_start((I2C_7BITADDR << 1) | I2C_WRITE) && timeout_us > 0) {
    delayMicroseconds(20);
    timeout_us -= 20;
  }

  if (timeout_us <= 0) { // start transfer
    Serial.println("I2C device busy");
    return;
  }

  // Serial.print("size: ");
  // Serial.println(data[0]);

  for (byte i = 0 ; i < data[0]; i++) {
    i2c_write(data[i + 1]);
    //Serial.print(data[i + 1], HEX);
    //Serial.print(" ");
  }
  i2c_stop(); // send stop condition
  //Serial.println();
  //decode_i2c(data[]);
}

void fix_data(uint8_t data[howmanybytesinpacket]) {

}

void decode_i2c(uint8_t data[howmanybytesinpacket]) {
  uint8_t increments = 1; //at least 1 iteration of next FOR must run...
  if (data[1] > 0xf) {
    //autoincrement of subaddress:
    //packet is subbadress+data, data[0] is packet size , so number of incrementation is this number -1
    increments = data[0] - 1;
    //actual subaddress is only low 4bits:
    data[1] = data[1] & 0xf;
  }
  for (uint8_t i = 0; i < increments; i++) {
    uint8_t subaddress = data[1] + i;//subbadress is always 2nd field in array plus increment
    uint8_t c = data[i + 2];//0=size, 1= subaddress
    //Serial.println(subaddress);
    switch (subaddress) {
      case 0:
        { // input selector
          
          Serial.print("Input selector: ");
          //Serial.print(c,HEX);
          //Serial.print(" ");
          switch (c & B0000111) {
            case 1:
              Serial.println("TAPE selected (IN2)");
              break;
            case 2:
              Serial.println("FM/AM selected (IN1) ");
              break;
            case 3:
              Serial.println("TP selected (AM mono)");
              break;

          }
          switch (c & B01000111) {
            case 0:
              Serial.println("CD selected (0dB diferential input gain (IN3))");
              break;
            case 40:
              Serial.println("CD selected (-6dB diferential input gain (IN3))");
              break;

          }
          switch (c & B00011000) {
            case 0:
              Serial.println("11.25dB gain");
              break;
            case 1:
              Serial.println("7.5dB gain");
              break;
            case 2:
              Serial.println("3.75dB gain");
              break;
            case 3:
              Serial.println("0dB gain");
              break;
          }
        }
        break;
      case 1:
        { // loudness
          Serial.print("Loudness: ");
          if (c > 0xf) {
            Serial.println ("OFF");
          } else {
            Serial.print("-");
            Serial.print(((c & 0xF) * 1.25), DEC);
            Serial.println("dB");
          }
        }
        break;
      case 2:
        { // volume
          Serial.print("volume ");
          //          Serial.println(c);
          float volume = 0;
          //          Serial.println((c & B00000011) * (-0.31));
          //          Serial.println(((c >> 2) & B00000111) * (-1.25));
          //          Serial.println(20 - (((c >> 5) & B00000111) * 10));
          volume = (20 - (((c >> 5) & B00000111) * 10)) + (((c >> 2) & B00000111) * (-1.25)) + ((c & B00000011) * (-0.31));
          Serial.print(volume);
          Serial.print("dB");
          Serial.print(" ( ");
          Serial.print(c,HEX);
          Serial.println(" )");
        }
        break;
      case 3: // bass, treble
        {
          Serial.print("Bass: ");
          //Bass
          switch (c >> 4) {
            case 0:
              Serial.print(-14);
              break;
            case 1:
              Serial.print(-12);
              break;
            case 2:
              Serial.print(-10);
              break;
            case 3:
              Serial.print(-8);
              break;
            case 4:
              Serial.print(-6);
              break;
            case 5:
              Serial.print(-4);
              break;
            case 6:
              Serial.print(-2);
              break;
            case 7:
            case 15:
              Serial.print(0);
              break;
            case 8:
              Serial.print(14);
              break;
            case 9:
              Serial.print(12);
              break;
            case 10:
              Serial.print(10);
              break;
            case 11:
              Serial.print(8);
              break;
            case 12:
              Serial.print(6);
              break;
            case 13:
              Serial.print(4);
              break;
            case 14:
              Serial.print(2);
              break;
          }
          Serial.print("dB, Trebble: ");
          //trebble
          switch (c & 0xF) {
            case 0:
              Serial.print(18);
              break;
            case 1:
              Serial.print(16);
              break;
            case 2:
              Serial.print(-10);
              break;
            case 3:
              Serial.print(-8);
              break;
            case 4:
              Serial.print(-6);
              break;
            case 5:
              Serial.print(-4);
              break;
            case 6:
              Serial.print(-2);
              break;
            case 7:
            case 15:
              Serial.print(0);
              break;
            case 8:
              Serial.print(14);
              break;
            case 9:
              Serial.print(12);
              break;
            case 10:
              Serial.print(10);
              break;
            case 11:
              Serial.print(8);
              break;
            case 12:
              Serial.print(6);
              break;
            case 13:
              Serial.print(4);
              break;
            case 14:
              Serial.print(2);
              break;
          }
          Serial.println("dB");
        }
        break;
      case 4: // Speaker Attenuator left front
        Serial.print("Speaker Attenuator left front: ");
        spk_atten(c);
        break;
      case 5: // Speaker Attenuator left rear
        Serial.print("Speaker Attenuator left rear: ");
        spk_atten(c);
        break;
      case 6: // Speaker Attenuator right front
        Serial.print("Speaker Attenuator right front: ");
        spk_atten(c);
        break;
      case 7: // Speaker Attenuator left rear
        Serial.print("Speaker Attenuator left rear: ");
        spk_atten(c);
        break;
      case 8: // mute
        Serial.print("Mute: ");
        //0th and 1st bits
        switch (c & B00000011) {
          case 1:
            Serial.println("Soft Mute with fast slope (I = Imax)");
            break;
          case 3:
            Serial.println("Soft Mute with slow slope (I = Imin)");
            break;
        }
        //3th bit
        if ((c >> 3) & 1) Serial.println("Direct Mute");
        //2nd and 4th bit
        if (!((c >> 5) & 1)) {
          Serial.print("Zero Crossing Mute ");
          if ((c >> 2) & 1) {
            Serial.println("On");
          } else {
            Serial.println("Off");
          }
        }
        //5th and 6th bit
        switch ((c >> 5) & B00000011) {
          case 0:
            Serial.println("160mV ZC Window Threshold (WIN = 00)");
            break;
          case 1:
            Serial.println("80mV ZC Window Threshold (WIN = 01)");
            break;
          case 2:
            Serial.println("40mV ZC Window Threshold (WIN = 10)");
            break;
          case 3:
            Serial.println("20mV ZC Window Threshold (WIN = 11)");
            break;
        }
        switch ((c >> 7) & B00000011) {
          case 0:
            Serial.println("Nonsymmetrical Bass Cut");
            break;
          case 1:
            Serial.println("Symmetrical Bass Cut");
            break;
        }
        break;
    }
  }
}

void spk_atten(uint8_t c) {
  if ((c & B00011111) == 0x1F)
  {
    Serial.println("Muted");
  } else {

    float low = (c & B00000111) * 1.25;
    uint8_t high = ((c >> 3) & B00000011) * 10;
    Serial.print(-(low + high));
    Serial.println("dB");
  }
}
