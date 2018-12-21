#ifndef IS2020_h
#define IS2020_h

#include <Arduino.h>
#include "MMI.h"
#include "Music.h"
#include "commands.h"
#include "events.h"
#include  "AVRCP.h"


#define STARTBYTE 0xAA

#define DUMMYBYTE 0x00
#define DEBUG 1

#define DEVICENAME_LENGHT_SUPPORT 24

//class SoftwareSerial;

class IS2020
{
  public:
    IS2020(HardwareSerial *ser);
    void begin(uint32_t baudrate, uint8_t _reset);
    ~IS2020();


    uint8_t  MakeCall(uint8_t deviceId, char phoneNumber[19]); //return event from BT will be handled elseware??? event call status
    uint8_t  Make_Extension_Call(uint8_t deviceId, char phoneNumber[10]); ;
    uint8_t  MMI_Action  (uint8_t deviceId, uint8_t action);
    uint8_t  Event_Mask_Setting  ();
    uint8_t  Music_Control (uint8_t deviceId, uint8_t action);
    uint8_t  Change_Device_Name(String name);
    uint8_t  Change_PIN_Code ();
    uint8_t  BTM_Parameter_Setting ();
    uint8_t  Read_BTM_Version  ();
    uint8_t  Get_PB_By_AT_Cmd  (uint8_t deviceId);
    uint8_t  Vendor_AT_Command ();
    uint8_t  AVRCP_Specific_Cmd  ();
    uint8_t  AVRCP_Group_Navigation  ();
    uint8_t  Read_Link_Status  ();
    uint8_t  Read_Paired_Device_Record ();
    uint8_t  Read_Local_BD_Address ();
    uint8_t  Read_Local_Device_Name  ();
    uint8_t  Set_Access_PB_Method  ();
    uint8_t  Send_SPP_iAP_Data ();
    uint8_t  BTM_Utility_Function  ();
    uint8_t  Event_ACK (uint8_t cmd);
    uint8_t  Additional_Profiles_Link_Setup  ();
    uint8_t  Read_Linked_Device_Information(uint8_t deviceId, uint8_t query);
    uint8_t  Profiles_Link_Back  ();
    uint8_t  Disconnect  ();
    uint8_t  MCU_Status_Indication ();
    uint8_t  User_Confirm_SPP_Req_Reply  ();
    uint8_t  Set_HF_Gain_Level ();
    uint8_t  EQ_Mode_Setting ();
    uint8_t  DSP_NR_CTRL ();
    uint8_t  GPIO_Control  ();
    uint8_t  MCU_UART_Rx_Buffer_Size ();
    uint8_t  Voice_Prompt_Cmd  ();
    uint8_t  MAP_REQUEST ();
    uint8_t  Security_Bonding_Req  ();
    uint8_t  Set_Overall_Gain();
    int      SerialAvailable();
    int      SerialRead();

    uint8_t checkResponce(uint8_t eventID);
    uint8_t getNextEventFromBT();
    uint8_t BTMstatusChanged = 0; // set to 1 in EVT_BTM_Status, when this event happen main program shoud read array BTMStatus[]
    uint8_t callStatusChanged = 0; //set to 1 in case we are making/receiving call then check callStatus[]
    uint8_t BTMState;
    uint8_t BTMlink_info;
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
    String deviceName[2];
    uint8_t deviceInBandRingtone[2] = {0, 0}; //remake this for some bit masked variable .... this is just 0 or 1 for each device ..
    uint8_t deviceIsiAP[2] = {0, 0};
    uint8_t deviceSupportAVRCPA13[2] = {0, 0} ; //reply if remote device support AVRCP v1.3
    uint8_t deviceHfAndA2dpGain[2] = {0, 0};
    uint8_t deviceLineInGain[2] = {0, 0};
    int     BTMUartVersion = 0;
    int     BTMFWVersion = 0;
    uint8_t LinkStatus[7] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t btAddress[7][7]; //byte0 = link priority, 1-6 BT addr
    uint8_t moduleBtAddress[6];
    void removeInfoAboutDevice(uint8_t deviceId);

