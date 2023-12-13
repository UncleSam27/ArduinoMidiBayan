#include <GyverTimers.h>
#include <GyverEncoder.h>
#include "config.h"
#include "control.h"

#ifdef SCREEN_RepRapDiscount
unsigned char  Enc_Type = TYPE2; 
unsigned char  Enc_Direction = 1; // 0-norm  1-reversed
#endif

#ifdef SCREEN_MINI12864_2_0 
unsigned char  Enc_Type = TYPE1;
unsigned char  Enc_Direction = 0; // 0-norm  1-reversed
#endif

#ifdef SCREEN_MINI12864_2_1 
unsigned char  Enc_Type = TYPE1;
unsigned char  Enc_Direction = 0; // 0-norm  1-reversed
#endif

unsigned char  LCD_rotation = 0;  // 0-norm  1-180rotated


// Setup Pin as Input width pool up
void   InitKeyPullUp(int pin){
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

void   InitKeys(){
  // Buttons PullUp
  InitKeyPullUp(BTN_RST);
  InitKeyPullUp(ENC_BTN);
  InitKeyPullUp(ENC_L);
  InitKeyPullUp(ENC_R);
}


//FastFix  (переделать потом)
Encoder Encodr(ENC_L, ENC_R, ENC_BTN);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Прерывание таймера для опроса энкодера
ISR(TIMER1_A) {
  Encodr.tick();  // отработка в прерывании тиков энкодера
}


void IRQEnable(){
  Timer1.enableISR(CHANNEL_A);

}

void IRQDisable(){
  Timer1.disableISR(CHANNEL_A);
}

void MenuInit(){
  // Настраиваем таймер и включаем прерывания
  Timer1.setFrequency(20);
  Timer1.disableISR(CHANNEL_A);
  //Timer1.enableISR(CHANNEL_A);  // не включаем, пользуемся опросом, включаем только в графическом меню 

}

void MenuEncoderSetup(){
  // Настраиваем тип и режим работы энкодера
  Encodr.setType(Enc_Type);
  Encodr.setDirection(Enc_Direction);
  Encodr.setTickMode(MANUAL);  
}
