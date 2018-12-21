#include "IS2020.h"
#include <Arduino.h>
#include "MMI.h"
#include "Music.h"
#include "commands.h"
#include "events.h"

IS2020::IS2020(HardwareSerial *ser) {
  btSerial = ser;
}

/*s
   Destructor
*/
IS2020::~IS2020() {
}

void IS2020::begin(uint32_t baudrate, uint8_t resetPin) {
  _reset = resetPin;
  btSerial -> begin(baudrate);
  pinMode(_reset, OUTPUT);
  IS2020::resetHigh();
}

/*
   debug output
*/
void IS2020::DBG(String text) {
  if (DEBUG) /*return "DBG: ");*/ Serial3.print(text);;
}

void IS2020::resetLow() {
  digitalWrite(_reset, LOW);
}

void IS2020::resetHigh() {
  digitalWrite(_reset, HIGH);
}

void IS2020::resetModule() {
  DBG(F("reseting module\n"));
  resetLow();
  delay(100);
  resetHigh();
}

uint8_t IS2020::checkResponce(uint8_t eventId) {
  IS2020::getNextEventFromBT();
  return true;
  //DBG(F("\nChecking responce : ")); decodeEvent(eventId);DBG(F("\n"));
  //uint8_t responceId = IS2020::getNextEventFromBT();
  //DBG(F("Get responce: "));decodeEvent(responceId); DBG(F("\n"));
  
//  if (responceId == eventId) {
//    DBG("OK\n\n");
//    return true;
//  } else {
//    DBG("FAIL: ");
//    decodeEvent(responceId);
//    DBG(" != : ");
//    decodeEvent(eventId);
//    DBG("\n");
//    return false;
//  }
}


/*
*/
void  IS2020::SendPacketInt(uint8_t cmd, uint8_t data) {
  // DBG("sending int: ");
  decodeCommand(cmd); DBG("\n");

  btSerial -> write(STARTBYTE); //DBG(String(STARTBYTE, HEX));
  btSerial -> write((byte)0x00);// DBG(F(" 0x00"));
  btSerial -> write(0x02);//DBG(F(" 0x02"));
  uint8_t checkSum = 0x02;
  btSerial -> write(cmd); //DBG(F(" ")); DBG(String(cmd, HEX));
  checkSum += (cmd);
  btSerial -> write(data); //DBG(F(" ")); DBG(String(data, HEX));
  checkSum += data;
  btSerial -> write(0x100 - checkSum); //DBG(F(" ")); DBG(String((0x100 - checkSum), HEX));
}

/*

*/
void  IS2020::SendPacketString(uint8_t cmd, String str) {
  //DBG("sending string: ");
  decodeCommand(cmd); DBG(" String: " + str + "\n");

  uint16_t packetSize = str.length() + 1; //length of string + cmd
  btSerial -> write(STARTBYTE); //DBG(String(STARTBYTE, HEX));
  btSerial -> write(packetSize >> 8); //DBG(F(" ")); DBG(String((packetSize >> 8), HEX));
  uint8_t checkSum = packetSize >> 8;
  btSerial -> write(packetSize & 0xFF); //DBG(F(" ")); DBG(String((packetSize & 0xFF), HEX));
  checkSum += (packetSize & 0xFF);
  btSerial -> write(cmd); DBG(F(" ")); //DBG(String(cmd, HEX));
  checkSum += (cmd);
  for (uint16_t dataPos = 0; dataPos < packetSize - 1; dataPos++) {
    btSerial -> write(str[dataPos]); //DBG(F(" ")); DBG(String(str[dataPos], HEX));
    checkSum += str[dataPos];
  }
  btSerial -> write(0x100 - checkSum);// DBG(F(" ")); DBG(String(0x100 - checkSum, HEX));
}
/*

*/
void  IS2020::SendPacketArrayInt (uint16_t packetSize, uint8_t cmd, uint8_t deviceId, uint8_t data[]) {
  //DBG("sending array int: ");
  decodeCommand(cmd); DBG(": ");

  btSerial -> write(STARTBYTE); DBG(String(STARTBYTE, HEX));
  btSerial -> write(packetSize >> 8); DBG(F(" ")); DBG(String((packetSize >> 8), HEX));
  uint8_t checkSum = packetSize >> 8;
  btSerial -> write(packetSize & 0xFF); DBG(F(" "));DBG(String((packetSize & 0xFF), HEX));
  checkSum += (packetSize & 0xFF);
  btSerial -> write(cmd); DBG(F(" ")); DBG(String(cmd, HEX));
  checkSum += (cmd);
  btSerial -> write(deviceId); DBG(F(" ")); //DBG(String(deviceId, HEX));
  checkSum += (deviceId);
  for (uint16_t dataPos = 0; dataPos < packetSize - 2; dataPos++) {
    btSerial -> write(data[dataPos]); DBG(F(" ")); DBG(String(data[dataPos], HEX));
    checkSum += data[dataPos];
  }
  btSerial -> write(0x100 - checkSum); DBG(F(" ")); DBG(String(0x100 - checkSum, HEX)+"\n");
}

