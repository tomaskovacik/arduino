/*
enhanced version of:
credits: https://github.com/arildlangseid/vw_t4_tcu_temp_to_fis

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
#include "VAGFISWriter.h"
#include <Arduino.h>

// #define ENABLE_IRQ 1
// #define DEBUG_MEM 1

/**
   Constructor
*/
VAGFISWriter::VAGFISWriter(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin, uint8_t forced = 0)
{
  _FIS_WRITE_CLK = clkPin;
  _FIS_WRITE_DATA = dataPin;
  _FIS_WRITE_ENA = enaPin;
  __forced = forced;

}

/**
   Destructor
*/
VAGFISWriter::~VAGFISWriter()
{
}

/**

   Initialize instrument-cluster

*/
void VAGFISWriter::begin() {
//pinMode(5,OUTPUT);
  stopENA();
  pinMode(_FIS_WRITE_CLK, OUTPUT);
  setClockLow();
  pinMode(_FIS_WRITE_DATA, OUTPUT);
  setDataLow();
};



static char tx_array[64];

void VAGFISWriter::sendString(String line1, String line2, bool center) {
  line1.toUpperCase();
  line2.toUpperCase();
  // fill lines to 8 chars
  while (line1.length() < 8) line1 += " ";
  while (line2.length() < 8) line2 += " ";
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 0xF0; //radio text id 

  line1.toCharArray(&tx_array[3], 9);
  if (tx_array[10] == 32 && !center) tx_array[10] = 28; // set last char to 28 if not center
  line2.toCharArray(&tx_array[11], 9);
  if (tx_array[18] == 32 && !center) tx_array[18] = 28; // set last char to 28 if not center
  tx_array[19] = (char)checkSum((uint8_t*)tx_array);

  sendRawData(tx_array);

}

//for compatibility with FICuntrol
void VAGFISWriter::sendStringFS(int x, int y, uint8_t font, String line) {
  line.toUpperCase();
  tx_array[0] = 0x56; // command to set text-display in FIS
  tx_array[1] = line.length() + 4; // Length of this message (command and this length not counted
  tx_array[2] = font; //font parameters (see below) sendMsgFS notes 
  tx_array[3] = x;
  tx_array[4] = y;
  line.toCharArray(&tx_array[5], line.length() + 1);
  sendRawData(tx_array);
}

uint8_t VAGFISWriter::sendMsg(char msg[]) {
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS, only 0x81 works, none of 0x80,0x82,0x83 works ...
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 0xF0; // 0x0F = 0xFF ^ 0xF0, same ID as in radio message... id for radio text
  memcpy(&tx_array[3],msg,16);
  return sendRawData(tx_array);
}

void VAGFISWriter::initScreen(uint8_t mode,uint8_t X,uint8_t Y,uint8_t X1,uint8_t Y1) {
/*
---------------------------
| Initializing the screen |
---------------------------

Initialization is required to go into graphical mode and to be able to output data to any point on the screen.
After initialization, you must constantly send data.
If the break between data packets is greater than ~ 3-4 seconds, the dashboard will automatically switch to standard mode.


Format of the package:

53 CN CM X1 Y1 X2 Y2 xx

Where:

53 ----> ID

CN ----> Number of bytes

CM ----> initialization command
0x80,0x81 - Initializing the screen without cleaning
0x82 - screen initialization with cleaning, positive screen
0x83 - screen initialization with cleaning, negative screen

X1 ----> starting X coordinate (can not be greater than the final X coordinate)

Y1 ----> start Y coordinate (can not be greater than the final Y coordinate)

X2 ----> the final X coordinate (can not be less than the initial X coordinate and greater than 64)

Y2 ----> the final Y coordinate (can not be less than the initial Y coordinate)

xx ----> checksum

Initialization is possible in parts:
To initialize only the middle part of the screen: 0x00, 0x1B, 0x40, 0x30 (64x48)
To initialize the entire screen: 0x00, 0x00, 0x40, 0x58 (64x88) - the height size specifies a multiple of eight

To switch from the graphical mode to the standard one, you must send the initialization message with the coordinates (0,0) (1,1)

*/

  tx_array[0] = 0x53; // ID
  tx_array[1] = 0x06; // Length of this message (command and this length not counted
  tx_array[2] = mode; //0x80,0x81 - Initializing the screen without cleaning; 0x82 - screen initialization with cleaning, positive screen; 0x83 - screen initialization with cleaning, negative screen 
  tx_array[3] = X;
  tx_array[4] = Y;
  tx_array[5] = X1;
  tx_array[6] = Y1;
  sendRawData(tx_array);
  if (X==0 && Y==0 && X1==1 && Y1==1) delay(25); //18ms pulse from cluster, probably ack that screen is out of graphix mode..
}


