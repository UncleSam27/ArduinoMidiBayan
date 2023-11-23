#include <EEPROM.h>
#include "config.h"
#include "beeper.h"
#include "lcd.h"
#include "midi.h"
#include "tests.h"
#include "strings.h"
#include "menufunc.h"
#include "keyboards.h"
#include "keybloadsave.h"
#include "debug.h"
#include "control.h"

void StartLogo() {
  Klick();
  //Logo
  MsgPrint("Midi Bayan");
  MsgPrint(Version);
}


char MenuStr[MaxAllMenuStringsLen];

//################################### MIDI CONTROLLERS ###################################

//----------------------------------------------------------------------
// Get Analog Input Menu
AnalogIn* MenuGetAnalogInput(AnalogIn* Input) {

  MenuStr[0] = 0;
  strcat(MenuStr, "Select Input");

  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (MyKeyb->AnalogInputs[Counter] != NULL) {
      char Temp[10];
      strcat(MenuStr, "> A");
      itoa (MyKeyb->AnalogInputs[Counter]->Input, Temp, 10);
      strcat(MenuStr, Temp);
    }
  }
  unsigned int Choise = MyKeyb->GetAnalogInIndex(Input) + 1;
  Choise = Menu(Choise, MenuStr);
  return MyKeyb->AnalogInputs[Choise - 1];
}



//----------------------------------------------------------------------
// Cange Midi controller Menu
MidiController* MenuChangeMidiController(MidiController* Controller, MidiChanel *Chanel) {

  int Choise = 1;
  while (1) {
    //empty cycle for local MenuStr
    do {
      strcpy(MenuStr, "Change Ctrl>Back..>Ctrl Num    0   >Analog IN.. A0   >DELETE");

      InsertIntAnalogToStr(MenuStr, 31,  Controller->GetMSB());

      int Counter;
      for (Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (MyKeyb->AnalogInputs[Counter] != NULL) {
          if (&MyKeyb->AnalogInputs[Counter]->Value == Controller->Data->ExternalValue) {
            break;
          }
        }
      }
      InsertIntAnalogToStr(MenuStr, 49, Counter);//AnalogIn

      Choise = Menu(Choise, MenuStr);
    } while (0);

    if (Choise == 1) {
      break;
    }

    if (Choise == 2) {
      int CtrlNum = MenuGetInt("Controller", Controller->GetMSB(), 1, 119);
      Controller->SetMSB(CtrlNum);
      Controller->SetLSB(0);
      //Controller->SetLSB(CtrlNum+32);
    }

    if (Choise == 3) {
      int Counter;
      for (Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (MyKeyb->AnalogInputs[Counter] != NULL) {
          if (&MyKeyb->AnalogInputs[Counter]->Value == Controller->Data->ExternalValue) {
            break;
          }
        }
      }
      AnalogIn* TempIn = MenuGetAnalogInput(MyKeyb->AnalogInputs[Counter]);
      Controller->Data->SetExternalValue (&TempIn->Value);
    }

    if (Choise == 4) {
      //X3 как это сделать
      if (Chanel->DeleteMidiController(Controller) == 0) {
        MsgPrint("Ready!!!");
      } else {
        MsgPrint("Cant delete!");
      }
      delay(1000);
      break;

    }
  }
}


//----------------------------------------------------------------------
// Setup Midi controller Menu
void MenuMidiControllerSetup(MidiChanel *Chanel) {
  int Choise = 1;

  while (1) {
    strcpy(MenuStr, "Select Ctrl>Back..>New Controller..");

    for (int Counter = 0; Counter < MaxMidiControllers ; Counter++) {
      if (Chanel->Controllers[Counter] != NULL) {
        char Temp[10];
        itoa (Chanel->Controllers[Counter]->GetMSB(), Temp, 10);
        strcat(MenuStr, ">Ctrl ");
        strcat(MenuStr, Temp);
      }
    }

    Choise = Menu(Choise, MenuStr);

    if (Choise == 1) {
      break;
    }
    if (Choise == 2) {
      if (MyKeyb->GetMidiChanelCount() < MaxMidiChanels) {
        AnalogIn* TempIn = MenuGetAnalogInput(MyKeyb->AnalogInputs[0]);
        MenuChangeMidiController(Chanel->AddNewController(&TempIn->Value, 1, 33), Chanel);
      }
    }
    if (Choise > 2) {
      MenuChangeMidiController(Chanel->Controllers[Choise - 3], Chanel);
    }
  }
}

