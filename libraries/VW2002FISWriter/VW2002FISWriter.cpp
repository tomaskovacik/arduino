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
#include "VW2002FISWriter.h"
//#include <MemoryFree.h>
#include <Arduino.h>

// #define ENABLE_IRQ 1
// #define DEBUG_MEM 1

/**
   Constructor
*/
VW2002FISWriter::VW2002FISWriter(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin)
{
  _FIS_WRITE_CLK = clkPin;
  _FIS_WRITE_DATA = dataPin;
  _FIS_WRITE_ENA = enaPin;

}

/**
   Destructor
*/
VW2002FISWriter::~VW2002FISWriter()
{
}

/**

   Initialize instrument-cluster

*/
void VW2002FISWriter::FIS_init() {
  // set port signals
  pinMode(_FIS_WRITE_ENA, OUTPUT);
  digitalWrite(_FIS_WRITE_ENA, LOW);
  FIS_WRITE_stopENA();
  pinMode(_FIS_WRITE_CLK, OUTPUT);
  setClockHigh();
  pinMode(_FIS_WRITE_DATA, OUTPUT);
  setDataHigh();

  // init port
  //FIS_WRITE_send_3LB_singleByteCommand(0xF0);
  //delay(200);
  //FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  //delay(200);
  //FIS_WRITE_send_3LB_singleByteCommand(0xF0);
  //delay(200);
  //FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  //delay(200);
}; // FIS_init



static char tx_array[25];

#ifdef USE_STRINGS
void VW2002FISWriter::sendMsg(String line1, String line2, bool center) {
  // fill lines to 8 chars
  while (line1.length() < 8) line1 += " ";
  while (line2.length() < 8) line2 += " ";
  /*
    Serial.print("[");
    Serial.print(line1);
    Serial.print("]");
    Serial.print("[");
    Serial.print(line2);
    Serial.println("]");
    Serial.println("f");
  */
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 240; // unsure what this is

  line1.toCharArray(&tx_array[3], 9);
  if (tx_array[10] == 32 && !center) tx_array[10] = 28; // set last char to 28 if not center
  line2.toCharArray(&tx_array[11], 9);
  if (tx_array[18] == 32 && !center) tx_array[18] = 28; // set last char to 28 if not center
  tx_array[19] = (char)checksum((uint8_t*)tx_array);

  sendRawMsg(tx_array);

}
#endif

void VW2002FISWriter::sendMsg(char msg[]) {
  // build tx_array
  tx_array[0] = 0x81; // command to set text-display in FIS
  tx_array[1] = 18; // Length of this message (command and this length not counted
  tx_array[2] = 240; // unsure what this is

  for (uint8_t i = 0; i < 16; i++) { // TODO: use memcpy
    tx_array[3 + i] = msg[i];
  }
  tx_array[19] = (char)checksum((uint8_t*)tx_array);

  sendRawMsg(tx_array);
}

void VW2002FISWriter::initScreen(uint8_t mode,uint8_t X,uint8_t Y,uint8_t X1,uint8_t Y1) {
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

uint8_t myArray[7] = {0x53,0x06,mode,X,Y,X1,Y1};

/*
  tx_array[0] = 0x53; // ID
  tx_array[1] = 0x06; // Length of this message (command and this length not counted
  tx_array[2] = mode; //0x80,0x81 - Initializing the screen without cleaning; 0x82 - screen initialization with cleaning, positive screen; 0x83 - screen initialization with cleaning, negative screen 
  tx_array[3] = X;
  tx_array[4] = Y;
  tx_array[5] = X1;
  tx_array[6] = Y1;
*/
  sendRawData(myArray);

}


void VW2002FISWriter::sendMsgFS(uint8_t X,uint8_t Y,uint8_t font, uint8_t size,char msg[]) {
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



splay text |
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
z is emptyV

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
  for (int16_t i = 0; i < size; i++) { // TODO: use memcpy
    tx_array[5 + i] = ' ';
    tx_array[5 + i] = msg[i];
  }

  sendRawMsg(tx_array);
}

/*
----------------
| Graph Output |
----------------

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

void VW2002FISWriter::GraphicOut(uint8_t x,uint8_t y,uint16_t size,uint8_t data[],uint8_t mode,uint8_t offset){
uint8_t myArray[size+5];
myArray[0] = 0x55;
myArray[1] = size+4;
myArray[2] = mode;
myArray[3] = x;
myArray[4] = y;
for (uint16_t a=0;a<size;a++){
	myArray[a+5] = data[offset+a];
}

sendRawData(myArray);
}

void VW2002FISWriter::sendRawData(uint8_t data[]){

#ifdef ENABLE_IRQ
  cli();
#endif
  // Send FIS-command
  //FIS_WRITE_startENA();
  //FIS_WRITE_3LB_sendByte(data[FIS_MSG_COMMAND]); //ID
  //FIS_WRITE_stopENA(); 
FIS_WRITE_send_3LB_singleByteCommand(data[FIS_MSG_COMMAND]);
  uint8_t crc =data[FIS_MSG_COMMAND];

  for (uint16_t a=1;a<data[1]+1;a++)
  {
  // Step 2 - wait for response from cluster to set ENA-High
  waitEnaHigh();
  // Step 9.2 - ENA-Low detected
  delayMicroseconds(40);
  // calculate checksum
  crc ^= data[a];
  FIS_WRITE_3LB_sendByte(data[a]);
/*    // Step 10.2 - wait for response from cluster to set ENA-High
    waitEnaHigh();
}*/
  }
