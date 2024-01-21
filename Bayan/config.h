#ifndef CONFIG_H
#define CONFIG_H

#define Version "2.4.17 Mini"

//select one of screen
#define SCREEN_MINI12864_2_1 
//#define SCREEN_MINI12864_2_0 
//#define SCREEN_RepRapDiscount 

/* uncomment for debug ver */
//#define Debug

/* uncomment for compile with dram machine support*/
#define DRAM_MACHINE_USED

/* uncomment for compile with BMP280 pressure sensor support*/
//#define BMP280_USED


//############################## Hardware pin defines ###################################

// Bayan Shild preset
#define BeeperPin 40
#define LED_Out 13

#ifdef SCREEN_RepRapDiscount
/* old dead schematic  
  #define LCD_RS 45 //CS
  #define LCD_RW 44 //MOSI
  #define LCD_E  46 //SCK
  #define SD_DET 47
/**/
  #define LCD_RS 43 //CS
  #define LCD_RW 42 //MOSI
  #define LCD_E  45 //SCK
  #define SD_DET 44

#else //SCREEN_MINI12864_2_0 or SCREEN_MINI12864_2_1
  #define LCD_RS 43 //CS
  #define LCD_RW 42 //MOSI
  #define LCD_E  45 //SCK
  #define SD_DET 44
#endif

#define SD_CS 53
#define SD_MISO 50
#define SD_MOSI 51
#define SD_SCK 52

#define ENC_BTN 41
#define ENC_L   49
#define ENC_R   48
#define BTN_RST 46

/**/

//############################## Global defines ###################################

#define KeybFilename "keyboard.cfg"

#define DelayNorm                250
#define DelaySmall               100
#define DelayMicro               10

#define LCD_WIDTH                128
#define LCD_HEIGHT               64

#define MaxMenuString            5
#define MaxMenuStringWide        25
#define MaxAllMenuStringsLen     200

#define FontHigh                 10
#define FontWidth                8
#define FontFace                 u8g_font_8x13

#define AddressBeepOn            3999        
#define AddressMIDIOut           3998     
#define AddressProgramChangeMode 3997
#define AddressStartFile         3996
#define AddressBMP280Norm        3990
#define AddressInputKeysMask     3980

#define InitScanMode             1  // 0-scan in passive mode (gerkon)    1-scan in active mode (hall)
#define InitNormal_Pin_State     2  // 0-"normal opened contacts"  1-"normal closed contacts"  2-"auto detect"
#define InitKeyInputPinStart     22  
#define InitKeyInputPinStop      35
#define InitKeyOutputPinStart    2
#define InitKeyOutputPinStop     9
#define InitScanCodeNums         0
#define KeyCodesCount            64

#define MaxKeyboards             16
#define MaxMidiChanels           16
#define MaxMidiControllers       5
#define MaxKeysOneTime           32
#define MaxNameLen               15
#define MaxHotkeys               24
#define MaxAnalogInputs          16
#define MaxOutCount              8

#define MaxHotkeyMidiLen         5
#define CountMIDIChanels         16

#define RecordLen                20  // count of bytes in one word
#define MaxFileStringLen         40
//############################## Global Varibles ###################################


extern unsigned char  NeedLoad;
extern char           FileProgram[20];
extern char           FileProgramPKG[20];

extern unsigned char  GlobalProgramChangeMode; 

#endif
