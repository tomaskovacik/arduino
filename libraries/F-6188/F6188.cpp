/*
        GNU GPL v3
        (C) Tomas Kovacik [nail at nodomain dot sk]
        https://github.com/tomaskovacik/

	F-6188 module library
*/

#include "F6188.h"
#include <Arduino.h>

#if defined(USE_SW_SERIAL)
#include <SoftwareSerial.h>
#endif



#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
F6188::F6188(SoftwareSerial *ser)
#else
F6188::F6188(NewSoftSerial *ser)
#endif
#endif
{
#if defined(USE_SW_SERIAL)
  btHwSerial = NULL;
  btSwSerial = ser;
}
#endif
F6188::F6188(HardwareSerial *ser) {
  btSwSerial = NULL;
  btHwSerial = ser;
}

/*
   Destructor
*/
F6188::~F6188() {
}

void F6188::begin(uint32_t baudrate = 9600) {
#if defined(USE_SW_SERIAL)
  if (btSwSerial)
    btSwSerial->begin(baudrate);
  else
#endif
    btHwSerial->begin(baudrate);
}

/*
   debug output
*/
void F6188::DBG(String text) {
  if (DEBUG) /*return "DBG: ");*/ Serial.print(text);;
}

uint8_t F6188::decodeReceivedString(String receivedString) {
  DBG(receivedString);
  DBG("\n");
  switch (receivedString[0]) {
    case 'A':
      {
        if (receivedString[1] == 'D' && receivedString[2] == ':') {
            BT_ADDR = receivedString.substring(5);
            DBG("BT ADDRESS: " + BT_ADDR);
        }
      }
      break;
    case 'C':
      switch (receivedString[1]) {
        case '1':
          BTState = Connected;
          break;
        case '0':
          BTState = Disconnected;
          break;
      }
      break;
    case 'E':
      switch (receivedString[1]) {
        case 'R':
          if (receivedString[2] == 'R') return 0;
          break;
      }
      break;
    case 'I':// connection info
      switch (receivedString[1]) {
        case 'I': //BT connected
          BTState = Connected;
          break;
        case 'A': //BT disconected
          BTState = Disconnected;
          break;
        case 'R': //caller
          if (receivedString[2] == '-') CallState = IncomingCall;
          CallerID = returnCallerID(receivedString);
          break;
      }
      break;
    case 'M': //music
      switch (receivedString[1]) {
        case 'B':
          MusicState = Playing;
          break;
        case 'A':
          MusicState = Idle;
          break;
        case '0':
          BTState = Disconnected;
          break;
        case '1':
          BTState = Connected;
          break;
        case '2':
          CallState = IncomingCall;
          break;
        case '3':
          CallState = OutgoingCall;
          break;
        case '4':
          CallState = CallInProgress;
          break;
      }
      break;
    case 'N':
        if (receivedString[1] == 'A' && receivedString[2] == ':') {//name
          BT_NAME = F6188::returnBtModuleName(receivedString);
        }
      break;
    case 'P':
      switch (receivedString[1]) {
      case 'R': //outgoing call
        if (receivedString[2] == '-') CallState = OutgoingCall;
          CallerID = returnCallerID(receivedString);
      break;
      case 'N':
          if (receivedString[2] == ':') {
            BT_PIN = receivedString.substring(4);
          }
      break;
      }
    break;
    case 'O': //BT On
        switch (receivedString[1]) {
        case 'N':
          PowerState = On;
          break;
        case 'K':
          return 1;
          break;
      }
    break;
    case 0xA: //\r
	F6188::decodeReceivedString(receivedString.substring(1));
    break;
    case 0x20: //space
	F6188::decodeReceivedString(receivedString.substring(1));
    break;
}
return 1;
}

String F6188::returnCallerID(String receivedString) {
	DBG("Calling: " + receivedString.substring(4,(receivedString.length() - 2)) + "\n");
	return receivedString.substring(4,(receivedString.length() - 2)); //start at 4 cose: IR-"+123456789" or PR-"+123456789" and one before end to remove " and \0
}

