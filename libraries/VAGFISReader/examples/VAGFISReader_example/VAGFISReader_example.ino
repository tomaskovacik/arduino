#include <VAGFISReader.h>

#define RADIO_CLK PB0
#define RADIO_DATA PA1
#define RADIO_ENA PB1

VAGFISReader radio_read(RADIO_CLK,RADIO_DATA,RADIO_ENA);

void setup() {
  digitalWrite(RADIO_ENA,LOW);

  // put your setup code here, to run once:
radio_read.init();
Serial.begin(115200);
}

void loop() {

if (radio_read.has_new_msg()){
  Serial.println("new msg");
  uint8_t grg=radio_read.read_data(0);
  Serial.print("pocet poli v data: ");Serial.println(grg);
  for (uint8_t i = 0;i<grg;i++){
    grg=radio_read.read_data(i);
    Serial.write(grg);
  }
  Serial.println();
  radio_read.clear_new_msg_flag();
}
}
