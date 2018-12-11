//#include <Wire.h>
#include <Wire_slave.h> //wireslave for stm32, there is no single lib for slave/master

#include <SlowSoftWire.h> //so we do not have single lib for slave/master, so we have to init another one for master .... cose we do not have 3HW i2c .... tiktak ...
//stm32
SlowSoftWire SWire = SlowSoftWire(PB11, PB10);



/*
    SPI comunication between motorola MC68HC05B32 cpu to front panel ST6280
    basics:
    SPI MODE 0

    CLK idle high, while
    STATUS idle high, it's like CS but it's triggered by slave here)
    DATA idle high

    comunication principle:
    CLK line going LOW is signaling someone wanna talk
    whatever it's slave (panel, cose we have buttons there ...) or master
    after CLK is low, slave bring STATE line LOW, and when he is ready to
    receive data he will bring STATE HIGH, then master will clock out data
    or clock in data, after everything is transmited, CLOCK goes HIGH and
    shortly after this STATE goes HIGH signaling end of packet.
    While CLK is still low after STATE goes LOW, signaling there is probably
    more data to come.

*/
//ARDUINO
//#define displayCLK 3 //CLK
//#define displaySTATUS 2 //STATUS/CS
//#define displayDATA 4 //PD2 - DATA
//#define displayRESET 8
//STM32
#define displayCLK PB3 //CLK
#define displaySTATUS PA15 //STATUS/CS
#define displayDATA PB4//DATA
#define displayRESET 8


//this is SW i2c for arduino, did not work on STM32, cose there is some ASM woodoo :)))
#define DATA_IS_HIGH (PIND & (1<<PD4))
//#define SDA_PORT PORTC
#define SDA_PIN A2 // = A2
//#define SCL_PORT PORTC
#define SCL_PIN A3 // = A3
//#define I2C_SLOWMODE 1
//#include <SoftI2CMaster.h>
//TwoWire Wire(PB9,PB8, SOFT_STANDARD);
//SlowSoftWire SWire(SDA_PIN, SCL_PIN);

//we are trying to interface with comunication to TDA7342 which is 0x44 so...
#define I2C_7BITADDR 0x44
//array is klasik byte howmanypackets X howmanybytesinpacket
#define howmanypackets 10
#define howmanybytesinpacket 15



const byte MY_ADDRESS = I2C_7BITADDR;

uint8_t wdp = 0; //write data pointer, data for i2c, going from 0 to howmanypackets
uint8_t rdp = 0; //read data pointer for i2c comunication, going from 0 to howmanypackets
uint8_t reading_i2c = 0; //flag indicating that we are busy grabing i2c data, so we should not mess with them in main loop
/*
   array for i2c data, size+subaddress+4 (for seting balance and fade ...) I never see more data  so, 8 should be preaty safe...
   also never see more then 3 packet send one after another, so 6 packets should be ok
   but also after while, I implemented grabing display data which can be more then 8 like 15-16 bytes per packet
*/
volatile uint8_t data[howmanypackets][howmanybytesinpacket];

volatile uint8_t _byte; //temporary, incoming byte is shiffted here, then when we are done grabbing it, it is stored in array each packet alone in one row
volatile uint8_t _msg[howmanypackets][howmanybytesinpacket]; //here we have array for packet for display
volatile uint8_t dwdp = 0; //display write data pointer, going from 0 to howmanypackets
volatile uint8_t dwbp = 0; //display write byte pointer, for each dwdp there is "howmanybytesinpacket" dwbp,  going from to howmanybytesinpacket, we do not need this, cose i2c comunication has exact number of byte per packet ...
volatile uint8_t grabing_SPI = 0; //flag indicating we are busy grabing front panel display data, so we should not mess with them in main loop
volatile uint8_t drdp = 0; //display read data pointer for front panel comunication
volatile uint8_t volume = 0xFF; //set start volume here ...
volatile uint8_t set_volume = 0xFF; //set start volume here ..
volatile uint8_t grab_volume = 1;