    //MMI specific functions
    uint8_t Add_remove_SCO_link(uint8_t deviceID);
    uint8_t Force_end_active_call(uint8_t deviceID);
    uint8_t Accept_incoming_call(uint8_t deviceID);
    uint8_t Reject_incoming_call(uint8_t deviceID);
    uint8_t End_call_transfer_to_headset(uint8_t deviceID);
    uint8_t Togle_mic_mute(uint8_t deviceID);
    uint8_t Mute_mic(uint8_t deviceID);
    uint8_t Unmute_mic(uint8_t deviceID);
    uint8_t Voice_dial(uint8_t deviceID);
    uint8_t Cancel_voice_dial(uint8_t deviceID);
    uint8_t Last_number_redial(uint8_t deviceID);
    uint8_t Togle_active_hold_call(uint8_t deviceID);
    uint8_t Voice_transfer(uint8_t deviceID);
    uint8_t Query_call_list_info(uint8_t deviceID);
    uint8_t Thee_way_call(uint8_t deviceID);
    uint8_t Release_waiting_or_hold_call(uint8_t deviceID);
    uint8_t End_active_call_accept_waiting_or_held_call(uint8_t deviceID);
    uint8_t Initiate_HF_connection(uint8_t deviceID);
    uint8_t Disconnect_HF_link(uint8_t deviceID);
    uint8_t Increase_microphone_gain(uint8_t deviceID);
    uint8_t Decrease_microphone_gain(uint8_t deviceID);
    uint8_t Switch_primary_seconday_HF(uint8_t deviceID);
    uint8_t Increase_speaker_gain(uint8_t deviceID);
    uint8_t Decrease_speaker_gain(uint8_t deviceID);
    uint8_t Next_song(uint8_t deviceID);
    uint8_t Previous_song(uint8_t deviceID);
    uint8_t Disconnect_A2DP_link(uint8_t deviceID);
    uint8_t Next_audio_efect(uint8_t deviceID);
    uint8_t Previous_audio_efect(uint8_t deviceID);
    uint8_t Enter_pairing_mode(uint8_t deviceID);
    uint8_t Power_on_button_press(uint8_t deviceID);
    uint8_t Power_on_button_release(uint8_t deviceID);
    uint8_t Eeprom_to_defaults(uint8_t deviceID);
    uint8_t Enter_pairing_mode_fast(uint8_t deviceID);
    uint8_t Power_off(uint8_t deviceID);
    uint8_t Toggle_buzzer(uint8_t deviceID);
    uint8_t Disable_buzzer(uint8_t deviceID);
    uint8_t Toggle_buzzer2(uint8_t deviceID);
    uint8_t Change_tone(uint8_t deviceID);
    uint8_t BatteryStatus(uint8_t deviceID);
    uint8_t Exit_pairing_mode(uint8_t deviceID);
    uint8_t Link_past_device(uint8_t deviceID);
    uint8_t Disconnect_all_link(uint8_t deviceID);
    uint8_t Triger_to_query_call_list_info(uint8_t deviceID);

    //Mucic control
    uint8_t STOP_FFW_RWD(uint8_t deviceId);
    uint8_t FFW(uint8_t deviceId);
    uint8_t REPFFW(uint8_t deviceId);
    uint8_t RWD(uint8_t deviceId);
    uint8_t REPRWD(uint8_t deviceId);
    uint8_t PLAY(uint8_t deviceId);
    uint8_t PAUSE(uint8_t deviceId);
    uint8_t TOGLE_PLAY_PAUSE(uint8_t deviceId);
    uint8_t STOP(uint8_t deviceId);


    String ModuleState();
    String BtStatus();
    String MusicStatus(uint8_t deviceID);
    String ConnectionStatus(uint8_t deviceId);
    String StreamStatus(uint8_t deviceId);
    uint8_t BatteryLevel(uint8_t deviceId);