crc--;
    // Step 10.2 - wait for response from cluster to set ENA-High
waitEnaHigh();
FIS_WRITE_3LB_sendByte(crc);

#ifdef ENABLE_IRQ
  sei();
#endif
delay(2);
}

void VW2002FISWriter::GraphicFromArray(uint8_t x,uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t data[],uint8_t mode){
// 22x32bytes = 704 
uint8_t packet_size = (sizex+7)/8; // how much byte per packet
//Serial.println(packet_size);
for (uint8_t line = 0;line<sizey;line++){
//Serial.println(line);
uint8_t _data[packet_size];
for (uint8_t i=0;i<packet_size;i++){
        _data[i]=data[(line*packet_size)+i];
}
GraphicOut(x,line+y,packet_size,_data,mode,0);
delay(5);

}
}

/**

   Prepare and send Text-Message

*/
void VW2002FISWriter::sendRawMsg(char in_msg[]) {
  FIS_WRITE_send_3LB_msg(in_msg);
}

/**

   Send Text-Message out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_send_3LB_msg(char in_msg[]) {
#ifdef ENABLE_IRQ
  cli();
#endif

	// Send FIS-command
	FIS_WRITE_startENA();
	FIS_WRITE_3LB_sendByte(in_msg[FIS_MSG_COMMAND]);
	FIS_WRITE_stopENA();

  byte msg_length = in_msg[FIS_MSG_LENGTH];
  byte msg_end = msg_length + 1;

  uint8_t crc =in_msg[FIS_MSG_COMMAND];
  
  for (uint16_t i = 1; i <= msg_end; i++)
  {
  // Step 2 - wait for response from cluster to set ENA-High
waitEnaHigh();

    delayMicroseconds(40);

    // calculate checksum
    if (i == msg_end) {
      crc --;
      in_msg[i] = crc;
    } else {
      crc ^= in_msg[i];
    }

    FIS_WRITE_3LB_sendByte(in_msg[i]);
  }

#ifdef ENABLE_IRQ
  sei();
#endif
}


/**

   Send Keep-Alive message

*/
void VW2002FISWriter::sendKeepAliveMsg() {
  delay(100);
  FIS_WRITE_send_3LB_singleByteCommand(0xC3);
  delay(100);
}

void VW2002FISWriter::displayOff() {
  sendRawMsg((char*)off);
}
void VW2002FISWriter::displayBlank() {
  sendRawMsg((char*)blank);
}

/**

   Send Single-Command out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_send_3LB_singleByteCommand(uint8_t txByte) {

#ifdef ENABLE_IRQ
  cli();
#endif

	FIS_WRITE_startENA();
	// Send FIS-command
	FIS_WRITE_3LB_sendByte(txByte);
	FIS_WRITE_stopENA();
	delayMicroseconds(30);

#ifdef ENABLE_IRQ
  sei();
#endif
}

/**

   Send byte out on 3LB port to instrument cluster

*/
void VW2002FISWriter::FIS_WRITE_3LB_sendByte(int16_t in_byte) {

  uint16_t tx_byte = 0xff - in_byte;
  for (int8_t i = 7; i >= 0; i--) {//must be signed! need -1 to stop "for"iing

    switch ((tx_byte & (1 << i)) > 0 ) {
      case 1: setDataHigh();
        break;
      case 0: setDataLow();
        break;
    }
    setClockLow();
    delayMicroseconds(FIS_WRITE_PULSEW);
    setClockHigh();
    setDataHigh();
    delayMicroseconds(FIS_WRITE_PULSEW);
  }
delayMicroseconds(50);
}

/**
   Set 3LB ENA active High
*/
void VW2002FISWriter::FIS_WRITE_startENA() {
  pinMode(_FIS_WRITE_ENA, OUTPUT);
  digitalWrite(_FIS_WRITE_ENA, HIGH);
}
/**
   Set 3LB ENA paaive Low
*/
void VW2002FISWriter::FIS_WRITE_stopENA() {
  pinMode(_FIS_WRITE_ENA, INPUT);
}

/**
   Set 3LB CLK High
*/
void VW2002FISWriter::setClockHigh() {
  digitalWrite(_FIS_WRITE_CLK,HIGH);
}
/**
   Set 3LB CLK Low
*/
void VW2002FISWriter::setClockLow() {
digitalWrite(_FIS_WRITE_CLK,LOW);
}
/**
   Set 3LB DATA High
*/
void VW2002FISWriter::setDataHigh() {
digitalWrite(_FIS_WRITE_DATA,HIGH);
}
/**
   Set 3LB DATA Low
*/
void VW2002FISWriter::setDataLow() {
digitalWrite(_FIS_WRITE_DATA,LOW);
}

/**
   checksum routine to calculate the crc

   takes komplete message as parameter including messagelength and the 240 constant followed by the 16 byte message
   example: uint8_t msg[] = {18, 240, 32, 78, 82, 75, 32, 80, 49, 32, 70, 77, 49, 46, 49, 32, 32, 28};

*/
uint8_t VW2002FISWriter::checksum( volatile uint8_t in_msg[]) {
  uint8_t crc = in_msg[0];
  for (int16_t i = 1; i < sizeof(in_msg); i++)
  {
    crc ^= in_msg[i];
  }
  crc --;

  return crc;
}


void VW2002FISWriter::waitEnaHigh(){
  uint16_t timeout_us = 1000;
  while (!digitalRead(_FIS_WRITE_ENA) && timeout_us > 0) {
    delayMicroseconds(1);
    timeout_us -= 1;
  }

}