//----------------------------------------------------------------------
// Get MSB, LSB, Prog from text string
int GetProgramFromString(char STR[], unsigned char *Prog, unsigned char *MSB, unsigned char *LSB) {
  char * pEnd;
  char * pStart;
  pStart = &STR[24];

  *Prog = strtol(pStart, &pEnd, 10);
  *MSB = strtol(pEnd,    &pEnd, 10);
  *LSB = strtol(pEnd,    NULL,  10);

  /*  if( strlen(STR) == 36){
      Prog = atoi(STR[24]);
      MSB  = atoi(STR[28]);
      LSB  = atoi(STR[32]);
      return 0;
    }else{
      return -1;
    }/**/
}

//----------------------------------------------------------------------
// Create Midi Instrument package
int8_t CheckInstrumentPKG() {


  if (SD.exists(FileProgramPKG)) {
    return 0;
  } else {
    if (SD.exists(FileProgram)) {
      MsgPrintNoWait("Parsing file");
      char CharBuff[40];
      char WriteBuff[RecordLen];
      unsigned char MSB, LSB, Prog;

      File outputFile;
      File inputFile;

      outputFile = SD.open(FileProgramPKG, FILE_WRITE);
      if (!outputFile) {
        DebugPrint("Error: cant open file: ");
        DebugPrintLn(FileProgramPKG);
        MsgPrintWait("File Error");
        return -1;
      }

      inputFile = SD.open(FileProgram, FILE_READ);
      if (!inputFile) {
        DebugPrint("Error: cant open file: ");
        DebugPrintLn(FileProgram);
        MsgPrintWait("File Error");
        return -1;
      }

      while (ReadStringFromSD(&inputFile, CharBuff) != -1) {
        GetProgramFromString(CharBuff, &Prog, &MSB, &LSB);
        strcpy(WriteBuff, CharBuff);
        WriteBuff[RecordLen - 4] = 0;
        WriteBuff[RecordLen - 3] = MSB;
        WriteBuff[RecordLen - 2] = LSB;
        WriteBuff[RecordLen - 1] = Prog;
        outputFile.write(WriteBuff, RecordLen);
      }
      outputFile.close();
      inputFile.close();
    }

    return 0;
  }
}


//----------------------------------------------------------------------
//return string from file
int8_t SetupProgramFromPKGFile(char header[], MidiChanel *Chanel ) {
  int16_t MaxValue, MinValue, CurrentValue;

  char CharBuff[RecordLen];

  File myFile;

  myFile = SD.open(FileProgramPKG, FILE_READ);
  DebugPrint("Try read preset file ");
  DebugPrintLn(FileProgramPKG);
  // if the file opened okay, read line by line
  if (!myFile) {
    DebugPrintLn("Error: cant open file.");
    return -1;
  } else {
    DebugPrintLn("File open success");

    MinValue = 0;
    CurrentValue = 0;
    MaxValue = (myFile.available() / RecordLen) - 1; //get count of records


    DebugPrintLn("Get Current Instrument");
    for (uint16_t Counter = 0; Counter <= MaxValue; Counter++) {
      myFile.read(CharBuff, RecordLen);

      /*
        CharBuff[RecordLen-3] = MSB;
        CharBuff[RecordLen-2] = LSB;
        CharBuff[RecordLen-1] = Prog;/**/

      if ( (Chanel->Prog == CharBuff[RecordLen - 1]) && (Chanel->MSB == CharBuff[RecordLen - 3]) && (Chanel->LSB == CharBuff[RecordLen - 2]) ) {
        CurrentValue = Counter;
        break;
      }
    }


    while (1) {
      // Get current string
      myFile.seek(CurrentValue * RecordLen);

      myFile.read(CharBuff, RecordLen);
      CharBuff[15] = 0;

      Chanel->MSB = CharBuff[RecordLen - 3];
      Chanel->LSB = CharBuff[RecordLen - 2];
      Chanel->Prog = CharBuff[RecordLen - 1];

      Chanel->SetupChanelProgram(GlobalProgramChangeMode);

      MenuDisplaySelect("Instrument", MaxValue, CurrentValue, CharBuff);

      // key scan cycle
      while (1) {
        MyKeyb->Play();

        Encodr.tick();
        if (Encodr.isLeft()) {
          Klick();
          CurrentValue++;
          if (CurrentValue > MaxValue) {
            CurrentValue = MinValue;
          }
          break;
        }

        if (Encodr.isRight()) {
          Klick();
          CurrentValue--;
          if (CurrentValue < MinValue) {
            CurrentValue = MaxValue;
          }
          break;
        }

        if (Encodr.isPress()) {
          Klick();
          return;
        }
      }
    }
    myFile.close();
  }
}


