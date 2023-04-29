#ifndef BEEPER_H
#define BEEPER_H

#include "config.h"

//Global varible - Да это ужасно, потом надо будет переделать.
// 1-Beep on
// 2-Beep off
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

#endif
