/* (C) T.Kovacik
 *  http://github.com/tomaskovacik/
 *  
 *  example of using VAGFISREader.h on STM32, arduino untested, yet
 *  
 */

#include <VAGFISReader.h>
//arduino
//#define RADIO_CLK 2
//#define RADIO_DATA 4
//#define RADIO_ENA 3
//stm32
#define RADIO_CLK PB0 //on EXTI0
#define RADIO_DATA PA1 //no interrupt attached to this pin only using digitalRead here
#define RADIO_ENA PB1 //on EXTI1

VAGFISReader radio_read(RADIO_CLK, RADIO_DATA, RADIO_ENA);
long last_update=0;
void setup() {
  radio_read.begin();
  Serial.begin(115200);
}

void loop() {
  if (radio_read.hasNewMsg()) {
    if (radio_read.msgIsNavi()) {
     for (uint8_t i = 2; i <radio_read.getSize()-1; i++) { //1st byte is msg ID, second one is packet size, last is checksumm so we skip them
        Serial.write(radio_read.readData(i));
    }
    } else {
    for (uint8_t i = 1; i <radio_read.getSize()-1; i++) { //1st byte is msg ID, last is checksumm
        Serial.write(radio_read.readData(i));
    }
    }
    Serial.println();
    radio_read.clearNewMsgFlag();
    last_update=millis();
  }

  if((millis()-last_update)>1000){
    radio_read.request();
    last_update=millis();
  }
}
