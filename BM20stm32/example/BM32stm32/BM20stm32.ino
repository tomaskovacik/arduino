#include "IS2020.h"

#define DEBUG 0
#define RESET PA1
#define BTSerial Serial2 //Serial2 => TX = PA2, RX = PA3

IS2020 BT(&BTSerial);

uint8_t selectedDevID = 0x00;
uint8_t ff = 0;
uint8_t rw = 0;
uint8_t repeat_mode = 0x01;

void device_info(uint8_t dMeviceId);
void module_info();


void setup() {
  // Open serial communications and wait for port to open:
  Serial3.begin(115200); //TX = PB10, RX=PB11
  Serial3.println("begin");
  // delay(1000);

  //  while (!Serial3) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  BT.begin(115200, RESET);
  BT.BTMstatusChanged = 1;
  BT.Read_Local_Device_Name();
  BT.Read_Local_BD_Address();
  BT.Read_BTM_Version();
  BT.Read_Link_Status();
}

void loop() { // run over and over
  //  Serial3.println("loop");
  //  delay(1000);
  if (Serial3.available() > 0)
  {
    // read the incoming byte:
    char c = Serial3.read();
    switch (c)
    {
      case 'a':
        BT.AVRCP_Get_Play_Status(0);
        break;
      case 'b':
        if (BT.LinkStatus[1] > 0) {
          BT.BatteryStatus(0x00);
          BT.getNextEventFromBT();
          Serial3.println(BT.maxBatteryLevel[0]);
          Serial3.println(BT.currentBatteryLevel[0]);
        }
        if (BT.LinkStatus[2] > 0) {
          BT.BatteryStatus(0x00);
          Serial3.println(BT.maxBatteryLevel[1]);
          Serial3.println(BT.currentBatteryLevel[1]);
        }
        break;
      case 'e':
        BT.Event_Mask_Setting();
        break;
      case 'o':
        break;
      case 'd':
        BT.Read_Local_Device_Name();
        BT.Read_Local_BD_Address();
        BT.Read_BTM_Version();
        BT.Read_Link_Status();
        BT.Read_Paired_Device_Record();
        break;
      case 'n':
        BT.query_device_name(0x00);
        break;
      case '=':
        // NEXT SONG
        BT.Next_song(0x00);
        break;
      case '-':
        // PREVIOUS SONG
        BT.Previous_song(0x00);
        break;
      case 'D':
        BT.Eeprom_to_defaults(0);
        break;
      case 'E':
        BT.enableAllSettingEvent();
        BT.Event_Mask_Setting();
        break;

      case 'P':
        // seek rewind
        BT.PLAY(0x00);
        break;
      case 'p':
        // PLAY/PAUSE
        BT.TOGLE_PLAY_PAUSE(0x00);

        break;
      // seek forward            f
      // seek rewind             r
      // scan mode               s
      // shuffle mode            h
      case 'f':
        BT.FFW(0x00);
        break;
      case 'F':
        //  seek frward
        if (!ff) {
          ff = 1;
          BT.REPFFW(0x00);
        } else {
          ff = 0;
          BT.STOP_FFW_RWD(0x00);
        }
        break;
      case 'r':
        // seek rewind
        BT.RWD(0x00);
        break;
      case 'A':
        BT.AVRCP_Get_Element_Attributes_All(0x00);
        break;
      case 'R':
        if (!rw) {
          rw = 1;
          BT.REPRWD(0x00);
        } else {
          rw = 0;
          BT.STOP_FFW_RWD(0x00);
        }
        break;
      case 'S':
        // seek rewind
        BT.STOP(0x00);
        break;
      case 'c':
        {
          delay(1000);
          char pn[19];
          uint8_t i = 0;
          while (Serial3.available() > 0) {
            pn[i++] = Serial3.read();
          }
          BT.MakeCall(0x00, pn);
        }
        break;
      case 'l':
        if (BT.Read_Local_Device_Name()) Serial3.print("local device name: "); Serial3.println(BT.LocalDeviceName);
        break;
      case 'L':
        {
          delay(1000);
          String str;
          c = 0;
          while (Serial3.available() > 0) {
            c = Serial3.read();
            str += c;
          }
          BT.Change_Device_Name(str);
          if (BT.Read_Local_Device_Name()) Serial3.print("local device name: "); Serial3.println(BT.LocalDeviceName);
        }
        break;
      case 'h': //help
        Serial3.println("battery status          b");
        Serial3.println("Enable defualt Events   e");
        Serial3.println("Enable All Events       E");
        Serial3.println("next track button       =");
        Serial3.println("previous track button   -");
        Serial3.println("play/pause               p");
        Serial3.println("PLAY                     P");
        Serial3.println("STOP               S");
        Serial3.println("seek forward/repeat ff f/F");
        Serial3.println("seek rewind/repeate rw r/R");
        Serial3.println("reset module q");
        Serial3.println("call c");
        Serial3.println("                        l");
        Serial3.println("help                    h");
        Serial3.println("read dev info           d");
        Serial3.println("Eeprom_to_defaults      D");
        Serial3.println("read info about connected phone      n");
        Serial3.println("link info I");
        Serial3.println("Switch_primary_seconday_HF C");
        break;
      case 'i': //info
        {
          module_info();
          device_info(0);
          device_info(1);
        }
        break;
      case 'C':
        {
          BT.Switch_primary_seconday_HF(1);
        }
        break;
      case 'I':
        {
          BT.Read_BTM_Version();

        }
        break;
      case 'q':
        BT.resetModule();
      break;
    }
  }

  BT.getNextEventFromBT();
  if (BT.BTMstatusChanged) {
    //    module_info();
    //    device_info(0);
    //    device_info(1);
    BT.BTMstatusChanged = 0;
  }
}

