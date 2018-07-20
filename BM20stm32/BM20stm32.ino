#include "IS2020.h"

#define DEBUG 0
#define BTSerial Serial2
uint8_t verbose = 0, play = 0;

IS2020 BT(&BTSerial); // RX, TX

uint8_t selectedDevID = 0x00;
uint8_t ff = 0;
uint8_t rw = 0;

void info();


void setup() {

  //pinMode(12, OUTPUT);
  //digitalWrite(12, LOW);


  //pinMode(2, INPUT);
  // Open serial communications and wait for port to open:
  Serial3.begin(115200);
  Serial3.println("begin");
  // delay(1000);

  //  while (!Serial1) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  BT.begin(115200);
  BT.BTMstatusChanged = 1;
  //  delay(500);
  //  Serial3.println("Read_Linked_Device_Information 0");
  //  BT.Read_Linked_Device_Information(0, CRLDI_query_device_name);
  //  delay(500);
  //  Serial3.println("Read_Linked_Device_Information 1");
  //  BT.Read_Linked_Device_Information(1, CRLDI_query_device_name);
  //  delay(500);
  //  Serial3.println("Read_Paired_Device_Record");
  //  BT.Read_Paired_Device_Record();
}

void loop() { // run over and over
  //  Serial3.println("loop");
  //  delay(1000);
  if (Serial3.available() > 0)
  {
    // read the incoming byte:
    char c = Serial3.read();
    Serial3.println();
    switch (c)
    {
      case 'D':
        BT.Eeprom_to_defaults(0);
        break;
      case 'E':
        BT.enableAllSettingEvent();
        BT.Event_Mask_Setting();
        break;
      case 'e':
        BT.Event_Mask_Setting();
        break;
      case 'a':
        BT.Get_PB_By_AT_Cmd(0);
        break;
      case 'b':
        BT.Battery_status(0x00);
        BT.Battery_status(0x01);
        BT.getNextEventFromBT();
        Serial3.println(BT.maxBatteryLevel[0]);
        Serial3.println(BT.maxBatteryLevel[1]);
        Serial3.println(BT.currentBatteryLevel[0]);
        Serial3.println(BT.currentBatteryLevel[1]);
        break;
      case 'o':
        BT.AVC_Specific_Cmd();
        break;
      case 'd':
        //read pair device record
        BT.Read_Linked_Device_Information(0, CRLDI_query_device_name);
        delay(100);
        BT.getNextEventFromBT();
        BT.Read_Linked_Device_Information(1, CRLDI_query_device_name);
        delay(100);
        BT.getNextEventFromBT();
        BT.Read_Paired_Device_Record ();
        delay(100);
        BT.getNextEventFromBT();
        //        BT.IS2020::Read_Local_BD_Address();
        //        delay(100);
        //         BT.getNextEventFromBT();
        //        BT.Set_Access_PB_Method();
        //        delay(100);
        //        BT.getNextEventFromBT();
        BT.Read_Link_Status();
        delay(100);
        BT.getNextEventFromBT();
        break;
      case '=':
        // NEXT SONG
        BT.Next_song(0x00);
        break;
      case '-':
        // PREVIOUS SONG
        BT.Previous_song(0x00);
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
      case 's':
        Serial3.println("calling");
        // make call      1234567891234567892
        BT.MakeCall(0x00, "1234567891234567892");
        break;
      case 'l':
        BT.Read_Local_Device_Name();
        delay(100);
        BT.getNextEventFromBT();
        // list
        //BT.Query_call_list_info(0);
        //BT.Triger_to_query_call_list_info(0);
        break;
      case 'v': //verbose
        verbose = !verbose;
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
        Serial3.println("call 1234567891234567.. s");
        Serial3.println("                        l");
        Serial3.println("help                    h");
        Serial3.println("read dev info           d");
        break;
      case 'i': //info
        {
          info();
          break;
        }

    }

    BT.getNextEventFromBT();
    //delay(1000);

    if (BT.BTMstatusChanged) {
      BT.BTMstatusChanged = 0;

      //if (BT.deviceName[0][0] == 0x00)
      if (BT.device0Name == "")
        BT.Read_Linked_Device_Information(0, CRLDI_query_device_name);
      BT.getNextEventFromBT();
      //if (BT.deviceName[1][0] == 0x00)
      if (BT.device1Name == "")
        BT.Read_Linked_Device_Information(1, CRLDI_query_device_name);
        BT.getNextEventFromBT();
      // set to 1 in EVT_BTM_Status, when this event happen main program shoud read array BTMStatus[]


      info();
    }
    if (BT.callStatusChanged) {
      BT.callStatusChanged = 0;
      //set to 1 in case we are making/receiving call then check callStatus[]
      Serial3.print("Call status: dev0: "); Serial3.print(BT.callStatus[0], HEX); Serial3.print(" dev1: "); Serial3.println(BT.callStatus[1], HEX);
    }
    if (BT.SMSstatus[0]) {
      Serial3.print("Device ");
      Serial3.print(BT.SMSstatus[0]);
      Serial3.println(" has unreaded sms");
    }
    if (BT.SMSstatus[1]) {
      Serial3.print("Device ");
      Serial3.print(BT.SMSstatus[1]);
      Serial3.println(" has unreaded sms");
    }


  }

}