void  IS2020::SendPacketArrayChar (uint16_t packetSize, uint8_t cmd, uint8_t deviceId, char data[]) {
  // DBG("sending array char: ");
  decodeCommand(cmd);// DBG("\n");

  btSerial -> write(STARTBYTE);//DBG(String(STARTBYTE, HEX));
  btSerial -> write(packetSize >> 8);// DBG(F(" ")); DBG(String((packetSize >> 8), HEX));
  uint8_t checkSum = packetSize >> 8;
  btSerial -> write(packetSize & 0xFF);// DBG(F(" ")); DBG(String((packetSize & 0xFF), HEX));
  checkSum += (packetSize & 0xFF);
  btSerial -> write(cmd); DBG(F(" ")); //DBG(String(cmd, HEX));
  checkSum += (cmd);
  btSerial -> write(deviceId); //DBG(F(" ")); DBG(String(deviceId, HEX));
  checkSum += (deviceId);
  for (uint16_t dataPos = 0; dataPos < packetSize - 2; dataPos++) {
    btSerial -> write(data[dataPos]);//DBG(F(" ")); DBG(String(data[dataPos], HEX));
    checkSum += data[dataPos];
  }
  btSerial -> write(0x100 - checkSum);//DBG(F(" ")); DBG(String((0x100 - checkSum), HEX));
}

int IS2020::SerialAvailable () {
  return btSerial -> available();
}

int IS2020::SerialRead() {
  return btSerial -> read();
}

uint8_t IS2020::checkCkeckSum(int size, uint8_t data[]) {
  //DBG(F(" "));
  //DBG(F("dumping event data:"));
  //DBG(F("packet size: ")); DBG(String(size, DEC)); DBG(F(" (")); DBG(String(size, HEX)); DBG(F(")"));
  //DBG(F("Event:   "));
  //decodeEvent(data[0]);
  //DBG(F("data: "));
  uint8_t csum = (size >> 8);
  csum += (size & 0xFF);
  //csum += data[0];
  for (uint8_t i = 0; i < size; i++) { //
    csum += data[i]; //DBG(String(data[i], HEX)); DBG(F((" "));
  }
  //DBG(F(("\n"));

  if (data[size] == (0x100 - csum) ) {
    //DBG(F("Checksum OK\n"));
    return true;
  } else {
    //DBG(F("Bad checksum\n"));
    return false;
  }
}

