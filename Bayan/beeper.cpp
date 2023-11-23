#include <arduino.h>
#include "config.h"
#include "beeper.h"

//Global varible - Да это ужасно, потом надо будет переделать.
char GlobalBeepOn;

// Long Beep 
void Beep(int del) {
  if(GlobalBeepOn==1){
    digitalWrite(BeeperPin, HIGH);
    delay(del);
    digitalWrite(BeeperPin, LOW);
  }  
}

//Short Beep
void Klick() {
    Beep(2); 
}