/*
   functions
*/
/*
   function is sending i2c packet, one day with fixed volume values :)
*/
void sendI2C(uint8_t data[howmanybytesinpacket]);
/*
   decode all i2c from MCU heading to TDA7342, with probably fked volume data
*/
void decode_i2c(uint8_t data[howmanybytesinpacket]);
/*
   yeh this would one day fix fked volume based on data we have from front panel and so on...
*/
void fix_data(uint8_t data[howmanybytesinpacket]);
/*
   calculate speaker attuenations, cose we are calculating this for each speaker, so I make fction to avoid long code...
*/
void spk_atten(uint8_t c);
/*
   RISING interupt on STATE line
   it enable RISING interupt on CLK line to grab data on dataline when fired
   this one also change interupt on STATE line to FALLING

   first after fired, we check if CLK is LOW or HIGH:

   if HIGH - indicating end of packet:
            immediatly detach this interrupt, without this  it was acting strangely
            change rest of packet in arary to zero so there is no junc from previous comunication (or should I zeroed it before? nooooo  here we have lot of time, cose we are at end of packet \o/
            set display byte pointer to 0
            increment dispay write pointer
            again attach same interupt RISING on STATE line

   if LOW - we are going to receive data/more data - next byte in packet
            enabling FALLING interupt on STATE line
            zeroing _byte variable, just in case, we should not need this, cose ther should be 8 runs of CLK pulses, so it should overflow all old data .... but just in case we are doing it
            seting grabing_SPI flag
            enabling RISING interupt on CLK line

*/
void enableInteruptOnCLK();
/*

    fired when STATE goes LOW, disale any CLK interupt, and enable RISING interupt on STATE line

*/
void disableInteruptOnCLK();
/*

   function to grabing data if CLK goes HIGH

*/
void readCLK();


/*

   clasic setup function

*/
void setup ()
{
  //init pins for display SPI
  pinMode(displayCLK, INPUT_PULLUP);
  pinMode(displaySTATUS, INPUT_PULLUP);
  pinMode(displayDATA, INPUT_PULLUP);
  //pinMode(displayRESET, INPUT);
  //init interrupt on STATUS line to grab data send betwen display and main CPU
  attachInterrupt(digitalPinToInterrupt(displaySTATUS), enableInteruptOnCLK, RISING);
  //init slave i2c to grab data for TDA7342
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);
  //master i2c to send data(fixed) to TDA7342
  SWire.begin();
  //serial for debug
  Serial2.begin(115200);
  Serial2.println(F("start"));
  //arduino
  //      if (!i2c_init()) // Initialize everything and check for bus lockup
  //        Serial2.println(F("I2C init failed");
}  // end of setup


