#ifndef ISSC2020_h
#define ISSC2020_h


#include <inttypes.h>
#include <Arduino.h>
//#include <SoftwareSerial.h>
#include "MMI.h"
#include "Music.h"
#include "commands.h"
#include "events.h"


#define STARTBYTE 0xAA

#define DUMMYBYTE 0x00
#define DEBUG 1
#define DEVICENAME_LENGHT_SUPPORT 254

//class SoftwareSerial;

class IS2020
{
  public:
    IS2020(uint32_t baudrate);
    //: is2020Serial(USART3, TX, RX) {} //todo MFB and P0 port
    ~IS2020();

    
    uint8_t MakeCall(uint8_t deviceId, char phoneNumber[19]); //return event from BT will be handled elseware??? event call status
    uint8_t Make_Extension_Call(uint8_t deviceId, char phoneNumber[10]); ;
    void  MMI_Action  (uint8_t deviceId, uint8_t action);
    void  Event_Mask_Setting  ();
    void  Music_Control (uint8_t deviceId, uint8_t action);
    void  Change_Device_Name  ();
    void  Change_PIN_Code ();
    void  BTM_Parameter_Setting ();
    void  Read_BTM_Version  ();
    void  Get_PB_By_AT_Cmd  ();
    void  Vendor_AT_Command ();
    void  AVC_Specific_Cmd  ();
    void  AVC_Group_Navigation  ();
    void  Read_Link_Status  ();
    void  Read_Paired_Device_Record ();
    void  Read_Local_BD_Address ();
    void  Read_Local_Device_Name  ();
    void  Set_Access_PB_Method  ();
    void  Send_SPP_iAP_Data ();
    void  BTM_Utility_Function  ();
    void  Event_ACK (uint8_t cmd);
    void  Additional_Profiles_Link_Setup  ();
    void  Read_Linked_Device_Information(uint8_t deviceId, uint8_t query);
    void  Profiles_Link_Back  ();
    void  Disconnect  ();
    void  MCU_Status_Indication ();
    void  User_Confirm_SPP_Req_Reply  ();
    void  Set_HF_Gain_Level ();
    void  EQ_Mode_Setting ();
    void  DSP_NR_CTRL ();
    void  GPIO_Control  ();
    void  MCU_UART_Rx_Buffer_Size ();
    void  Voice_Prompt_Cmd  ();
    void  MAP_REQUEST ();
    void  Security_Bonding_Req  ();
    void  Set_Overall_Gain();
    int SerialAvailable();
    int SerialRead();
    int getNextEventFromBT();
    uint8_t BTMstatusChanged = 0; // set to 1 in EVT_BTM_Status, when this event happen main program shoud read array BTMStatus[]
    uint8_t callStatusChanged = 0; //set to 1 in case we are making/receiving call then check callStatus[]
    uint8_t BTMStatus[2] = {0, 0}; //state, link_info
    uint8_t callStatus[2] = {0, 0}; //data_base_index, call_status
    uint8_t callerId[2][32];
    uint8_t SMSstatus[2] = {0, 0}; //[0] - link 0 status, [1] link 1 status
    uint8_t missedCallStatus[2] = {0, 0};
    uint8_t maxBatteryLevel[2] = {0, 0};
    uint8_t currentBatteryLevel[2] = {0, 0};
    uint8_t roamingStatus[2] = {0, 0};
    uint8_t maxSignalLevel[2] = {0, 0};
    uint8_t currentSignalLevel[2] = {0, 0};
    uint8_t serviceStatus[2] = {0, 0};
    uint8_t BTMBatteryStatus[2] = {0, 0};
    uint8_t BTMChargingStatus = 0;
    uint8_t HFGainLevel[2] = {0, 0}; // this can be only 0x0F max, so we can bitbang device zero and one to one byte if we got to low mem state ...
    uint8_t EQMode = 0;
    uint8_t PBAPAccessFinish = 0;
    uint8_t deviceName[2][DEVICENAME_LENGHT_SUPPORT];//= {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    uint8_t deviceInBandRingtone[2] = {0, 0}; //remake this for some bit masked variable .... this is just 0 or 1 for each device ..
    uint8_t deviceIsiAP[2] = {0, 0};
    uint8_t deviceSupportAVRCPA13[2] = {0, 0} ; //reply if remote device support AVRCP v1.3
    uint8_t deviceHfAndA2dpGain[2] = {0, 0};
    uint8_t deviceLineInGain[2] = {0, 0};
    int BTMUartVersion = 0;
    int BTMFWVersion = 0;
    uint8_t LinkStatus[7] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t btAddress[7][6];

  private:
    //SoftwareSerial is2020Serial;
    //void IS2020_init(uint32_t baudrate);
    void SendPacketArrayInt(uint16_t packetSize, uint8_t cmd, uint8_t deviceId, uint8_t data[]);
    void SendPacketInt(uint8_t cmd, uint8_t data);
    void SendPacketArrayChar(uint16_t packetSize, uint8_t cmd, uint8_t deviceId, char data[]);
    uint8_t checkCkeckSum(int size, uint8_t data[]);
    void DBG(String text);

};


#endif