void VAGFISWriter::reset(uint8_t mode){
	VAGFISWriter::initScreen(mode,0,0,1,1);
}

void VAGFISWriter::initMiddleScreen(uint8_t mode){
	VAGFISWriter::initScreen(mode,0,27,64,48);
}

void VAGFISWriter::initFullScreen(uint8_t mode){
	VAGFISWriter::initScreen(mode,0,0,64,88);
}

void VAGFISWriter::initFullScreenFilled(){
        VAGFISWriter::initScreen(0x83,0,0,64,88);
}


void VAGFISWriter::sendMsgFS(uint8_t X,uint8_t Y,uint8_t font, uint8_t size,char msg[]) {
/*
----------------
| Display text |
----------------

The text is displayed only after initialization and only in the initialized area of ​​the screen.

Format of the package:

56 CN SS XX YY bb bb bb bb bb bb xx

Where:

56 ----> ID

CN ----> Number of bytes

CC ----> font parameters (see below):

XX ----> start X output coordinate

YY ----> Start Y Output Coordinate

bb ----> ASCII data (count can be different, allowable characters see below)

xx ----> checksum

Font settings:

bit 0 -
0 = negative
1 = positive
bit 1 -
0 = output as XOR with output area
1 = normal output (the output area is completely wiped with text)
bit 2 -
0 = normal font
1 = compressed font
bit 3 -
0 = no special characters
1 = special characters
bit 5 -
0 = left alignment
1 = center alignment

eg:
0x00 - standard font, negative, left aligned
0x01 - standard font, positive, left aligned
0x04 - compressed font, negative, left aligned
0x05 - compressed font, positive, left-aligned
0x08 - special characters, negative, left aligned
0x09 - special characters, positive, left aligned
0x20 - standard font, negative, centered
0x21 - standard font, positive, centered
0x24 - compressed font, negative, centered
0x25 - compressed font, positive, centered
0x28 - special characters, positive, centered
0x29 - special characters, negative, centered

standard font - can fit ~ 10.5 characters, height 7 pixels
compressed font - holds ~ 14.5 characters, height 7 pixels

Valid characters:

Of course, there is no Russian.
letters are uppercase only, lowercase letters instead of letters

characters in standard font:
----------------------------------

a - umlaut
b - single quotation mark
c - fatty point
d - single quotation mark
e is empty
f - underscore
g - down arrow
h is empty
i - right arrow (shaded)
j - degree
k is empty
l is empty
m - asterisk
n - right arrow
o - empty
p is the right arrow (shaded)
q - Inverted exclamation point
r is empty
s is empty
t is empty
u is empty
v is empty
w is empty
x is empty
y is empty
z is empty

0x18 - Up arrow (with a tail)
0x19 - down arrow (with a tail)
0x1A - right arrow (with a tail)
0x1B - left arrow (with a tail)

0x1E - up arrow (shaded)
0x1F - down arrow (shaded)

characters in a compressed font:
----------------------------------

a - umlaut
b - small dot
c - fatty point
d is empty
e is empty
f - underscore
g is empty
h - dash
i - right arrow (shaded)
j - blunt arrow to the right
k is empty
l is empty
m - asterisk
n - right arrow
o - lattice (pixel through pixel)
p is a large lattice
q - Inverted exclamation point
r is empty
s is empty
t is empty
u is empty
v is empty
w is empty
x is empty
y is empty
z is empty
*/
  // build tx_array
  tx_array[0] = 0x56; // command to set text-display in FIS
  tx_array[1] = size+4; // Length of this message (command and this length not counted
  tx_array[2] = font; 
  tx_array[3] = X;
  tx_array[4] = Y;
  memcpy(&tx_array[5],msg,size);
  sendRawData(tx_array);
}

