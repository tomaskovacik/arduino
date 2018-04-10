#include "VW2002FISWriter.h"
#include "bitmaps.h"
//#include <TimerOne.h>

#define MinRefresh 100


// FIS
#define FIS_CLK 13  // - Arduino 13 - PB5
#define FIS_DATA 11 // - Arduino 11 - PB3
#define FIS_ENA 8   // - Arduino 8 - PB0
VW2002FISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA );
static char fisBuffer[10]= {'B','5',' ','F','A','M','I','L','I','A'} ;
int myInit=2;
uint8_t frameBuffer[704];

char grg[3];


void lf(){

for (uint8_t line=0;line<8;line++){
  uint8_t tmpdata[1] = {left_door[line]};
  fisWriter.GraphicOut(15,line+19,1,tmpdata,1,0);
delay(1);
}
  }
  
void lr(){

for (uint8_t line=0;line<8;line++){
  uint8_t tmpdata[1] = {left_door[line]};
  fisWriter.GraphicOut(15,line+19+8,1,tmpdata,1,0);
delay(1);
}
  }

void rf(){

for (uint8_t line=0;line<8;line++){
  uint8_t tmpdata[1] = {right_door[line]};
  fisWriter.GraphicOut(41,line+19,1,tmpdata,1,0);
delay(1);
}
  }
void rr(){

for (uint8_t line=0;line<8;line++){
  uint8_t tmpdata[1] = {right_door[line]};
  fisWriter.GraphicOut(41,line+19+8,1,tmpdata,1,0);
delay(1);
}
  }

void trunk_avant(){
  fisWriter.GraphicFromArray(25,41,14,4,avant_trunc,1);
}
void trunk_sedan(){
  fisWriter.GraphicFromArray(23,38,18,7,sedan_trunc,1);
}

void trunk(){
  trunk_sedan();
}

void redrawFrameBuffer(){
 //fisWriter.initScreen(0x82,0,0,64,88);
  //fisWriter.GraphicFromArray(0,0,64,88,frameBuffer,1);
  for(uint8_t line=0; line<88;line=line+4){
  
  fisWriter.GraphicOut(0,line,32,frameBuffer,1,line*8);
  }    
  }

void setup() {
  
//for (uint16_t i=0;i<704;i++){
//    frameBuffer[i]=random(255);
//}


//for (uint8_t p=0;p<0xFF;p++){
//  grg[p]=p;
//}


//Serial.begin(115200);  
fisWriter.FIS_init();
//Serial.println("hello");
//delay(3000);
fisWriter.initScreen(0x82,0,0,1,1);
delay(1000);
fisWriter.initScreen(0x82,0,0,64,88);
//delay(100);
//fisWriter.GraphicFromArray(0,0,64,88,frameBuffer,1);
//fisWriter.GraphicFromArray(0,0,64,88,frameBuffer,1);
//  Timer1.initialize(800000);
//  Timer1.attachInterrupt(redrawFrameBuffer); // blinkLED to run every 0.15 seconds

}


void loop() {
//  fisWriter.initScreen(0x82,0,0,1,1);
//  delay(100);
//  while(myInit>0){
//    fisWriter.initScreen(0x82,0,0,0x40,0x58);
//    delay(100);
//    myInit--;
//    }
//fisWriter.GraphicFromArray(0,0,64,88,frameBuffer,1);

//for (uint16_t i=0;i<704;i++){
//    frameBuffer[i]=random(255);
//}
fisWriter.GraphicFromArray(0,0,64,88,b5f,1);
//fisWriter.initScreen(0x82,0,0,64,88);
//delay(100);
//fisWriter.GraphicFromArray(0,0,64,65,Q,1);
////delay(1000);
//fisWriter.GraphicFromArray(0,70,64,16,QBSW,1);
//delay(3000);
//fisWriter.initScreen(0x82,0,0,1,1);
//delay(100);
//fisWriter.sendMsg("12345678  TEST  ");
//delay(1000);
//fisWriter.initScreen(0x82,0,0x1B,64,0x30);
//fisWriter.GraphicFromArray(22,1,20,46,sedan,2);
//for (uint8_t x=0;x<2;x++){
//lf();
//lr();
//rf();
//rr();
//trunk();
//lf();
//lr();
//rf();
//rr();
//trunk();
//}
//fisWriter.initScreen(0x82,0,0,1,1);
//delay(1000);
//fisWriter.sendMsg("  TEST  12345678");
//delay(MinRefresh);




//uint8_t b=0;
//for (uint8_t x=1;x<0xFF;x++){
//  fisWriter.initScreen(0x82,0,0,64,88);
//delay(100);
//sprintf(grg, "%d", b);
//  fisWriter.sendMsgFS(20,10,0x01,3,grg);
//  delay(100);
//  fisWriter.sendMsgFS(20,35,0x09,1,(char*) &b);
//  b++;
//  delay(1000);
//fisWriter.initScreen(0x82,0,0,64,88);
//
//  }

while(true){
  fisWriter.sendKeepAliveMsg();
  delay(1000);
}
}

