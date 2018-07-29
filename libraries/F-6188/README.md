# arduino library for F-6188 #

This directory contain all files needed to support F-6188 a2dp bluetooth module based on Beken BK8000L chip.

F-6188 have AT command control support, supported commands are descriped <a href="https://github.com/tomaskovacik/kicad-library/tree/master/library/datasheet/F-6188_BK8000L">here</a>

# how to use it #

Copy content of this repository directory or just this two files: F6188.cpp, F6188.h to ~/Arduino/libraries/F-6188/ directory (if did not exist, create one). Open new project in arduino and put this line on top :

<code>
#include "F6188.h"
#include <SoftwareSerial.h> //if using SW, with HW no need for this

SoftwareSerial swSerial(7, 6); //rxPin, txPin

F6188 BT(&swSerial); //in case of HW serial use for example: (&Serial1)

void(){
  BT.begin(); //or BT.begin(9600); for specific baudrate
}

void loop(){
BT.getNextEventFromBT(); //should be call periodically, to get notifications from module, for example if someone calling...
}
</code>

for more examples look ate examples/F-6188/F-6188.ino

# supported functions #

begin(uint32_t baudrate) - parameter baudrate is communication speed between MCU and arduino, default 9600
sendData(String cmd) - construct string of AT+cmd and send it to serial port 
getNextEventFromBT() - parse data send from module and send internal variales, call this periodicaly, to parse data received from module ASAP
PairingInit() - initialize pairing mode
PairingExit() - exit pairing mode
ConnectLastDevice() - connect to last device 
Disconnect() - disconnect from currently connected device
callAnsware() - answare currently receiving call, see CallerID variable to see who is calling
callReject() - reject currently receiving call,  see CallerID variable to see who is calling
callHangUp() - hang up current call, see CallerID variable to see who you are calling with
callRedial() - call last dialed number, see CallerID variable to see who you dialing
volumeUp() - set volume up
volumeDown() - set volume down
languageSwitch() - did not understand what is this doing based on provided info in datasheet (sending AT+CM)
channelSwitch() - did not understand what is this doing based on provided info in datasheet (sending AT+CO)
shutdownBT() - shutdown BT module
switchInput() - switch to AUX input 
openPhoneVoice() - did not understand what is this doing based on provided info in datasheet (sending AT+CV)
memoryClear() - did not understand what is this doing based on provided info in datasheet (sending AT+CZ)
languageSetNumber(uint8_t number) - did not understand what is this doing based on provided info in datasheet (sending AT+CM+number)
musicTogglePlayPause() - toggle play/pause music on mobile phone
musicStop() - stop playing music
musicNextTrack() - switch to next track
musicPreviousTrack() - switch to previous track
musicFastForward() - fast forward current track
musicRewind() - rewind current track
getName() - get bluetooth module name
getConnectioStatus() - get BT connection status 
getPinCode() - get pin
getAddress() - get HW address of module
getSoftwareVersion() - get SW version of module (this one did not work for me)
MusicGetStatus() - get playback status 
getHFPstatus() - get BT connection status
returnCallerID(String receivedString)
returnBtModuleName(String receivedString) 

