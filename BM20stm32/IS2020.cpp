#include "IS2020.h"
#include <Arduino.h>
//#include <SoftwareSerial3.h>
#include "MMI.h"
#include "Music.h"
#include "commands.h"
#include "events.h"

IS2020::IS2020(HardwareSerial *ser) {
  btHwSerial = ser;
}

/*
   Destructor
*/
IS2020::~IS2020() {
}

void IS2020::begin(uint32_t baudrate) {
  btHwSerial->begin(baudrate);
}

/*
   debug output
*/
void IS2020::DBG(String text) {
  if (DEBUG) /*return "DBG: ");*/ Serial.print(text);;
}

uint8_t IS2020::checkResponce(uint8_t eventId) {
  DBG("\nresponce : ");
  uint8_t responceId=IS2020::getNextEventFromBT();
  if (responceId == eventId) {
    DBG("OK\n\n");
    return true;
  } else {
    DBG("\nFAIL: responceId: ");
    DBG((String) responceId);
    DBG(" checkedId: ");
    DBG((String) eventId);
    DBG("\n");
    return false;
  }
}


/*
*/
void  IS2020::SendPacketInt (uint8_t cmd, uint8_t data) {
  DBG("sending: ");
  decodeCommand(cmd);
  DBG("\n");
  btHwSerial -> write(STARTBYTE);
  if (DEBUG) {
    Serial3.print(STARTBYTE, HEX);
  }
  btHwSerial -> write((byte)0x00);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print((0x00), HEX);
  }
  //uint8_t checkSum = 0x00;
  btHwSerial -> write(0x02);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print((0x02), HEX);
  }
  uint8_t checkSum = 0x02;
  btHwSerial -> write(cmd);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(cmd, HEX);
  }
  checkSum += (cmd);
  btHwSerial -> write(data);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(data, HEX);
  }
  checkSum += data;
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.println(0x100 - checkSum, HEX);
  }
  btHwSerial -> write(0x100 - checkSum);
}

void  IS2020::SendPacketArrayInt (uint16_t packetSize, uint8_t cmd, uint8_t deviceId, uint8_t data[]) {
  DBG("sending: ");
  decodeCommand(cmd);
  DBG("\n");
  btHwSerial -> write(STARTBYTE);
  if (DEBUG)  {
    Serial3.print(STARTBYTE, HEX);
  }
  btHwSerial -> write(packetSize >> 8);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print((packetSize >> 8), HEX);
  }
  uint8_t checkSum = packetSize >> 8;
  btHwSerial -> write(packetSize & 0xFF);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print((packetSize & 0xFF), HEX);
  }
  checkSum += (packetSize & 0xFF);
  btHwSerial -> write(cmd);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(cmd, HEX);
  }
  checkSum += (cmd);
  btHwSerial -> write(deviceId);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(deviceId, HEX);
  }
  checkSum += (deviceId);
  for (uint16_t dataPos = 0; dataPos < packetSize - 2; dataPos++) {
    btHwSerial -> write(data[dataPos]);
    if (DEBUG)  {
      Serial3.print(" ");
      Serial3.print(data[dataPos], HEX);
    }
    checkSum += data[dataPos];
  }
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.println(0x100 - checkSum, HEX);
  }
  btHwSerial -> write(0x100 - checkSum);
}

void  IS2020::SendPacketArrayChar (uint16_t packetSize, uint8_t cmd, uint8_t deviceId, char data[]) {
  DBG("sending: ");
  decodeCommand(cmd);
  DBG("\n");
  btHwSerial -> write(STARTBYTE);
  if (DEBUG)  {
    Serial3.print(STARTBYTE, HEX);
  }
  btHwSerial -> write(packetSize >> 8);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(packetSize, HEX);
  }
  uint8_t checkSum = packetSize >> 8;
  btHwSerial -> write(packetSize & 0xFF);
  checkSum += (packetSize & 0xFF);
  btHwSerial -> write(cmd);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(cmd, HEX);
  }
  checkSum += (cmd);
  btHwSerial -> write(deviceId);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.print(deviceId, HEX);
  }
  checkSum += (deviceId);
  for (uint16_t dataPos = 0; dataPos < packetSize - 2; dataPos++) {
    btHwSerial -> write(data[dataPos]);
    if (DEBUG)  {
      Serial3.print(" ");
      Serial3.print(data[dataPos]);
    }
    checkSum += data[dataPos];
  }
  btHwSerial -> write(0x100 - checkSum);
  if (DEBUG)  {
    Serial3.print(" ");
    Serial3.println(checkSum, HEX);
  }
}