void loop()
{
  if (!grabing_SPI) { //no data are send on SPI line
    while (drdp != dwdp) { //reading and writing pointers are not in sync, we have some data which should be analyzed
      //move current reading data from array of packet in separate variable,
      //here should be memcopy, no for ... but... who cares ...
      //or we should just send pointer drdp as function parameter, array with packet is not local ....no I try it and it will use 1% more of program storage space  ...
      uint8_t _data[howmanybytesinpacket];
      for (uint8_t i = 0; i < howmanybytesinpacket; i++) {
        _data[i] = _msg[drdp][i];
        // Serial2.print(_data[i],HEX);
      }
      // Serial2.println(_data[0],HEX);
      if (_data[0] == 0x25)//button push
      {
        // decode_button_push(_data[1]); //function which send to serial port real function of pressed button in human language
        // Serial2.println(grab_volume);

        if (grab_volume == 1 && _data[1] == 0x86) { //volume nob was turned up, and cose grab_volume is set to 1, we  know that is volume not  bass/trebble/ballance/fade, we set grab_volume=0 when display shows bass/trebble/ballance/fade)
          // volume, 0xFF=off, 0x00=full on, steps in 4
          //Serial2.println(F("+");
          if (volume < 4) {
            volume = 0;
          } else {
            volume -= 4;
          }
        }
        if (grab_volume == 1 && _data[1] == 0x88) { //some as up but nob goes down
          //volume--;
          // Serial2.println(F("-");
          if (volume > 0xFF - 4) {
            volume = 0xFF;
          } else {
            volume += 4;
          }
        }
      } //else {
      if (_data[0] == 0x9A) { // packet starting with 0x95 is update for pannel, text, indications leds ....
        decode_display_data(_data);
      }
      drdp++; //after everything increment read pointer
      if (drdp == howmanypackets) drdp = 0; //reset to zero if we are at top
    }
  }
  if (!reading_i2c) {// not capturing i2c, safe to mess with it
    while (rdp != wdp) {//reading and writing pointers are not in sync, we have some data which should be analyzed
      //move current reading data from array of packet in separate variable,
      //here should be memcopy, no for ... but... who cares ...
      //or we should just send pointer as function paramete, array with packet is not local ....

      // Serial2.println(F("array copied, sending to TDA");
      if ((data[rdp][1] & 0xf) == 2) {//volume was set by panel, and is probably fucked :) , only fixing volume packet, subbaddress = ?
        //fix_data(rdp);
      }
      uint8_t _data[howmanybytesinpacket];
      for (uint8_t i = 0; i < howmanybytesinpacket; i++) {
        _data[i] = data[rdp][i];
      }
      sendI2C(_data);
      //decode_i2c(_data);
      rdp++;
      if (rdp == howmanypackets) rdp = 0;
      //      Serial2.print(F("wdp: "); Serial2.print(wdp); Serial2.print(F(" rdp "); Serial2.println(rdp);
    }
  }

}
/*

   function which should fix volume data, somehow ... :))

*/
void fix_data(uint8_t rdp) {
  Serial2.print(F("volume from MCU:")); Serial2.println(data[rdp][2], HEX);

  Serial2.println(volume, HEX);
  Serial2.println(set_volume, HEX);

  if (volume != set_volume) { //volume -set byt display and set_volume- current volume are same, so nothing need to be fixed
    //Serial2.println(F("dbg 1");
    //volume was initialized by user action, and this data are probably corupted, so we will calculcate next step based
    //on previously saved volume level in variable volume and value of grab_volume variable
    if (set_volume > volume ) { //so volume is less then curent(set)volume = we are increasing volume (lower value is actualy higher sound output, less attuneation of speakers)...
      if (set_volume > 4) { //do not go under 0, overflow fix
        set_volume = set_volume - 4; //make this 2 if we are less then 0xBA
      } else {
        set_volume = 0; //make this 2 if we are less then 0xBA
      }
    }
    if (set_volume > volume) {
      if (set_volume < (0xFF - 4)) {
        set_volume = set_volume + 4;
      } else {
        set_volume = 0xFF;
      }
    }
    data[rdp][2] = set_volume;
  } else {
    set_volume = data[rdp][2]; //seting variables to be same as send from mcu
    volume = data[rdp][2];
  }

  Serial2.println(volume, HEX);
  Serial2.print(F("set volume: ")); Serial2.println(set_volume, HEX);
}