void info() {
  Serial3.print("local device name: "); Serial3.println(BT.LocalDeviceName);
  Serial3.println("Device state: "); Serial3.println(BT.ModuleState());

  Serial3.println("Device 0 Status: ");
  Serial3.println(BT.BtStatus(0x00));
  Serial3.println(BT.Connect_status(0x00));
  Serial3.print("Music Status: "); Serial3.println(BT.MusicStatus(0x00));
  Serial3.println(BT.StreamStatus(0x00));

  Serial3.println("Device 1 Status: ");
  Serial3.println(BT.BtStatus(0x01));
  Serial3.println(BT.Connect_status(0x01));
  Serial3.print("Music Status: "); Serial3.println(BT.MusicStatus(0x01));
  Serial3.println(BT.StreamStatus(0x01));

  Serial3.print("Battery level dev0: "); Serial3.print(BT.BatteryLevel(0x00)); Serial3.println("%");
  Serial3.print("Battery level dev1: "); Serial3.print(BT.BatteryLevel(0x01)); Serial3.println("%");

  Serial3.println();
  //  for (uint8_t i = 0; i < DEVICENAME_LENGHT_SUPPORT; i++) {
  //    if (BT.deviceName[0][i] != 0x00)
  //      Serial3.write(BT.deviceName[0][i]);
  //  }
  Serial3.print(BT.device0Name);

  Serial3.println(" info:");
  if (BT.deviceInBandRingtone[0])
    Serial3.print("Has in Band Rington.");
  if (BT.deviceIsiAP[0]) {
    Serial3.println("- iAP device");
  } else {
    Serial3.println("- SPP device");
  }

  Serial3.println();

  if (BT.deviceSupportAVRCPA13[0])
    Serial3.println("- AVRCP 1.3 supported"); //reply if remote device support AVRCP v1.3
  if (BT.deviceHfAndA2dpGain[0] != 0x00 ) {
    Serial3.print("A2DP Gain: ");
    Serial3.println(BT.deviceHfAndA2dpGain[0] << 4);
    Serial3.print("HF Gain: ");
    Serial3.println(BT.deviceHfAndA2dpGain[0] & 0x0F);
  }
  if ( BT.deviceLineInGain[0] != 0x00 ) {
    Serial3.print("Line in Gain: ");
    Serial3.println(BT.deviceLineInGain[0]);
  }

  //  for (uint8_t i = 0; i < DEVICENAME_LENGHT_SUPPORT; i++) {
  //    if (BT.deviceName[1][i] != 0x00)
  //      Serial3.write(BT.deviceName[1][i]);
  //  }
  Serial3.print(BT.device1Name);
  Serial3.println(" info:");
  if (BT.deviceInBandRingtone[1])Serial3.print("Has in Band Rington.");
  if (BT.deviceIsiAP[1]) {
    Serial3.println("- iAP device");
  } else {
    Serial3.println("- SPP device");
  }
  if (BT.deviceSupportAVRCPA13[1])
    Serial3.println("- AVRCP 1.3 supported"); //reply if remote device support AVRCP v1.3
  if ( BT.deviceHfAndA2dpGain[1] != 0x00 ) {
    Serial3.print("A2DP Gain: ");
    Serial3.println(BT.deviceHfAndA2dpGain[1] << 4);
    Serial3.print("HF Gain: ");
    Serial3.println(BT.deviceHfAndA2dpGain[1] & 0x0F);
  }
  if ( BT.deviceLineInGain[1] != 0x00 ) {
    Serial3.print("Line in Gain: ");
    Serial3.println(BT.deviceLineInGain[1]);
  }

  if (DEBUG) {
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
    Serial3.println("       UART version: ");
    Serial3.print("        flash version: "); Serial3.println((uint8_t)(BT.BTMUartVersion >> 13), HEX);
    Serial3.print("          rom version: "); Serial3.println((uint8_t)((BT.BTMUartVersion >> 8) & 0x1F), HEX);
    Serial3.print("    flash sub version: "); Serial3.println(((uint8_t)(BT.BTMUartVersion >> 4) & 0x0F), HEX);
    Serial3.print("flash control version: "); Serial3.println((uint8_t)(BT.BTMUartVersion & 0x0F), HEX);
    Serial3.println();
    Serial3.println("      BT FW version: ");
    Serial3.print("        flash version: "); Serial3.println((uint8_t)(BT.BTMFWVersion >> 13), HEX);
    Serial3.print("          rom version: "); Serial3.println((uint8_t)((BT.BTMFWVersion >> 8) & 0x1F), HEX);
    Serial3.print("    flash sub version: "); Serial3.println(((uint8_t)(BT.BTMFWVersion >> 4) & 0x0F), HEX);
    Serial3.print("flash control version: "); Serial3.println((uint8_t)(BT.BTMFWVersion & 0x0F), HEX);

  }

}



