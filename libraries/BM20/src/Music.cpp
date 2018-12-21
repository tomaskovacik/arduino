#include "IS2020.h"
#include <Arduino.h>

uint8_t IS2020::STOP_FFW_RWD(uint8_t deviceId){ // Stop fast forward or rewind 
  return IS2020::Music_Control(deviceId,Music_Control_STOP_FFW_RWD);
}
  
uint8_t IS2020::FFW(uint8_t deviceId){ //fast forward 
  return IS2020::Music_Control(deviceId,Music_Control_FFW);
}

uint8_t IS2020::REPFFW(uint8_t deviceId){ //fast forward with repeat send fast forward command every 800ms
  return IS2020::Music_Control(deviceId,Music_Control_REPFFW);
}

uint8_t IS2020::RWD(uint8_t deviceId){ //rewind
  return IS2020::Music_Control(deviceId,Music_Control_RWD);
}

uint8_t IS2020::REPRWD(uint8_t deviceId){ //rewind with repeat send rewind command every 800ms
  return IS2020::Music_Control(deviceId,Music_Control_REPRWD);
}

uint8_t IS2020::PLAY(uint8_t deviceId){ //PLAY command
  return IS2020::Music_Control(deviceId,Music_Control_PLAY);
}

uint8_t IS2020::PAUSE(uint8_t deviceId){  //PAUSE command
  return IS2020::Music_Control(deviceId,Music_Control_PAUSE);
}

uint8_t IS2020::TOGLE_PLAY_PAUSE(uint8_t deviceId){ //PLAY PAUSE toggle
  return IS2020::Music_Control(deviceId,Music_Control_TOGLE_PLAY_PAUSE);
}

uint8_t IS2020::STOP(uint8_t deviceId){ //STOP command
  return IS2020::Music_Control(deviceId,Music_Control_STOP);
}