int IS2020::SerialAvailable () {
  return btHwSerial -> available();
}

int IS2020::SerialRead() {
  return btHwSerial -> read();
  //  if (btHwSerial -> available()) {
  //    Serial3.write(btHwSerial -> read());
  //  }
}

uint8_t IS2020::checkCkeckSum(int size, uint8_t data[]) {
  if (DEBUG)
  {
    Serial3.println();
    Serial3.println("dumping event data:");
    Serial3.print("packet size: "); Serial3.print(size, DEC); Serial3.print(" ("); Serial3.print(size, HEX); Serial3.println(")");
    Serial3.print("Event:   ");decodeEvent(data[0]);//Serial3.println(data[0], HEX);
    Serial3.print("data: ");
  }
  uint8_t csum = (size >> 8);
  csum += (size & 0xFF);
  //csum += data[0];
  for (uint8_t i = 0; i < size; i++) { //
    csum += data[i];
    if (DEBUG) {
      Serial3.print(data[i], HEX); Serial3.print(" ");
    }
  }

  if (DEBUG)Serial3.println();
  if (data[size] == (0x100 - csum) ) {
    if (DEBUG)
    {
      Serial3.println("Checksum OK");
      Serial3.println();
      Serial3.println();
    }
    return true;
  } else {
    if (DEBUG)
    {
      Serial3.println("Bad checksum");
      Serial3.println();
      Serial3.println();
    }
    return false;
  }
}

String IS2020::Connect_status(uint8_t deviceId){
  String ConnectStatus="";
  /*
            Value Parameter Description
            0xXX  "1 indicate connected
            0x01 Bit0 : A2DP profile signaling channel connected
            0x02 Bit1 : A2DP profile stream channel connected
            0x04 Bit2 : AVRCP profile connected
            0x08 Bit3 : HF profile connected
            0x10 Bit4 : SPP connected"
   */
  if (LinkStatus[1+deviceId] &(1<<A2DP_profile_signaling_channel_connected)) ConnectStatus+="A2DP profile signaling channel connected\n";
  if (LinkStatus[1+deviceId] &(1<<A2DP_profile_stream_channel_connected)) ConnectStatus+="A2DP profile stream channel connected\n";
  if (LinkStatus[1+deviceId] &(1<<AVRCP_profile_connected)) ConnectStatus+="AVRCP profile connected\n";
  if (LinkStatus[1+deviceId] &(1<<HF_profile_connected)) ConnectStatus+="HF profile connected\n";
  if (LinkStatus[1+deviceId] &(1<<SPP_connected)) ConnectStatus+="SPP connected\n";
  return ConnectStatus;
}

String IS2020::MusicStatus(uint8_t deviceId){
  /*
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
   */
  switch (LinkStatus[3+deviceId]) {
    case 0x00:
      return ("STOP");
      break;
    case 0x01:
      return ("PLAYING");
      break;
    case 0x02:
      return ("PAUSED");
      break;
    case 0x03:
      return ("FWD SEEK");
      break;
    case 0x04:
      return ("REV SEEK");
      break;
    case 0x05:
      return ("FAST FWD");
      break;
    case 0x06:
      return ("REWIND");
      break;
    case 0x07:
      return ("WAIT_TO_PLAY");
      break;
    case 0x08:
      return ("WAIT_TO_PAUSE");
      break;
  }
}

String IS2020::StreamStatus(uint8_t deviceId){
  /*
             database1_stream_status," SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "1 indicate connected
            0x00: no stream
            0x01: stream on-going"
   */
  if (LinkStatus[5+deviceId] == 0x00) {
    return ("No stream");
  } else {
    return ("Streaming");
  }
}