//----------------------------------------------------------------------
// Setup Midi Program Menu
int8_t MenuMidiProgramSetup(MidiChanel *Chanel) {

  if (CheckInstrumentPKG() == 0) {

    SetupProgramFromPKGFile("Instrument", Chanel );
    Chanel->SetupChanelProgram(GlobalProgramChangeMode);

  } else {
    // Classic Midi
    if (GlobalProgramChangeMode == 1) {
      Chanel->Prog = MenuGetInt("Instrument", Chanel->Prog, 0, 127);
    }
    // New MIDI Instrument
    else if (GlobalProgramChangeMode == 2) {
      //unsigned char MSB, LSB, PC;
      Chanel->MSB = MenuGetInt("Set MSB", Chanel->MSB, 0, 127);
      Chanel->LSB = MenuGetInt("Set LSB", Chanel->LSB, 0, 127);
      Chanel->Prog = MenuGetInt("Set Program", Chanel->Prog, 0, 127) ;
    }
  }
  Chanel->SetupChanelProgram(GlobalProgramChangeMode);
}


//################################### MIDI CHANELS ###################################



//----------------------------------------------------------------------
// Cange Midi Chanel Menu
uint8_t MenuChangeMidiChanels(MidiChanel *Chanel) {

  int Choise = 1;
  while (1) {
    //empty cycle for local MenuStr
    do {
      //sprintf(MenuStr,"Change Midi Ch>Back..>Instrument>Volume      %s>Pressure    %s>Pitch       %s>Velocity    %s>Midi Chanel %s>Controllers..>DELETE", )
      strcpy(MenuStr, "Change Midi Ch>Back..>Instrument>Volume      0   >Pressure    0   >Pitch       0   >Velocity    0   >Midi Chanel 0   >Controllers..>DELETE");

      InsertIntAnalogToStr(MenuStr, 45, MyKeyb->GetBoxMenuValue(Chanel->Volume));
      InsertIntAnalogToStr(MenuStr, 62, MyKeyb->GetBoxMenuValue(Chanel->Pressure));
      InsertIntAnalogToStr(MenuStr, 79, MyKeyb->GetBoxMenuValue(Chanel->Pitch));
      InsertIntAnalogToStr(MenuStr, 96, MyKeyb->GetBoxMenuValue(Chanel->Velocity));
      InsertIntAnalogToStr(MenuStr, 113, Chanel->Chanel);
      Choise = Menu(Choise, MenuStr);
    } while (0);

    if (Choise == 1) {
      break;
    }

    if (Choise == 2) {
      MenuMidiProgramSetup(Chanel);
    }

    if (Choise == 3) {
      int Temp = MenuGetIntAndAnalog("Volume", MyKeyb->GetBoxMenuValue(Chanel->Volume), 0 - MyKeyb->GetAnalogInCount(), 127);
      if (Temp < 0) {
        Temp = ~Temp; //Temp = (Temp * -1) - 1
        Chanel->Volume->SetExternalValue(&MyKeyb->AnalogInputs[Temp]->Value);
      }
      else {
        Chanel->Volume->SetExternalValue(NULL);
        Chanel->Volume->Set(Temp);
      }
    }

    if (Choise == 4) {
      int Temp = MenuGetIntAndAnalog("Pressure", MyKeyb->GetBoxMenuValue(Chanel->Pressure), 0 - MyKeyb->GetAnalogInCount(), 127);
      if (Temp < 0) {
        Temp = ~Temp; //Temp = (Temp * -1) - 1
        Chanel->Pressure->SetExternalValue(&MyKeyb->AnalogInputs[Temp]->Value);
      } else {
        Chanel->Pressure->SetExternalValue(NULL);
        Chanel->Pressure->Set(Temp);
      }
    }

    if (Choise == 5) {
      int Temp =  MenuGetIntAndAnalog("Pitch", MyKeyb->GetBoxMenuValue(Chanel->Pitch), 0 - MyKeyb->GetAnalogInCount(), 127);
      if (Temp < 0) {
        Temp = ~Temp; //Temp = (Temp * -1) - 1
        Chanel->Pitch->SetExternalValue(&MyKeyb->AnalogInputs[Temp]->Value);
      } else {
        Chanel->Pitch->SetExternalValue(NULL);
        Chanel->Pitch->Set(Temp);
      }
    }

    if (Choise == 6) {
      int Temp =  MenuGetIntAndAnalog("Velocity", MyKeyb->GetBoxMenuValue(Chanel->Velocity), 0 - MyKeyb->GetAnalogInCount(), 127);
      if (Temp < 0) {
        Temp = ~Temp; //Temp = (Temp * -1) - 1
        Chanel->Velocity->SetExternalValue(&MyKeyb->AnalogInputs[Temp]->Value);
      } else {
        Chanel->Velocity->SetExternalValue(NULL);
        Chanel->Velocity->Set(Temp);
      }
    }

    if (Choise == 7) {
      Chanel->Chanel =  MenuGetIntAndAnalog("Midi Chanel", Chanel->Chanel, 0, 15);
    }

    if (Choise == 8) {
      MenuMidiControllerSetup(Chanel);
    }

    if (Choise == 9) {
      if (MyKeyb->DeleteMidiChanel(Chanel) == 0) {
        MsgPrint("Ready!!!");
      } else {
        MsgPrint("Cant delete!");
      }
      delay(1000);
      return 1; //return chanal delete flag
    }
    Chanel->SetupChanelProgram(GlobalProgramChangeMode);
  }
  return 0;
}

