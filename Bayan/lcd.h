#ifndef LCD_H
#define LCD_H

#include <U8glib.h>
#include "config.h"

extern unsigned char  ColorScreenRGB[4];
extern unsigned char  ColorButtonRGB[4];
extern unsigned char  ColorCaseRGB[4];

 #ifdef SCREEN_RepRapDiscount
   extern U8GLIB_ST7920_128X64_4X u8g;
 #endif //SCREEN_RepRapDiscount

 #ifdef SCREEN_MINI12864_2_0 
   extern U8GLIB_MINI12864       u8g;
 #endif //SCREEN_MINI12864_2_0

#ifdef SCREEN_MINI12864_2_1 
   extern U8GLIB_MINI12864       u8g;
 #endif //SCREEN_MINI12864_2_1

void ColorSetup();
void LCDRotation(unsigned char Rotation);
void LCDInit();


#endif
