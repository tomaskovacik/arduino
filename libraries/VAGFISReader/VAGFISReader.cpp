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
  pinMode(FIS_READ_CLK,INPUT_PULLUP);//_PULLUP ?
  pinMode(FIS_READ_DATA,INPUT_PULLUP);//_PULLUP?
  pinMode(FIS_READ_ENA,INPUT_PULLDOWN);//no pull up! this is inactive state low, active is high
  attachInterrupt(FIS_READ_ENA,&VAGFISReader::detect_ena_line_rising,RISING);

};

void VAGFISReader::read_data_line(){ //fired on falling edge
  newmsg_from_radio=0;
/*    if(!adrok){
      read_adr();
    }
    else if (!newmsg1){
      read_msg1();
    }
    else if (!newmsg2){
      read_msg2();
    }
    else if (!cksumok){
      read_cksum();
    }*/
if(digitalRead(FIS_READ_DATA)){
    data[msgbit/8] = (data[msgbit/8]<<1) | 0x00000001;
    msgbit++;
  }
  else
  {
    data[msgbit/8] = (data[msgbit/8]<<1);
    msgbit++;
  }
}
/*
void VAGFISReader::read_cksum(){
  if(digitalRead(FIS_READ_DATA)){
    cksum = (cksum<<1) | 0x00000001;
    msgbit++;
  }
  else
  {
    cksum = (cksum<<1);
    msgbit++;
  }
  if (msgbit==8)
  {
    newmsg1=0;
    newmsg2=0;
    adrok=0;
    tmp_cksum=(0xFF^adr);
	Serial.print(adr);
    for (int i=56;i>=0;i=i-8){
	      tmp_cksum=tmp_cksum+(0xFF^((msg1>>i) & 0xFF))
		+(0xFF^((msg2>>i) & 0xFF));
		Serial.write(0xFF^((msg1>>i) & 0xFF));
		Serial.write(0xFF^((msg2>>i) & 0xFF));
    }
    if (!cksumok){//d we display what we reveived last time?
    if((tmp_cksum%256)==cksum){
    cksumok=1;
    } else {
      msg1 = 0x00000000;
      msg2 = 0x00000000;
      cksumok=0;
    }
    msgbit=0;
    }
Serial.println();
  }

}

void VAGFISReader::read_msg1(){
  if(digitalRead(FIS_READ_DATA)){
    msg1 = (msg1<<1) | 0x00000001;
    msgbit++;
  }
  else
  {
    msg1 = (msg1<<1);
    msgbit++;
  }
  if (msgbit==64)
  {
    newmsg1=1;
    msgbit=0;
  }
}

void VAGFISReader::read_msg2(){
  if(digitalRead(FIS_READ_DATA)){
    msg2 = (msg2<<1) | 0x00000001;
    msgbit++;
  }
  else
  {
    msg2 = (msg2<<1);
    msgbit++;
  }
  if (msgbit==64)
  {
    newmsg2=1;
    msgbit=0;
  }
}

void VAGFISReader::read_adr(){
  if(digitalRead(FIS_READ_DATA)){
    adr = (adr<<1) | 0x00000001;
    msgbit++;
  }
  else
  {
    adr = (adr<<1);
    msgbit++;
  }
  if (msgbit==8)
  {
    adrok=1;
    msgbit=0;
  }
}
*/


void VAGFISReader::detect_ena_line_rising(){
    //init all again
    //msgbit=1; //0 position is size of packet
//    newmsg1=0;
//   newmsg2=0;
//    adrok=0;
//    cksumok=0;
//    tmp_cksum=0;
//    grabbing=1;
    attachInterrupt(FIS_READ_CLK,&VAGFISReader::read_data_line,FALLING);//data are valid on falling edge of FIS_READ_CLK 
    attachInterrupt(FIS_READ_ENA,&VAGFISReader::detect_ena_line_falling,FALLING); //if enable changed to low, data on data line are no more valid
}

void VAGFISReader::detect_ena_line_falling(){
  //if (cksum == 1) grabbing=0;
  detachInterrupt(FIS_READ_CLK);//enable is low, data on data line are no more valid
  detachInterrupt(FIS_READ_ENA);
data[0] = msgbit/8;
    msgbit=8; //0 position is size of packet 8/8=1
  newmsg_from_radio=1;
  attachInterrupt(FIS_READ_ENA,&VAGFISReader::detect_ena_line_rising,RISING);
}

bool VAGFISReader::has_new_msg(){
        if(newmsg_from_radio) return true; 
	else return false;
}

void VAGFISReader::clear_new_msg_flag(){
	newmsg_from_radio=0;
}

uint8_t VAGFISReader::read_data(int8_t id){
	if (id>0)
	return (0xFF^data[id]);
	else 
	return data[0];
}
/*
uint64_t VAGFISReader::GetMsg(int msgid){
   if (!grabbing){
	if(msgid==1)
	return msg1;
	else
	return msg2;
	} else {
	return false;
	}
	
}*/