//----------------------------------------------------------------------
// Midi Chanel Menu
void MenuMidiChanels() {
  int Choise = 1;
  while (1) {
    strcpy(MenuStr, "Select Chanel>Back..>New Chanel..");

    for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
      if (MyKeyb->MidiChanels[Counter] != NULL) {
        char Temp[10];
        strcat(MenuStr, ">Ch ");
        itoa (MyKeyb->MidiChanels[Counter]->Chanel, Temp, 10);
        strcat(MenuStr, Temp);
      }
    }

    Choise = Menu(Choise, MenuStr);

    if (Choise == 1) {
      break;
    }
    if (Choise == 2) {
      if (MyKeyb->GetMidiChanelCount() < MaxMidiChanels) {
        MenuChangeMidiChanels(MyKeyb->AddNewMidiChanel(0, 127, 127, 64, 127, 0, 0, 0));
      }
    }
    if (Choise > 2) {
      if (MenuChangeMidiChanels(MyKeyb->MidiChanels[Choise - 3]) == 1) {
        Choise = 1;
      }
    }
  }
}

//----------------------------------------------------------------------
// SelectMidiChanel Menu
MidiChanel* MenuGetMidiChanel(MidiChanel* Chanel) {
  unsigned int Choise;

  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MyKeyb->MidiChanels[Counter] == Chanel) {
      Choise = Counter + 1;
    }
  }

  strcpy(MenuStr, "Select Chanel");

  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MyKeyb->MidiChanels[Counter] != NULL) {
      char Temp[10];
      strcat(MenuStr, ">Ch ");
      itoa (MyKeyb->MidiChanels[Counter]->Chanel, Temp, 10);
      strcat(MenuStr, Temp);
    }
  }
  Choise = Menu(Choise, MenuStr);
  return MyKeyb->MidiChanels[Choise - 1];
}



//################################### MENU KEYBOARD ###################################

//----------------------------------------------------------------------
// keyboard Menu
int MenuChangeKeyboard(Keyboard *Keybrd) {

  //check for
  if (Keybrd == NULL) {
    return -1;
  }

  //STOP ALL SOUND
  Keybrd->Chanel->AllNotesOff();

  int Choise = 1;
  while (1) {

    //empty cycle for local MenuStr
    do {
      strcpy(MenuStr, "Keyboard Settings>Back..>                >Start Key   0   >Last Key    0   >Chanel      0   >Base note   0   >Mode        0   >DELETE!");
      SubStrCopy(MenuStr, 25, Keybrd->Name);
      InsertIntToStr(MenuStr, 54, Keybrd->KeyBegin);
      InsertIntToStr(MenuStr, 71, Keybrd->KeyEnd);
      InsertIntToStr(MenuStr, 88, Keybrd->Chanel->Chanel);
      InsertIntToStr(MenuStr, 105, Keybrd->MidiBaseNote);
      InsertIntToStr(MenuStr, 122, Keybrd->Increment);
      Choise = Menu(Choise, MenuStr);
    } while (0);

    if (Choise == 1) {
      break;
    }

    if (Choise == 2) {
      MenuEditString(Keybrd->Name, MaxNameLen - 1);
    }
    if (Choise == 3) {
      //Keybrd->KeyBegin = MenuGetKey("Press start key");
      Keybrd->KeyBegin = MenuGetInt("Select Key", Keybrd->KeyBegin, 1, MyKeyb->ScanCodeNums);
    }
    if (Choise == 4) {
      //Keybrd->KeyEnd = MenuGetKey("Press end key");
      Keybrd->KeyEnd = MenuGetInt("Select Key", Keybrd->KeyEnd, 1, MyKeyb->ScanCodeNums);
    }
    if (Choise == 5) {
      Keybrd->Chanel = MenuGetMidiChanel(Keybrd->Chanel);
    }
    if (Choise == 6) {
      Keybrd->MidiBaseNote =  MenuGetInt("Select Note", Keybrd->MidiBaseNote, 0, 127);
    }
    if (Choise == 7) {
      Keybrd->Increment = (Menu(Keybrd->Increment + 1, "Mode>0 Single Note>1 Normal>2 Chords Normal>3 Chords single")) - 1;
      //Keybrd->Increment = MenuGetInt("Increment Note", Keybrd->Increment, 0, 3);
    }
    if (Choise == 8) {
      if (MyKeyb->DeleteKeyboard(Keybrd) == 0) {
        MsgPrint("Ready!!!");
      } else {
        MsgPrint("Cant delete!");
      }
      delay(1000);
      return 1;
    }
  }

  return 0;
}



