#include <M93Cx6.h>

/******************************************************************************
 **
 **  read symphony II eeprom
 **   - extract code
 **   - fix lock(SAFE2)
 **   - set to nolock
 **  based on M93Cx6 Test Program
 **
 **  Pin-Out:
 **                       _____
 **  Chip Select (cs)  --|1   8|--  (pwr) Vcc
 ** Serial Clock (sk)  --|2   7|--
 **      Data In (di)  --|3   6|--  (org) Organization Select
 **     Data Out (do)  --|4   5|--  (gnd) Vss/Ground
 **                       -----
 **
 **  Arduino Connection:
 **    Vcc          - +5v (or Pin 9 as an option)
 **    Vss          - GND
 **    Chip Select  - Pin 10
 **    Serial Clock - Pin 13
 **    Data In      - Pin 11
 **    Data Out     - Pin 12
 **    Org Select   - Pin 8
 **
 *****************************************************************************/


#define PWR_PIN   7
#define CS_PIN    10
#define SK_PIN    13
#define DO_PIN    12
#define DI_PIN    11
#define ORG_PIN   8
#define ORG       16
#define CHIP      56

M93Cx6 eeprom = M93Cx6(PWR_PIN, CS_PIN, SK_PIN, DO_PIN, DI_PIN, ORG_PIN);

void setup() {
  int i = 0;
  int c = 0;
  Serial.begin(9600);
  eeprom.setChip(66);    // set chip 93C56
  eeprom.setOrg(ORG_16);  // 8-bit data organization
  //eeprom.writeEnable();
  //eeprom.write(1, 0x01);
  //eeprom.writeDisable();
  int a = 0;

  Serial.print("code: "); Serial.print(highByte(eeprom.read(1)), HEX); Serial.println(lowByte(eeprom.read(1)), HEX);
  if (eeprom.read(2) > 1) {
    Serial.println("LOCKED,unlocking: ");
    eeprom.writeEnable();
    eeprom.write(2, 1);
    eeprom.writeDisable();

    if (eeprom.read(2) == 1) {
      Serial.println("SUCCESS!");
    }
  }
  if (eeprom.read(2) == 1)
    Serial.println("UNLOCKED.");

  // dump whole eeprom
//  for (i = 0; i < 256; i++) {
//    c = eeprom.read(i);
//    Serial.print(lowByte(c), HEX);
//    Serial.print(" ");
//    Serial.print(highByte(c), HEX);
//     Serial.print(" ");
//    a++;
//    if (a == 8) {
//      a = 0;
//      Serial.println();
//    }
//  }


}

void loop() {
  // put your main code here, to run repeatedly:

}
