//code for fixing acceledent aura when it stops working and the LED Indicator is flashing alternating green and orange 

/* Example program for use with EEPROMsimple.h 
   Arduino Uno Memory Expansion Sample Program
   Author:  J. B. Gallaher       07/09/2016
   Library created and expanded by: D. Dubins 23-Jan-20

   Sample program to use a Serial EEPROM chip to expand memory for an Arduino Uno
   giving access to an additional 128kB of random access memory.  The 25LC1024 uses
   the Serial Peripheral Interface (SPI) to transfer data and commands between the
   UNO and the memory chip.

   Used the following components:
   (1) Arduino Uno
   (2) Microchip 25LC1024 SPI EEPROM chip
   
   Wiring:
   25LC1024 - Uno:
   ---------------
   pin 1 -- D10 (SS)
   pin 2 -- D12 (MISO)
   pin 3, 7, 8 -- +5V
   pin 4 -- GND
   pin 5 -- D11 (MOSI)
   pin 6 -- D13 (SCK)
*/
#include <EEPROMsimple.h>

#define CSPIN 10       // Default Chip Select Line for Uno (change as needed)
EEPROMsimple EEPROM;       //initialize an instance of this class

/*******  Set up code to define variables and start the SCI and SPI serial interfaces  *****/
void setup()
{
  uint32_t address = 0;                       // create a 32 bit variable to hold the address (uint32_t=long)
  Serial.begin(115200);                         // set communication speed for the serial monitor
  SPI.begin();                                // start communicating with the memory chip
    
 
/********* Read a single Byte *********************/
  //Serial.println("Reading each data byte individually: ");
  byte value;                                 // create variable to hold the data value read
  for(long i = 0; i <=131072; i++){
    address = i;                   
    EEPROM.WriteByte(address, 0xFF);    
  }
Serial.println("done");
}

void loop(){
}