//----------------------------------------------------------------------
// Keyboard Menu
void MenuKeyboard() {
  int Choise = 1;

  while (1) {
    //empty cycle for local MenuStr

    //Copy Init String to Menu String
    strcpy( MenuStr, "Keyboards Settings>Back..>Add Keyboard" );

    //Copy All Keyboard names to Menu String
    for (int counter = 0; counter < MaxKeyboards; counter++) {
      if (MyKeyb->Keyboards[counter] != NULL) {
        strcat( MenuStr, ">-" );
        strcat( MenuStr, MyKeyb->Keyboards[counter]->Name );
      }
    }

    Choise = Menu(Choise, MenuStr);

    // exit
    if (Choise == 1) {
      break;
    }

    // Add New Keyboard
    if (Choise == 2) {
      if (MyKeyb->GetMidiChanelCount() == 0) {
        MsgPrint("Add midi chan");
        delay(1000);
      } else {
        Keyboard *Keybrd;

        Keybrd = MyKeyb->AddNewKeyboard("Keyboard", 1, 1, MyKeyb->MidiChanels[0], 36, 1); //Name, BeginKey, EndKey, Chan, BaseNote, Increment
        if (Keybrd != NULL) {
          MsgPrint("Ready!!!");
          delay(1000);
          MenuChangeKeyboard(Keybrd);
        } else {
          MsgPrint("Error!");
          delay(1000);
        }
      }
    }

    // Change Keyboard
    if (Choise > 2) {
      if (MenuChangeKeyboard(MyKeyb->Keyboards[Choise - 3]) == 1) { // if keyboard deleted Choise put on return to back
        Choise = 1;
      }
    }
  }
}



//################################### HOTKEY MENU ###################################

//----------------------------------------------------------------------
// Hotkey change Menu
Keyboard* MenuSelectKeyboard(Keyboard* StartKeyb) {
  int Choise = 1;

  //Copy Init String to Menu String
  strcpy( MenuStr, "Select Keyb" );

  //Copy All Keyboard names to Menu String
  for (int counter = 0; counter < MaxKeyboards; counter++) {
    if (MyKeyb->Keyboards[counter] != NULL) {
      strcat( MenuStr, ">" );
      strcat( MenuStr, MyKeyb->Keyboards[counter]->Name );
    }
  }
  Choise = Menu(Choise, MenuStr);

  return MyKeyb->Keyboards[Choise - 1];
}



//----------------------------------------------------------------------
// Hotkey Modifier change Menu
int MenuChangeHotkeyModifier(Hotkey *Hotk) {
  //check for
  if (Hotk == NULL) {
    return -1;
  }

  //Update Menu String
  char HexStr[(MaxHotkeyMidiLen * 2) + 1] = "";

  MassToHexStr(Hotk->Modifier, HexStr, "");

  MenuEditHEXString(HexStr, MaxHotkeyMidiLen * 2);
  boolean Err = false;
  for (unsigned char Index = 0; HexStr[Index] != 0, Index++;) { // check for non hex chars in number
    if (!((HexStr[Index] >= '0' && HexStr[Index] <= '9') || (HexStr[Index] >= 'A' && HexStr[Index] <= 'F'))) {
      Err = true;
      break;
    }
  }

  if (Err == false) { // if no errors in hex string
    HexStrToMass(HexStr, Hotk->Modifier);
  } else {                        // if errors in hex string
    MsgPrintWait("Error in HEX");
  }
}

//----------------------------------------------------------------------
// Hotkey change Menu
uint8_t MenuChangeHotkey(Hotkey *Hotk) {
  //check for
  if (Hotk == NULL) {
    return -1;
  }

  unsigned char Choise = 1;
  while (1) {
    // Empty cycle (varible localization)
    do {

      char MenuStr[] = "Hotkey Settings  >Back..>                >For-                 >Key         0   >Modifier..>DELETE!";
      //Update Menu String
      SubStrCopy(MenuStr, 25, Hotk->Name);
      if (Hotk->Keyb != NULL) {
        SubStrCopy(MenuStr, 46, Hotk->Keyb->Name);
      } else {
        SubStrCopy(MenuStr, 46, "None");
      }
      InsertIntToStr(MenuStr, 76, Hotk->Key);

      Choise = Menu(Choise, MenuStr);
    } while (0);

    if (Choise == 1) {
      break;
    }
    if (Choise == 2) {
      MenuEditString(Hotk->Name, MaxNameLen - 1);
    }
    if (Choise == 3) {
      if (Hotk->Keyb != NULL) {
        Hotk->Keyb = MenuSelectKeyboard(Hotk->Keyb);
      }
    }
    if (Choise == 4) {
      Hotk->Key = MenuGetKey("Press Key");
    }
    if (Choise == 5) {
      if (Hotk->Action == 8) { // if action == Send Midi
        MenuChangeHotkeyModifier(Hotk);   //Menu Change send midi
      } else {
        Hotk->Modifier[0] = MenuGetInt("Modifier",  Hotk->Modifier[0], 0, 255);
        Hotk->Modifier[1] = 0;
      }
    }
    if (Choise == 6) {
      if (MyKeyb->DeleteHotkey(Hotk) == 0) {
        MsgPrint("Ready!!!");
      } else {
        MsgPrint("Cant delete!");
      }
      delay(1000);
      return 1;
    }
  }
  return 0;

}