String IS2020::ConnectionStatus(uint8_t deviceId) {
  String ConnectionStatus = "";
  /*
            Value Parameter Description
            0xXX  "1 indicate connected
            0x01 Bit0 : A2DP profile signaling channel connected
            0x02 Bit1 : A2DP profile stream channel connected
            0x04 Bit2 : AVRCP profile connected
            0x08 Bit3 : HF profile connected
            0x10 Bit4 : SPP connected"
  */
  if (LinkStatus[1 + deviceId] & (1 << A2DP_profile_signaling_channel_connected)) ConnectionStatus += "A2DP profile signaling channel connected\n";
  if (LinkStatus[1 + deviceId] & (1 << A2DP_profile_stream_channel_connected)) ConnectionStatus += "A2DP profile stream channel connected\n";
  if (LinkStatus[1 + deviceId] & (1 << AVRCP_profile_connected)) ConnectionStatus += "AVRCP profile connected\n";
  if (LinkStatus[1 + deviceId] & (1 << HF_profile_connected)) ConnectionStatus += "HF profile connected\n";
  if (LinkStatus[1 + deviceId] & (1 << SPP_connected)) ConnectionStatus += "SPP connected\n";
  if (LinkStatus[1 + deviceId] == 0) ConnectionStatus = "Disconnected\n";
  return ConnectionStatus;
}

String IS2020::MusicStatus(uint8_t deviceId) {
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
  switch (LinkStatus[3 + deviceId]) {
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

String IS2020::StreamStatus(uint8_t deviceId) {
  /*
             database1_stream_status," SIZE: 1 BYTE
            Value Parameter Description
            0xXX  "1 indicate connected
            0x00: no stream
            0x01: stream on-going"
  */
  if (LinkStatus[5 + deviceId] == 0x00) {
    return ("No stream");
  } else {
    return ("Streaming");
  }
}

uint8_t IS2020::BatteryLevel(uint8_t deviceId) {
  return (currentBatteryLevel[deviceId]*100)/maxBatteryLevel[deviceId];
}

String IS2020::ModuleState() {
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

String IS2020::BtStatus() {

  switch (BTMState) {
    case 0x00:
      return (F("Power OFF state"));
      break;
    case 0x01:
      return (F("pairing state (discoverable mode)"));
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
      return ("unknown state" + (char)BTMState);
      break;
  }
}


void IS2020::setEventMask(uint32_t mask) {
  EventMask |= mask;
  Serial3.println(EventMask,HEX);
}

void IS2020::enableAllSettingEvent() {
  //0 : BTM will send this event
  //1 : BTM will not send this event"
  EventMask = 0x00000000;
  //bit0 reserved
  /*setEventMask(EMB_SPK_Module_state);
  setEventMask(EMB_call_status);
  setEventMask(EMB_incoming_call_number_or_caller_id);
  setEventMask(EMB_SMS_received);
  setEventMask(EMB_Missed_call);
  setEventMask(EMB_Max_cell_phone_battery_level);
  setEventMask(EMB_current_cell_phone_battery_level);

  setEventMask(EMB_cell_phone_roamming);
  setEventMask(EMB_Max_cell_phone_signal_strength);
  setEventMask(EMB_current_cell_phone_signal_strength);
  setEventMask(EMB_cell_phone_service_status);*/
      setEventMask(EMB_BTM_battery_level);
      setEventMask(EMB_BTM_charging_status);
      setEventMask(EMB_BMT_reset_to_default_setting_OK);
      setEventMask(EMB_BTM_DAC_gain_level);

 /* setEventMask(EMB_EQ_mode);
  setEventMask(EMB_remote_device_friendly_name);
  setEventMask(EMB_AVC_specific_response);
  setEventMask(EMB_unknown_AT_command_result_code);
  setEventMask(EMB_Page_status);
  setEventMask(EMB_Ringtone_status);
  setEventMask(EMB_amp_indication);
  setEventMask(EMB_line_in_status);*/
}


void IS2020::removeInfoAboutDevice(uint8_t deviceId) {
  deviceName[deviceId] = "";
  deviceInBandRingtone[deviceId] = 0;
  deviceIsiAP[deviceId] = 0;
  deviceSupportAVRCPA13[deviceId] = 0;
  deviceHfAndA2dpGain[deviceId] = 0;
  deviceLineInGain[deviceId] = 0;
  maxBatteryLevel[deviceId] = 0;
  currentBatteryLevel[deviceId] = 0;

  BTMstatusChanged = 1;
}
