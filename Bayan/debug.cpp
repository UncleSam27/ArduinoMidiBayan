#include "debug.h"
#include "config.h"
#include <Arduino.h>

void DebugPrint(char Str[]){
    if(Debug) Serial.write(Str);
}

void DebugPrintLn(char Str[]){
    if(Debug) {
      Serial.write(Str); 
      Serial.write("\r\n");
    }
}

void DebugPrint(int Num){
    if(Debug) Serial.print(Num);
}

void DebugPrintLn(int Num){
    if(Debug) Serial.print(Num);
    Serial.write("\r\n");

}

int print_array(int* x, int len) { 
 for (int Counter = 0; Counter < len; Counter++ ){
  printf("%d " , *( x + (Counter * sizeof(int) )));
 }
 return 0;
}