//----------------------------------------------------------------------
// Create Hotkey Menu
void MenuCreateHotkey() {
  Hotkey *Hotk;
  strcpy( MenuStr, "Select type>Mute key>Velocity Up>Velocity Down>Chanel Up>Chanel Down>Volume Up>Volume Down>Send MIDI>Load Preset" );
  int Choise = Menu(1, MenuStr);

  unsigned char Modifier[2] = {1, 0};
  if (Choise == 1) {
    Hotk = MyKeyb->AddNewHotkey("Mute",       NULL,                                     MenuGetKey("Select Key"), Modifier, 1);
  }
  else if (Choise == 2) {
    Hotk = MyKeyb->AddNewHotkey("VelUp",      MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 2);
  }
  else if (Choise == 3) {
    Hotk = MyKeyb->AddNewHotkey("VelDown",    MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 3);
  }
  else if (Choise == 4) {
    Hotk = MyKeyb->AddNewHotkey("ToneUp",     MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 4);
  }
  else if (Choise == 5) {
    Hotk = MyKeyb->AddNewHotkey("ToneDown",   MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 5);
  }
  else if (Choise == 6) {
    Hotk = MyKeyb->AddNewHotkey("VolumeUp",   MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 6);
  }
  else if (Choise == 7) {
    Hotk = MyKeyb->AddNewHotkey("VolumeDown", MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 7);
  }
  else if (Choise == 8) {
    Hotk = MyKeyb->AddNewHotkey("SendMIDI",   MenuSelectKeyboard(MyKeyb->Keyboards[0]), MenuGetKey("Select Key"), Modifier, 8);
  }
  else if (Choise == 9) {
    Hotk = MyKeyb->AddNewHotkey("LoadPreset", NULL, MenuGetKey("Select Key"), Modifier, 9);
  }
  /*else if (Choise == 10) {
    Hotk = MyKeyb->AddNewHotkey("SelPreset",  NULL, MenuGetKey("Select Key"), Modifier, 10);
    }
    else if (Choise == 11) {
    Hotk = MyKeyb->AddNewHotkey("FreezeIN",   NULL, MenuGetKey("Select Key"), Modifier, 11);
    } */


  if (Hotk != NULL) {
    MsgPrint("Ready!!!");
    delay(1000);
    MenuChangeHotkey(Hotk);
  }
}


//----------------------------------------------------------------------
// Hotkey Menu
void MenuHotkey() {
  int Choise = 1;

  while (1) {
    //Copy Init String to Menu String
    strcpy( MenuStr, "Hotkeys Settings>Back..>Add Hot Key" );

    //Copy All Keyboard names to Menu String
    for (int counter = 0; counter < MaxHotkeys; counter++) {
      if (MyKeyb->Hotkeys[counter] != NULL) {
        strcat( MenuStr, ">-" );
        strcat( MenuStr, MyKeyb->Hotkeys[counter]->Name );
      }
    }

    Choise = Menu(Choise, MenuStr);

    // exit
    if (Choise == 1) {
      break;
    }

    // Add New Hotkey
    if (Choise == 2) {
      if (MyKeyb->GetHotkeyCount() == 0) { // if no keyboard error msg
        MsgPrint("Add keyboard");
        delay(1000);
      } else {
        MenuCreateHotkey();
      }
    }

    // Change Keyboard
    if (Choise > 2) {
      if (MenuChangeHotkey(MyKeyb->Hotkeys[Choise - 3]) == 1) { // if keyboard deleted Choise put on return to back
        Choise = 1;
      }
    }
  }
}


//################################### SETTINGS MENU ###################################


