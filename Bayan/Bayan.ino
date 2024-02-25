#include <EEPROM.h>
#include "config.h"
#include "beeper.h"
#include "midi.h"
#include "lcd.h"
#include "menu.h"
#include "menufunc.h"
#include "keyboard.h"
#include "keyboards.h"
#include "tests.h"
#include "sdcard.h"
#include "keybloadsave.h"
#include "debug.h"
#include "control.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"


///////////////////////////////////////////////////////////////////////////////////////////////////
// Service function




//################################### SETUP ########################################
void setup() {
  MidiInit();
  DebugPrint("\nMidi Init Done\r\n"); 
  
  InitKeys();
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
  
  // Настраиваем тип и режим работы энкодера
  MenuEncoderSetup();
  DebugPrintLn("Encoder init Done");
    
  MenuInit();
  DebugPrintLn("Menu init Done");

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

  MyKeyb = InitKeyboard(NULL);
  DebugPrintLn("Keyboard obj init");
  
  if(LoadKeybFromFlash(KeybFilename) !=0){
    MsgPrint("SD Cfg Error");
    DebugPrintLn("Load SD " KeybFilename "Error!"); 
    while(1);
  }else{
    DebugPrintLn("Load SD " KeybFilename " sucess!");
    MenuEncoderSetup();
    LCDRotation(LCD_rotation);
  }
  
  if (digitalRead(BTN_RST)){   
    //Normal start
    char FName[10];
    itoa(EEPROM.read(AddressStartFile),FName,10);
    strcat(FName," Load");
    MsgPrint(FName); 

    LoadConfigFromFlashEEPROM(); //Load Config

#ifdef DRAM_MACHINE_USED
    LoadDrumConfigFromFlashEEPROM(MyKeyb->DrumMachin); // Load drum machine config
#endif
    
    ColorSetup();  //Final color setup
  }else{     // if Reset is pushed load default settings
    MsgPrintWait("Set defaults!");
    EEPROM.write(AddressStartFile, 0);           //Use "0.cfg" next time
    EEPROM.write(AddressBeepOn, 1);              //Beep ON
    EEPROM.write(AddressMIDIOut, 3);             //USB and Midi out
    EEPROM.write(AddressProgramChangeMode, 2);   //New Style (MSB+LSB+Prog)
    EEPROM.write(AddressStartDrumFile, 0);       //Use "0.drm" next time
    StartWithDefaultCfg();

  }
  MyKeyb->SetupAllPrograms(GlobalProgramChangeMode); 
  DebugPrintLn("All ready. Start!");
}


//#################################### MAIN LOOP ##################################
void loop() {
  MenuMain();
}
