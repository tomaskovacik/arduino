#include "IS2020.h"
#include <Arduino.h>

uint8_t  IS2020::AVC_Get_Element_Attributes_command() {
  IS2020::getNextEventFromBT();
  IS2020::DBG("AVC_Specific_Cmd _Get Element Attributes command\n");
  uint8_t data[17] = {0x20, //6
                      0x00, //7
                      0x00, 0x0d, // 8-9 D => 13 bytes
                      0x00, 0x00, 0x00, 0x00,//10,11,12,13 Identifier 8
                      0x02, //14 NumAttributes (N) 1
                      0x00, 0x00, 0x00, 0x01, //15,16,17,18,19,20,21,22
                      0x00, 0x00, 0x00, 0x07
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(19, CMD_AVC_Specific_Cmd, 0, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVC_Registration_for_notification_of_Event_Track_changed() {
  IS2020::getNextEventFromBT();
  IS2020::DBG("AVC_Specific_Cmd Registration_for_notification_of_Event_Track_changed\n");
  uint8_t data[7] = {0x31, //PDU ID (0x31 – Register Notification)
                      0x00, //reserved
                      0x00, 0x05, // Parameter Length (0x5)
                      0x02, //EventID2: 0x2 (EVENT_TRACK_CHANGED)
                      0x00, 0x00 //Playback interval: 0x0 (Ignored for this event)
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(9, CMD_AVC_Specific_Cmd, 0, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVC_List_Application_Settings_Attributes_command() {
  IS2020::getNextEventFromBT();
  IS2020::DBG("AVC_Specific_Cmd Registration_for_notification_of_Event_Track_changed\n");
  uint8_t data[4] = {0x11, //PDU ID (0x11 – ListApplicationSettingAttributes)
                      0x00, //reserved
                      0x00, 0x00 //Parameter Length (0x0)
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(6, CMD_AVC_Specific_Cmd, 0, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVC_List_Application_Settings_Attributes_command() {
  IS2020::getNextEventFromBT();
  IS2020::DBG("AVC_Specific_Cmd Registration_for_notification_of_Event_Track_changed\n");
  uint8_t data[5] = {0x10, //PDU ID (0x10 - Get Capabilities)
                      0x00, //reserved
                      0x00, 0x01, //Parameter Length (0x0001)
                      0x03 //Capability ID: 0x3 (EventsID)
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(7, CMD_AVC_Specific_Cmd, 0, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVC_List_Application_Settings_Attributes_command() {
  IS2020::getNextEventFromBT();
  IS2020::DBG("AVC_Specific_Cmd Registration_for_notification_of_Event_Track_changed\n");
  uint8_t data[5] = {0x10, //PDU ID (0x10 - Get Capabilities)
                      0x00, //reserved
                      0x00, 0x01, //Parameter Length (0x0001)
                      0x02 //Capability ID: 0x2 (CompanyID)
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(7, CMD_AVC_Specific_Cmd, 0, data);
  return checkResponce(EVT_Command_ACK);
}
