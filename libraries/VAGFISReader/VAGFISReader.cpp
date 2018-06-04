/*
 * (C) Tomas Kovacik
 * https://github.com/tomaskovacik/
 * GNU GPL3
 */


#include "VAGFISReader.h"
#include <Arduino.h>

// do not forget to put pull down resistor on enable line
// pull up on data/clk line is made usng internal pullup



/**
   Constructor
*/
VAGFISReader::VAGFISReader(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin)
{
  FIS_READ_CLK = clkPin;
  FIS_READ_DATA = dataPin;
  FIS_READ_ENA = enaPin;
}

/**
   Destructor
*/
VAGFISReader::~VAGFISReader(){}

void VAGFISReader::init()
{
  //delay (2000);
  pinMode(FIS_READ_CLK,INPUT_PULLUP);
  pinMode(FIS_READ_DATA,INPUT_PULLUP);
  pinMode(FIS_READ_ENA,INPUT);//no pull up! this is inactive state low, active is high
  digitalWrite(FIS_READ_ENA,LOW);
  attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_rising,RISING);
};

void VAGFISReader::read_data_line(){ //fired on falling edge
// The lines Data and Clock use negative logic, i.e. the logical unit corresponds to the low level on the line, the high level on the line corresponds to the logical zero. 
  newmsg_from_radio=0;
  if(digitalRead(FIS_READ_DATA)){
    data[msgbit/8] = (data[msgbit/8]<<1);
  }
  else
  {
    data[msgbit/8] = (data[msgbit/8]<<1) | 1;
  }
  if(pre_navi) {
	if (((msgbit+1)%8) == 0 ){ //each 8bit we have to put ena low and back high..
		digitalWrite(FIS_READ_ENA,LOW);
		delayMicroseconds(100);
		if(msgbit == 15){
			packet_size = data[1] + 2;
		}
		if ((msgbit+1) != packet_size*8){
			digitalWrite(FIS_READ_ENA,HIGH); //based on data[1]+2 which is packet size +id+actual byte with packet size, we can calculate if we need another byte receive
		} else {	
			pinMode(FIS_READ_ENA,INPUT);
			digitalWrite(FIS_READ_ENA,LOW); //pull down, just in case
			attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_rising,RISING); //standard start scenario
			msgbit=0;
			pre_navi=0;
			if (check_data()) //check cksum...
       	        	        newmsg_from_radio=1;
			detachInterrupt(digitalPinToInterrupt(FIS_READ_CLK));
			} 
	}
  }
msgbit++;
}

void VAGFISReader::detect_ena_line_rising(){
	msgbit=0;
	newmsg_from_radio=0;
	navi=0;
	pre_navi=0;
	
	attachInterrupt(digitalPinToInterrupt(FIS_READ_CLK),&VAGFISReader::read_data_line,FALLING);//data are valid on falling edge of FIS_READ_CLK 
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_falling,FALLING); //if enable changed to low, data on data line are no more valid
}

void VAGFISReader::detect_ena_line_falling(){
  detachInterrupt(digitalPinToInterrupt(FIS_READ_ENA));
  detachInterrupt(digitalPinToInterrupt(FIS_READ_CLK));
  packet_size=0;
  if(msgbit>0){
	packet_size = msgbit/8;
	if (packet_size > 1){ //NAVI start with just 1 packet so if we have more then 1 here, we are safe to zero msbit variable
		if (check_data()) //check cksum...
			newmsg_from_radio=1;
		attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_rising,RISING);
	}
	else  { 
		if(!pre_navi){
			packet_size=0;
	    		attachInterrupt(digitalPinToInterrupt(FIS_READ_CLK),&VAGFISReader::read_data_line,FALLING);//data are valid on falling edge of FIS_READ_CLK 
			pre_navi=1;
			pinMode(FIS_READ_ENA,OUTPUT);
			digitalWrite(FIS_READ_ENA,LOW);
			delayMicroseconds(5);
			digitalWrite(FIS_READ_ENA,HIGH);
		}
	}
  }
  else {
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_rising,RISING);
	}
}

bool VAGFISReader::has_new_msg(){
	return newmsg_from_radio;
}

void VAGFISReader::clear_new_msg_flag(){
	newmsg_from_radio=0;
}

uint8_t VAGFISReader::read_data(int8_t id){
		return data[id];
}

bool VAGFISReader::request(){
if (!digitalRead(FIS_READ_ENA)) {//safe to ack/request another packet from radio	
	detachInterrupt((FIS_READ_ENA));
	pinMode(FIS_READ_ENA,OUTPUT);
	digitalWrite(FIS_READ_ENA,HIGH);
	delay(3);
	digitalWrite(FIS_READ_ENA,LOW);
	pinMode(FIS_READ_ENA,INPUT);
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detect_ena_line_rising,RISING);
	return true;
} else {
	return false;
}
}

bool VAGFISReader::msg_is_navi(){
	return navi;
}


bool VAGFISReader::check_data(){
	if (packet_size == 18 && data[0] == 0xF0){ // radio mode
		navi = 0;
		if(calc_checksum()) return true;
	} else {
		navi=1;
		if (calc_checksum()) return true;
	}
	return false;
}

uint8_t VAGFISReader::get_msg_id(){
	return data[0];
}

uint8_t VAGFISReader::get_size(){
	return packet_size;
}

bool VAGFISReader::calc_checksum(){
	uint8_t tmp=0;
	if (!navi){
		for (uint8_t i=0;i<17;i++){
				tmp=tmp+data[i];
		}
		if (data[packet_size-1] == ((0xFF ^ tmp ) & 0xFF)){
			return true;}
	} else { //navi
                for (uint8_t i=0;i<packet_size-1;i++){
                                tmp ^= data[i];
                }
                if (data[packet_size-1] == tmp-1){
                        return true;	}
	}
	return false;
}

uint8_t VAGFISReader::get_checksum(){
	return data[packet_size-1];
}
