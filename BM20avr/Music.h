#define STOP_FFW_RWD 0x00 // Stop fast forward or rewind   
#define FFW  0x01  //fast forward    
#define REPFFW 0x02  //fast forward with repeat send fast forward command every 800ms    
#define RWD 0x03  //rewind    
#define REPRWD 0x04  //rewind with repeat send rewind command every 800ms    
#define PLAY 0x05  //PLAY command    
#define PAUSE 0x06  //PAUSE command   
#define TOGLE_PLAY_PAUSE0x07  //PLAY PAUSE toggle   
#define STOP 0x08  //STOP command 

#define CRLDI_query_device_name 0x00
#define CRLDI_query_in_band_ringtone_status 0x01
#define CRLDI_query_if_remote_device_is_iAP_device 0x02
#define CRLDI_query_if_remote_device_support_AVRCP_v13 0x03
#define CRLDI_query_HF_A2DP_gain 0x04
#define CRLDI_query_Line_In_gain 0x05

