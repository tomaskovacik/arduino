#include "IS2020.h"
#include <Arduino.h>

#define MMI_add_remove_SCO_link 0x01
#define MMI_force_end_active_call 0x02
#define MMI_accept_incoming_call 0x04
#define MMI_reject_incoming_call 0x05
#define MMI_end_call_transfer_to_headset 0x06 //1. End call if SCO exist. OR 2. Voice transfer to headset if SCO not exist."
#define MMI_togle_mic_mute 0x07 //1. Mute microphone if microphone is not mute OR 2. Active microphone if microphone is mute"
#define MMI_mute_mic 0x08	//Mute microphone
#define MMI_unmute_mic 0x09	//Active microphone
#define MMI_voice_dial 0x0A
#define MMI_cancel_voice_dial 0x0Bl
#define MMI_last_number_redial 0x0C
#define MMI_togle_active_hold_call 0x0D //Set the active call on hold and active the hold call
#define MMI_voice_transfer 0x0E	
#define MMI_query_call_list_info 0x0F	//Query call list information(CLCC)
#define MMI_thee_way_call 0x10
#define MMI_release_waiting_or_hold_call 0x11	//release the waiting call or on hold call
#define MMI_end_active_call_accept_waiting_or_held_call 0x12	//accept the waiting call or active the on hold call and release the active call
#define MMI_initiate_HF_connection 0x16
#define MMI_disconnect_HF_link 0x17	//disconnect HF link
#define MMI_increase_microphone_gain 0x24	
#define MMI_decrease_microphone_gain 0x25
#define MMI_switch_primary_seconday_HF 0x26	//switch primary HF device and secondary HF device role
#define MMI_increase_speaker_gain 0x30	
#define MMI_decrease_speaker_gain 0x31
#define MMI_next_song 0x34
#define MMI_previous_song 0x35
#define MMI_disconnect_A2DP_link 0x3B
#define MMI_next_audio_efect 0x3C
#define MMI_previous_audio_efect 0x3D
#define MMI_enter_pairing_mode 0x50	//enter pairing mode (from power off state)
#define MMI_power_on_button_press 0x51 //power on button press
#define MMI_power_on_button_release 0x52 //power on button release
#define MMI_eeprom_to_defaults 0x56	//Reset some eeprom setting to default setting
#define MMI_enter_pairing_mode_fast 0x5D	//fast enter pairing mode (from non-off mode)
#define MMI_power_off 0x5E	//switch power off: to execute the power_off process directly, actually, the combine command set, power off button press and release, could be replace by this command.
#define MMI_toggle_buzzer 0x60 //Enable buzzer if buzzer is OFF Disable buzzer if buzzer is ON
#define MMI_disable_buzzer 0x61
#define MMI_toggle_buzzer2 0x62	//"Enable buzzer Disable buzzer if buzzer is ON"
#define MMI_change_tone 0x63	//Change tone set (SPK module support two sets of tone)
#define MMI_battery_status 0x6A	//Indicate battery status
#define MMI_exit_pairing_mode 0x6B	//Exit pairing mode
#define MMI_link_past_device 0x6C	//link last device
#define MMI_disconnect_all_link 0x6D	//disconnect all link
#define MMI_triger_to_query_call_list_info 0x0f //trigger SPK Module to query call list information

/*

Ex:
[+CLCC:<id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>]][<CR><LF>+CLCC:<id2>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>]][...]]]

idx : Call identification number
dir : 
0:Mobile originated (MO) call
1:Mobile originated (MT) call
state: State of the call
0:ACTIVE
1:HELD
2: Dialing (MO)
3: Alerting (MO)
4: Incoming (MT)
5: Waiting (MT)
mode:
0: Voice
1: Data
2: Fax
3: Voice followed by data, voice mode
4: Alternating voice/data, voice mode
5: Alternating voice/fax, voice mode
6: Voice followed by data, data mode
7: Alternating voice/data, data mode
8: Alternating voice/fax, fax mode
9: Unknown
mpty:
0:Call is not one of multiparty (conference) call parties
1:Call is one of multiparty (conference) call parties
*/