String F6188::returnBtModuleName(String receivedString) {
	DBG("Bluetooth module name: " + receivedString.substring(4) + "\n");
	return receivedString.substring(4);
}

uint8_t F6188::getNextEventFromBT() {

  char c;
  String receivedString = "";

#if defined(USE_SW_SERIAL)
  while (btSwSerial -> available() > 0) {
    c = (btSwSerial -> read());
    //Serial.write(c);Serial.print(" ");Serial.println(c,HEX);
    if (c == 0xD) {
      if (receivedString == "") { //nothing before enter was received
        //DBG("received only empty string\n running again myself...\n");
        F6188::getNextEventFromBT();
      }
      receivedString = receivedString + c;
      decodeReceivedString(receivedString);
      break;
    }
    //append received buffer with received character
    receivedString = receivedString + c;  // cose += c did not work ...
  }

#else
  while (btHwSerial -> available() > 0) {
    c = (btHwSerial -> read());
    //Serial.write(c);Serial.print(" ");Serial.println(c,HEX);
    if (c == 0xD) {
      if (receivedString == "") { //nothing before enter was received
        F6188::getNextEventFromBT();
      }
      receivedString = receivedString + c;
      decodeReceivedString(receivedString);
      break;
    }
    //append received buffer with received character
    receivedString = receivedString + c;  // cose += c did not work ...
#endif
}

uint8_t F6188::sendData(String cmd) {
  String Command = "AT+" + cmd + "\r\n";
  DBG("sending " + Command);
  delay(100);
#if defined(USE_SW_SERIAL)
  if (btSwSerial)
    btSwSerial -> print(Command);
  else
#endif
    btHwSerial -> print(Command);
}

uint8_t F6188::PairingInit() { //  pairing   AT+CA\r\n
  F6188::sendData(F6188_PAIRING_INIT);
  F6188::getNextEventFromBT();
}

uint8_t F6188::PairingExit() {//  Exit pairing  AT+CB\r\n
  F6188::sendData(F6188_PAIRING_EXIT);
  F6188::getNextEventFromBT();
}

uint8_t F6188::ConnectLastDevice() {//  The last paired device connected  AT+CC\r\n     what this should do? connect to last connected device?
  F6188::sendData(F6188_CONNECT_LAST_DEVICE);
  F6188::getNextEventFromBT();
}

uint8_t F6188::Disconnect() {//  disconnect  AT+CD\r\n
  F6188::sendData(F6188_DISCONNECT);
  F6188::getNextEventFromBT();
}

uint8_t F6188::callAnsware() { //  Answer the call   AT+CD\r\n
  F6188::sendData(F6188_CALL_ANSWARE);
  F6188::getNextEventFromBT();
}

uint8_t F6188::callReject() { //  reject a call   AT+CF\r\n
  F6188::sendData(F6188_CALL_REJECT);
  F6188::getNextEventFromBT();
}

uint8_t F6188::callHangUp() { //  Hang up   AT+CG\r\n
  F6188::sendData(F6188_CALL_HANGUP);
  F6188::getNextEventFromBT();
}

uint8_t F6188::callRedial() { //  redial  AT+CH\r\n     last called number?
  F6188::sendData(F6188_CALL_REDIAL);
  F6188::getNextEventFromBT();
}

uint8_t F6188::volumeUp() { //  volume up   AT+CK\r\n
  F6188::sendData(F6188_VOLUME_UP);
  F6188::getNextEventFromBT();
}

uint8_t F6188::volumeDown() { //  volume down   AT+CL\r\n
  F6188::sendData(F6188_VOLUME_DOWN);
  F6188::getNextEventFromBT();
}

uint8_t F6188::languageSwitch() { //  Multi-language switch   AT+CM\r\n
  F6188::sendData(F6188_LANGUAGE_SWITCH);
  F6188::getNextEventFromBT();
}

uint8_t F6188::channelSwitch() { //  Channel switching (invalid)   AT+CO\r\n     to be tested
  F6188::sendData(F6188_CHANNEL_SWITCH);
  F6188::getNextEventFromBT();
}

uint8_t F6188::shutdownBT() { //  Shutdown  AT+CP\r\n
  F6188::sendData(F6188_SHUTDOWN);
  F6188::getNextEventFromBT();
}

uint8_t F6188::switchInput() { //  Enter the test mode   AT+CT\r\n
  F6188::sendData(F6188_SWITCH_INPUT);
  F6188::getNextEventFromBT();
}

uint8_t F6188::openPhoneVoice() { //  Open phone VOICE  AT+CV\r\n
  F6188::sendData(F6188_OPEN_PHONE_VOICE);
  F6188::getNextEventFromBT();
}

uint8_t F6188::memoryClear() { //  Memory clear  AT+CZ\r\n
  F6188::sendData(F6188_MEMORY_CLEAR);
  F6188::getNextEventFromBT();
}

uint8_t F6188::languageSetNumber(uint8_t number) { //  Number:( 0-4 )  Set the number of multi-lingual   AT+CMM4\r\n
  //DBG((String)number);
  String command = F6188_LANGUAGE_SET_NUMBER + (String)number;
  F6188::sendData(command);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicTogglePlayPause() { //  Music Play / Pause  AT+MA\r\n
  F6188::sendData(F6188_MUSIC_TOGGLE_PLAY_PAUSE);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicStop() { //  The music stops   AT+MC\r\n
  F6188::sendData(F6188_MUSIC_STOP);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicNextTrack() { //  next track  AT+MD\r\n
  F6188::sendData(F6188_MUSIC_NEXT_TRACK);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicPreviousTrack() { //  previous track  AT+ME\r\n
  F6188::sendData(F6188_MUSIC_PREVIOUS_TRACK);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicFastForward() { //  fast forward  AT+MF\r\n     test how does this exacly works?
  F6188::sendData(F6188_MUSIC_FAST_FORWARD);
  F6188::getNextEventFromBT();
}

uint8_t F6188::musicRewind() { //  rewind  AT+MH\r\n     test how does this exacly works?
  F6188::sendData(F6188_MUSIC_REWIND);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getName() { //  Query bluetooth name  AT+MN\r\n   NA:BK8000L\r\n  test this
  F6188::sendData(F6188_GET_NAME);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getConnectionStatus() { //  Bluetooth connection status inquiry   AT+MO\rn  connection succeeded:" C1\r\n"no connection:"C0\r\n"
  F6188::sendData(F6188_GET_CONNECTION_STATUS);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getPinCode() {           //  PIN Code query  AT+MP\r\n   PN:0000\r\n
  F6188::sendData(F6188_GET_PIN_CODE);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getAddress() { //  Query bluetooth address   AT+MR\r\n   AD:111111111111\r\n
  F6188::sendData(F6188_GET_ADDRESS);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getSoftwareVersion() { //  Query software version  AT+MQ\r\n   XZX-V1.2\r\n
  F6188::sendData(F6188_GET_SOFTWARE_VERSION);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getMusicStatus() { //  Bluetooth playback status inquiry   AT+MV\r\n   Play: "MB\r\n", time out:"MA\r\n", disconnect:" M0\r\n"
  F6188::sendData(F6188_MUSIC_GET_STATUS);
  F6188::getNextEventFromBT();
}

uint8_t F6188::getHFPstatus() { //Bluetooth inquiry HFP status  AT+MY\r\n   disconnect:"M0\r\n", connection:"M1\r\n", Caller: "M2\r\n", Outgoing: "M3\r\n", calling:"M4\r\n"
  F6188::sendData(F6188_GET_HFP_STATUS);
  F6188::getNextEventFromBT();
}


