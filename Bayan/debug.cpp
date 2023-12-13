#include "debug.h"
#include "config.h"
#include <Arduino.h>

void __DebugPrint(char Str[]){
    Serial.write(Str);
}

void __DebugPrintLn(char Str[]){
    Serial.write(Str); 
    Serial.write("\r\n");
}

void __DebugPrint(unsigned char Num){
    Serial.print(Num);
}

void __DebugPrintLn(unsigned char Num){
    Serial.print(Num);
    Serial.write("\r\n");
}

void __DebugPrint(int Num){
    Serial.print(Num);
}

void __DebugPrintLn(int Num){
    Serial.print(Num);
    Serial.write("\r\n");

}

int print_array(int* x, int len) { 
 for (int Counter = 0; Counter < len; Counter++ ){
  printf("%d " , *( x + (Counter * sizeof(int) )));
 }
 return 0;
}