/*

   decoding display data  parameter is array with data packet
   I try to send just pointer, cose data array is not local, we can access it everywhere
   but it will get 1% more of storage program and it's probably not faster ...

   packet struckture:
   1st byte in packet is packet definition or something,.... it's always 0x96
   2nd byte in packet identified data send in packet:
          - 0x48 - plain asci data to display
          - 0x13 - leds on buttons indicating mode/functions
                  -3th byte: [nan|I|I|I|FM|AS|RDS|REG]
                              - 5th bit is  "pipe" between FM lethers, which make AM symbol something like F|M
                              - 6th and 7th bits are same "pipe" which made FM 1 and FM 2 like FM I(I)
                  - 4th byte: [nan|nan|nan|RD|Dolby|CPS|presets|presets]
                              - 0th and 1st bit are for stations presets 1,2,3,4,5,6
                  - 5th byte: LEDS [nan|MODE|AS|SCAN|FM|TP|AM|RDS]

          - 0x32 AM/FM frequency display
                  - 3th byte:??
                  - 4th byte: actual freq:
                      AM mode: (531+(9*4th byte)) in kHz
                      FM mode: (875+4th byte)/10 in Mhz
          - 0xA2 CD changer mode
                 - 3th packet is CD number, in hex (but here it's not important, we have only 6CD)
                 - 4th packet is Track number, again in hex, but no ABCDEF is used ...
          - 0x23 - display clear.
          - 0x61 - TAPE mode (display shows TAPE)
                   3th byte: 1/2 indicate direction of playback (/\ or \/)
                             3/4 indicate fast forward or rewind (< or > )
                             0 indicate eject

*/
void decode_display_data(uint8_t _data[howmanybytesinpacket]) {
  //  if(_data[1] == 0x13) Serial2.println(_data[2],BIN); //debug
  uint8_t dump = 0;
  switch (_data[1]) { //switching second byte, which indicate type of packet data
    case 0x48:
      {
        grab_volume = 1;
        Serial2.print(F("display data ASCI: "));
        // }
        for (uint8_t i = 2; i < howmanybytesinpacket; i++) {
          Serial2.write(_data[i]);
          //debug
          //Serial2.print(F(" ");
          //Serial2.write(_data[i]);
        }
        Serial2.println();
      }
      break;
    case 0x13:
      //leds: whole packet: 9A 13 2E 0 29 0 0 0 0 0 0 0 0 0 0
      {
        grab_volume = 1;
        //Serial2.println(_data[2],BIN);
        if (_data[2] & B00000001) Serial2.print(F("REG ")); //REG bit
        if (_data[2] & B00000010) Serial2.print(F("RDS ")); //RDS bit
        if (_data[2] & B00000100) Serial2.print(F("AS ")); //AS bit
        if (_data[2] & B00001000) {
          if (_data[2] & B01000000) Serial2.print(F("FM1 ")); //FM + 1 bit
          if (_data[2] & B00100000) Serial2.print(F("FM2 ")); //FM + 2 bit
          if (_data[2] & B00010000) Serial2.print(F("AM ")); //FM+|=AM bit
        }
        Serial2.println();
        Serial2.print(F("Memory: ")); Serial2.println(_data[3] & B00000011); //Memory


        Serial2.println();
        Serial2.println(F("LEDS: "));
        //this are in data[2]
        if (_data[3] & B00001000) Serial2.print(F("CPS ")); //CPS
        if (_data[3] & B00100000) Serial2.print(F("Dolby ")); //Dolby
        if (_data[3] & B00010000) Serial2.print(F("RD ")); //RD
        //other in data[4]
        if (_data[4] & B00000001) Serial2.print(F("RDS "));
        if (_data[4] & B00000010) Serial2.print(F("AM "));
        if (_data[4] & B00000100) Serial2.print(F("TP "));
        if (_data[4] & B00001000) Serial2.print(F("FM "));
        if (_data[4] & B00010000) Serial2.print(F("SCAN "));
        if (_data[4] & B00100000) Serial2.print(F("AS "));
        if (_data[4] & B01000000) Serial2.print(F("MODE "));
        Serial2.println();
      }
      break;
    case 0x32: //freq?
      {
        grab_volume = 1;
        if (_data[3] == 0x10) { //AM
          uint16_t freq = 531;
          freq = freq + (_data[2] * 9);
          Serial2.print(F("freq: "));
          Serial2.print(freq);
          Serial2.println(F(" kHz (AM)"));
        } else { //FM
          float freq = 875;
          freq = freq + _data[2];
          Serial2.print(F("freq: "));
          Serial2.print(freq / 10, 1);
          Serial2.println(F(" Mhz (FM)"));
        }
      }
      break;

    case 0xA2:
      {
        grab_volume = 1;
        Serial2.print(F("CD"));
        Serial2.print(_data[2], HEX);
        Serial2.print(F(" TR"));
        Serial2.println(_data[3], HEX);
      }
      break;
    case 0x23:
      {
        Serial2.println(F("display clear"));
      }
      break;
    case 0x61:
      {
        grab_volume = 1;
        Serial2.print(F("TAPE: "));
        //TAPE mode
        switch (_data[2]) {
          case 0x01:
            Serial2.println(F("TAPE: /\\"));
            break;
          case 2:
            Serial2.println(F("TAPE: \\/"));
            break;
          case 3:
            Serial2.println(F("TAPE:  > (FF)"));
            break;
          case 4:
            Serial2.println(F("TAPE:  < (FR)"));
            break;
          case 0:
            Serial2.println(F("TAPE: Eject"));
            break;
          case 0x10:
            Serial2.println(F("TP-INFO"));
            break;
          default:
            dump = 1;
        }
      }
      break;
    case 0x71:
      {
        //  Serial2.print(F("Stored text::");
        switch (_data[2] >> 4) {
          case 0x00:
            grab_volume = 0;
            Serial2.print(F("BAS "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("+"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x1:
            grab_volume = 0;
            Serial2.print(F("BAS "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("-"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x2:
            grab_volume = 0;
            Serial2.print(F("TRE "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("+"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x3:
            Serial2.print(F("TRE "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("-"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x4:
            grab_volume = 0;
            Serial2.print(F("BAL "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("R"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x5:
            grab_volume = 0;
            Serial2.print(F("BAL "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("L"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x6:
            grab_volume = 0;
            Serial2.print(F("FAD "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("F"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0x7:
            grab_volume = 0;
            Serial2.print(F("FAD "));
            if ((_data[2] & 0x0F) == 0) {
              Serial2.println(0);
            } else {
              Serial2.print(F("R"));
              Serial2.println((_data[2] & 0x0F), DEC);
            }
            break;
          case 0xA:
            grab_volume = 1;
            Serial2.print(F("TP - MEM ")); //A1 is MEM 1?
            Serial2.print((_data[2] & 0x0F), DEC);
            break;
          default:
            dump = 1;
        }
      }
    case 0xE1:
      {
        if (_data[2] == 0xFB) Serial2.println(F("Start"));
        volume = 0xFF;
        set_volume = 0xFF;
      }
      break;
    case 0x80:
      {
        if (_data[2] == 0x00) Serial2.println(F("Shutdown"));
      }
      break;
    default:
      {
        dump = 1;
      }

  }
  if (dump) {
    Serial2.print(F("unknown display data: "));
    // }
    for (uint8_t i = 0; i < howmanybytesinpacket; i++) {
      Serial2.print(_data[i], HEX);
      Serial2.print(F(" "));
      //    Serial2.write(_data[i]);
    }
    Serial2.println();
  }
}


//enable RISING interupt on CLK line when STATUS line RISED
void enableInteruptOnCLK()
{
  if (digitalRead(displayCLK)) {
    detachInterrupt(digitalPinToInterrupt(displaySTATUS)); //we need  to do this, cose otherwise it's doing strange things

    //CLK is HIGH, this is end of  packet
    while (dwbp < howmanybytesinpacket) { //clean array from current write pointer to end of packet
      _msg[dwdp][dwbp++] = 0;
    }
    dwbp = 0; //set byte write pointer to begining
    dwdp++; //increment write pointer
    if (dwdp == howmanypackets) dwdp = 0; //if we reach last+1 position in array for packet, go back to 0
    attachInterrupt(digitalPinToInterrupt(displaySTATUS), enableInteruptOnCLK, RISING); //enable this interrupt again, with same parameters
    //after this interupt is still set to rising on STATUS line,
    grabing_SPI = 0;//we are safe to manipulate data in main loop, I just move this from disableInteruptOnCLK function
  } else {
    //clk is low, start of packet
    attachInterrupt(digitalPinToInterrupt(displaySTATUS), disableInteruptOnCLK, FALLING); //seting falling interupt on STATE line, indicating end of byte transfer
    _byte = 0; //new data, zeroing temporary variable used to clock in data , just to be sure
    grabing_SPI = 1;//set grabit flag to avoid messing with live packet data in main loop
    attachInterrupt(digitalPinToInterrupt(displayCLK), readCLK, RISING); //enabling interupt on CLK like, to grab data after each fire of this int routine
  }
}

//disable CLK interupt while STATUS is low
void disableInteruptOnCLK()
{
  detachInterrupt(digitalPinToInterrupt(displayCLK)); //so STATUS is low, so all data are clocked in:
  _msg[dwdp][dwbp++] = _byte; //move data from tempporary variable to array based on pointer of current packet and current byte in packet
  if (dwbp == howmanybytesinpacket ) { //this can happend, but it must be last byte in packet, otherwise we will rewrite data in packet row
    dwbp = 0;
    Serial2.println(F("dwbp overflow"));//put this out, just to know,
  }
  //grabing_SPI = 0;//we are safe to manipulate data in main loop, I just move this to enableInteruptOnCLK function to part wich indicate end of packet transfer
  attachInterrupt(digitalPinToInterrupt(displaySTATUS), enableInteruptOnCLK, RISING);// enable RISING interupt on STATE line, indicating start of transmition of data
}

void readCLK()
{
  // Serial2.println(digitalRead(displayDATA),DEC);
  if (digitalRead(displayDATA)) {
    // if (DATA_IS_HIGH) {
    // Serial2.print(1);
    _byte = (_byte << 1) | 1;
  } else {
    // Serial2.print(0);
    _byte = (_byte << 1);
  }
}

// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
{
  //Serial2.print(F("grabing i2c: wdp: "); Serial2.print(wdp); Serial2.print(F(" howmany: "); Serial2.println(howMany);
  reading_i2c = 1;
  data[wdp][0] = howMany;
  for (uint8_t i = 0; i < howMany; i++) {

    data[wdp][i + 1] = Wire.read();
    //Serial2.print(data[wdp][i + 1], HEX);
  }

  wdp++;
  if (wdp == howmanypackets) wdp = 0;
  reading_i2c = 0;
}  // end of receiveEvent



void sendI2C (uint8_t data[howmanybytesinpacket]) {
  //  int timeout_us = 5000;
  //  while (!i2c_start((I2C_7BITADDR << 1) | I2C_WRITE) && timeout_us > 0) {
  //    delayMicroseconds(20);
  //    timeout_us -= 20;
  //  }
  //
  //  if (timeout_us <= 0) { // start transfer
  //    Serial2.println(F("I2C device busy");
  //    return;
  //  }

  SWire.beginTransmission(MY_ADDRESS); // transmit to device

  // Serial2.print(F("size: ");
  // Serial2.println(data[0]);

  for (byte i = 0 ; i < data[0]; i++) {
    //i2c_write(data[i + 1]);
    SWire.write(data[i + 1]);              // sends one byte

    //Serial2.print(data[i + 1], HEX);
    //Serial2.print(F(" ");
  }
  //i2c_stop(); // send stop condition
  SWire.endTransmission();    // stop transmitting
  //Serial2.println();
  //decode_i2c(data[]);
}



void decode_i2c(uint8_t data[howmanybytesinpacket]) {
  uint8_t increments = 1; //at least 1 iteration of next FOR must run...
  if (data[1] > 0xf) {
    //autoincrement of subaddress:
    //packet is subbadress+data, data[0] is packet size , so number of incrementation is "packet_size - 1"
    increments = data[0] - 1;
    //actual subaddress is only low 4bits:
    data[1] = data[1] & 0xf;
  }
  for (uint8_t i = 0; i < increments; i++) {
    uint8_t subaddress = data[1] + i;//subbadress is always 2nd field in array plus increment
    uint8_t c = data[i + 2];//0=size, 1= subaddress
    //Serial2.println(subaddress);
    switch (subaddress) {
      case 0:
        { // input selector

          Serial2.print(F("Input selector: "));
          //Serial2.print(c,HEX);
          //Serial2.print(F(" ");
          switch (c & B0000111) {
            case 1:
              Serial2.println(F("TAPE selected (IN2)"));
              break;
            case 2:
              Serial2.println(F("FM / AM selected (IN1) "));
              break;
            case 3:
              Serial2.println(F("TP selected (AM mono)"));
              break;

          }
          switch (c & B01000111) {
            case 0:
              Serial2.println(F("CD selected (0dB diferential input gain (IN3))"));
              break;
            case 40:
              Serial2.println(F("CD selected (-6dB diferential input gain (IN3))"));
              break;

          }
          switch (c & B00011000) {
            case 0:
              Serial2.println(F("11.25dB gain"));
              break;
            case 1:
              Serial2.println(F("7.5dB gain"));
              break;
            case 2:
              Serial2.println(F("3.75dB gain"));
              break;
            case 3:
              Serial2.println(F("0dB gain"));
              break;
          }
        }
        break;
      case 1:
        { // loudness
          Serial2.print(F("Loudness: "));
          if (c > 0xf) {
            Serial2.println(F("OFF"));
          } else {
            Serial2.print(F(" - "));
            Serial2.print(((c & 0xF) * 1.25), DEC);
            Serial2.println(F("dB"));
          }
        }
        break;
      case 2:
        { // volume
          Serial2.print(F("volume "));
          //          Serial2.println(c);
          float _volume = 0;
          //          Serial2.println((c & B00000011) * (-0.31));
          //          Serial2.println(((c >> 2) & B00000111) * (-1.25));
          //          Serial2.println(20 - (((c >> 5) & B00000111) * 10));
          _volume = (20 - (((c >> 5) & B00000111) * 10)) + (((c >> 2) & B00000111) * (-1.25)) + ((c & B00000011) * (-0.31));
          Serial2.print(_volume);
          Serial2.print(F("dB"));
          Serial2.print(F(" ( "));
          Serial2.print(c, HEX);
          Serial2.println(F(" )"));
        }
        break;
      case 3: // bass, treble
        {
          Serial2.print(F("Bass: "));
          //Bass
          switch (c >> 4) {
            case 0:
              Serial2.print(-14);
              break;
            case 1:
              Serial2.print(-12);
              break;
            case 2:
              Serial2.print(-10);
              break;
            case 3:
              Serial2.print(-8);
              break;
            case 4:
              Serial2.print(-6);
              break;
            case 5:
              Serial2.print(-4);
              break;
            case 6:
              Serial2.print(-2);
              break;
            case 7:
            case 15:
              Serial2.print(0);
              break;
            case 8:
              Serial2.print(14);
              break;
            case 9:
              Serial2.print(12);
              break;
            case 10:
              Serial2.print(10);
              break;
            case 11:
              Serial2.print(8);
              break;
            case 12:
              Serial2.print(6);
              break;
            case 13:
              Serial2.print(4);
              break;
            case 14:
              Serial2.print(2);
              break;
          }
          Serial2.print(F("dB, Trebble: "));
          //trebble
          switch (c & 0xF) {
            case 0:
              Serial2.print(18);
              break;
            case 1:
              Serial2.print(16);
              break;
            case 2:
              Serial2.print(-10);
              break;
            case 3:
              Serial2.print(-8);
              break;
            case 4:
              Serial2.print(-6);
              break;
            case 5:
              Serial2.print(-4);
              break;
            case 6:
              Serial2.print(-2);
              break;
            case 7:
            case 15:
              Serial2.print(0);
              break;
            case 8:
              Serial2.print(14);
              break;
            case 9:
              Serial2.print(12);
              break;
            case 10:
              Serial2.print(10);
              break;
            case 11:
              Serial2.print(8);
              break;
            case 12:
              Serial2.print(6);
              break;
            case 13:
              Serial2.print(4);
              break;
            case 14:
              Serial2.print(2);
              break;
          }
          Serial2.println(F("dB"));
        }
        break;
      case 4: // Speaker Attenuator left front
        Serial2.print(F("Speaker Attenuator left front: "));
        spk_atten(c);
        break;
      case 5: // Speaker Attenuator left rear
        Serial2.print(F("Speaker Attenuator left rear: "));
        spk_atten(c);
        break;
      case 6: // Speaker Attenuator right front
        Serial2.print(F("Speaker Attenuator right front: "));
        spk_atten(c);
        break;
      case 7: // Speaker Attenuator left rear
        Serial2.print(F("Speaker Attenuator left rear: "));
        spk_atten(c);
        break;
      case 8: // mute
        Serial2.print(F("Mute: "));
        //0th and 1st bits
        switch (c & B00000011) {
          case 1:
            Serial2.println(F("Soft Mute with fast slope (I = Imax)"));
            break;
          case 3:
            Serial2.println(F("Soft Mute with slow slope (I = Imin)"));
            break;
        }
        //3th bit
        if ((c >> 3) & 1) Serial2.println(F("Direct Mute"));
        //2nd and 4th bit
        if (!((c >> 5) & 1)) {
          Serial2.print(F("Zero Crossing Mute "));
          if ((c >> 2) & 1) {
            Serial2.println(F("On"));
          } else {
            Serial2.println(F("Off"));
          }
        }
        //5th and 6th bit
        switch ((c >> 5) & B00000011) {
          case 0:
            Serial2.println(F("160mV ZC Window Threshold (WIN = 00)"));
            break;
          case 1:
            Serial2.println(F("80mV ZC Window Threshold (WIN = 01)"));
            break;
          case 2:
            Serial2.println(F("40mV ZC Window Threshold (WIN = 10)"));
            break;
          case 3:
            Serial2.println(F("20mV ZC Window Threshold (WIN = 11)"));
            break;
        }
        switch ((c >> 7) & B00000011) {
          case 0:
            Serial2.println(F("Nonsymmetrical Bass Cut"));
            break;
          case 1:
            Serial2.println(F("Symmetrical Bass Cut"));
            break;
        }
        break;
    }
  }
}

void spk_atten(uint8_t c) {
  if ((c & B00011111) == 0x1F)
  {
    Serial2.println(F("Muted"));
  } else {

    float low = (c & B00000111) * 1.25;
    uint8_t high = ((c >> 3) & B00000011) * 10;
    Serial2.print(-(low + high));
    Serial2.println(F("dB"));
  }
}

void decode_button_push(uint8_t data) {
  // Serial2.print(data,HEX);
  switch (data) {
    case 0x01:
      Serial2.println(F(" 1"));
      break;
    case 0x02:
      Serial2.println(F(" 2"));
      break;
    case 0x03:
      Serial2.println(F(" 3"));
      break;
    case 0x04:
      Serial2.println(F(" 4"));
      break;
    case 0x05:
      Serial2.println(F(" 5"));
      break;
    case 0x06:
      Serial2.println(F(" 6"));
      break;
    case 0x07:
      Serial2.println(F(" seek > "));
      break;
    case 0x08:
      Serial2.println(F(" TP"));
      break;
    case 0x09:
      Serial2.println(F(" RDS"));
      break;
    case 0x0A:
      Serial2.println(F(" CPS"));
      break;
    case 0x0B:
      Serial2.println(F(" MODE"));
      break;
    case 0x0C:
      Serial2.println(F(" RD(ranodm ? )"));
      break;
    case 0x0D:
      Serial2.println(F(" << "));
      break;
    case 0x0E:
      Serial2.println(F(" FAD"));
      break;
    case 0x0F:
      Serial2.println(F(" BALANCE"));
      break;
    case 0x10:
      Serial2.println(F(" BASS"));
      break;
    case 0x11:
      Serial2.println(F(" AM"));
      break;
    case 0x12:
      Serial2.println(F(" Dolby"));
      break;
    case 0x13:
      Serial2.println(F(" >>"));
      break;
    case 0x14:
      Serial2.println(F(" TREB"));
      break;
    case 0x15:
      Serial2.println(F(" AS"));
      break;
    case 0x16:
      Serial2.println(F(" SCAN"));
      break;
    case 0x17:
      Serial2.println(F(" FM"));
      break;
    case 0x18:
      Serial2.println(F(" Seek < "));
      break;
    case 0x19:
      Serial2.println(F(" REV"));
      break;
    case 0x86:
      Serial2.println(F(" Knob + "));
      break;
    case 0x88:
      Serial2.println(F(" Knob - "));
      break;
    case 0x1E:
      Serial2.println(F(" Code in (TP + RDS)"));
      break;
    case 0x1F:
      Serial2.println(F("eject"));
      break;
    case 0x21:
      Serial2.println(F("button release"));
      break;
    default:
      Serial2.print(F(" uknown")); Serial2.println(data, HEX);
      break;
  }
}