void module_info() {
  Serial3.print(F("Module info:"));
  Serial3.print(F("BT module name: ")); Serial3.println(BT.LocalDeviceName);
  Serial3.print(F("BT module addr: "));
  for (uint8_t _byte = 0; _byte < 6; _byte++) {
    Serial3.print(BT.moduleBtAddress[_byte], HEX);
    if (_byte < 5) Serial3.print(":");
  }
  Serial3.println();
  Serial3.println(F("Paired devices: "));
  for (uint8_t dev = 1; dev < 8; dev++) {
    Serial3.print(dev); Serial3.print(". ");
    for (uint8_t _byte = 0; _byte < 7; _byte++) {
      Serial3.print(BT.btAddress[dev][_byte], HEX);
      if (_byte < 5) Serial3.print(":");
    }
    Serial3.println();
  }
  Serial3.println();
  Serial3.println(BT.ModuleState());
  Serial3.println(BT.BtStatus());

  /*
    Event Format: Event Event Code  Event Parameters
    Read_BTM_Version_Reply  0x18  type, version

    Description:

    Event Parameters: type  SIZE: 1 BYTE
    Value Parameter Description
    0x00  uart version
    0x01  BTM FW version

    version SIZE: 2 Octets
    Value Parameter Description
    0xXXYY
    1st byte bit[7:5]: flash version
    1st byte bit[4:0]: rom version
    2nd byte bit[7:4] : flash sub version
    2nd byte bit[3:0] : flash control version
    for example 00 07 means version 0.07

  */
  Serial3.println();
  Serial3.println(F("       UART version: ")); Serial3.println(BT.BTMUartVersion, HEX);
  Serial3.print(F("        flash version: ")); Serial3.println((uint8_t)(BT.BTMUartVersion >> 13), HEX);
  Serial3.print(F("          rom version: ")); Serial3.println((uint8_t)((BT.BTMUartVersion >> 8) & 0x1F), HEX);
  Serial3.print(F("    flash sub version: ")); Serial3.println(((uint8_t)(BT.BTMUartVersion >> 4) & 0x0F), HEX);
  Serial3.print(F("flash control version: ")); Serial3.println((uint8_t)(BT.BTMUartVersion & 0x0F), HEX);
  Serial3.println();
  Serial3.println(F("      BT FW version: ")); Serial3.println(BT.BTMFWVersion >> 13, HEX);
  Serial3.print(F("        flash version: ")); Serial3.println((uint8_t)(BT.BTMFWVersion >> 13), HEX);
  Serial3.print(F("          rom version: ")); Serial3.println((uint8_t)((BT.BTMFWVersion >> 8) & 0x1F), HEX);
  Serial3.print(F("    flash sub version: ")); Serial3.println(((uint8_t)(BT.BTMFWVersion >> 4) & 0x0F), HEX);
  Serial3.print(F("flash control version: ")); Serial3.println((uint8_t)(BT.BTMFWVersion & 0x0F), HEX);

}

void device_info(uint8_t deviceId) {
  Serial3.println(F("================================="));
  Serial3.print(F("Device ")); Serial.print(deviceId); Serial3.println(F("info:"));
  Serial3.println(BT.ConnectionStatus(deviceId));
  Serial3.print("Name: "); Serial3.println(BT.deviceName[deviceId]);
  Serial3.print("Music Status: "); Serial3.println(BT.MusicStatus(deviceId));
  Serial3.println(BT.StreamStatus(deviceId));
  Serial3.print("Battery level dev0: "); Serial3.print(BT.currentBatteryLevel[deviceId]); Serial3.print("/"); Serial3.println(BT.maxBatteryLevel[deviceId]);
  Serial3.print("current signal strength : "); Serial3.print(BT.currentSignalLevel[deviceId]); Serial3.print("/"); Serial3.println(BT.maxSignalLevel[deviceId]);
  //    Serial3.println(" info:");
  //    if (BT.deviceInBandRingtone[deviceId])
  //      Serial3.print("Has in Band Rington.");
  //    if (BT.deviceIsiAP[deviceId]) {
  //      Serial3.println("- iAP device");
  //    } else {
  //      Serial3.println("- SPP device");
  //    }
  //
  //    if (BT.deviceSupportAVRCPA13[deviceId])
  //      Serial3.println("- AVRCP 1.3 supported"); //reply if remote device support AVRCP v1.3
  //    if (BT.deviceHfAndA2dpGain[deviceId] != 0x00 ) {
  //      Serial3.print("A2DP Gain: ");
  //      Serial3.println(BT.deviceHfAndA2dpGain[deviceId] << 4);
  //      Serial3.print("HF Gain: ");
  //      Serial3.println(BT.deviceHfAndA2dpGain[deviceId] & 0x0F);
  //    }
  //    if ( BT.deviceLineInGain[deviceId] != 0x00 ) {
  //      Serial3.print("Line in Gain: ");
  //      Serial3.println(BT.deviceLineInGain[deviceId]);
  //    }
  //  }


}



