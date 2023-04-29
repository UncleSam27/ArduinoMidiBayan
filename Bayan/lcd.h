#ifndef LCD_H
#define LCD_H
#include <U8glib.h>
#include "config.h"


//For SCREEN_RepRapDiscount section
 #ifdef SCREEN_RepRapDiscount 
   U8GLIB_ST7920_128X64_4X u8g(LCD_E, LCD_RW, LCD_RS); // SPI Com: SCK = en = LCD_E, MOSI = rw = LCD_RW, CS = di = LCD_RS
 #endif
/**/

//For SCREEN_MINI12864_2_0 section
 #ifdef SCREEN_MINI12864_2_0 
   #define LCD_R_Control_PIN 12
   #define LCD_G_Control_PIN 11
   #define LCD_B_Control_PIN 10
    
   U8GLIB_MINI12864 u8g(LCD_RW, LCD_RS, LCD_E); // uint8_t cs, uint8_t a0, uint8_t reset = U8G_PIN_NONE
 #endif



//For SCREEN_MINI12864_2.1 section
#ifdef SCREEN_MINI12864_2_1 
   #define LCD_RGB_Control_PIN 12
   #define LCD_RGB_Pixels 3
   #include <Adafruit_NeoPixel.h>
   Adafruit_NeoPixel pixels(LCD_RGB_Pixels, LCD_RGB_Control_PIN, NEO_RGB + NEO_KHZ800);
   U8GLIB_MINI12864 u8g(LCD_RW, LCD_RS, LCD_E); // uint8_t cs, uint8_t a0, uint8_t reset = U8G_PIN_NONE
 #endif



//----------------------------------------------------------------------
//Color Setup Function
void ColorSetup(){


//For SCREEN_MINI12864_2.0 section
  #ifdef SCREEN_MINI12864_2_0 
    pinMode(LCD_R_Control_PIN, OUTPUT);   
    pinMode(LCD_G_Control_PIN, OUTPUT);   
    pinMode(LCD_B_Control_PIN, OUTPUT);   
    
    analogWrite(LCD_R_Control_PIN, ColorScreenRGB[0]);
    analogWrite(LCD_G_Control_PIN, ColorScreenRGB[1]);
    analogWrite(LCD_B_Control_PIN, ColorScreenRGB[2]);/**/
  #endif



  #ifdef SCREEN_MINI12864_2_1 
    pixels.setPixelColor(0, pixels.Color(ColorScreenRGB[0], ColorScreenRGB[1],  ColorScreenRGB[2]));
    pixels.setPixelColor(1, pixels.Color(ColorButtonRGB[0], ColorButtonRGB[1],  ColorButtonRGB[2]));
    pixels.setPixelColor(2, pixels.Color(ColorCaseRGB[0],   ColorCaseRGB[1],    ColorCaseRGB[2]));
    pixels.show();
  #endif
/**/

}


//----------------------------------------------------------------------
void LCDRotation(unsigned char Rotation){
      switch(Rotation) {
        case 0: u8g.undoRotation(); break;
        case 1: u8g.setRot180(); break;
      }
}
//----------------------------------------------------------------------
//LCD init
void LCDInit(){
  #ifdef SCREEN_MINI12864_2_1 
    //включаем динамическую подсветку 
    pixels.begin();
    pixels.clear();
  
    //цвета по умолчанию случайные
    randomSeed(analogRead(15));
    for(unsigned char i=0;i<3;i++) ColorScreenRGB[i]=random(254);
    for(unsigned char i=0;i<3;i++) ColorButtonRGB[i]=random(254);
    for(unsigned char i=0;i<3;i++) ColorCaseRGB[i]=random(254);
  #endif
 /**/ 

//For SCREEN_MINI12864_2.0 section
  #ifdef SCREEN_MINI12864_2_0 
    //цвета по умолчанию случайные
    randomSeed(analogRead(15));
    for(unsigned char i=0;i<3;i++) ColorScreenRGB[i]=random(254);
    for(unsigned char i=0;i<3;i++) ColorButtonRGB[i]=random(254);
    for(unsigned char i=0;i<3;i++) ColorCaseRGB[i]=random(254);
  #endif
 /**/ 
  //настраиваем экран
  u8g.setContrast(250);
  u8g.setFont(FontFace);
  
}

#endif
