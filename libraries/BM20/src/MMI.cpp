#include "IS2020.h"
#include <Arduino.h>

/*

  Command Format:   Command Command ID  Command Parameters      Return Event
  MMI_Action        0x02                data_base_index, action

  Description:  MMI action

  Command Parameters: data_base_index SIZE: 1 BYTE
  Value Parameter Description
  0x00  database 0 that related to a dedicate HF device
  0x01  database 1 that related to a dedicate HF device

  action  SIZE: 1 BYTE
  Value Parameter Description
  0x01  add/remove SCO link
  0x02  force end active call
  0x04  Accept an incoming call
  0x05  Reject an incoming call
  0x06  1. End call if SCO exist. 2. Voice transfer to headset if SCO not exist.
  0x07  1. Mute microphone if microphone is not mute 2. Active microphone if microphone is mute
  0x08  Mute microphone
  0x09  Active microphone
  0x0A  voice dial
  0x0B  cancel voice dial
  0x0C  last number redial
  0x0D  Set the active call on hold and active the hold call
  0x0E  voice transfer
  0x0F  Query call list information(CLCC)
  0x10  three way call
  0x11  release the waiting call or on hold call
  0x12  accept the waiting call or active the on hold call and release the active call
  0x16  initiate HF connection
  0x17  disconnect HF link
  0x24  increase microphone gain
  0x25  decrease microphone gain
  0x26  switch primary HF device and secondary HF device role
  0x30  increase speaker gain
  0x31  decrease speaker gain
  0x34  Next song
  0x35  previous song
  0x3B  Disconnect A2DP link
  0x3C  next audio effect
  0x3D  previous audio effect
  0x50  enter pairing mode (from power off state)
  0x51  power on button press
  0x52  power on button release
  0x56  Reset some eeprom setting to default setting
  0x5D  fast enter pairing mode (from non-off mode)
  0x5E  switch power off:  to execute the power_off process directly, actually, the combine command set, power off button press and release, could be replace by this command.
  0x60  Enable buzzer if buzzer is OFF Disable buzzer if buzzer is ON
  0x61  Disable buzzer
  0x62  Enable buzzer Disable buzzer if buzzer is ON
  0x63  Change tone set (SPK module support two sets of tone)
  0x6A  Indicate battery status
  0x6B  Exit pairing mode
  0x6C  link last device
  0x6D  disconnect all link


  Note1:Query call list information(CLCC)
  action  SIZE: 1 BYTE
  Value Parameter Description
  0x0f  trigger SPK Module to query call list information


*/

uint8_t IS2020::Add_remove_SCO_link(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_add_remove_SCO_link);
  }
  
uint8_t IS2020::Force_end_active_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_force_end_active_call);
  }
  
uint8_t IS2020::Accept_incoming_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_accept_incoming_call);
  }
  
uint8_t IS2020::Reject_incoming_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_reject_incoming_call);
  }
  
uint8_t IS2020::End_call_transfer_to_headset(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_end_call_transfer_to_headset);
  }
  
uint8_t IS2020::Togle_mic_mute(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_togle_mic_mute);
  }
  
uint8_t IS2020::Mute_mic(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_mute_mic);
  }
  
uint8_t IS2020::Unmute_mic(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_unmute_mic);
  }
  
uint8_t IS2020::Voice_dial(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_voice_dial);
  }
  
uint8_t IS2020::Cancel_voice_dial(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_cancel_voice_dial);
  }
  
uint8_t IS2020::Last_number_redial(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_last_number_redial);
  }
  
uint8_t IS2020::Togle_active_hold_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_togle_active_hold_call);
  }
  
uint8_t IS2020::Voice_transfer(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_voice_transfer);
  }
  
uint8_t IS2020::Query_call_list_info(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_query_call_list_info);
  }
  
uint8_t IS2020::Thee_way_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_thee_way_call);
  }
  
uint8_t IS2020::Release_waiting_or_hold_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_release_waiting_or_hold_call);
  }
  
uint8_t IS2020::End_active_call_accept_waiting_or_held_call(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_end_active_call_accept_waiting_or_held_call);
  }
  
uint8_t IS2020::Initiate_HF_connection(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_initiate_HF_connection);
  }
  
uint8_t IS2020::Disconnect_HF_link(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_disconnect_HF_link);
  }
  
uint8_t IS2020::Increase_microphone_gain(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_increase_microphone_gain);
  }
  
uint8_t IS2020::Decrease_microphone_gain(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_decrease_microphone_gain);
  }
  
uint8_t IS2020::Switch_primary_seconday_HF(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_switch_primary_seconday_HF);
  }
  
uint8_t IS2020::Increase_speaker_gain(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_increase_speaker_gain);
  }
  
uint8_t IS2020::Decrease_speaker_gain(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_decrease_speaker_gain);
  }
  
uint8_t IS2020::Next_song(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_next_song);
  }
  
uint8_t IS2020::Previous_song(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_previous_song);
  }
  
uint8_t IS2020::Disconnect_A2DP_link(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_disconnect_A2DP_link);
  }
  
uint8_t IS2020::Next_audio_efect(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_next_audio_efect);
  }
  
uint8_t IS2020::Previous_audio_efect(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_previous_audio_efect);
  }
  
uint8_t IS2020::Enter_pairing_mode(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_enter_pairing_mode);
  }
  
uint8_t IS2020::Power_on_button_press(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_power_on_button_press);
  }
  
uint8_t IS2020::Power_on_button_release(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_power_on_button_release);
  }
  
uint8_t IS2020::Eeprom_to_defaults(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_eeprom_to_defaults);
  }
  
uint8_t IS2020::Enter_pairing_mode_fast(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_enter_pairing_mode_fast);
  }
  
uint8_t IS2020::Power_off(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_power_off);
  }
  
uint8_t IS2020::Toggle_buzzer(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_toggle_buzzer);
  }
  
uint8_t IS2020::Disable_buzzer(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_disable_buzzer);
  }
  
uint8_t IS2020::Toggle_buzzer2(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_toggle_buzzer2);
  }
  
uint8_t IS2020::Change_tone(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_change_tone);
  }
  
uint8_t IS2020::BatteryStatus(uint8_t deviceID){
  //Serial3.println("Battery_status");
      return IS2020::MMI_Action(deviceID,MMI_battery_status);
  }
  
uint8_t IS2020::Exit_pairing_mode(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_exit_pairing_mode);
  }
  
uint8_t IS2020::Link_past_device(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_link_past_device);
  }
  
uint8_t IS2020::Disconnect_all_link(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_disconnect_all_link);
  }
  
uint8_t IS2020::Triger_to_query_call_list_info(uint8_t deviceID){
      return IS2020::MMI_Action(deviceID,MMI_triger_to_query_call_list_info);
  }
  

