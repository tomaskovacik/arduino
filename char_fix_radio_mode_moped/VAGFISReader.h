/*
  (C) Tomas Kovacik
  https://github.com/tomaskovacik/
  GNU GPL3

  VAGFISReader.h

  arduino library for reading V.A.G. 3lb line from radio

WHAT IS FULLY TESTED:

full screen mode and radio mode on Arduino Nano (328) 5Vmode with internall pullups
radio mode on stm32 (bluepill board) over 3v resistor divider (10k/47k) with 10k pullup on data and clk line, these pull up are required! I check lot of schematics of radios, and most of them do not have internall pullups(using open colector here), so if it works without nice ,but mostly it will not.
In case of 5V board (AVR MCU) internal pull up is used, in case of 3v3 mcu external pull up + divider is erquired, if transistor based level shifter with internal pullup is used, check what value are these pullups and make stronger pulldown (half value resistor will do) on both low a high level side of ENABLE line(tested with mosfet based level shifter)




BASICS:

	3lb: 3line protocoli, inverted logic: 0v=logic 1, 5V=logic 0
	data: 5V logic, in same cases (for example audi concert/chorus1) use open colector here
	clk: 5V logic,  in same cases (for example audi concert/chorus1) use open colector here
	ena: 5V logic, inverted CS, bidirectional, in radio mode, only maste use this line, NAVI 

RADIO MODE:

REFRESH:
packet from master (radio) is sent after event when text on extenal display need to be changed, cluster  ack received data by 3ms high pulse on ENA line, see example

ACTUAL PACKET CONSIST OF: 
- master make 100us pulse on ENABLE line, i did not know why, maybe it's just bug or something in blaupunk mcu(I use audi concert radio)
- then goes ENABLE high and begin transfer on data/clk line 
- 18bytes: ID 2x8 charactes CRC
- ID: always 0xF0 
- 2x8 chars 
- CHECKSUM 0xFF ^ SUM(ID+TEXT) 
- ENA goes LOW

NAVI MODE:

- we have to determinate that we are in NAVI mode, in this case only ID is sent as packet if ENA line is LOW  and other parts of packet are send as requested by cluster (or this SW) by pulling ENA line HIGH(this is latching right?)
- so we got first packet, it's only 8bits and it is not 0xF0 (radio mode), so presume we are in NAVI mode store this packet and also put it in (CRC=1stByte) and lets "latch other data in":

- set ena HIGH
- byte is received on data/clk from radio/navi, this one is "packet size"
- set ena LOW (or this is set by radio to LOW?)
- store this byte calculate CRC like CRC^(this byte) and  we can set loop with this number for number of bytes we are going to receive
- in for while packet_size-1=0
	- set ena HIGH
	- receive byte
	- set ena LOW
	-process byte, store it and if byte id == packet_size (if (byte == (crc-1) set flag CRC_IS_OK) else CRC=CRC^(byte)
- end of transmittion, if (CRC_IS_OK): store actual data


schematics for 3V3 MCU:

RADIO ENA o-----| 10k |-------+----------------------o MCU 
                              |
                             ---
                             | |
                             47k
                             | |
                             ---
                              |
                             _|_
                             GND

                             
	     +5V
              o
              |
             ---
             | |
             10k
             | |
             ---
RADIO         |
DATA    o-----+---| 10k |-----+--------------------0 MCU
                              |
                             ---
                             | |
                             47k
                             | |
                             ---
                              |
                             _|_
                             GND

             +5V
              o
              |
             ---
             | |
             10k
             | |
             ---
RADIO         |
 CLK    o-----+---| 10k |-----+--------------------0 MCU
                              |
                             ---
                             | |
                             47k
                             | |
                             ---
                              |
                             _|_
                             GND




with level shifter interla pullups are 10k for this shifter:


RADIO DATA   o---------|5V  level shift 3V3|-----------o MCU
RADIO CLK    o---------|5V  level shift 3V3|-----------o MCU
RADIO ENABLE o----+----|5V  level shift 3V3|-----+-----o MCU
                  |                              |
                 ---                            ---
                 | |                            | |
                 4k3                            4k3
                 | |                            | |
                 ---                            ---
                  |                              |
                 _|_                            _|_
                 GND                            GND


more for russion forum passatworld.ru translated on google:

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

#ifndef VAGFISReader_h
#define VAGFISReader_h

#include <inttypes.h>
#include <Arduino.h>
volatile static uint16_t msgbit;
volatile static uint8_t navi=0;
volatile static uint8_t newMsgFromRadio=0;
volatile static uint8_t packetSize=0;
volatile static uint8_t preNavi=0;
volatile static uint8_t data[64] ;
static uint8_t FIS_READ_CLK;
static uint8_t FIS_READ_DATA;
static uint8_t FIS_READ_ENA;

class VAGFISReader
{
public:
	VAGFISReader(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin);
	~VAGFISReader();
	void begin();
	uint8_t readData(int8_t id);
	bool hasNewMsg();
	bool msgIsNavi();
	void clearNewMsgFlag();
	bool ACK(); //3ms pulse on ENA line by cluster to ACK received packet
	uint8_t getMsgId();
	uint8_t getSize();
	static bool checkData();
	static bool calcChecksum();
	static uint8_t getChecksum();
	bool msgIsRadioText();
	bool msgIsText();
	bool msgIsGraphics();
	bool msgIsInit();
	bool msgIsKeepAlive();
	bool _wait();
	bool _continue();

private:
static void readDataLine();
static void detectEnaLineRising();
static void detectEnaLineFalling();

};
#endif