void MenuColorSettings() {

  int Choise = 1;                     //16  32  48
  char MenuString[] = "Colors RGB>Back..>Screen  #FFFFFF>Button1 #FFFFFF>Button2 #FFFFFF";
  while (1) {
    ColorSetup();
    InsertArrayToStrAsHex(MenuString, 27, ColorScreenRGB, 3);
    InsertArrayToStrAsHex(MenuString, 43, ColorButtonRGB, 3);
    InsertArrayToStrAsHex(MenuString, 59, ColorCaseRGB, 3);
    Choise = Menu(Choise, MenuString);

    //Back
    if (Choise == 1) {
      break;
    }

    if (Choise == 2) {
      ColorScreenRGB[0] = MenuGetIntHEX("RED",   ColorScreenRGB[0], 0, 255);
      ColorScreenRGB[1] = MenuGetIntHEX("GREEN", ColorScreenRGB[1], 0, 255);
      ColorScreenRGB[2] = MenuGetIntHEX("BLUE",  ColorScreenRGB[2], 0, 255);
    }

    if (Choise == 3) {
      ColorButtonRGB[0] = MenuGetIntHEX("RED",   ColorButtonRGB[0], 0, 255);
      ColorButtonRGB[1] = MenuGetIntHEX("GREEN", ColorButtonRGB[1], 0, 255);
      ColorButtonRGB[2] = MenuGetIntHEX("BLUE",  ColorButtonRGB[2], 0, 255);
    }

    if (Choise == 4) {
      ColorCaseRGB[0] = MenuGetIntHEX("RED",   ColorCaseRGB[0], 0, 255);
      ColorCaseRGB[1] = MenuGetIntHEX("GREEN", ColorCaseRGB[1], 0, 255);
      ColorCaseRGB[2] = MenuGetIntHEX("BLUE",  ColorCaseRGB[2], 0, 255);
    }

  }
}

//----------------------------------------------------------------------
// Settings Menu
void MenuSettings() {
  int Choise = 1;
  while (1) {
    Choise = Menu(Choise, "Main settings>Back..>Beeper>MIDI Outputs>ProgChange Mode>Colors");

    //Back
    if (Choise == 1) {
      break;
    }

    //Beeper
    if (Choise == 2) {
      GlobalBeepOn = Menu(GlobalBeepOn, "Beeper>On>Off");
      EEPROM.write(AddressBeepOn, GlobalBeepOn);
    }


    //MIDI
    if (Choise == 3) {
      GlobalMIDIOut = Menu(GlobalMIDIOut, "MIDI outs>MIDI out>USB out>Use both");
      EEPROM.write(AddressMIDIOut, GlobalMIDIOut);
    }

    //Program Change Mode
    if (Choise == 4) {
      GlobalProgramChangeMode = Menu(GlobalProgramChangeMode, "Select Mode>Prog only>Prog+MSB+LSB");
      EEPROM.write(AddressProgramChangeMode, GlobalProgramChangeMode);
    }
    //Program Change Mode
    if (Choise == 5) {
      MenuColorSettings();
    }


  }
}



//################################### TEST MENU ###################################

//----------------------------------------------------------------------
// TEST Menu
void MenuTest() {
  int Choise = 1;
  while (1) {
    Choise = Menu(Choise, "Tests>Back..>Midi test>Midi Prog>Scancode test>Scancode low>Analog inputs>Graph analog>Test SD>FW Version");
    if (Choise == 1) {
      break;
    }
    if (Choise == 2) {
      TestMidi();
    }
    if (Choise == 3) {
      TestMidiInstruments();
    }
    if (Choise == 4) {
      TestScancode();
    }
    if (Choise == 5) {
      TestScancode3();
    }
    if (Choise == 6) {
      TestAnalog();
    }
    if (Choise == 7) {
      GraphAnalogAlgoritm();
    }
    if (Choise == 8) {
      TestSD();
    }
    if (Choise == 9) {
      MsgPrintWait(Version);
    }

  }
}

//################################### INFO MENU ###################################

//----------------------------------------------------------------------
// Menu width info about current status
void MenuInfo(int DisplayCursor) {

  //Clear String
  MenuStr[0] = 0;
  //Header
  strcat(MenuStr, "Keyb Ch M  L  P");

  for (int Counter = 0; Counter < MyKeyb->GetKeyboardsCount(); Counter++) {
    char TempStr[MaxMenuStringWide];
    strcat(MenuStr, ">");

    TempStr[0] = 0;
    strcat(TempStr, MyKeyb->Keyboards[Counter]->Name);
    TempStr[4] = 0;
    strcat(MenuStr, TempStr);
    strcat(MenuStr, " ");

    itoa(MyKeyb->Keyboards[Counter]->Chanel->Chanel, TempStr, 10);
    strcat(MenuStr, TempStr);
    strcat(MenuStr, " ");

    int  MSB = MyKeyb->Keyboards[Counter]->Chanel->MSB;
    int  LSB = MyKeyb->Keyboards[Counter]->Chanel->LSB;
    int  PC =  MyKeyb->Keyboards[Counter]->Chanel->Prog;

    itoa(MSB, TempStr, 10);
    strcat(MenuStr, TempStr);
    strcat(MenuStr, " ");

    itoa(LSB, TempStr, 10);
    strcat(MenuStr, TempStr);
    strcat(MenuStr, " ");

    itoa(PC, TempStr, 10);
    strcat(MenuStr, TempStr);
    strcat(MenuStr, " ");
  }

  MenuPrint(DisplayCursor, MenuStr);

}


