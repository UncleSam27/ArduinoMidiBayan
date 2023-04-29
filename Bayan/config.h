#ifndef CONFIG_H
#define CONFIG_H

#define SCREEN_MINI12864_2_1 
//#define SCREEN_MINI12864_2_0 
//#define SCREEN_RepRapDiscount 

#define Debug false
#define Version "2.4.15 Mini"

//############################## Hardware pin defines ###################################


// Bayan Shild preset
#define BeeperPin 40
#define LED_Out 13

#define LCD_RS 43 //CS
#define LCD_RW 42 //MOSI
#define LCD_E  45 //SCK

#define SD_DET 44
#define SD_CS 53
#define SD_MISO 50
#define SD_MOSI 51
#define SD_SCK 52

#define ENC_BTN 41
#define ENC_L   49
#define ENC_R   48
#define BTN_RST 46


//FastFix  (переделать потом)
Encoder Encodr(ENC_L, ENC_R, ENC_BTN);
/**/

//############################## Global defines ###################################

#define DelayNorm                250
#define DelaySmall               100
#define DelayMicro               10

#define LCD_WIDTH                128
#define LCD_HEIGHT               64

#define MaxMenuString            5
#define MaxMenuStringWide        25
#define FontHigh                 10
#define FontWidth                8
#define FontFace                 u8g_font_8x13

#define AddressBeepOn            3999        
#define AddressMIDIOut           3998     
#define AddressProgramChangeMode 3997
#define AddressStartFile         3996
#define AddressBMP280Norm        3990
#define AddressInputKeysMask     3980



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

//############################## Global Varibles ###################################


unsigned char  ScanMode = 1;           // 0-scan in passive mode (gerkon)    1-scan in active mode (hall)
unsigned char  Normal_Pin_State  = 2;  // 0-"normal opened contacts"  1-"normal closed contacts"  2-"auto detect"
unsigned char  KeyInputPinStart  = 22;  
unsigned char  KeyInputPinStop   = 35;
unsigned char  KeyOutputPinStart = 2;
unsigned char  KeyOutputPinStop  = 9;
//unsigned char  GlobalQuickBoot   = 0;
unsigned char  ColorScreenRGB[4];
unsigned char  ColorButtonRGB[4];
unsigned char  ColorCaseRGB[4];

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

unsigned char  BMP280_Use         = 0;
unsigned char  BMP280_Sensitivity = 3;
unsigned char  BMP280_Chanel      = 0;
unsigned char  BMP280_AutoAdjust  = 1;
unsigned int   BMP280_AutoAdjust_Limit  = 100;

unsigned char  NeedLoad = 0;

char FileProgram[20]              = "INSTR.CFG";
char FileProgramPKG[20]           = "INSTR.PKG";

// Format of scancode
// D7 D6 D5 D4 D3 D2 D1 D0
//  |  |  |  |  |  |  |  | 
//  |  |  |  |  +--+--+--+------ D0-D3 - Number of input Pin  
//  +--+--+--+------------------ D4-D7 - Number of output Pin

unsigned char ScanCodeNums  =  0;
unsigned char KeyCodes[128];


/**/
//Global Program Change Mode
unsigned char GlobalProgramChangeMode; //1-Default only   2-Roland MSB+LSB

// All pressed keys there
unsigned char PressedKeys[MaxKeysOneTime+1];

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

#endif
