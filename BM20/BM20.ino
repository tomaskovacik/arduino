#include "IS2020.h"
#include "MMI.h"
#include "Music.h"
#include "commands.h"
//#include "events.h"
#define DEBUG 1

uint8_t verbose = 0, play = 0;

IS2020 BT(10, 11); // RX, TX

uint8_t selectedDevID = 0x00;

void setup() {
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  pinMode(2, INPUT);
  // Open serial communications and wait for port to openM:
  Serial1.begin(57600);

  Serial1.println("starting up ..");
  BT.init(115200);
  
//  delay(500);
//  Serial1.println("Read_Linked_Device_Information 0");
//  BT.Read_Linked_Device_Information(0, CRLDI_query_device_name);
//  delay(500);
//  Serial1.println("Read_Linked_Device_Information 1");
//  BT.Read_Linked_Device_Information(1, CRLDI_query_device_name);
//  delay(500);
//  Serial1.println("Read_Paired_Device_Record");
//  BT.Read_Paired_Device_Record();
}

void loop() { // run over and over
  if (Serial1.available() > 0)
  {
    
    // read the incoming byte:
    switch (Serial1.read())
    {
      case 'd':
        //read pair device record
        BT.Read_Paired_Device_Record ();
        break;
      case '=':
        // NEXT SONG
        BT.MMI_Action(0x00, next_song);
        break;
      case ']':
        // NEXT CD

        break;
      case '[':
        // PREVIOUS CD

        break;
      case '-':
        // PREVIOUS SONG
        BT.MMI_Action(0x00, previous_song);    
        break;
      case 'p':
        // PLAY/PAUSE
        play = !play;
        if (play) {
          BT.Music_Control(0x00, PAUSE);
        }
        else
        {
          BT.Music_Control(0x00, PLAY);
        }
        break;
      // seek forward            f
      // seek rewind             r
      // scan mode               s
      // shuffle mode            h
      case 'f':
        //  seek frward
        BT.Music_Control(0x00, PLAY);
        break;
      case 'r':
        // seek rewind

        break;
      case 's':
        Serial1.println("calling");
        // make call      1234567891234567892
        BT.MakeCall(0x00, "1234567891234567892");
        break;
      case 'l':
        // list
        BT.MMI_Action(0x00, query_call_list_info);
        BT.MMI_Action(0x00, triger_to_query_call_list_info);
        break;
      case 'v': //verbose
        verbose = !verbose;
        break;
      case 'h': //help
        Serial1.println("next track button       =");
        Serial1.println("previous track button   -");
        Serial1.println("next CD                 ]");
        Serial1.println("previous CD             [");
        Serial1.println("play/stop               p");
        Serial1.println("seek forward            f");
        Serial1.println("seek rewind             r");
        Serial1.println("call 1234567891234567891              s");
        Serial1.println("shuffle mode            l");
        Serial1.println("help                    h");
        break;
      case 'i': //info
        {

          Serial1.print("Battery level MAX dev0: "); Serial1.print(BT.maxBatteryLevel[0]); Serial1.print(" dev1: "); Serial1.println(BT.maxBatteryLevel[1]);

          Serial1.print("Battery level CURRENT dev0: "); Serial1.print(BT.currentBatteryLevel[0]); Serial1.print(" dev1: "); Serial1.println(BT.currentBatteryLevel[1]);
          Serial1.println();
          Serial1.println();
          for (uint8_t i = 0; i < DEVICENAME_LENGHT_SUPPORT; i++) {
            if (BT.deviceName[0][i] != 0x00)
              Serial1.write(BT.deviceName[0][i]);
          }
          Serial1.println(" info:");
          if (BT.deviceInBandRingtone[0])Serial1.print("Has in Band Rington.");
          if (BT.deviceIsiAP[0]) {
            Serial1.println("- iAP device");
          } else {
            Serial1.println("- SPP device");
          }
          if (BT.deviceSupportAVRCPA13[0])
            Serial1.println("- AVRCP 1.3 supported"); //reply if remote device support AVRCP v1.3
          if (BT.deviceHfAndA2dpGain[0] != 0x00 ) {
            Serial1.print("A2DP Gain: ");
            Serial1.println(BT.deviceHfAndA2dpGain[0] << 4);
            Serial1.print("HF Gain: ");
            Serial1.println(BT.deviceHfAndA2dpGain[0] & 0x0F);
          }
          if ( BT.deviceLineInGain[0] != 0x00 ) {
            Serial1.print("Line in Gain: ");
            Serial1.println(BT.deviceLineInGain[0]);
          }

          for (uint8_t i = 0; i < DEVICENAME_LENGHT_SUPPORT; i++) {
            if (BT.deviceName[1][i] != 0x00)
              Serial1.write(BT.deviceName[1][i]);
          }
          Serial1.println(" info:");
          if (BT.deviceInBandRingtone[1])Serial1.print("Has in Band Rington.");
          if (BT.deviceIsiAP[1]) {
            Serial1.println("- iAP device");
          } else {
            Serial1.print("- SPP device");
          }
          if (BT.deviceSupportAVRCPA13[1])
            Serial1.println("- AVRCP 1.3 supported"); //reply if remote device support AVRCP v1.3
          if ( BT.deviceHfAndA2dpGain[1] != 0x00 ) {
            Serial1.print("A2DP Gain: ");
            Serial1.println(BT.deviceHfAndA2dpGain[1] << 4);
            Serial1.print("HF Gain: ");
            Serial1.println(BT.deviceHfAndA2dpGain[1] & 0x0F);
          }
          if ( BT.deviceLineInGain[1] != 0x00 ) {
            Serial1.print("Line in Gain: ");
            Serial1.println(BT.deviceLineInGain[1]);
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
            Serial1.println();
            Serial1.println("       UART version: ");
            Serial1.print("        flash version: "); Serial1.println((uint8_t)(BT.BTMUartVersion >> 13), HEX);
            Serial1.print("          rom version: "); Serial1.println((uint8_t)((BT.BTMUartVersion >> 8) & 0x1F), HEX);
            Serial1.print("    flash sub version: "); Serial1.println(((uint8_t)(BT.BTMUartVersion >> 4) & 0x0F), HEX);
            Serial1.print("flash control version: "); Serial1.println((uint8_t)(BT.BTMUartVersion & 0x0F), HEX);
            Serial1.println();
            Serial1.println("      BT FW version: ");
            Serial1.print("        flash version: "); Serial1.println((uint8_t)(BT.BTMFWVersion >> 13), HEX);
            Serial1.print("          rom version: "); Serial1.println((uint8_t)((BT.BTMFWVersion >> 8) & 0x1F), HEX);
            Serial1.print("    flash sub version: "); Serial1.println(((uint8_t)(BT.BTMFWVersion >> 4) & 0x0F), HEX);
            Serial1.print("flash control version: "); Serial1.println((uint8_t)(BT.BTMFWVersion & 0x0F), HEX);

          }

        }
        break;
    }

  }
 BT.getNextEventFromBT();
  if (BT.BTMstatusChanged) {
    BT.BTMstatusChanged = 0;

    if (BT.deviceName[0][0] == 0x00)
      BT.Read_Linked_Device_Information(0, CRLDI_query_device_name);
    delay(500);
    if (BT.deviceName[1][0] == 0x00)
      BT.Read_Linked_Device_Information(1, CRLDI_query_device_name);
    // set to 1 in EVT_BTM_Status, when this event happen main program shoud read array BTMStatus[]
    Serial1.print("BTM status: dev0: "); Serial1.print(BT.BTMStatus[0], HEX); Serial1.print(" dev1: "); Serial1.println(BT.BTMStatus[1], HEX);
  }

  if (BT.callStatusChanged) {
    BT.callStatusChanged = 0;
    ;//set to 1 in case we are making/receiving call then check callStatus[]
    Serial1.print("Call status: dev0: "); Serial1.print(BT.callStatus[0], HEX); Serial1.print(" dev1: "); Serial1.println(BT.callStatus[1], HEX);
  }

  if (BT.SMSstatus[0]) {
    Serial1.print("Device ");
    Serial1.print(BT.SMSstatus[0]);
    Serial1.println(" has unreaded sms");
  }

  if (BT.SMSstatus[1]) {
    Serial1.print("Device ");
    Serial1.print(BT.SMSstatus[1]);
    Serial1.println(" has unreaded sms");
  }


}
