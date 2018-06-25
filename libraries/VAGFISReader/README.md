library for reading V.A.G. 3lb line from radio

library in development, 

input are 3lines: ena/data/clk

interupt on ena: rising edge enable falling edge detection on clk like

interupt on CLK line read data lina value and store it as input packet

packet overvie: http://kovo-blog.blogspot.sk/2013/11/audi-fis-3-line-protocol.html


FULLY TESTED: radio mode on stm32 (bluepill board) over 3v resistor devider (10k to 47k) with 10k pullup on data and clk line, these pull up are required! I checked lot of schematics of radios, and most of them do not have internall pullups, so if it works without nice ,but mostly it will not. In case of 5V board (AVR MCU) internal pull up is ok, in case of 3v3 mcu external pull up + divider is erquired, if transistor based level shifter with internal pullup is used, check what value are these pullups and make stronger pulldown (half value resistor will do) on both low a high level side of ENABLE line (tested with mosfet based level shifter)



  how it works:

	3lb: 3line protocol (invert logic)
		data: 5V logic, 0v=logic 1, 5V=logic 0
		clk: 5V logic,
		ena: 5V logic, inverted CS, bidirectional, in radio mode, only maste use this line, NAVI 

radio mode: master make 100us pulse on ENABLE line, i did not know why, maybe it's just bug or something in blaupunk mcu(I use audi concert radio) , then goes ENABLE high and begin transfer on data/clk line 

schematics for 3V3 MCU:

RADIO ENA


        o-----| 10k |-------+----------------------o MCU 
                            |
                           ---
                           | |
                           47k
                           | |
                           ---
                            |
                           _|_
                           GND


RADIO DATA:

                             
	     +5V
              o
              |
             ---
             | |
             10k
             | |
             ---
              |
        o-----+---| 10k |-----+--------------------0 MCU
                              |
                             ---
                             | |
                             47k
                             | |
                             ---
                              |
                             _|_
                             GND

RADIO CLK:

             +5V
              o
              |
             ---
             | |
             10k
             | |
             ---
              |
        o-----+---| 10k |-----+--------------------0 MCU
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


more from russion forum passatworld.ru translated on google:

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
