#include "IS2020.h"
#include <Arduino.h>

/*
  was thinking using SErial event here, but based on comment in code serialEvent is called after loop() so calling function  in loop is the same
*/
uint8_t IS2020::getNextEventFromBT() {

  if (btHwSerial -> available()) {
    // if(DEBUG) Serial3.println("get next Event from BT");

    if (btHwSerial -> read() == 0xAA) { //start of event

      uint16_t packetSize = (btHwSerial -> read() << 8) | (btHwSerial -> read() & 0xff);

      uint8_t event[packetSize + 1];

      for (uint8_t i = 0; i < packetSize + 1; i++) {
        event[i] = btHwSerial -> read();
      }

      if (checkCkeckSum(packetSize, event)) {
        Event_ACK(event[0]);
        decodeEvent(event[0]);
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
            { ;
              switch (event[2]) {
                case 0x00:
                  DBG("Command complete");// : BTM can handle this command.");
                  break;
                case 0x01:
                  DBG("Command disallow");// : BTM can not handle this command.");
                  break;
                case 0x02:
                  DBG("Unknow command");
                  break;
                case 0x03:
                  DBG("Parameters error");
                  break;
                case 0x04:
                  DBG("BTM is busy");
                  break;
                case 0x05:
                  DBG("BTM memory is full");
                  break;
              }
              DBG(": ");
              DBG(decodeCommand(event[1]));
              //DBG((String) event[1]);
              Serial3.print(event[1], HEX);
              DBG("\n");
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
              BTMstatusChanged = 1;
              BTMStatus[0] = event[1];
              BTMStatus[1] = event[2];
              BTMstatusChanged = 1; //there is more to do with data but not here, we do not have time ...
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
              DBG("current Phone batery");
              DBG((String) event[2]);
              currentBatteryLevel[event[1]] = event[2];
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
              DBG("EVT_Read_Linked_Device_Information_Reply\n");

              switch (event[2]) { //event[1] is device id
                case 0x00://reply device name
                  device0Name = "";
                  device1Name = "";
                  DBG("Reply device name\n");
                  //N bytes bluetooth name with NULL terminated. (N <= 249 with NULL terminated)

                  for (uint8_t i = 3; i < /*DEVICENAME_LENGHT_SUPPORT + 2*/packetSize - 1; i++) { //event[2] is information type definition, last is checksum
                    //check if data are not NULL
                    if (event[i] == 0x00) break;
                    //deviceName[event[1]][i - 3] = event[i];
                    Serial3.write(event[i]);
                    if (event[1] == 0)
                      device0Name+=(char)event[i];
                    if (event[1] == 1)
                      device1Name+=(char)event[i];
                  }

                  if (event[1] == 0) {
                    DBG("Receive device 0 name: ");
                    DBG(device0Name);
                  }
                  if (event[1] == 1) {
                    DBG("Receive device 1 name: ");
                    DBG(device1Name);
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
              if (DEBUG) {
                Serial3.println();
                Serial3.println("EVT_Read_BTM_Version_Reply");
              }
              if (event[1] == 0x00) BTMUartVersion = (event[2] << 8) | (event[3] & 0xff);
              if (event[1] == 0x01) BTMFWVersion = (event[2] << 8) | (event[3] & 0xff);

            }
            break;
          case EVT_Call_List_Report:
            {
            }
            break;
          case EVT_AVC_Specific_Rsp:
            {
              if (DEBUG) {
                Serial3.println();
                Serial3.println("EVT_AVC_Specific_Rsp");
              }
              Serial3.println("data dump: ");

              for (uint8_t i = 0; i < packetSize; i++) {
                Serial3.print(event[0 + i], HEX);
                Serial3.print(" ");
              }
              Serial3.println();


              Serial3.print("device ID: "); Serial3.println(event[1]);
              /* event[2] - Response: 0xC
                 event [3] - Subunit_type: 0x9 (PANEL), Subunit_ID: 0x0
                 event [4] - Opcode: 0x0 (VENDOR DEPENDENT)
                 event [5,6,7] = 0x00 0x19 0x58 - Company ID: Bluetooth SIG registered CompanyID
              */
              Serial3.print("AVRCP1.3 AVC specific command PDU Id: "); Serial3.println(event[8], HEX);
              Serial3.print("packet type: ");
              switch (event[9])
              {
                case 0x00:
                  Serial3.println("single packet");
                  break;
                case 0x01:
                  Serial3.println("start packet");
                  break;
                case 0x02:
                  Serial3.println("continue packet");
                  break;
                case 0x03:
                  Serial3.println("end packet");
                  break;
                default:
                  Serial3.println(event[3], HEX);
                  break;
              }

              uint16_t rspSize = (event[10] << 8) | (event[11] & 0xff);
              Serial3.print("rsp parameter length: "); //Serial3.print(rspSize);
              Serial3.print(rspSize, HEX);
              Serial3.print("rsp data: ");
              for (uint8_t rsp_i = 0; rsp_i < rspSize; rsp_i++) {
                Serial3.print(event[rsp_i + 16], HEX);
                Serial3.print(" ");
                Serial3.write(event[rsp_i + 16]);
                Serial3.println();
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
              //if (DEBUG) Serial3.println("ReadLinkStatusReply");
              for (uint8_t i = 0; i < 8; i++) {
                LinkStatus[i] = event[i + 1]; //link status is array of 7 bytes, like response is 7bytes.
              }
              //Serial3.print("LinkStatus[1]: "); Serial3.print(LinkStatus[1]);
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

              */
              uint8_t deviceId = event[1];
              if (DEBUG) {
                Serial3.print("Read paired device record reply: ");
                Serial3.print("[");
                Serial3.print(deviceId, HEX);
                Serial3.print("] [");
              }


              for (uint8_t i = 6; i > 0; i--) { // -- cose : byte1~byte6 linked device BD address (6 bytes with low byte first)
                btAddress[deviceId][i] = event[i + 2]; // low byte first so 6,5,4,3,2,1 ? check in reality!
                if (DEBUG) {
                  Serial3.print(event[i + 2], HEX);
                  if (i > 1)Serial3.print(":");
                }
              }
              if (DEBUG) Serial3.println("]");
            }
            break;
          case EVT_Read_Local_BD_Address_Reply:
            {
            }
            break;
          case EVT_Read_Local_Device_Name_Reply:
            {
              IS2020::LocalDeviceName = "";
              DBG("size: ");
              DBG((String) event[1]);
              for (uint8_t _byte = 0; _byte < event[1]; _byte++) {
                //Serial3.write(event[_byte+2]);
                IS2020::LocalDeviceName += (char)event[_byte + 2];
              }
              DBG("BT name: ");
              DBG(LocalDeviceName);
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
      DBG("EVT_Command_ACK");
      break;
    case 0x01:
      DBG("EVT_BTM_Status");
      break;
    case 0x02:
      DBG("EVT_Call_Status");
      break;
    case 0x03:
      DBG("EVT_Caller_ID");
      break;
    case 0x04:
      DBG("EVT_SMS_Received_Indication");
      break;
    case 0x05:
      DBG("EVT_Missed_Call_Indication");
      break;
    case 0x06:
      DBG("EVT_Phone_Max_Battery_Level");
      break;
    case 0x07:
      DBG("EVT_Phone_Current_Battery_Level");
      break;
    case 0x08:
      DBG("EVT_Roaming_Status");
      break;
    case 0x09:
      DBG("EVT_Phone_Max_Signal_Strength_Level");
      break;
    case 0x0A:
      DBG("EVT_Phone_Current_Signal_Strength_Level");
      break;
    case 0x0B:
      DBG("EVT_Phone_Service_Status");
      break;
    case 0x0C:
      DBG("EVT_BTM_Battery_Status");
      break;
    case 0x0D:
      DBG("EVT_BTM_Charging_Status");
      break;
    case 0x0E:
      DBG("EVT_Reset_To_Default");
      break;
    case 0x0F:
      DBG("EVT_Report_HF_Gain_Level");
      break;
    case 0x10:
      DBG("EVT_EQ_Mode_Indication");
      break;
    case 0x11:
      DBG("EVT_PBAP_Missed_Call_History");
      break;
    case 0x12:
      DBG("EVT_PBAP_Received_Call_History");
      break;
    case 0x13:
      DBG("EVT_PBAP_Dialed_Call_History");
      break;
    case 0x14:
      DBG("EVT_PBAP_Combine_Call_History");
      break;
    case 0x15:
      DBG("EVT_Get_PB_By_AT_Cmd_Reply");
      break;
    case 0x16:
      DBG("EVT_PBAP_Access_Finish");
      break;
    case 0x17:
      DBG("EVT_Read_Linked_Device_Information_Reply");
      break;
    case 0x18:
      DBG("EVT_Read_BTM_Version_Reply");
      break;
    case 0x19:
      DBG("EVT_Call_List_Report");
      break;
    case 0x1A:
      DBG("EVT_AVC_Specific_Rsp");
      break;
    case 0x1B:
      DBG("EVT_BTM_Utility_Req");
      break;
    case 0x1C:
      DBG("EVT_Vendor_AT_Cmd_Reply");
      break;
    case 0x1D:
      DBG("EVT_Report_Vendor_AT_Event");
      break;
    case 0x1E:
      DBG("EVT_Read_Link_Status_Reply");
      break;
    case 0x1F:
      DBG("EVT_Read_Paired_Device_Record_Reply");
      break;
    case 0x20:
      DBG("EVT_Read_Local_BD_Address_Reply");
      break;
    case 0x21:
      DBG("EVT_Read_Local_Device_Name_Reply");
      break;
    case 0x22:
      DBG("EVT_Report_SPP_iAP_Data");
      break;
    case 0x23:
      DBG("EVT_Report_Link_Back_Status");
      break;
    case 0x24:
      DBG("EVT_Ringtone_Finish_Indicate");
      break;
    case 0x25:
      DBG("EVT_User_Confrim_SSP_Req");
      break;
    case 0x26:
      DBG("EVT_Report_AVRCP_Vol_Ctrl");
      break;
    case 0x27:
      DBG("EVT_Report_Input_Signal_Level");
      break;
    case 0x28:
      DBG("EVT_Report_iAP_Info");
      break;
    case 0x29:
      DBG("EVT_REPORT_AVRCP_ABS_VOL_CTRL");
      break;
    case 0x2A:
      DBG("EVT_Report_Voice_Prompt_Status");
      break;
    case 0x2B:
      DBG("EVT_Report_MAP_Data");
      break;
    case 0x2C:
      DBG("EVT_Security_Bonding_Res");
      break;
    case 0x2D:
      DBG("EVT_Report_Type_Codec");
      break;
  }
  DBG("\n");
}