uint8_t IS2020::BatteryLevel(uint8_t deviceId){  
  return /*((IS2020::currentBatteryLevel[deviceId]*100)/*/maxBatteryLevel[deviceId];
}

String IS2020::ModuleState(){
  /*
            Event Parameters: device_state  SIZE: 1 BYTE
            Value Parameter Description
            0x00  Power OFF state
            0x01  pairing state (discoverable mode)
            0x02  standby state
            0x03  Connected state with only HF profile connected
            0x04  Connected state with only A2DP profile connected
            0x05  Connected state with only SPP profile connected
            0x06  Connected state with multi-profile connected
   */
    switch (LinkStatus[0]) {
    case 0x00:
      return ("Power OFF");
      break;
    case 0x01:
      return ("pairing state (discoverable mode)");
      break;
    case 0x02:
      return ("standby state");
      break;
    case 0x03:
      return ("Connected state with only HF profile connected");
      break;
    case 0x04:
      return ("Connected state with only A2DP profile connected");
      break;
    case 0x05:
      return ("Connected state with only SPP profile connected");
      break;
    case 0x06:
      return ("Connected state with HP and A2DP profile connected");
      break;

  }
}

String IS2020::BtStatus(uint8_t deviceId){

   switch (BTMStatus[deviceId]) {
        case 0x00:
          return ("Power OFF state");
          break;
        case 0x01:
          return ("pairing state (discoverable mode)");
          break;
        case 0x02:
          return ("Power ON state");
          break;
        case 0x03:
         return ("pairing successful");
          break;
        case 0x04:
          return ("pairing fail");
          break;
        case 0x05:
          return ("HF link established");
          break;
        case 0x06:
          return ("A2DP link established");
          break;
        case 0x07:
          return ("HF link disconnected");
          break;
        case 0x08:
          return ("A2DP link disconnected");
          break;
        case 0x09:
          return ("SCO link connected");
          break;
        case 0x0A:
          return ("SCO link disconnected");
          break;
        case 0x0B:
          return ("AVRCP link established");
          break;
        case 0x0C:
          return ("AVRCP link disconnected");
          break;
        case 0x0D:
          return ("Standard SPP connected");
          break;
        case 0x0E:
          return ("Standard_SPP / iAP disconnected");
          break;
        case 0x0F:
          return ("Standby state");
          break;
        case 0x10:
          return ("iAP connected");
          break;
        case 0x11:
          return ("ACL disconnected");
          break;
        default:
          return ("unknown state"+ (char)BTMStatus[0]);
          break;
      } 
  }


void IS2020::setEventMask(uint32_t mask){
      EventMask |= mask;
     // Serial3.println(EventMask,BIN);
  }

void IS2020::enableAllSettingEvent(){
    //bit0 reserved
    setEventMask(EMB_SPK_Module_state);
    setEventMask(EMB_call_status);
    setEventMask(EMB_incoming_call_number_or_caller_id);
    setEventMask(EMB_SMS_received);
    setEventMask(EMB_Missed_call);
    setEventMask(EMB_Max_cell_phone_battery_level);
    setEventMask(EMB_current_cell_phone_battery_level);
    
    setEventMask(EMB_cell_phone_roamming);
    setEventMask(EMB_Max_cell_phone_signal_strength);
    setEventMask(EMB_current_cell_phone_signal_strength);
    setEventMask(EMB_cell_phone_service_status);
/*    setEventMask(EMB_BTM_battery_level);
    setEventMask(EMB_BTM_charging_status);
    setEventMask(EMB_BMT_reset_to_default_setting_OK);
    setEventMask(EMB_BTM_DAC_gain_level);*/
    
    setEventMask(EMB_EQ_mode);
    setEventMask(EMB_remote_device_friendly_name);
    setEventMask(EMB_AVC_specific_response);
    setEventMask(EMB_unknown_AT_command_result_code);
    setEventMask(EMB_Page_status);
    setEventMask(EMB_Ringtone_status);
    setEventMask(EMB_amp_indication);
    setEventMask(EMB_line_in_status);
}