//----------------------------------------------------------------------
// Menu width info about current status
void MenuInfoNow() {
  delay(DelayNorm);
  int Counter = 0;
  while (GetButtonState() == 0) {
    Counter++;
    MyKeyb->Play();
    if (Counter == 10) {
      MenuInfo(0);
      Counter = 0;
    }
  }
}



//################################### PRESETS MENU ###################################


//----------------------------------------------------------------------
// Presets Menu
void MenuPresets() {
  int Choise = 2;
  char fname[20];
  while (1) {
    Choise = Menu(Choise, "Presets>Back..>Load fast>Save fast>Load preset..>Save preset..");

    if (Choise == 1) {
      break;
    }

    if (Choise == 2) {
      unsigned char Key = MenuGetKey("Preset key") - 1;
      itoa(Key, fname, 10);
      strcat(fname, ".cfg");
      if (SD.exists(fname)) {
        MsgPrintInt(Key);
        EEPROM.write(AddressStartFile, Key);
        LoadConfigFromFlashEEPROM();
      } else {
        MsgPrint("No such CFG");
      }
    }

    if (Choise == 3) {
      //get keynum
      unsigned char Key = MenuGetKey("Preset key") - 1;
      MsgPrintInt(Key);

      //create file name from key number like 2.cfg
      itoa(Key, fname, 10);
      strcat(fname, ".cfg");

      if (SaveConfigToFlash(fname) == 0) {
        MsgPrint("Saved!");
        delay(1000);
      } else {
        MsgPrint("Write Error!");
        delay(2000);
      }
    }

    if (Choise == 4) {
      unsigned char Key  = MenuGetInt("Load preset",  EEPROM.read(AddressStartFile), 0, 127);
      itoa(Key, fname, 10);
      strcat(fname, ".cfg");
      if (SD.exists(fname)) {

        //MsgPrintInt(Key);
        EEPROM.write(AddressStartFile, Key);
        LoadConfigFromFlashEEPROM();
      } else {
        MsgPrint("No such CFG");
      }
    }

    if (Choise == 5) {
      //get keynum
      unsigned char Key = MenuGetInt("Save preset",  EEPROM.read(AddressStartFile), 0, 127);
      MsgPrintInt(Key);

      //create file name from key number like 2.cfg
      itoa(Key, fname, 10);
      strcat(fname, ".cfg");

      if (SaveConfigToFlash(fname) == 0) {
        MsgPrint("Saved!");
        delay(1000);
      } else {
        MsgPrint("Write Error!");
        delay(2000);
      }
    }

  }
}




//################################### ANALOG INPUTS MENU ###################################

//----------------------------------------------------------------------
// Analog Inputs Menu
void  MenuAnalogIn() {
  int Choise = 1;

  //Copy Init String to Menu String
  strcpy( MenuStr, "Analog In>Back..");

  //Copy All analog in to Menu String
  for (int Counter = 0; Counter < MyKeyb->GetAnalogInCount(); Counter++) {
    char TempString[5];
    strcat( MenuStr, ">AnalogIn " );
    itoa(Counter, TempString, 10); //(integer, yourBuffer, base)
    strcat( MenuStr, TempString );
  }

  while (1) {
    Choise = Menu(Choise, MenuStr);


    if (Choise == 1) {
      break;
    } else {
      ChangeModeAnalogIn(Choise - 2);
    }
  }
}


//----------------------------------------------------------------------
// Main Menu
void MenuMain() {
  int Choise = 1;
  while (1) {
    Choise = Menu(Choise, "Main Menu>Presets>Midi Chanels>Keyboards>Hot keys>Analog In>Info>Settings>Tests");

    if (Choise == 1) {
      MenuPresets();
    }

    if (Choise == 2) {
      MenuMidiChanels();
    }

    if (Choise == 3) {
      MenuKeyboard();
    }

    if (Choise == 4) {
      MenuHotkey();
    }

    if (Choise == 5) {
      MenuAnalogIn();
    }

    if (Choise == 6) {
      MenuInfoNow();
    }

    if (Choise == 7) {
      MenuSettings();
    }

    if (Choise == 8) {
      MenuTest();
    }
  }
}