    uint32_t EventMask = 0xFC0F0200;
    void setEventMask(uint32_t mask);
    void enableAllSettingEvent();



    String LocalDeviceName;

    //AVRCP
    void decodeAVRCP_Pdu(uint8_t pdu);
    void decodeAVRCP_Event(uint8_t Event);
    void decodeAVRCP_PlayerAtributes(uint8_t attribute);
    void RegisterAllEvents(uint8_t deviceId);
    uint8_t  AVRCP_Get_Element_Attributes_command(uint8_t deviceId);
    uint8_t  AVRCP_Registration_for_notification_of_Event(uint8_t deviceId, uint8_t event, uint8_t param1=0x00, uint8_t param2=0x00, uint8_t param3=0x00, uint8_t param4=0x00);
    uint8_t  AVRCP_List_Application_Settings_Attributes_command(uint8_t deviceId);
    uint8_t  AVRCP_Get_Capabilities(uint8_t deviceId, uint8_t capId = CAP_EVENTS_SUPPORTED);
    uint8_t  AVRCP_List_Player_Attributes(uint8_t deviceId);
    uint8_t  AVRCP_List_Player_Values(uint8_t deviceId, uint8_t attribute);
    uint8_t  AVRCP_Get_Current_Player_Value(uint8_t deviceId, uint8_t attribute);
    uint8_t  AVRCP_Set_Player_Value(uint8_t deviceId, uint8_t attribute, uint8_t value);
    uint8_t  AVRCP_Get_Player_Attribute_Text(uint8_t deviceId,uint8_t attribute);
    uint8_t  AVRCP_Get_Player_Value_Text(uint8_t deviceId, uint8_t attribute, uint8_t setting);
    uint8_t  AVRCP_Displayable_Charset(uint8_t deviceId);
    uint8_t  AVRCP_Get_Element_Attributes(uint8_t deviceId);
    uint8_t  AVRCP_Get_Element_Attributes_All(uint8_t deviceId);
    uint8_t  AVRCP_Get_Play_Status(uint8_t deviceId);
    uint8_t  AVRCP_Request_Continuing(uint8_t deviceId, uint8_t pdu);
    uint8_t  AVRCP_Abort_Continuing(uint8_t deviceId, uint8_t pdu);
    uint8_t  AVRCP_Set_Absolute_Volume(uint8_t deviceId, uint8_t volume);
    uint8_t  AVRCP_Set_Addressed_Player(uint8_t deviceId, uint16_t player);
    uint8_t  AVRCP_Set_Browsed_Player(uint8_t deviceId, uint16_t player);
    uint8_t  AVRCP_Get_Folder_Items(uint8_t deviceId, uint8_t scope,uint32_t start, uint8_t end);
    uint8_t  AVRCP_Change_Path(uint8_t deviceId, uint8_t direction=0x01, uint64_t folderUID=5);

    //reset module
    void resetModule();
    uint8_t query_device_name(uint8_t deviceId);
    uint8_t query_Line_In_gain(uint8_t deviceId);
    uint8_t query_HF_A2DP_gain(uint8_t deviceId);
    uint8_t query_if_remote_device_support_AVRCP_v13(uint8_t deviceId);
    uint8_t query_if_remote_device_is_iAP_device(uint8_t deviceId);
    uint8_t query_in_band_ringtone_status(uint8_t deviceId);

  private:
    void     decodeEvent(uint8_t Event);
    void decodeCommand(uint8_t cmd);
    uint8_t _reset;
    void resetLow();
    void resetHigh();
    void SendPacketArrayInt(uint16_t packetSize, uint8_t cmd, uint8_t deviceId, uint8_t data[]);
    void SendPacketInt(uint8_t cmd, uint8_t data);
    void SendPacketString(uint8_t cmd, String str);
    void SendPacketArrayChar(uint16_t packetSize, uint8_t cmd, uint8_t deviceId, char data[]);
    uint8_t checkCkeckSum(int size, uint8_t data[]);
    void DBG(String text);
    HardwareSerial *btSerial;
};


#endif
