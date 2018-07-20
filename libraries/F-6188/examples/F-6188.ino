/*
        GNU GPL v3
        (C) Tomas Kovacik [nail at nodomain dot sk]
        https://github.com/tomaskovacik/

	example for F-6188 module library
*/


#include "F6188.h"
#include <SoftwareSerial.h>

uint8_t BTState;
uint8_t CallState;
uint8_t MusicState;
uint8_t PowerState;

SoftwareSerial swSerial(7, 6); //rxPin, txPin, inverse_logic

F6188 BT(&swSerial);

void setup() {
  BT.begin(9600);
  Serial.begin(115200);
  Serial.println(F("press h for help"));
  BT.getName();
  BT.getConnectioStatus();
  BT.getPinCode();
  BT.getAddress();
  BT.getSoftwareVersion();
  BT.MusicGetStatus();
  BT.getHFPstatus();
}

void loop() {

  if (Serial.available() > 0)
  {
    // read the incoming byte:
    char c = Serial.read();
    switch (c)
    {
      case '1':
        BT.PairingInit();
        break;
      case '2':
        BT.PairingExit();
        break;
      case '3':
        BT.ConnectLastDevice();
        break;
      case '4':
        BT.Disconnect();
        break;
      case '5':
        BT.callAnsware();
        break;
      case '6':
        BT.callReject();
        break;
      case '7':
        BT.callHangUp();
        break;
      case '8':
        BT.callRedial();
        break;
      case '9':
        BT.volumeUp();
        break;
      case 'a':
        BT.volumeDown();
        break;
      case 'b':
        BT.languageSwitch();
        break;
      case 'c':
        BT.channelSwitch();
        break;
      case 'd':
        BT.shutdown_BT();
        break;
      case 'e':
        BT.switchInput();
        break;
      case 'f':
        {
          Serial.read(); //read \r
          Serial.read(); //read \n
          Serial.println(F("type command (without AT+) to by send:"));
          String str;
          c = 0;
          while (!Serial.available()) {}; //wait until user input something ... then read it:
          while (Serial.available() > 0) {
            c = Serial.read();
            if ( c == 0xD ) {
              break;
            } else {
              str = str + c;
            }
          }
          BT.sendData(str);
        }
        break;
      case 'g':
        BT.open_phone_voice();
        break;
      case 'h': //help
        Serial.println(F("PairingInit                  1"));
        Serial.println(F("PairingExit                  2"));
        Serial.println(F("ConnectLastDevice            3"));
        Serial.println(F("Disconnect                   4"));
        Serial.println(F("callAnsware                  5"));
        Serial.println(F("callReject                   6"));
        Serial.println(F("callHangUp                   7"));
        Serial.println(F("callRedial                   8"));
        Serial.println(F("volumeUp                     9"));
        Serial.println(F("volumeDown                   a"));
        Serial.println(F("languageSwitch               b"));
        Serial.println(F("channelSwitch                c"));
        Serial.println(F("shutdown_BT                  d"));
        Serial.println(F("switch input                 e"));
        Serial.println(F("send custom AT+command       f"));
        Serial.println(F("open_phone_voice             g"));
        Serial.println(F("                 i"));
        Serial.println(F("memoryClear                  j"));
        Serial.println(F("language_set_number      k+num"));
        Serial.println(F("musicTogglePlayPause         l"));
        Serial.println(F("musicStop                    m"));
        Serial.println(F("musicNextTrack               n"));
        Serial.println(F("musicPreviousTrack           o"));
        Serial.println(F("musicFastForward             q"));
        Serial.println(F("musicRewind                  r"));
        Serial.println(F("getName                      s"));
        Serial.println(F("getConnectioStatus           t"));
        Serial.println(F("getPinCode                   u"));
        Serial.println(F("getAddress                   v"));
        Serial.println(F("getSoftwareVersion           w"));
        Serial.println(F("MusicGetStatus               x"));
        Serial.println(F("getHFPstatus                 y"));
        break;
      case 'i':

        break;
      case 'j':
        BT.memoryClear();
        break;
      case 'k':
        {
          Serial.read(); //read \r
          Serial.read(); //read \n
          Serial.println(F("give me number 0-4"));
          char b;
          c = 0;
          while (!Serial.available()) {}; //wait until user input something ... then read it:
          if (Serial.available() > 0) {
            b = Serial.read();
          }
          BT.language_set_number(b - 0x30);
          c = 0;
        }
        break;
      case 'l':
        BT.musicTogglePlayPause();
        break;
      case 'm':
        BT.musicStop();
        break;
      case 'n':
        BT.musicNextTrack();
        break;
      case 'o':
        BT.musicPreviousTrack();
        break;
      case 'p':
        BT.musicFastForward();
        break;
      case 'r':
        BT.musicRewind();
        break;
      case 's':
        BT.getName();
        break;
      case 't':
        BT.getConnectioStatus();
        break;
      case 'u':
        BT.getPinCode();
        break;
      case 'v':
        BT.getAddress();
        break;
      case 'w':
        BT.getSoftwareVersion();
        break;
      case 'x':
        BT.MusicGetStatus();
        break;
      case 'y':
        BT.getHFPstatus();
        break;

    }
  }
 
  BT.getNextEventFromBT();

  if (BTState != BT.BTState) {
    switch (BT.BTState) {
      case BT.Connected:
        Serial.println(F("Bluetooth connected"));
        break;
      case BT.Disconnected:
        Serial.println(F("Bluetooth disconnected"));
        break;
    }
    BTState = BT.BTState;
  }

  if (CallState != BT.CallState) {
    switch (BT.CallState) {
      case (BT.IncomingCall):
        Serial.println(F("Incoming call:"));
        Serial.println(BT.CallerID);
        break;
      case (BT.OutgoingCall):
        Serial.println(F("Dialing:"));
        Serial.println(BT.CallerID);
        break;
      case (BT.CallInProgress):
        Serial.println(F("Calling:"));
        Serial.println(BT.CallerID);
        break;
    }
    CallState = BT.CallState;
  }

  if (MusicState != BT.MusicState) {
    switch (BT.MusicState) {
      case (BT.Playing):
        Serial.println(F("Playing music"));
        break;
      case (BT.Idle):
        Serial.println(F("Music stoped"));
        break;
    }
    MusicState = BT.MusicState;
  }

  if (PowerState != BT.PowerState) {
    switch (BT.PowerState) {
      case (BT.On):
        Serial.println(F("Module ON"));
        Serial.print(F("NAME: "));Serial.println(BT.BT_NAME);
        Serial.print(F("PIN: "));Serial.println(BT.BT_PIN);
        break;
      case (BT.Off):
        Serial.println(F("Module OFF"));
        break;
    }
    PowerState = BT.PowerState;
  }

}


