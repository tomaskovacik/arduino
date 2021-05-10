// sketch to go over all modes of automatic transmition on audi cluster 
// Selector lever display 
//
// contrl is over transistor switch, so signals in sketch are inverted (high switch transistor ON so pull down signal line)
// high (low in this sketch) pusle is identification of transmition : 
//
//type of trans. in miliseconds
//5  -> PRND432
//10 -> PRND321
//20 -> PRNDL
//
// low pusle (high in this sketch) is multiply of lenght of identification pulse (5,10,20) and selected gear/mode
// 
// for example:
// [P]RND432 -> 5ms HIGH -> 5*1ms LOW
// P[R]ND432 -> 5ms HIGH -> 5*2ms LOW 
// PR[N]D432 -> 5ms HIGH -> 5*3ms LOW
// PRN[D]432 -> 5ms HIGH -> 5*4ms LOW
//
// pin 2 of arduino uno is connected to base of NPN transistor (2n5551 in my case) via 1k resistor
// emitor of transistor to ground
// collector to pin 6 of 20pin red connector of audi a4 1998 cluster (or black connector in early models)

//                       cluster 
//                          |
//                          |
//                          |         
//         ____             |
// D2 ____|1k  |__________|/  NPN
//        |____|          |\e
//                          |
//                          |
//                          |
//                         _|_ GND
//
// video: https://www.youtube.com/watch?v=0d18H4fjk1M


int tranIdent=5;
int i=1;
long mytime=0;

void setup() {
pinMode(2,OUTPUT);
digitalWrite(2,LOW);
Serial.begin(9600);
}

void loop() {

//if ((millis()-mytime)>1000){
//    mytime=millis();
//    i++;
//  }

digitalWrite(2,HIGH);
delay(tranIdent*i);
digitalWrite(2,LOW);
delay(tranIdent);
if (i == 20){
  i=1;
  tranIdent=tranIdent*2;
  if (tranIdent==40) tranIdent=5;
}
}
