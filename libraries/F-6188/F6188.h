/*
	GNU GPL v3
	(C) Tomas Kovacik [nail at nodomain dot sk]
	https://github.com/tomaskovacik/

	F-6188 module library

	supports SW and HW serial

	to enable SW serial set USE_SW_SERIAL to 1
	to enable debug output set DEBUG to 1, this is using HW Serial (hardcoded)

*/

#ifndef F6188_h
#define F6188_h
#include <Arduino.h>

#define USE_SW_SERIAL 1

#define DEBUG 0 

//cmds definitions:
#define F6188_PAIRING_INIT "CA"            //  pairing   AT+CA\r\n     
#define F6188_PAIRING_EXIT "CB"            //  Exit pairing  AT+CB\r\n     
#define F6188_CONNECT_LAST_DEVICE "CC"     //  The last paired device connected  AT+CC\r\n     what this should do? connect to last connected device?
#define F6188_DISCONNECT "CD"              //  disconnect  AT+CD\r\n     
#define F6188_CALL_ANSWARE "CE"            //  Answer the call   AT+CD\r\n     
#define F6188_CALL_REJECT "CF"             //  reject a call   AT+CF\r\n     
#define F6188_CALL_HANGUP "CG"             //  Hang up   AT+CG\r\n     
#define F6188_CALL_REDIAL "CH"             //  redial  AT+CH\r\n     last called number?
#define F6188_VOLUME_UP "CK"               //  volume up   AT+CK\r\n     
#define F6188_VOLUME_DOWN "CL"             //  volume down   AT+CL\r\n     
#define F6188_LANGUAGE_SWITCH  "CM"        //  Multi-language switch   AT+CM\r\n     
#define F6188_CHANNEL_SWITCH "CO"          //  Channel switching (invalid)   AT+CO\r\n     to be tested
#define F6188_SHUTDOWN "CP"                //  Shutdown  AT+CP\r\n     
#define F6188_SWITCH_INPUT "CT"            //  Enter the test mode   AT+CT\r\n     
#define F6188_OPEN_PHONE_VOICE "CV"        //  Open phone VOICE  AT+CV\r\n     
#define F6188_MEMORY_CLEAR "CZ"            //  Memory clear  AT+CZ\r\n     
#define F6188_LANGUAGE_SET_NUMBER "CMM"    //  Number:( 0-4 )  Set the number of multi-lingual   AT+CMM4\r\n     
#define F6188_MUSIC_TOGGLE_PLAY_PAUSE "MA" //  Music Play / Pause  AT+MA\r\n     
#define F6188_MUSIC_STOP "MC"              //  The music stops   AT+MC\r\n     
#define F6188_MUSIC_NEXT_TRACK "MD"        //  next track  AT+MD\r\n     
#define F6188_MUSIC_PREVIOUS_TRACK "ME"    //  previous track  AT+ME\r\n     
#define F6188_MUSIC_FAST_FORWARD "MF"      //  fast forward  AT+MF\r\n     test how does this exacly works?
#define F6188_MUSIC_REWIND "MH"            //  rewind  AT+MH\r\n     test how does this exacly works?
#define F6188_GET_NAME "MN"                //  Query bluetooth name  AT+MN\r\n   NA:BK8000L\r\n  test this
#define F6188_GET_CONNECTION_STATUS "MO"   //  Bluetooth connection status inquiry   AT+MO\rn  connection succeeded:" C1\r\n"no connection:"C0\r\n"  
#define F6188_GET_PIN_CODE "MP"            //  PIN Code query  AT+MP\r\n   PN:0000\r\n   
#define F6188_GET_ADDRESS "MR"             //  Query bluetooth address   AT+MR\r\n   AD:111111111111\r\n   
#define F6188_GET_SOFTWARE_VERSION "MQ"    //  Query software version  AT+MQ\r\n   XZX-V1.2\r\n  
#define F6188_MUSIC_GET_STATUS "MV "       //  Bluetooth playback status inquiry   AT+MV\r\n   Play: "MB\r\n", time out:"MA\r\n", disconnect:" M0\r\n"   
#define F6188_GET_HFP_STATUS "MY"          //  Bluetooth inquiry HFP status  AT+MY\r\n   disconnect:"M0\r\n", connection:"M1\r\n", Caller: "M2\r\n", Outgoing: "M3\r\n", calling:"M4\r\n"  



#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
#include <SoftwareSerial.h>
#else
#include <NewSoftSerial.h>
#endif
#endif


class F6188
{
  public:

    uint8_t BTState;
    uint8_t CallState;
    uint8_t MusicState;
    uint8_t PowerState;

    String CallerID;

    enum STATES
    {
      Playing, //MA
      Idle, //MB
      IncomingCall, //IR- or M2
      OutgoingCall, //PR- or M3
      CallInProgress, //M4
      Connected, // M1
      Disconnected, //M0
      On,
      Off,
      Pairing
    };

#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
    F6188(SoftwareSerial *ser);
#else
    F6188(NewSoftSerial  *ser);
#endif
#endif
    F6188(HardwareSerial *ser);
    void begin(uint32_t baudrate);
    ~F6188();
    uint8_t sendData(String cmd);
    uint8_t getNextEventFromBT();

    uint8_t PairingInit();
    uint8_t PairingExit();
    uint8_t ConnectLastDevice();
    uint8_t Disconnect();
    uint8_t callAnsware();
    uint8_t callReject();
    uint8_t callHangUp();
    uint8_t callRedial();
    uint8_t volumeUp();
    uint8_t volumeDown();
    uint8_t languageSwitch();
    uint8_t channelSwitch();
    uint8_t shutdownBT();
    uint8_t switchInput();
    uint8_t openPhoneVoice();
    uint8_t memoryClear();
    uint8_t languageSetNumber(uint8_t number);
    uint8_t musicTogglePlayPause();
    uint8_t musicStop();
    uint8_t musicNextTrack();
    uint8_t musicPreviousTrack();
    uint8_t musicFastForward();
    uint8_t musicRewind();
    uint8_t getName();
    uint8_t getConnectioStatus();
    uint8_t getPinCode();
    uint8_t getAddress();
    uint8_t getSoftwareVersion();
    uint8_t MusicGetStatus();
    uint8_t getHFPstatus();

    uint8_t decodeReceivedString(String receivedString);
    String returnCallerID(String receivedString); //return number between ""
    String returnBtModuleName(String receivedString); //return module name between : and \0
    String BT_ADDR;
    String BT_NAME;
    String BT_PIN;

  private:
    void DBG(String text);

#if  defined(USE_SW_SERIAL)
#if ARDUINO >= 100
    SoftwareSerial *btSwSerial;
#else
    NewSoftSerial  *btSwSerial;
#endif
#endif
    HardwareSerial *btHwSerial;

};


#endif