/*
------------------
| Graphic Output |
------------------

The output is available only after the screen is initialized and only in the initialized area.

Format of the package:

55 CN UU X1 Y1 bb bb bb bb bb bb bb bb xx

Where:

55 ----> ID

CN ----> Number of bytes

UU ---->
bit 0 -
1 = output as XOR with output area
bit 1 -
1 = output as standard

X1 ----> start X output coordinate

Y1 ----> start Y coordinate of the output

bb ----> data bytes. With the bit set, the pixel lights up, one bit = one pixel.
The minimum data size is 1 byte, i.e. 8 pixels. Filling is horizontal.

xx ----> checksum

*/

void VAGFISWriter::GraphicOut(uint8_t x,uint8_t y,uint16_t size,uint8_t data[],uint8_t mode){
tx_array[0] = 0x55;
tx_array[1] = size+4;
tx_array[2] = mode;
tx_array[3] = x;
tx_array[4] = y;
memcpy(&tx_array[5],data,size);
sendRawData(tx_array);
}

void VAGFISWriter::GraphicOut(uint8_t x,uint8_t y,uint16_t size,const char * const data,uint8_t mode){
tx_array[0] = 0x55;
tx_array[1] = size+4;
tx_array[2] = mode;
tx_array[3] = x;
tx_array[4] = y;
memcpy(&tx_array[5],data,size);
sendRawData(tx_array);
}

void VAGFISWriter::GraphicOut_P(uint8_t x,uint8_t y,uint16_t size,const uint8_t * const data,uint8_t mode){
tx_array[0] = 0x55;
tx_array[1] = size+4;
tx_array[2] = mode;
tx_array[3] = x;
tx_array[4] = y;
memcpy_P(&tx_array[5],data,size);
sendRawData(tx_array);
}


uint8_t VAGFISWriter::sendRawData(char data[]){

#ifdef ENABLE_IRQ
  cli();
#endif
  // Send FIS-command
  if(!__forced){
  if (!sendSingleByteCommand(data[FIS_MSG_COMMAND])) return false;

  if(!waitEnaHigh(100)) {
  delay(2);
  return sendRawData(data);
  }
  } else 
     sendByte(data[FIS_MSG_COMMAND]);
  
  uint8_t crc =data[FIS_MSG_COMMAND];
  for (uint16_t a=1;a<data[1]+1;a++)
  {
  // calculate checksum
  crc ^= data[a];
  // Step 2 - wait for response from cluster to set ENA-High
  sendByte(data[a]);
  // wait for response from cluster to set ENA LOW
  if(!waitEnaLow()) return false;
  // Step 10.2 - wait for response from cluster to set ENA-High
  if(!waitEnaHigh()) return false;
  }
  crc--;
  sendByte(crc);
  if (__forced)delay(3);
  
  if(!waitEnaLow()) return false;

#ifdef ENABLE_IRQ
  sei();
#endif
return true;
}
/**
 * GraphicFromArray(x,y,sizex,sizey,data,mode)
 *
 * funtions take array of data and splits it to packet which are then send by GraphicOut()
 *
 * parameters:
 * x - start position x coordinate
 * y - start position y coordinate
 * sizex - horizontal size of picture 
 * sizey - vertical size of picture
 * data - pointer to array with picture data, each bit represent point on display
 * mode - 
 */
void VAGFISWriter::GraphicFromArray(uint8_t x,uint8_t y, uint8_t sizex, uint8_t sizey,const char * const data,uint8_t mode)
{
// 22x32bytes = 704 
if (sizex == 64) // send jumbo packets
{
        for (uint8_t line = 0;line<sizey/4;line++){ //32/8=4
                GraphicOut(x,line*4+y,JUMBO_PACKET_SIZE,data+(line*JUMBO_PACKET_SIZE),mode);//4=32/8
        }
        if ((sizey*8)%JUMBO_PACKET_SIZE>0){//few bytes left to by send
                uint8_t line = 4*(sizey/4);
                GraphicOut(x,line+y,JUMBO_PACKET_SIZE-(sizey*8)%JUMBO_PACKET_SIZE,data+(line*8),mode);//4=32/8
        }
}
else
{//stick to safe 1packet per line
	uint8_t packet_size = (sizex+7)/8; // how much byte per packet
        for (uint8_t line = 0;line<sizey;line++){
                GraphicOut(x,line+y,packet_size,data+(line*packet_size),mode);
        }
}
}

