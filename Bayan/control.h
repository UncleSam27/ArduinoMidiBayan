#ifndef CONTROL_H
#define CONTROL_H
#include <GyverEncoder.h>

extern Encoder        Encodr;
extern unsigned char  Enc_Type; 
extern unsigned char  Enc_Direction; // 0-norm  1-reversed
extern unsigned char  LCD_rotation;  // 0-norm  1-180rotated 

void        InitKeyPullUp(int pin);
void        InitKeys();
void        MenuInit();
void        MenuEncoderSetup();
void        IRQEnable();
void        IRQDisable();

#endif // CONTROL_H
