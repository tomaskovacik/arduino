#include "IS2020.h"
#include "AVRCP.h"
#include <Arduino.h>

/*
  was thinking using SErial event here, but based on comment in code serialEvent is called after loop() so calling function  in loop is the same
*/
uint8_t IS2020::getNextEventFromBT() {
  delay(100);
  //if (btSerial -> available()) {
  while (btSerial -> available() > 0) {
    //DBG(F("Get next Event from BT\n"));
    if (btSerial -> read() == 0xAA) { //start of event

      uint16_t packetSize = (btSerial -> read() << 8) | (btSerial -> read() & 0xff);

      uint8_t event[packetSize + 1]; //data+cksum

      for (uint8_t i = 0; i < packetSize + 1; i++) {
        event[i] = btSerial -> read();
      }

      if (checkCkeckSum(packetSize, event)) {
        //if (event[0] != EVT_Command_ACK) Event_ACK(event[0]);
        // DBG(F("Event from module: "));
        decodeEvent(event[0]); DBG(F("\n"));
        switch (event[0]) {
          /*
             Event Format: Event       Event Code  Event Parameters
                           Command ACK 0x00        cmd_id, status

             Description:

             Event Parameters:
                cmd_id       SIZE: 1 BYTE

                Value Parameter Description
                0xXX  the command id to ack

            status  SIZE: 1 BYTE
            Value Parameter Description
            0x00  command complete : BTM can handle this command.
            0x01  command disallow : BTM can not handle this command.
            0x02  unknow command
            0x03  parameters error
            0x04  "BTM is busy:
            This status is used for SPP data cannot be sent out in this moment because of ACL Tx buffer or RFCOMM credit issue. BTM will reply the ""Complete"" status once the SPP data can be processed.
            "
            0x05  "BTM memory is full:
            This status is used for SPP data cannot be sent out in this moment because of os heap memory is full. BTM will reply the ""Complete"" status once the SPP data can be processed. "
            others  RFD

          */
          case EVT_Command_ACK:
            {
              switch (event[2]) {
                case 0x00:
                  DBG(F("Command complete"));// : BTM can handle this command.");
                  break;
                case 0x01:
                  DBG(F("Command disallow"));// : BTM can not handle this command.");
                  break;
                case 0x02:
                  DBG(F("Unknow command"));
                  break;
                case 0x03:
                  DBG(F("Parameters error"));
                  break;
                case 0x04:
                  DBG(F("BTM is busy"));
                  break;
                case 0x05:
                  DBG(F("BTM memory is full"));
                  break;
              }
              DBG(F(": "));
              decodeCommand(event[1]);
              //DBG(" "+String(event[1], HEX));
              DBG(F("\n"));
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            BTM_Status  0x01  state, link_info

            Description:  This event is used to indicate about the BTM status. BTM will send it event while the status was changed.

            Event Parameters: state SIZE: 1 BYTE
            Value Parameter Description
            0x00  Power OFF state
            0x01  pairing state (discoverable mode)
            0x02  Power ON state
            0x03  pairing successful
            0x04  pairing fail
            0x05  HF link established
            0x06  A2DP link established
            0x07  HF link disconnected
            0x08  A2DP link disconnected
            0x09  SCO link connected
            0x0A  SCO link disconnected
            0x0B  AVRCP link established
            0x0C  AVRCP link disconnected
            0x0D  Standard SPP connected
            0x0E  Standard_SPP / iAP disconnected
            0x0F  Standby state
            0x10  iAP connected
            0x11  ACL disconnected

            link_info SIZE: 1 BYTE
            state:0x04
            Value Parameter Description
            0xXX  "For pairing not complete(0x04) case, this parameter indicate not completed reason
            0: time out
            1: fail

            "
            state:0x05,0x06,0x0B,0x0D
            Value Parameter Description
            0xXX  "this parameter indicate both link device and data base information.
            The format is shown as below:
            Bit7~4 : linked device id(0~7)
            Bit3~0 : linked data base(0 or 1)

            "
            state:0x07~0x08,0x0C,0x0E
            Value Parameter Description
            0xXX  "his parameter show the linked_data_base (0 or 1)


            "
            state:0x11
            Value Parameter Description
            0xXX  "0:disconnection
            1:link loss


            "

          */
          case EVT_BTM_Status:
            {
              BTMState = event[1];
              BTMlink_info = event[2];
              IS2020::Read_Link_Status();
              BTMstatusChanged = 1;
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Call_Status 0x02  data_base_index, call_status

            Description:  This event is used to indicate about the HF call status of BTM.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            call_status SIZE: 1 BYTE
            Value Parameter Description
            0x00  Idle
            0x01  voice dial
            0x02  incoming call
            0x03  outgoing call
            0x04  call active
            0x05  a call active with a call waiting
            0x06  a call active with a call hold

          */
          case EVT_Call_Status:
            {
              BTMstatusChanged = 1;
              callStatus[event[1]] = event[2];
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Caller_ID 0x03  data_base_index, num

            Description:  This event is used to indicate about the caller ID of the incoming call.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            num SIZE: N OCTETS
            Value Parameter Description
            0xXX..  caller Id or phone number

          */
          case EVT_Caller_ID:
            {
              BTMstatusChanged = 1;
              for (uint8_t i = 0; i < packetSize; i++) {
                callerId[event[1]][packetSize] = event[i + 2]; //no need to any sort of flag bit, cose we have callstatus change flag and in case of incoming call we just read this array
              }
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            SMS_Received_Indication 0x04  data_base_index, indication

            Description:  This event is used to indicate about the sms status that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            indication  SIZE: 1 BYTE
            Value Parameter Description
            0x00  no new sms received
            0x01  new sms received

          */
          case EVT_SMS_Received_Indication:
            {
              BTMstatusChanged = 1;
              SMSstatus[event[1]] = event[2];
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Missed_Call_Indication  0x05  data_base_index, information

            Description:  This event is used to indicate about the missed call indication that BTM received from mobile phone

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            information SIZE: 1 BYTE
            Value Parameter Description
            0xXX  reserved

          */
          case EVT_Missed_Call_Indication:
            {
              BTMstatusChanged = 1;
              missedCallStatus[event[1]] = event[2];
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Phone_Max_Battery_Level 0x06  data_base_index, battery_level

            Description:  This event is used to indicate about the mobile phone max battery level that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            battery_level SIZE: 1 BYTE
            Value Parameter Description
            0xXX  max battery level

          */
          case EVT_Phone_Max_Battery_Level:
            {
              BTMstatusChanged = 1;
              maxBatteryLevel[event[1]] = event[2];
              Serial3.println(F("================================="));
              Serial3.println(F("EVT_Phone_Max_Battery_Leveld"));
              Serial3.println(F("================================="));
              Serial3.println(event[2], DEC);
              Serial3.println(F("================================="));
              Serial3.println(F("================================="));
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Phone_Current_Battery_Level 0x07  data_base_index, battery_level

            Description:  This event is used to indicate about the mobile phone current battery level that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            battery_level SIZE: 1 BYTE
            Value Parameter Description
            0xXX  current battery level

          */
          case EVT_Phone_Current_Battery_Level:
            {
              BTMstatusChanged = 1;

              currentBatteryLevel[event[1]] = event[2];
              Serial3.println(F("================================="));
              Serial3.println(F("EVT_Phone_Current_Battery_Level: "));
              Serial3.println(F("================================="));
              Serial3.println(event[2], DEC);
              Serial3.println(F("================================="));
              Serial3.println(F("================================="));
            }
            break;
          /*
            Event Format:  Event Event Code  Event Parameters
            Roaming_Status  0x08  data_base_index, status

            Description:  This event is used to indicate about the roaming status that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            status  SIZE: 1 BYTE
            Value Parameter Description
            0x00  non-roaming
            0x01  roaming

          */
          case EVT_Roaming_Status:
            {
              BTMstatusChanged = 1;
              roamingStatus[event[1]] = event[2];
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Phone_Max_Signal_Strength_Level 0x09  data_base_index, signal_level

            Description:  This event is used to indicate about the max signal strength level that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            signal_level  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  phone's max signal strength level

          */
          case EVT_Phone_Max_Signal_Strength_Level:
            {
              BTMstatusChanged = 1;
              maxSignalLevel[event[1]] = event[2];
              Serial3.println(F("================================="));
              Serial3.println(F("EVT_Phone_Max_Signal_Strength_Level"));
              Serial3.println(F("================================="));
              Serial3.println(event[2], DEC);
              Serial3.println(F("================================="));
              Serial3.println(F("================================="));
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Phone_Current_Signal_Strength_Level 0x0A  data_base_index, signal_strength

            Description:  This event is used to indicate about the signal strength level that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            This event is used to indicate the MCU about the current signal strength level that BTM received from mobile phone. SIZE: 1 BYTE
            Value Parameter Description
            0xXX  phone's current signal strength level

          */
          case EVT_Phone_Current_Signal_Strength_Level:
            {
              BTMstatusChanged = 1;
              currentSignalLevel[event[1]] = event[2];
              Serial3.println(F("================================="));
              Serial3.println(F("EVT_Phone_Current_Signal_Strength_Level"));
              Serial3.println(F("================================="));
              Serial3.println(event[2], DEC);
              Serial3.println(F("================================="));
              Serial3.println(F("================================="));
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Phone_Service_Status  0x0B  data_base_index, service

            Description:  This event is used to indicate about the service status that BTM received from mobile phone.

            Event Parameters: data_base_index SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            service SIZE: 1 BYTE
            Value Parameter Description
            0x00  No service available
            0x01  service available

          */
          case EVT_Phone_Service_Status :
            {
              BTMstatusChanged = 1;
              serviceStatus[event[1]] = event[2];
            }
            break;
          /*
            Event Format: Event               Event Code  Event Parameters
                          BTM_Battery_Status  0x0C        battery_status, voltage_level

            Description:  This event is used to indicate about the BTM's battery status.

            Event Parameters: battery_status  SIZE: 1 BYTE
            Value Parameter Description
            0x00  dangerous level, and will auto shutdown
            0x01  low level
            0x02  normal level
            0x03  high level
            0x04  full level
            0x05  in charging
            0x06  charging complete

            voltage_level SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "0x00: 3.0V
            0x01: 3.1V
            0x02: 3.2V
            0x03: 3.3V
            0x04: 3.4V
            0x05: 3.5V
            0x06: 3.6V
            0x07: 3.7V
            0x08: 3.8V
            0x09: 3.9V
            0x0A: 4.0V
            0x0B: 4.1V
            0x0C: 4.2V"

          */
          case EVT_BTM_Battery_Status :
            {
              BTMstatusChanged = 1;
              BTMBatteryStatus[0] = event[1];
              BTMBatteryStatus[1] = event[2];
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            BTM_Charging_Status 0x0D  charger_status, dummy

            Description:

            Event Parameters: charger_status  SIZE: 1 BYTE
            Value Parameter Description
            0x00  No charger plug in
            0x01  in charging
            0x02  charging complete
            0x03  charging fail

            dummy SIZE: 1 BYTE
            Value Parameter Description
            0xXX  reserved

          */
          case EVT_BTM_Charging_Status:
            {
              BTMChargingStatus = event[1];
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Reset_To_Default  0x0E  dummy1,dummy2

            Description:  This event is used to indicate the BTM finish the Master Reset command for the MMI command (0x56) trigger.

            Event Parameters: dummy1  SIZE: 1 BYTE
            Value Parameter Description
            0x00

            dummy2  SIZE: 1 BYTE
            Value Parameter Description
            0x00

          */
          case EVT_Reset_To_Default:
            {
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Report_HF_Gain_Level  0x0F  database_index, level

            Description:  This is used to report the HF gain level set by remote Audio Gateway (Phone)

            Event Parameters:
            database_index  SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate HF/HS device
            0x01  database 1 for a dedicate HF/HS device

            level     SIZE: 1 BYTE
            Value Parameter Description
            0x00-0x0F gain level that sync. with HF device

          */
          case EVT_Report_HF_Gain_Level:
            {
              HFGainLevel[event[1]] = event[2];
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            EQ_Mode_Indication  0x10  mode, dummy

            Description:  This event is used to notice the EQ_mode setting by MMI or EQ_Mode_Setting command

            Event Parameters: mode  SIZE: 1 BYTE
            Value Parameter Description
            0x00  Off Mode
            0x01  Soft Mode
            0x02  Bass Mode
            0x03  Treble Mode
            0x04  Classical Mode
            0x05  Rock Mode
            0x06  Jazz Mode
            0x07  Pop Mode
            0x08  Dance Mode
            0x09  R&B Mode
            0x0A  User Mode 1

            dummy SIZE: 1 BYTE
            Value Parameter Description
            0xXX  reserved

          */
          case EVT_EQ_Mode_Indication:
            {
              EQMode = event[1];
            }
            break;
          case EVT_PBAP_Missed_Call_History:
            {
            }
            break;
          case EVT_PBAP_Received_Call_History:
            {
            }
            break;
          case EVT_PBAP_Dialed_Call_History:
            {
            }
            break;
          case EVT_PBAP_Combine_Call_History:
            {
            }
            break;
          case EVT_Get_PB_By_AT_Cmd_Reply:
            {
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            PBAP_Access_Finish  0x16  type, reserved

            Description:

            Event Parameters: type  SIZE: 1 BYTE
            Value Parameter Description
            0x0B  Retrieve miss call history records finish
            0x0C  Retrieve answered call history records finish
            0x0D  Retrieve dialed call history records finish
            0x0E  Retrieve combined call(missed, dialed,answered) history records finish
            0x0F  Retrieve phone book contacts finish


            reserved  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  reserved

          */
          case EVT_PBAP_Access_Finish:
            {
              PBAPAccessFinish = event[1];
            }
            break;
          /*
            Event Format: Event Event Code  Event Parameters
            Read_Linked_Device_Information_Reply  0x17  database_index, type, info

            Description:  This event is used to reply Read_Linked_Device_Information command.

            Event Parameters: database_index  SIZE: 1 BYTE
            Value Parameter Description
            0x00  database 0 for a dedicate link
            0x01  database 1 for a dedicate link

            type  SIZE: 1 BYTE
            Value Parameter Description
            0x00  reply device name
            0x01  reply in-band ringtone status
            0x02  reply if remote device is a iAP device
            0x03  reply if remote device support AVRCP v1.3
            0x04  reply HF&A2DP gain
            0x05  reply Line_In gain
            others  reserved

            info (for device name)  SIZE: N OCTETS
            Value Parameter Description
            0xXX… "N bytes bluetooth name with NULL terminated. (N <= 249 with NULL terminated)

            Note:If remote device response empty name string, then BTM will report name with data NULL terminate(0x00) only."

            info (for in-band ringtone status)  SIZE: 1 BYTE
            Value Parameter Description
            0x00  disable
            0x01  enable

            info (for spp type : iAP device or not) SIZE: 1 BYTE
            Value Parameter Description
            0x00  Standard SPP device
            0x01  iAP device

            info (for AVRCP type : AVRCP v1.3 device or not)  SIZE: 1 BYTE
            Value Parameter Description
            0x00  not support AVRCP v1.3
            0x01  support AVRCP v1.3

            info (for type 0x04)  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "bit[3:0]: A2DP gain
            bit[7:4]: HF gain"

            info (for type 0x05)  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  Line_In gain

          */
          case EVT_Read_Linked_Device_Information_Reply:
            {
              //DBG(F("Read_Linked_Device_Information_Reply\n"));
              BTMstatusChanged = 1;
              switch (event[2]) { //event[1] is device id
                case 0x00://reply device name
                  {
                    deviceName[event[1]] = "";
                    //DBG("Reply device name\n");
                    //N bytes bluetooth name with NULL terminated. (N <= 249 with NULL terminated)

                    for (uint8_t i = 3; i < /*DEVICENAME_LENGHT_SUPPORT + 2*/packetSize - 1; i++) { //event[2] is information type definition, last is checksum
                      //check if data are not NULL
                      if (event[i] == 0x00) break;
                      //deviceName[event[1]][i - 3] = event[i];
                      //Serial3.write(event[i]);
                      deviceName[event[1]] = deviceName[event[1]] + (char)event[i];

                    }
                  }
                  break;
                case 0x01://reply in-band ringtone status
                  /*
                    info (for in-band ringtone status) SIZE: 1 BYTE
                    Value Parameter Description
                    0x00  disable
                    0x01  enable
                  */
                  deviceInBandRingtone[event[1]] = event[3];
                  break;
                case 0x02://reply if remote device is a iAP device
                  /*
                    info (for spp type : iAP device or not) SIZE: 1 BYTE
                    Value Parameter Description
                    0x00  Standard SPP device
                    0x01  iAP device
                  */
                  deviceIsiAP[event[1]] = event[3];
                  break;
                case 0x03://reply if remote device support AVRCP v1.3
                  deviceSupportAVRCPA13[event[1]] = event[3];
                  break;
                case 0x04://reply HF&A2DP gain
                  /*
                    info (for type 0x04) SIZE: 1 BYTE
                    Value Parameter Description
                    0xXX  "bit[3:0]: A2DP gain
                    bit[7:4]: HF gain"
                  */
                  deviceHfAndA2dpGain[event[1]] = event[3];
                  break;
                case 0x05: //reply Line_In gain
                  /*
                    info (for type 0x05)  SIZE: 1 BYTE
                    Value Parameter Description
                    0xXX  Line_In gain
                  */
                  deviceLineInGain[event[1]] = event[3];
                  break;
                default:
                  if (DEBUG) {
                    Serial3.print("Reserved response: ");
                    Serial3.println(event[1], HEX);
                  }
              }
            }
            break;
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
          case EVT_Read_BTM_Version_Reply:
            {
              if (event[1] == 0x00) BTMUartVersion = (event[2] << 8) | (event[3] & 0xff);
              if (event[1] == 0x01) BTMFWVersion = (event[2] << 8) | (event[3] & 0xff);

            }
            break;
          case EVT_Call_List_Report:
            {
            }
            break;
          case EVT_AVRCP_Specific_Rsp:
            {
              //              if (DEBUG) {
              //                Serial3.println();
              //                Serial3.print("EVT_AVRCP_Specific_Rsp ");
              //              }
              //              //Serial3.println("data dump: ");

              for (uint8_t i = 0; i < packetSize; i++) {
                Serial3.print(event[0 + i], HEX);
                Serial3.print(" ");
              }
              Serial3.println();


              //Serial3.print("device ID: "); Serial3.println(event[1]);
              /* event[2] - Response: 0xC
                 event [3] - Subunit_type: 0x9 (PANEL), Subunit_ID: 0x0
                 event [4] - Opcode: 0x0 (VENDOR DEPENDENT)
                 event [5,6,7] = 0x00 0x19 0x58 - Company ID: Bluetooth SIG registered CompanyID
              */
              Serial3.print("PDU Id: "); Serial3.print(String(event[8], HEX) + " "); IS2020::decodeAVRCP_Pdu(event[8]);
              //              Serial3.print("packet type: ");
              //              switch (event[9])
              //              {
              //                case AVRCP_PACKET_TYPE_SINGLE:
              //                  Serial3.println("single packet");
              //                  break;
              //                case AVRCP_PACKET_TYPE_START:
              //                  Serial3.println("start packet");
              //                  break;
              //                case AVRCP_PACKET_TYPE_CONTINUING:
              //                  Serial3.println("continue packet");
              //                  break;
              //                case AVRCP_PACKET_TYPE_END:
              //                  Serial3.println("end packet");
              //                  break;
              //                default:
              //                  Serial3.println(event[3], HEX);
              //                  break;
              //              }
              uint16_t parameter_length =   (event[10] << 8) | (event[11] & 0xff);
              //DBG("parameter length: "+String(parameter_length,DEC));

              if (parameter_length == 1) {
                DBG(F("\nError: "));
                switch (event[12])
                {
                  case AVRCP_STATUS_INVALID_COMMAND:
                    {
                      DBG(F("INVALID COMMAND\n\n"));
                    }
                    break;
                  case AVRCP_STATUS_INVALID_PARAM:
                    {
                      DBG(F("INVALID PARAM\n\n"));
                    }
                    break;
                  case AVRCP_STATUS_PARAM_NOT_FOUND:
                    {
                      DBG(F("PARAM NOT FOUND\n\n"));
                    }
                    break;
                  case AVRCP_STATUS_INTERNAL_ERROR:
                    {
                      DBG(F("INTERNAL ERROR\n\n"));
                    }
                    break;
                }
                return false;
              }
              switch (event[8]) //event[9] si always 0x00 10+11=size of sending responce
              {
                case AVRCP_REGISTER_NOTIFICATION:
                  {
                    IS2020::decodeAVRCP_Event(event[12]);
                    switch (event[12])
                    {
                      case AVRCP_EVENT_STATUS_CHANGED://AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
                        {
                          /*
                            Description:
                            Indicates the current status of
                            playback

                            Values:
                            0x00: STOPPED
                            0x01: PLAYING
                            0x02: PAUSED
                            0x03: FWD_SEEK
                            0x04: REV_SEEK
                            0xFF: ERROR
                          */
                          switch (event[13]) {
                            case 0x00:
                              DBG(F("STOPPED\n"));
                              break;
                            case 0x01:
                              DBG(F("PLAYING\n"));
                              break;
                            case 0x02:
                              DBG(F("PAUSED\n"));
                              break;
                            case 0x03:
                              DBG(F("FWD_SEEK\n"));
                              break;
                            case 0x04:
                              DBG(F("REV_SEEK\n"));
                              break;
                            case 0xFF:
                              DBG(F("ERROR\n"));
                              break;
                          }
                        }
                        break;
                      case AVRCP_EVENT_TRACK_CHANGED:
                        {
                          /*Description:
                            Unique Identifier to identify an
                            element on TG, as is used for
                            GetElementAttribute command in
                            case Browsing is not supported
                            and GetItemAttribute command in
                            case Browsing is supported.

                            Values:
                            If a track is selected, then return 0x0
                            in the response. If no track is
                            currently selected, then return
                            0xFFFFFFFFFFFFFFFF in the
                            INTERIM response.

                            If Browsing is not supported and a
                            track is selected, then return 0x0 in
                            the response. If Browsing is
                            supported, then 0x0 is not allowed.

                            If Browsing is supported and a track
                            is selected, the identifier shall
                            correspond to the currently played
                            media element as listed in the
                            NowPlaying folder.
                          */
                          DBG("AVRCP_EVENT_TRACK_CHANGED responce parameter: ");
                          for (uint16_t parameter_byte = 14; parameter_byte < packetSize; parameter_byte++) {
                            DBG(String(event[parameter_byte], HEX));
                          }
                          DBG(F("\n"));
                          IS2020::AVRCP_Get_Element_Attributes(event[1]);
                        }
                        break;
                      case AVRCP_EVENT_TRACK_REACHED_END:
                        {

                        }
                        break;
                      case AVRCP_EVENT_TRACK_REACHED_START:
                        {
                        }
                        break;
                      case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
                        {
                          DBG("PLAYBACK_POS_CHANGED current playback position: ");

                          DBG(String(event[13] << 24 | event[14] << 16 | event[15] << 8 | event[16], DEC));
                          DBG(F("s\n"));
                        }
                        break;
                      case AVRCP_EVENT_BATT_STATUS_CHANGED:
                        {
                          /*
                            NOTE: Battery status notification defined in this specification is expected to be
                            deprecated in favor of Attribute profile specification in the future.
                          */
                          switch (event[13])
                          {
                              /*
                                0x0 – NORMAL – Battery operation is in normal state
                                0x1 – WARNING - unable to operate soon. Is provided when the battery level is going down.
                                0x2 – CRITICAL – Cannot operate any more. Is provided when the battery level is going down.
                                0x3 – EXTERNAL – Plugged to external power supply
                                0x4 - FULL_CHARGE – when the device is completely charged from the external power supply
                              */
                              DBG(F("Battery status: "));
                            case 0x00:
                              {
                                DBG(F("NORMAL\n"));
                              }
                              break;
                            case 0x01:
                              {
                                DBG(F("WARNING\n"));
                              }
                              break;
                            case 0x02:
                              {
                                DBG(F("CRITICAL\n"));
                              }
                              break;
                            case 0x03:
                              {
                                DBG(F("EXTERNAL\n"));
                              }
                              break;
                            case 0x04:
                              {
                                DBG(F("FULL_CHARGE\n"));
                              }
                              break;
                          }

                        }
                        break;
                      case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
                        {
                          DBG("PlayerId:" + String(event[13], HEX) + String(event[14], HEX) + "\n");
                          DBG("UID:" + String(event[15], HEX) + String(event[16], HEX) + "\n");
                        }
                        break;
                    }
                  }
                  break;
                case AVRCP_GET_CAPABILITIES: //event[12] determinate what capabilities we are receiving, 0x02 = company ID, 0x03=player caps.
                  {
                    if (event[12] == CAP_COMPANY_ID) {//company ID:
                      DBG(F("Company IDs:\n"));
                      Serial.println(event[13]);
                      for (uint8_t numCompID = 0; numCompID < event[13]; numCompID++) { //event[13] = CapabilityCount
                        for (uint8_t i = 0; i < 3; i++) {
                          DBG(String(event[14 + (numCompID * 3) + i], HEX));
                        }
                        DBG(F("\n"));
                      }
                    } else if (event[12] == CAP_EVENTS_SUPPORTED) {
                      for (uint16_t parameter_byte = 14; parameter_byte < (14 + event[13]); parameter_byte++) { //event[13] = CapabilityCount
                        IS2020::decodeAVRCP_Event(event[parameter_byte]);
                        if (event[parameter_byte] == AVRCP_EVENT_PLAYBACK_POS_CHANGED) {
                          IS2020::AVRCP_Registration_for_notification_of_Event(event[1], event[parameter_byte], 0x00, 0x00, 0x13, 0x88);
                        } else {
                          IS2020::AVRCP_Registration_for_notification_of_Event(event[1], event[parameter_byte], 0x00, 0x00, 0x00, 0x00);
                        }
                      }
                    } else {
                      DBG("Unknown capabilities ID! " + String(event[12], HEX) + "\n");
                    }
                  }
                  break;
                case AVRCP_LIST_PLAYER_ATTRIBUTES:
                  {
                    for (uint16_t parameter_byte = 13; parameter_byte < (13 + event[12]); parameter_byte++) {
                      switch (event[parameter_byte]) {
                        case AVRCP_ATTRIBUTE_EQUALIZER:
                          DBG(F("Equalizer\n"));
                          break;
                        case AVRCP_ATTRIBUTE_REPEAT_MODE:
                          DBG(F("Repeat\n"));
                          break;
                        case AVRCP_ATTRIBUTE_SHUFFLE:
                          DBG(F("Shuffle\n"));
                          break;
                        case AVRCP_ATTRIBUTE_SCAN:
                          DBG(F("Scan\n"));
                          break;
                      }
                    }
                    for (uint16_t parameter_byte = 13; parameter_byte < (13 + event[12]); parameter_byte++) {
                      IS2020::AVRCP_Get_Player_Attribute_Text(event[1], event[parameter_byte]);
                      IS2020::AVRCP_List_Player_Values(event[1], event[parameter_byte]);
                    }
                  }
                  break;
                case AVRCP_LIST_PLAYER_VALUES:
                  {
                    for (uint16_t parameter_byte = 13; parameter_byte < packetSize; parameter_byte++) {
                      switch (event[12]) { //ID
                        case AVRCP_ATTRIBUTE_ILEGAL:
                          {
                            DBG(F("AVRCP_ATTRIBUTE_ILEGAL"));
                          }
                          break;
                        case AVRCP_ATTRIBUTE_EQUALIZER:
                          {
                            DBG(F("AVRCP_ATTRIBUTE_EQUALIZER:\n"));
                            switch (event[parameter_byte])
                            {
                              case AVRCP_EQUALIZER_OFF:
                                {
                                  DBG(F("AVRCP_EQUALIZER_OFF\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_EQUALIZER, AVRCP_EQUALIZER_OFF);
                                }
                                break;
                              case AVRCP_EQUALIZER_ON:
                                {
                                  DBG(F("AVRCP_EQUALIZER_ON\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_EQUALIZER, AVRCP_EQUALIZER_ON);
                                }
                                break;
                            }
                          }
                          break;
                        case AVRCP_ATTRIBUTE_REPEAT_MODE:
                          {
                            DBG(F("AVRCP_ATTRIBUTE_REPEAT_MODE:\n"));
                            switch (event[parameter_byte])
                            {
                              case AVRCP_REPEAT_MODE_OFF:
                                {
                                  DBG(F("AVRCP_REPEAT_MODE_OFF\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_REPEAT_MODE, AVRCP_REPEAT_MODE_OFF);
                                }
                                break;
                              case AVRCP_REPEAT_MODE_SINGLE:
                                {
                                  DBG(F("AVRCP_REPEAT_MODE_SINGLE\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_REPEAT_MODE, AVRCP_REPEAT_MODE_SINGLE);
                                }
                                break;
                              case AVRCP_REPEAT_MODE_ALL:
                                {
                                  DBG(F("AVRCP_REPEAT_MODE_ALL\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_REPEAT_MODE, AVRCP_REPEAT_MODE_ALL);
                                }
                                break;
                              case AVRCP_REPEAT_MODE_GROUP:
                                {
                                  DBG(F("AVRCP_REPEAT_MODE_GROUP\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_REPEAT_MODE, AVRCP_REPEAT_MODE_GROUP);
                                }
                                break;
                            }
                          }
                          break;
                        case AVRCP_ATTRIBUTE_SHUFFLE:
                          {
                            DBG(F("AVRCP_ATTRIBUTE_SHUFFLE:\n"));
                            switch (event[parameter_byte])
                            {
                              case AVRCP_SHUFFLE_OFF:
                                {
                                  DBG(F("AVRCP_SHUFFLE_OFF\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SHUFFLE, AVRCP_SHUFFLE_OFF);
                                }
                                break;
                              case AVRCP_SHUFFLE_ALL:
                                {
                                  DBG(F("AVRCP_SHUFFLE_ALL\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SHUFFLE, AVRCP_SHUFFLE_ALL);
                                }
                                break;
                              case AVRCP_SHUFFLE_GROUP:
                                {
                                  DBG(F("AVRCP_SHUFFLE_GROUP\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SHUFFLE, AVRCP_SHUFFLE_GROUP);
                                }
                                break;
                            }
                          }
                          break;
                        case AVRCP_ATTRIBUTE_SCAN:
                          {
                            DBG(F("AVRCP_ATTRIBUTE_SCAN:\n"));
                            switch (event[parameter_byte])
                            {
                              case AVRCP_SCAN_OFF:
                                {
                                  DBG(F("AVRCP_SCAN_OFF\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SCAN, AVRCP_SCAN_OFF);
                                }
                                break;
                              case AVRCP_SCAN_ALL:
                                {
                                  DBG(F("AVRCP_SCAN_ALL\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SCAN, AVRCP_SCAN_ALL);
                                }
                                break;
                              case AVRCP_SCAN_GROUP:
                                {
                                  DBG(F("AVRCP_SCAN_GROUP\n"));
                                  IS2020::AVRCP_Get_Player_Value_Text(event[1], AVRCP_ATTRIBUTE_SCAN, AVRCP_SCAN_GROUP);
                                }
                                break;
                            }
                          }
                          break;
                      }
                    }
                  }
                  break;
                case AVRCP_GET_ELEMENT_ATTRIBUTES:
                  {
                    DBG(F("Number of events:")); DBG(String(event[12], HEX));
                  }
                  break;
                case AVRCP_GET_PLAY_STATUS:
                  {
                    DBG("Song length: " + String((event[12]<<24||event[13]<<16||event[14]<<8||event[15])/1000, DEC));
                    DBG("Song position: " + String((event[16]<<24||event[17]<<16||event[18]<<8||event[19])/1000, DEC));
                    DBG(F("Play status: "));
                    switch (event[20]) {
                    case 0x00:
                      DBG(F("STOPPED\n"));
                        break;
                      case 0x01:
                        DBG(F("PLAYING\n"));
                        break;
                      case 0x02:
                        DBG(F("PAUSED\n"));
                        break;
                      case 0x03:
                        DBG(F("FWD_SEEK\n"));
                        break;
                      case 0x04:
                        DBG(F("REV_SEEK\n"));
                        break;
                      case 0xFF:
                        DBG(F("ERROR\n"));
                        break;
                    }
                  }
                  break;
              }
            }
            break;
          case EVT_BTM_Utility_Req:
            {
            }
            break;
          case EVT_Vendor_AT_Cmd_Reply:
            {
            }
            break;
          case EVT_Report_Vendor_AT_Event:
            {
            }
            break;
          /*
            Event Format:  Event    Event Code     Event Parameters
            Read_Link_Status_Reply  0x1E           device_state,
                                                   database0_connect_status,
                                                   database1_connect_status,
                                                   database0_play_status,
                                                   database1_play_status,
                                                   database0_stream_status,
                                                   database1_stream_status,

            Description:  This event is used to reply the Read_Link_Status command.

            Event Parameters: device_state  SIZE: 1 BYTE
            Value Parameter Description
            0x00  Power OFF state
            0x01  pairing state (discoverable mode)
            0x02  standby state
            0x03  Connected state with only HF profile connected
            0x04  Connected state with only A2DP profile connected
            0x05  Connected state with only SPP profile connected
            0x06  Connected state with multi-profile connected

            database0_connect_status  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "1 indicate connected
            Bit0 : A2DP profile signaling channel connected
            Bit1 : A2DP profile stream channel connected
            Bit2 : AVRCP profile connected
            Bit3 : HF profile connected
            Bit4 : SPP connected"

            database1_connect_status  SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "1 indicate connected
            Bit0 : A2DP profile signaling channel connected
            Bit1 : A2DP profile stream channel connected
            Bit2 : AVRCP profile connected
            Bit3 : HF profile connected
            Bit4 : SPP connected"

            "database0_play_status,
            database1_play_status," SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "0x00:STOP
            0x01:PLAYING
            0x02:PAUSED
            0x03:FWD_SEEK
            0x04:REV_SEEK
            0x05:FAST_FWD
            0x06:REWIND
            0x07:WAIT_TO_PLAY
            0x08:WAIT_TO_PAUSE"

            "database0_stream_status,
            database1_stream_status," SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "1 indicate connected
            0x00: no stream
            0x01: stream on-going"

          */
          case EVT_Read_Link_Status_Reply:
            {

              for (uint8_t i = 0; i < 8; i++) {
                LinkStatus[i] = event[i + 1]; //link status is array of 7 bytes, like response is 7bytes.
              }
              if (LinkStatus[1] > 0) {
                IS2020::query_device_name(0x00);
                //IS2020::query_in_band_ringtone_status(0x00);
                //IS2020::query_if_remote_device_is_iAP_device(0x00);
                //IS2020::query_if_remote_device_support_AVRCP_v13(0x00);
                //IS2020::query_HF_A2DP_gain(0x00);
                //IS2020::query_Line_In_gain(0x00);
                IS2020::AVRCP_Displayable_Charset(0x00);
                // IS2020::AVRCP_Get_Capabilities(0x00, 0x02);
                IS2020::AVRCP_Get_Capabilities(0x00, 0x03);
                IS2020::AVRCP_List_Player_Attributes(0x00);
                //IS2020::RegisterAllEvents(0x00);
              } else {
                removeInfoAboutDevice(0);
              }
              if (LinkStatus[2] > 0) {
                IS2020::query_device_name(1);
                //IS2020::query_in_band_ringtone_status(0x01);
                //IS2020::query_if_remote_device_is_iAP_device(0x01);
                //IS2020::query_if_remote_device_support_AVRCP_v13(0x01);
                //IS2020::query_HF_A2DP_gain(0x01);
                //IS2020::query_Line_In_gain(0x01);
                IS2020::AVRCP_Displayable_Charset(0x01);
                //IS2020::AVRCP_Get_Capabilities(0x01, 0x02);
                IS2020::AVRCP_Get_Capabilities(0x01, 0x03);
                IS2020::AVRCP_List_Player_Attributes(0x01);
                // IS2020::RegisterAllEvents(0x01);
              } else {
                removeInfoAboutDevice(1);
              }
            }
            break;
          case EVT_Read_Paired_Device_Record_Reply:
            {
              /*
                Event Format:   Event                             Event Code    Event Parameters
                                Read_Paired_Device_Record_Reply   0x1F          paired_device_number, paired_record

                Description:  This event is used to reply the  Read_Paired_Device_Information command.

                Event Parameters: paired_device_number  SIZE: 1 BYTE
                Parameter Description
                byte0 the paired device number.

                paired_record : 7bytes per record SIZE: (7*total_record) BYTE
                Parameter Description
                byte0 link priority : 1 is the highest(newest device) and 4 is the lowest(oldest device)
                byte1~byte6 linked device BD address (6 bytes with low byte first)
                …
                 notes:
                 cmd event[0]
                 deviceid event[1]
                 bt_5 evetn[2]
                 bt_4 evetn[3]
                 bt_3 evetn[4]
                 bt_2 evetn[5]
                 bt-1 evetn[6]
                 bt_0 evetn[7]
              */
              //link priority
              btAddress[event[1]][0] = event[2];
              for (uint8_t i = 1; i < 7; i++) { // -- cose : byte1~byte6 linked device BD address (6 bytes with low byte first)
                //evet[3] = byte6, evet[4]=byte5 ....
                btAddress[event[1]][i] = event[7 - i]; // low byte first so 7,6,5,4,3,2,1 ? check in reality!
              }
              //BTMstatusChanged=1;
            }
            break;
          case EVT_Read_Local_BD_Address_Reply:
            {
              DBG("Local BT adr: ");
              for (uint8_t _byte = 0; _byte < 6; _byte++) {
                DBG(String(event[_byte], HEX));
                moduleBtAddress[5 - _byte] = event[_byte + 1];
                if (_byte > 1) DBG(":");
              }
            }
            break;
          case EVT_Read_Local_Device_Name_Reply:
            {
              IS2020::LocalDeviceName = "";
              //DBG("size: ");
              //DBG(String(event[1])+"\n");
              for (uint8_t _byte = 0; _byte < event[1]; _byte++) {
                //Serial3.write(event[_byte+2]);
                IS2020::LocalDeviceName += (char)event[_byte + 2];
              }
              //DBG("BT name: ");
              //DBG(LocalDeviceName);
            }
            break;
          case EVT_Report_SPP_iAP_Data:
            {
            }
            break;
          case EVT_Report_Link_Back_Status:
            {
            }
            break;
          case EVT_Ringtone_Finish_Indicate:
            {
            }
            break;
          case EVT_User_Confrim_SSP_Req:
            {
            }
            break;
          case EVT_Report_AVRCP_Vol_Ctrl:
            {
            }
            break;
          case EVT_Report_Input_Signal_Level:
            {
            }
            break;
          case EVT_Report_iAP_Info:
            {
            }
            break;
          case EVT_REPORT_AVRCP_ABS_VOL_CTRL:
            {
            }
            break;
          case EVT_Report_Voice_Prompt_Status:
            {
            }
            break;
          case EVT_Report_MAP_Data:
            {
            }
            break;
          case EVT_Security_Bonding_Res:
            {
            }
            break;
          case EVT_Report_Type_Codec:
            {
            }
            break;
          default:
            {
              Serial3.println();
              Serial3.print("Unknown  BYTE: ");
              for (uint8_t i = 0; i < packetSize; i++) {


                Serial3.print(event[i], HEX);
              }
              Serial3.println();
            }
        }
        return event[0];
      }
    }
  }
  return false;
}

void IS2020::decodeEvent(uint8_t Event) {
  switch (Event) {
    case 0x00:
      DBG(F("Command_ACK"));
      break;
    case 0x01:
      DBG(F("BTM_Status"));
      break;
    case 0x02:
      DBG(F("Call_Status"));
      break;
    case 0x03:
      DBG(F("Caller_ID"));
      break;
    case 0x04:
      DBG(F("SMS_Received_Indication"));
      break;
    case 0x05:
      DBG(F("Missed_Call_Indication"));
      break;
    case 0x06:
      DBG(F("Phone_Max_Battery_Level"));
      break;
    case 0x07:
      DBG(F("Phone_Current_Battery_Level"));
      break;
    case 0x08:
      DBG(F("Roaming_Status"));
      break;
    case 0x09:
      DBG(F("Phone_Max_Signal_Strength_Level"));
      break;
    case 0x0A:
      DBG(F("Phone_Current_Signal_Strength_Level"));
      break;
    case 0x0B:
      DBG(F("Phone_Service_Status"));
      break;
    case 0x0C:
      DBG(F("BTM_Battery_Status"));
      break;
    case 0x0D:
      DBG(F("BTM_Charging_Status"));
      break;
    case 0x0E:
      DBG(F("Reset_To_Default"));
      break;
    case 0x0F:
      DBG(F("Report_HF_Gain_Level"));
      break;
    case 0x10:
      DBG(F("EQ_Mode_Indication"));
      break;
    case 0x11:
      DBG(F("PBAP_Missed_Call_History"));
      break;
    case 0x12:
      DBG(F("PBAP_Received_Call_History"));
      break;
    case 0x13:
      DBG(F("PBAP_Dialed_Call_History"));
      break;
    case 0x14:
      DBG(F("PBAP_Combine_Call_History"));
      break;
    case 0x15:
      DBG(F("Get_PB_By_AT_Cmd_Reply"));
      break;
    case 0x16:
      DBG(F("PBAP_Access_Finish"));
      break;
    case 0x17:
      DBG(F("Read_Linked_Device_Information_Reply"));
      break;
    case 0x18:
      DBG(F("Read_BTM_Version_Reply"));
      break;
    case 0x19:
      DBG(F("Call_List_Report"));
      break;
    case 0x1A:
      DBG(F("AVRCP_Specific_Rsp"));
      break;
    case 0x1B:
      DBG(F("BTM_Utility_Req"));
      break;
    case 0x1C:
      DBG(F("Vendor_AT_Cmd_Reply"));
      break;
    case 0x1D:
      DBG(F("Report_Vendor_AT_Event"));
      break;
    case 0x1E:
      DBG(F("Read_Link_Status_Reply"));
      break;
    case 0x1F:
      DBG(F("Read_Paired_Device_Record_Reply"));
      break;
    case 0x20:
      DBG(F("Read_Local_BD_Address_Reply"));
      break;
    case 0x21:
      DBG(F("Read_Local_Device_Name_Reply"));
      break;
    case 0x22:
      DBG(F("Report_SPP_iAP_Data"));
      break;
    case 0x23:
      DBG(F("Report_Link_Back_Status"));
      break;
    case 0x24:
      DBG(F("Ringtone_Finish_Indicate"));
      break;
    case 0x25:
      DBG(F("User_Confrim_SSP_Req"));
      break;
    case 0x26:
      DBG(F("Report_AVRCP_Vol_Ctrl"));
      break;
    case 0x27:
      DBG(F("Report_Input_Signal_Level"));
      break;
    case 0x28:
      DBG(F("Report_iAP_Info"));
      break;
    case 0x29:
      DBG(F("REPORT_AVRCP_ABS_VOL_CTRL"));
      break;
    case 0x2A:
      DBG(F("Report_Voice_Prompt_Status"));
      break;
    case 0x2B:
      DBG(F("Report_MAP_Data"));
      break;
    case 0x2C:
      DBG(F("Security_Bonding_Res"));
      break;
    case 0x2D:
      DBG(F("Report_Type_Codec"));
      break;
  }
}
