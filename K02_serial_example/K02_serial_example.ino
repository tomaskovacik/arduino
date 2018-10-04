/*
 * K02 exmaple
 * module has 5modes:
 * 
 * mode1 R19 not instaled: module wil send pulse on echo line after at least 10us pulse on trigger line (tested, works with external pulldown - 4k7)
 * 
 * mode2 R19 = 300k: module will send pulse on echo line after at least 10ms pulse on trigger line (tested, works with external pulldown - 4k7)
 * 
 * mode3 R19 = 120k(100k works): module will send serial data at 9600 each 100ms
 * data format:
 * 0xFF 
 * upper 8bit 
 * lower 8bit
 * check sum = ((upper+lower)&0xff)
 * 
 * mode3 R19 = 47k: module will send serial data at 9600  after receivind any data on RX line
 * data format:
 * 0xFF 
 * upper 8bit 
 * lower 8bit
 * check sum = ((upper+lower)&0xff)
 * 
 * mode5 R19 = 0: module will send continuesly asci data:
 * Gap=1234mm
 */

#include <SoftwareSerial.h>

SoftwareSerial K02(6, 7); // RX, TX

void setup()
{
  Serial.begin(115200);
  K02.begin(9600);

}

void loop()
{ 
  delay(100);
  //mySerial.write(0xFF); //for mode 4
  while(K02.available())
  {
    if (K02.read() == 0xFF ) { //start
      while(K02.available()<3){};//wait for all data to be buffered
      uint8_t upper_data = K02.read();
      uint8_t lower_data = K02.read();
      uint8_t sum = K02.read();
      if (((upper_data + lower_data) & 0xff) == sum) {
        Serial.print(F("distance: "));
        uint16_t distance = (upper_data << 8) | (lower_data & 0xff);
        Serial.print(distance / 10);
        Serial.println(" cm");
      }
    }
  }
}
