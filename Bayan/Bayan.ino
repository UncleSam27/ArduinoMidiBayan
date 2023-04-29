#include <GyverTimers.h>
#include <GyverEncoder.h>
#include "config.h"
#include "beeper.h"
#include "midi.h"
#include "lcd.h"
#include "menu.h"
#include "keyboard.h"
#include "keyboards.h"
#include "tests.h"
#include "comfunc.h"
#include "sdcard.h"


#pragma GCC diagnostic ignored "-Wwrite-strings"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Service function

// Setup Pin as Input width pool up
void   InitKeyPullUp(int pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Прерывание таймера для опроса энкодера
ISR(TIMER1_A) {
  Encodr.tick();  // отработка в прерывании тиков энкодера
}



//################################### SETUP ########################################
void setup() {
  MidiInit();
  DebugPrint("\nMidi Init Done\r\n"); 
  
  // Buttons PullUp
  InitKeyPullUp(BTN_RST);
  InitKeyPullUp(ENC_BTN);
  InitKeyPullUp(ENC_L);
  InitKeyPullUp(ENC_R);
  pinMode(BeeperPin, OUTPUT);   // Pin For Beeper
  DebugPrint("Pins init complete\r\n"); 

  DebugPrintLn("Read EEPROM settings");
  GlobalBeepOn =            EEPROM.read(AddressBeepOn);
  DebugPrint("GlobalBeepOn: ");
  DebugPrintLn(GlobalBeepOn);
  GlobalMIDIOut =           EEPROM.read(AddressMIDIOut);
  DebugPrint("GlobalMIDIOut: ");
  DebugPrintLn(GlobalMIDIOut);
  GlobalProgramChangeMode=  EEPROM.read(AddressProgramChangeMode);
  DebugPrint("GlobalProgramChangeMode: ");
  DebugPrintLn(GlobalProgramChangeMode);
  DebugPrintLn("EEPROM read complete\n"); 
  
  // Настраиваем тип и режим работы энкодера (вынести в отдельную функцию)
  Encodr.setType(Enc_Type);
  Encodr.setDirection(Enc_Direction);
  Encodr.setTickMode(MANUAL);


  // Настраиваем таймер и включаем прерывания
  Timer1.setFrequency(20);
  Timer1.disableISR(CHANNEL_A);
  //Timer1.enableISR(CHANNEL_A);  // не включаем, пользуемся опросом, включаем только в графическом меню 
  DebugPrintLn("Encoder init Done");


  LCDInit();
  DebugPrintLn("LCD Init Done"); 
  
  ColorSetup();
  StartLogo();
  DebugPrintLn("Show Logo - Done"); 
  
  while(SD_inserted()==0){
    MsgPrint("Insert SD");
  }
  
  while(SD_Init() !=0){
    MsgPrintWait("Cant init SD");
    TestSD();
    MsgPrint("Try Again...");
  }

  if(LoadKeybFromFlash("keyboard.cfg") !=0){
    MsgPrint("SD Cfg Error");
    DebugPrintLn("Load SD keyboard.cfg Error!"); 
    while(1);
  }else{
    DebugPrintLn("Load SD keyboard.cfg sucess!");
    Encodr.setType(Enc_Type); //final encoder init
    Encodr.setDirection(Enc_Direction);
    LCDRotation(LCD_rotation);
  }
  /**/


  if (digitalRead(BTN_RST)){   
    //Normal start
    char FName[10];
    itoa(EEPROM.read(AddressStartFile),FName,10);
    strcat(FName," Load");
    MsgPrint(FName); 

    LoadConfigFromFlashEEPROM(); //Load Config
    ColorSetup();  //Final color setup
  }else{     // if Reset is pushed load default settings
    MsgPrintWait("Set defaults!");
    EEPROM.write(AddressStartFile, 0);          //Use "0.cfg" next time
    EEPROM.write(AddressBeepOn, 1);              //Beep ON
    EEPROM.write(AddressMIDIOut, 3);             //USB and Midi out
    EEPROM.write(AddressProgramChangeMode, 2);   //New Style (MSB+LSB+Prog)
    StartWithDefaultCfg();

  }
  MyKeyb->SetupAllPrograms(GlobalProgramChangeMode); 
  DebugPrintLn("All ready. Start!");
}


//#################################### MAIN LOOP ##################################
void loop() {
    MenuMain();
   
}
