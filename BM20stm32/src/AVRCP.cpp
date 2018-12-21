#include "IS2020.h"
#include "AVRCP.h"
#include <Arduino.h>

uint8_t  IS2020::AVRCP_Get_Capabilities(uint8_t deviceId, uint8_t capId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Capability_command_for_Events\n"));
  uint8_t data[5] = {AVRCP_GET_CAPABILITIES, //6
                     0x00, //7 Reserved
                     0x00, 0x01, // 8-9 D => 13 bytes
                     capId //0x02 company ID, 0x03 EventsID
                    }; //23,24,25,26
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_List_Player_Attributes(uint8_t deviceId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_List_Player_Attributes\n"));
  uint8_t data[5] = {AVRCP_LIST_PLAYER_ATTRIBUTES, //6
                     0x00, //7 Reserved
                     0x00, 0x01,
                     0x00
                    };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_List_Player_Values(uint8_t deviceId, uint8_t attribute) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_List_Player_Values\n"));
  uint8_t data[5] = {AVRCP_LIST_PLAYER_VALUES, //6
                     0x00, //7 Reserved
                     0x00, 0x01,
                     attribute
                    };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}
/*

*/
uint8_t  IS2020::AVRCP_Get_Current_Player_Value(uint8_t deviceId, uint8_t attribute) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Current_Player_Value\n"));
  uint8_t data[5] = {AVRCP_GET_CURRENT_PLAYER_VALUE, //6
                     0x00, //7 Reserved
                     0x00, 0x01, //size
                     attribute
                    };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Set_Player_Value(uint8_t deviceId, uint8_t attribute, uint8_t value) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Player_Value\n"));
  uint8_t data[6] = {AVRCP_SET_PLAYER_VALUE, //6
                     0x00, //7 Reserved
                     0x00, 0x02, //size
                     attribute, value
                    };
  IS2020::SendPacketArrayInt(8, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Player_Attribute_Text(uint8_t deviceId, uint8_t attribute) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Player_Attribute_Text\n"));
  uint8_t data[5] = {AVRCP_GET_PLAYER_ATTRIBUTE_TEXT, //6
                     0x00, //7 Reserved
                     0x00, 0x01, //size
                     attribute
                     /*0x01,0x02,0x03,0x04*/
                    };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Player_Value_Text(uint8_t deviceId, uint8_t attribute, uint8_t setting) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Player_Value_Text\n"));
  uint8_t data[7] = {AVRCP_GET_PLAYER_VALUE_TEXT, //6
                     0x00, //7 Reserved
                     0x00, 0x01, //size
                     attribute,
                     0x01,
                     setting
                    };
  IS2020::SendPacketArrayInt(9, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Displayable_Charset(uint8_t deviceId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Displayable_Charset\n"));
  uint8_t data[6] = {AVRCP_DISPLAYABLE_CHARSET, //6
                     0x00, //7 Reserved
                     0x00, 0x02, //size
                     0x00, 0x6A
                    };
  IS2020::SendPacketArrayInt(8, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Element_Attributes(uint8_t deviceId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP Get element attributes command\n"));
  uint8_t data[45] = {AVRCP_GET_ELEMENT_ATTRIBUTES, //6
                      0x00, //7 Reserved
                      0x00, 0x29, // 8-9 D => 13 bytes
                      0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,//10,11,12,13,14,15,16,17 Identifier 8
                      0x02, //14 NumAttributes (N) 1
                      0x00, 0x00, 0x00, 0x01, //15,16,17,18,19,20,21,22
                      0x00, 0x00, 0x00, 0x02,
                      0x00, 0x00, 0x00, 0x03,
                      0x00, 0x00, 0x00, 0x04,
                      0x00, 0x00, 0x00, 0x05,
                      0x00, 0x00, 0x00, 0x06,
                      0x00, 0x00, 0x00, 0x07,
                      0x00, 0x00, 0x00, 0x08
                     }; //23,24,25,26
  IS2020::SendPacketArrayInt(47, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Element_Attributes_All(uint8_t deviceId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Element_Attributes\n"));
  uint8_t data[5] = {AVRCP_GET_ELEMENT_ATTRIBUTES, //1
                      0x00, //2
                      0x00,0x01,0x00/*, 9, //3,4 - size after this
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //1,2,3,4 5,6,7,8
                      0x00*/
                     };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Play_Status(uint8_t deviceId) {
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Get_Element_Attributes\n"));
  uint8_t data[3] = {AVRCP_GET_PLAY_STATUS, //command
                      0x00, //reserved
                      0x00//, 0x00
                     };
  IS2020::SendPacketArrayInt(5, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Registration_for_notification_of_Event(uint8_t deviceId, uint8_t event, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4) {
  IS2020::getNextEventFromBT();
  //IS2020::DBG(F("Registration for notification of event:"));
  //decodeAVRCP_Event(event); DBG(F("\n"));
  uint8_t data[9] = {AVRCP_REGISTER_NOTIFICATION, //PDU ID (0x31 – Register Notification)
                     0x00, //reserved
                     0x00, 0x05, // Parameter Length (0x5)
                     event, //EventID
                     param1, param2, param3, param4
                    };
  IS2020::SendPacketArrayInt(11, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
  IS2020::getNextEventFromBT(); IS2020::getNextEventFromBT(); IS2020::getNextEventFromBT();
}

uint8_t  IS2020::AVRCP_Request_Continuing(uint8_t deviceId, uint8_t pdu){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Request_Continuing\n"));
  uint8_t data[5] = {AVRCP_REQUEST_CONTINUING, //1
                      0x00, //2
                      0x00, 0x01,
                      pdu
                     };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Abort_Continuing(uint8_t deviceId, uint8_t pdu){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Abort_Continuing\n"));
  uint8_t data[5] = {AVRCP_ABORT_CONTINUING, //1
                      0x00, //2
                      0x00, 0x01,
                      pdu
                     };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Set_Absolute_Volume(uint8_t deviceId, uint8_t volume){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Absolute_Volume\n"));
  uint8_t data[5] = {AVRCP_SET_ABSOLUTE_VOLUME, //1
                      0x00, //2
                      0x00, 0x01,
                      volume
                     };
  IS2020::SendPacketArrayInt(7, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Set_Addressed_Player(uint8_t deviceId, uint16_t player){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Addressed_Player\n"));
  uint8_t data[6] = {ACRCP_SET_ADDRESSED_PLAYER, //1
                      0x00, //2
                      0x00, 0x02,
                      ((player>>8) & 0xFF),(player & 0xFF)
                     };
  IS2020::SendPacketArrayInt(8, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Set_Browsed_Player(uint8_t deviceId, uint16_t player){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Addressed_Player\n"));
  uint8_t data[6] = {AVRCP_SET_BROWSED_PLAYER, //1
                      0x00, //2
                      0x00, 0x02,
                      ((player>>8) & 0xFF),(player & 0xFF)
                     };
  IS2020::SendPacketArrayInt(8, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Get_Folder_Items(uint8_t deviceId, uint8_t scope, uint32_t start,uint8_t end){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Addressed_Player\n"));
  uint8_t data[14] = {AVRCP_GET_FOLDER_ITEMS, //1
                      0x00, //2,
                      0x00, 0x0B,
                      scope, //scope
                      ((start>>14)&0xFF),((start>>16)&0xFF),((start>>8)&0xFF),(start&0xFF),//start
                      ((end>>14)&0xFF),((end>>16)&0xFF),((end>>8)&0xFF),(end&0xFF),//end
                      0x00
                     };
  IS2020::SendPacketArrayInt(16, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

uint8_t  IS2020::AVRCP_Change_Path(uint8_t deviceId, uint8_t direction, uint64_t folderUID){
  IS2020::getNextEventFromBT();
  IS2020::DBG(F("AVRCP_Set_Addressed_Player\n"));
  uint8_t data[15] = {AVRCP_GET_FOLDER_ITEMS, //1
                      0x00, //2,
                      0x00, 0x0B,//Parameter Length
                      0x12,0x34, //UID Counter
                      direction,//Direction: 0x01 (Folder Down)
                     // 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05//Folder UID:
                      ((folderUID>>56)&0xFF),((folderUID>>48)&0xFF),((folderUID>>40)&0xFF),((folderUID>>32)&0xFF),
                      ((folderUID>>24)&0xFF),((folderUID>>16)&0xFF),((folderUID>>8)&0xFF),(folderUID&0xFF)
                     };
  IS2020::SendPacketArrayInt(16, CMD_AVRCP_Specific_Cmd, deviceId, data);
  return checkResponce(EVT_Command_ACK);
}

void IS2020::RegisterAllEvents(uint8_t deviceId) {
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_STATUS_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_TRACK_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_TRACK_REACHED_END, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_TRACK_REACHED_START, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_PLAYBACK_POS_CHANGED, 0x00, 0x00,0x13, 0x88);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_BATT_STATUS_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_SYSTEM_STATUS_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_UIDS_CHANGED, 0x00, 0x00, 0x00, 0x00);
  IS2020::AVRCP_Registration_for_notification_of_Event(deviceId, AVRCP_EVENT_VOLUME_CHANGED, 0x00, 0x00, 0x00, 0x00);
}

void IS2020::decodeAVRCP_Pdu(uint8_t pdu) {
  DBG(F("Decoded PDU: "));
  switch (pdu)
  {
    /* PDU types for metadata transfer */
    case 0x10:
      IS2020::DBG(F("AVRCP_GET_CAPABILITIES"));
      break;
    case 0x11:
      IS2020::DBG(F("AVRCP_LIST_PLAYER_ATTRIBUTES"));
      break;
    case 0x12:
      IS2020::DBG(F("AVRCP_LIST_PLAYER_VALUES"));
      break;
    case 0x13:
      IS2020::DBG(F("AVRCP_GET_CURRENT_PLAYER_VALUE"));
      break;
    case 0x14 :
      IS2020::DBG(F("AVRCP_SET_PLAYER_VALUE"));
      break;
    case 0x15:
      IS2020::DBG(F("AVRCP_GET_PLAYER_ATTRIBUTE_TEXT"));
      break;
    case 0x16:
      IS2020::DBG(F("AVRCP_GET_PLAYER_VALUE_TEXT"));
      break;
    case 0x17:
      IS2020::DBG(F("AVRCP_DISPLAYABLE_CHARSET"));
      break;
    case 0x18:
      IS2020::DBG(F("AVRCP_CT_BATTERY_STATUS"));
      break;
    case 0x20:
      IS2020::DBG(F("AVRCP_GET_ELEMENT_ATTRIBUTES"));
      break;
    case 0x30:
      IS2020::DBG(F("AVRCP_GET_PLAY_STATUS"));
      break;
    case 0x31:
      IS2020::DBG(F("AVRCP_REGISTER_NOTIFICATION"));
      break;
    case 0x40:
      IS2020::DBG(F("AVRCP_REQUEST_CONTINUING"));
      break;
    case 0x41:
      IS2020::DBG(F("AVRCP_ABORT_CONTINUING"));
      break;
    case 0x50:
      IS2020::DBG(F("AVRCP_SET_ABSOLUTE_VOLUME"));
      break;
    case 0x60:
      IS2020::DBG(F("ACRCP_SET_ADDRESSED_PLAYER"));
      break;
    case 0x70:
      IS2020::DBG(F("AVRCP_SET_BROWSED_PLAYER"));
      break;
    case 0x71:
      IS2020::DBG(F("AVRCP_GET_FOLDER_ITEMS"));
      break;
    case 0x72:
      IS2020::DBG(F("AVRCP_CHANGE_PATH"));
      break;
    case 0x73:
      IS2020::DBG(F("AVRCP_GET_ITEM_ATTRIBUTES"));
      break;
    case 0x74:
      IS2020::DBG(F("AVRCP_PLAY_ITEM"));
      break;
    case 0x80:
      IS2020::DBG(F("AVRCP_SEARCH"));
      break;
    case 0x90:
      IS2020::DBG(F("AVRCP_ADD_TO_NOW_PLAYING"));
      break;
    case AVRCP_GENERAL_REJECT:
      IS2020::DBG(F("AVRCP_GENERAL_REJECT"));
      break;
    default:
      DBG(F("Unknown: ")); DBG(String(pdu, HEX));
  }
  DBG(F("\n"));
}


void IS2020::decodeAVRCP_Event(uint8_t Event) {
  DBG(F("Decoded event: "));
  switch (Event)
  {
    case AVRCP_EVENT_STATUS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_STATUS_CHANGED"));
      break;
    case AVRCP_EVENT_TRACK_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_TRACK_CHANGED"));
      break;
    case AVRCP_EVENT_TRACK_REACHED_END:
      IS2020::DBG(F("AVRCP_EVENT_TRACK_REACHED_END"));
      break;
    case AVRCP_EVENT_TRACK_REACHED_START:
      IS2020::DBG(F("AVRCP_EVENT_TRACK_REACHED_START"));
      break;
    case AVRCP_EVENT_PLAYBACK_POS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_PLAYBACK_POS_CHANGED"));
      break;
    case AVRCP_EVENT_BATT_STATUS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_BATT_STATUS_CHANGED"));
      break;
    case AVRCP_EVENT_SYSTEM_STATUS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_SYSTEM_STATUS_CHANGED"));
      break;
    case AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED"));
      break;
    case AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED"));
      break;
    case AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED"));
      break;
    case AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED"));
      break;
    case AVRCP_EVENT_UIDS_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_UIDS_CHANGED"));
      break;
    case AVRCP_EVENT_VOLUME_CHANGED:
      IS2020::DBG(F("AVRCP_EVENT_VOLUME_CHANGED"));
      break;
    default:
      DBG(F("Unknown: ")); DBG(String(Event, HEX));
  }
  DBG(F("\n"));
}

void IS2020::decodeAVRCP_PlayerAtributes(uint8_t attribute) {
  /* player attributes */
  DBG(F("Decoded player attribute: "));
  switch (attribute)
  {
    case AVRCP_ATTRIBUTE_ILEGAL:
      DBG(F("AVRCP_ATTRIBUTE_ILEGAL"));
      break;
    case AVRCP_ATTRIBUTE_EQUALIZER:
      DBG(F("AVRCP_ATTRIBUTE_EQUALIZER"));
      break;
    case AVRCP_ATTRIBUTE_REPEAT_MODE:
      DBG(F("AVRCP_ATTRIBUTE_REPEAT_MODE"));
      break;
    case AVRCP_ATTRIBUTE_SHUFFLE:
      DBG(F("AVRCP_ATTRIBUTE_SHUFFLE"));
      break;
    case AVRCP_ATTRIBUTE_SCAN:
      DBG(F("AVRCP_ATTRIBUTE_SCAN"));
      break;
  }
  DBG(F("\n"));
}