void VAGFISWriter::GraphicFromArray_P(uint8_t x,uint8_t y, uint8_t sizex, uint8_t sizey, const uint8_t * const data,uint8_t mode)
{
// 22x32bytes = 704 
if (sizex == 64) // send jumbo packets
{
        for (uint8_t line = 0;line<sizey/4;line++){ //32/8=4
                GraphicOut_P(x,line*4+y,JUMBO_PACKET_SIZE,data+(line*JUMBO_PACKET_SIZE),mode);//4=32/8
        }
        if ((sizey*8)%JUMBO_PACKET_SIZE>0){//few bytes left to by send
                uint8_t line = 4*(sizey/4);
                GraphicOut_P(x,line+y,JUMBO_PACKET_SIZE-(sizey*8)%JUMBO_PACKET_SIZE,data+(line*8),mode);//4=32/8
        }
}
else
{//stick to safe 1packet per line
uint8_t packet_size = (sizex+7)/8; // how much byte per packet
        for (uint8_t line = 0;line<sizey;line++){
                GraphicOut_P(x,line+y,packet_size,data+(line*packet_size),mode);
        }
}
}


void VAGFISWriter::GraphicFromArray(uint8_t x,uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t data[],uint8_t mode)
{
// 22x32bytes = 704 
if (sizex == 64) // send jumbo packets
{
        for (uint8_t line = 0;line<sizey/4;line++){ //32/8=4
                GraphicOut(x,line*4+y,JUMBO_PACKET_SIZE,&data[line*JUMBO_PACKET_SIZE],mode);//4=32/8
        }
	if ((sizey*8)%JUMBO_PACKET_SIZE>0){//few bytes left to by send
		uint8_t line = 4*(sizey/4);
                GraphicOut(x,line+y,JUMBO_PACKET_SIZE-(sizey*8)%JUMBO_PACKET_SIZE,&data[line*8],mode);//4=32/8
        }
}
else 
{//stick to safe 1packet per line
uint8_t packet_size = (sizex+7)/8; // how much byte per packet
	for (uint8_t line = 0;line<sizey;line++){
		GraphicOut(x,line+y,packet_size,&data[(line*packet_size)],mode);
	}
}
}

/**

   Send Keep-Alive message

*/
void VAGFISWriter::sendKeepAliveMsg() {
  delay(100);
  sendSingleByteCommand(0xC3);
  delay(100);
}

void VAGFISWriter::radioDisplayOff() {
char off[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
VAGFISWriter::sendMsg(off);
}
void VAGFISWriter::radioDisplayBlank() {
char blank[16] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
VAGFISWriter::sendMsg(blank);
}

/**

   Send Single-Command out on 3LB port to instrument cluster

*/
uint8_t VAGFISWriter::sendSingleByteCommand(uint8_t txByte) {
  //wait for ena line to be low = noone sending to cluster anything
  if(!waitEnaLow()) return false;
#ifdef ENABLE_IRQ
  cli();
#endif
	startENA();
	sendByte(txByte);
	stopENA();
  	if(!waitEnaLow(100)) return false; //based on real comunication cluster responce is around 65us
#ifdef ENABLE_IRQ
  sei();
#endif
//if we are here, we are fine ...
return true;
}

/**

   Send byte out on 3LB port to instrument cluster

*/
void VAGFISWriter::sendByte(uint8_t in_byte) {
  if (__forced)startENA();
	uint8_t tx_byte = 0xff - in_byte;
	for (int8_t i = 7; i >= 0; i--) {//must be signed! need -1 to stop "for"iing

		switch ((tx_byte & (1 << i)) > 0 ) {
			case 1: setDataHigh();
				break;
			case 0: setDataLow();
				break;
		}
		delayMicroseconds(9);
		setClockLow();
		delayMicroseconds(FIS_WRITE_PULSEW);
		setClockHigh();
		delayMicroseconds(FIS_WRITE_PULSEW);
	}
if (__forced){
  stopENA();
  delayMicroseconds(80);
}

}

/**

   Set 3LB ENA active High

*/
void VAGFISWriter::startENA() {
  if (!__forced) detachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA));
  digitalWrite(_FIS_WRITE_ENA, HIGH);// avoid spikes
  pinMode(_FIS_WRITE_ENA, OUTPUT);
  digitalWrite(_FIS_WRITE_ENA, HIGH);
}
/**

   Set 3LB ENA as input (so low, as we should have pulldown on ena line)

*/
uint8_t VAGFISWriter::stopENA() {
  if (!__forced)
  {
    detachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA));
    //digitalWrite(_FIS_WRITE_ENA, LOW);
    pinMode(_FIS_WRITE_ENA, INPUT);
    //digitalWrite(_FIS_WRITE_ENA, LOW);
    attachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA),&VAGFISWriter::enableGoesHigh,RISING);
  }
  else
  {
    digitalWrite(_FIS_WRITE_ENA, LOW);
    delayMicroseconds(100);
  }
}

