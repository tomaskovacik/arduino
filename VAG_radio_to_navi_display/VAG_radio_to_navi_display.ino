/*
 * VAG radio  message to navigation mode converter
 * for STM32(this one is tested only for now) so USE LEVEL SHIFTER!
 * if transistor based one is used put pull down resistor 
 * on enable line with HALF value of pull up on level shifter (check schematic of that shifter)
 * 
 */

#include <VAGFISWriter.h>
#include <VAGFISReader.h>

//arduino
#define RADIO_CLK 2
#define RADIO_DATA 4
#define RADIO_ENA 3
//stm32
//#define RADIO_CLK PB0 //on EXTI0
//#define RADIO_DATA PA1 //no interrupt attached to this pin only using digitalRead here
//#define RADIO_ENA PB1 //on EXTI1

// FIS
//arduino
#define FIS_CLK 5
#define FIS_DATA 6
#define FIS_ENA 7
//stm32
//#define FIS_CLK PB3
//#define FIS_DATA PB5
//#define FIS_ENA PA15

VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA, 1);
VAGFISReader radio_read(RADIO_CLK, RADIO_DATA, RADIO_ENA);

long last_fis_refresh = 0;
long last_radio_update = 0;

char radioBuffer[16];

void setup() {
  Serial.begin(115200);
  radio_read.begin();
  fisWriter.begin();
  fisWriter.initScreen(0x80, 0, 0, 1, 1);
  delay(2000);
  fisWriter.sendMsg(" RADIO   2 NAVI ");
  delay(2000);
  fisWriter.sendMsg("   BY     KOVO  ");
  delay(2000);
}


void loop() {
  if (radio_read.hasNewMsg()) {
    //we are using RADIO mode to NAVI mode, so msg from RADIO is probably not NAVI :), no need to check if it is or not
    for (uint8_t i = 1; i < radio_read.getSize() - 1; i++) { //1st byte is msg ID, last is checksumm
      radioBuffer[i - 1] = radio_read.readData(i);
      Serial.write(radioBuffer[i - 1]);
    }
    Serial.println();
    radio_read.clearNewMsgFlag();
    last_radio_update = millis();

  }

  if ((millis() - last_fis_refresh) > 100) {
    fisWriter.sendMsg(radioBuffer);
    last_fis_refresh = millis();
  }

  if ((millis() - last_radio_update) > 3000) {
    radio_read.ACK();
    last_radio_update = millis();
  }
}
