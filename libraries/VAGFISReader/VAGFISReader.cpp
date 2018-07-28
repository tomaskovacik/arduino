/*
 * (C) Tomas Kovacik
 * https://github.com/tomaskovacik/
 * GNU GPL3
 * 
 * tested on stm32 
 */

/* info from passatworld.ru regarding 3lb:
Data transmission is carried out on 3 lines (Data, Clock, Enable). Working voltage on 5V lines.
Data and Clock lines are unidirectional, line management is performed by the master device. The default lines are high.
The lines Data and Clock use negative logic, i.e. the logical unit corresponds to the low level on the line, the high level on the line corresponds to the logical zero.
The Enable line is bi-directional, the master device initiates the transfer, the slave device confirms reception and is ready to receive the next data piece. The default line is low.
The initiation of transmission and confirmation is carried out by a high level on the line. 

The transmission speed is up to ~ 125-130kHz.
On the bus there is a master and slave device. The dashboard always acts as a slave.
Transmission is carried out by packages. The size of the packet depends on the data transmitted (see part 2).

The master device before the start of transmission looks at the presence of a low signal level on the Enable line.
Having a high level indicates that the line is busy or the slave device can not currently receive data.

The master device sets the Enable line to a high level and begins sending the first byte of the sending.
The next data bit from the line is read when the clock signal goes from high to low (from logical zero to one).
After transmission of the first byte, the master sets the low level on the Enable line and waits for the slave device to "raise" the Enable line, indicating that it is ready to receive the next byte.
By taking another byte slave, the device "drops" the Enable line, and the master device waits for the Enable line to "rise" again to transmit the next byte.
Thus, the Master controls the Enable line only when transmitting the first byte of each packet, and then only controls the presence on it of a high level of the speaker saying that the slave is ready to receive.
In case the slave did not raise the Enable line to receive the next byte within ~ 150-200us, it's necessary to start sending the packet again after waiting at least 3-4ms.
Do not raise the slave line Enable can also mean that the slave detected an error in the transmitted data and is not ready to continue receiving.

There is one more option for data transfer in which the master raises the Enable line before starting the transfer and drops it only after the transfer of the entire packet.
It is necessary to pause between bytes of approximately 80-100us. And also to pause at least 4-5ms between packets, especially if packets go on continuously.
Unfortunately, in this mode, it is not possible to control the transmitted data. A slave may simply not accept the package, and the master will not know about it.
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

void VAGFISReader::begin()
{
  //delay (2000);
  pinMode(FIS_READ_CLK,INPUT_PULLUP);
  pinMode(FIS_READ_DATA,INPUT_PULLUP);
  pinMode(FIS_READ_ENA,INPUT);//no pull up! this is inactive state low, active is high
  digitalWrite(FIS_READ_ENA,LOW);
  attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineRising,RISING);
};

void VAGFISReader::readDataLine(){ //fired on falling edge
// The lines Data and Clock use negative logic, i.e. the logical unit corresponds to the low level on the line, the high level on the line corresponds to the logical zero. 
  newMsgFromRadio=0;
  if(digitalRead(FIS_READ_DATA)){
    data[msgbit/8] = (data[msgbit/8]<<1);
  }
  else
  {
    data[msgbit/8] = (data[msgbit/8]<<1) | 1;
  }
  if(preNavi) {
	if (((msgbit+1)%8) == 0 ){ //each 8bit we have to put ena low and back high..
		digitalWrite(FIS_READ_ENA,LOW);
		delayMicroseconds(100);
		if(msgbit == 15){
			packetSize = data[1] + 2;
		}
		if ((msgbit+1) != packetSize*8){
			digitalWrite(FIS_READ_ENA,HIGH); //based on data[1]+2 which is packet size +id+actual byte with packet size, we can calculate if we need another byte receive
		} else {	
			pinMode(FIS_READ_ENA,INPUT);
			digitalWrite(FIS_READ_ENA,LOW); //pull down, just in case
			attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineRising,RISING); //standard start scenario
			msgbit=0;
			preNavi=0;
			if (checkData()) //check cksum...
       	        	        newMsgFromRadio=1;
			detachInterrupt(digitalPinToInterrupt(FIS_READ_CLK));
			} 
	}
  }
msgbit++;
}

void VAGFISReader::detectEnaLineRising(){
	msgbit=0;
	newMsgFromRadio=0;
	navi=0;
	preNavi=0;
	
	attachInterrupt(digitalPinToInterrupt(FIS_READ_CLK),&VAGFISReader::readDataLine,FALLING);//data are valid on falling edge of FIS_READ_CLK 
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineFalling,FALLING); //if enable changed to low, data on data line are no more valid
}

void VAGFISReader::detectEnaLineFalling(){
  detachInterrupt(digitalPinToInterrupt(FIS_READ_ENA));
  detachInterrupt(digitalPinToInterrupt(FIS_READ_CLK));
  packetSize=0;
  if(msgbit>0){
	packetSize = msgbit/8;
	if (packetSize > 1){ //NAVI start with just 1 packet so if we have more then 1 here, we are safe to zero msbit variable
		if (checkData()) //check cksum...
			newMsgFromRadio=1;
		attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineRising,RISING);
	}
	else  { 
		if(!preNavi){
			packetSize=0;
	    		attachInterrupt(digitalPinToInterrupt(FIS_READ_CLK),&VAGFISReader::readDataLine,FALLING);//data are valid on falling edge of FIS_READ_CLK 
			preNavi=1;
			pinMode(FIS_READ_ENA,OUTPUT);
			digitalWrite(FIS_READ_ENA,LOW);
			delayMicroseconds(5);
			digitalWrite(FIS_READ_ENA,HIGH);
		}
	}
  }
  else {
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineRising,RISING);
	}
}

bool VAGFISReader::hasNewMsg(){
	return newMsgFromRadio;
}

void VAGFISReader::clearNewMsgFlag(){
	newMsgFromRadio=0;
}

uint8_t VAGFISReader::readData(int8_t id){
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
	attachInterrupt(digitalPinToInterrupt(FIS_READ_ENA),&VAGFISReader::detectEnaLineRising,RISING);
	return true;
} else {
	return false;
}
}

bool VAGFISReader::msgIsNavi(){
	return navi;
}


bool VAGFISReader::checkData(){
	if (packetSize == 18 && data[0] == 0xF0){ // radio mode
		navi = 0;
		if(calcChecksum()) return true;
	} else {
		navi=1;
		if (calcChecksum()) return true;
	}
	return false;
}

uint8_t VAGFISReader::getMsgId(){
	return data[0];
}

uint8_t VAGFISReader::getSize(){
	return packetSize;
}

bool VAGFISReader::calcChecksum(){
	uint8_t tmp=0;
	if (!navi){
		for (uint8_t i=0;i<17;i++){
				tmp=tmp+data[i];
		}
		if (data[packetSize-1] == ((0xFF ^ tmp ) & 0xFF)){
			return true;}
	} else { //navi
                for (uint8_t i=0;i<packetSize-1;i++){
                                tmp ^= data[i];
                }
                if (data[packetSize-1] == tmp-1){
                        return true;	}
	}
	return false;
}

uint8_t VAGFISReader::getChecksum(){
	return data[packetSize-1];
}
