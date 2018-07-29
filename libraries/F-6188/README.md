# arduino library for F-6188 #

This directory contain all files needed to support F-6188 a2dp bluetooth module based on Beken BK8000L chip.

F-6188 have AT command control support, supported commands are descriped <a href="https://github.com/tomaskovacik/kicad-library/tree/master/library/datasheet/F-6188_BK8000L">here</a>

# how to use it #

<a href="https://www.arduino.cc/en/Guide/Libraries">Information about using libraries on arduino site</a>

Copy content of this repository directory or just this two files: F6188.cpp, F6188.h to ~/Arduino/libraries/F-6188/ directory (if did not exist, create one). Open new project in arduino and put this line on top :

```arduino
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
```

for more examples look ate examples/F-6188/F-6188.ino

# supported functions #

<p style='color:red'>begin(uint32_t baudrate)</p> - parameter baudrate is communication speed between MCU and arduino, default 9600

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

# Handling responce from module #

As far as getNextEventFromBT() is called periodicaly, (it is called in all of functions mentioned above, just to handle responce to send command ASAP) library handle responce like this:

library has its own variables, which are set/updated when data are received from module:

BT_ADDR (string), updated after getAddress() is called

BT_NAME (string), updated after getName() is called

BT_PIN (string), updated after getPinCode() is called

CallerID  String, updated automatically or by calling getHFPstatus()

BTState enum, updated automatically or by calling getHFPstatus() or getConnectionStatus()

CallState enum, updated automatically or by calling callRedial() or getHFPstatus()

MusicState enum, updated automatically  or by calling getMusicStatus()

PowerState enum, updated automaticaly or by calling getHFPstatus(),getMusicStatus() or getConnectionStatus()

If you init F6188 library as in example code this variables can be accessed directly from main sketch with prefix BT, for example:

Serial.print(BT.BT_NAME);


BTState, CallState, MusicState and PowerState have these states, these are accessible from main sketch with prefix like other variable:

BT.Playing, responce from module "MA"

BT.Idle, responce from module "MB"

BT.IncomingCall, responce from module "IR- or M2"

BT.OutgoingCall, responce from module "PR- or M3"

BT.CallInProgress, responce from module "M4"

BT.Connected, responce from module "M1"

BT.Disconnected, responce from module "M0"

BT.On, responce from module "ON"

BT.Off, set if shutdownBT() is called

BT.Pairing, set if PairingInit() is called

these can be used to triger actions in main sketch based on change of module state see F-6188.ino in examples directory.