/**

   Set 3LB CLK High

*/
void VAGFISWriter::setClockHigh() {
  digitalWrite(_FIS_WRITE_CLK,HIGH);
}
/**
   Set 3LB CLK Low
*/
void VAGFISWriter::setClockLow() {
digitalWrite(_FIS_WRITE_CLK,LOW);
}
/**
   Set 3LB DATA High
*/
void VAGFISWriter::setDataHigh() {
digitalWrite(_FIS_WRITE_DATA,HIGH);
}
/**
   Set 3LB DATA Low
*/
void VAGFISWriter::setDataLow() {
digitalWrite(_FIS_WRITE_DATA,LOW);
}

/**
   checksum routine to calculate the crc

   takes complete message as parameter including messagelength and the 240 constant followed by the 16 byte message
   example: uint8_t msg[] = {18, 240, 32, 78, 82, 75, 32, 80, 49, 32, 70, 77, 49, 46, 49, 32, 32, 28};

*/
uint8_t VAGFISWriter::checkSum( volatile uint8_t in_msg[]) {
  uint8_t crc = in_msg[0];
  for (int16_t i = 1; i < sizeof(in_msg); i++)
  {
    crc ^= in_msg[i];
  }
  crc --;
  return crc;
}

uint8_t VAGFISWriter::waitEnaHigh(uint16_t timeout_us)
{
  /*while (!digitalRead(_FIS_WRITE_ENA) && timeout_us > 0) {
    delayMicroseconds(1);
    timeout_us -= 1;
  }
  if (timeout_us == 0) return false;*/
return true;
}

uint8_t VAGFISWriter::waitEnaLow(uint16_t timeout_us){
  /*while (digitalRead(_FIS_WRITE_ENA) && timeout_us > 0) {
    delayMicroseconds(1);
    timeout_us -= 1;
  }
  if (timeout_us == 0) return false;*/
return true;
}
/*
bool VAGFISWriter::sendRadioMsg(char msg[16]){
if(!waitEnaLow()) return false;
stopENA();
delayMicroseconds(100);
startENA();
uint8_t crc=0xF0;
sendByte(0xF0); 

 for (uint16_t a=0;a<16;a++) //radio msg is always 16chars
  {
  crc += msg[a];//calculate checksum
  sendByte(msg[a]);
  }

sendByte(0xFF ^ crc);
stopENA();
return true;
}*/

bool VAGFISWriter::sendRadioMsg(char * msg)
{
	_radioDataOK=0;
	memcpy(&_radioData,msg,16);

	_radioDataOK=1;
	VAGFISWriter::sendRadioData(1);//force 1st packet
}


void VAGFISWriter::enableGoesHigh(void)
{
if(digitalRead(_FIS_WRITE_ENA)){
	attachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA),&VAGFISWriter::enableGoesLow,FALLING);
}
}

void VAGFISWriter::enableGoesLow(void)
{
	if(digitalRead(_FIS_WRITE_ENA)){
		_sendOutData=1;//cluster acknowleage previously received packet
		attachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA),&VAGFISWriter::enableGoesHigh,RISING);
	}
}

void VAGFISWriter::sendRadioData(uint8_t forced)
{
	if (forced or __forced) _sendOutData=1;
	else delay(100); //in future we will use timer for this ...
  
	if (_radioDataOK && _sendOutData)
	{
	detachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA));
	startENA();
	uint8_t crc=0xF0;
	sendByte(0xF0); 
	for (uint8_t a=0;a<16;a++)//radio msg is always 16chars
	{
		// calculate checksum
		crc += _radioData[a];
		sendByte(_radioData[a]);
	}
	sendByte(0xFF ^ crc);
	stopENA();
	_sendOutData=0;
	attachInterrupt(digitalPinToInterrupt(_FIS_WRITE_ENA),&VAGFISWriter::enableGoesHigh,RISING);
	}
}
