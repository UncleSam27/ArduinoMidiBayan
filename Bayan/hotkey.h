#ifndef HOTKEY_H
#define HOTKEY_H

#include <string.h>
#include "config.h"
#include "keyboard.h"
#include "BMP280.h"
#include "comfunc.h"

//--------------------------------------------------------------------------------------------------------------
// class Hotkey
//--------------------------------------------------------------------------------------------------------------

class Hotkey {
  private:
    unsigned char  DelayCycle;       // count of non working cycle after press the key
    int Cycle;    
  public:
    Keyboard * Keyb;
    char Name[MaxNameLen + 1];
    unsigned char  Key;              //Keynumber
    unsigned char  Action;
    unsigned char  Modifier[MaxHotkeyMidiLen+1];
    

    //----------------------------------------------------------------------
    //the constructor
    Hotkey(char initName[], Keyboard * initKeyb,  unsigned char initKey, unsigned char initModifier[], unsigned char  initAction) {

      //Init Name
      strcpy(Name,initName);
      strcpy(Modifier,initModifier);
          
      Keyb = initKeyb;
      Key = initKey;
      DelayCycle = 100;
      Cycle = 0;
      Action = initAction;
    }

  void(* ResetFunc) (void) = 0;//declare reset function at address 0

  //----------------------------------------------------------------------
  // Mute All Sounds
  void HKMuteAll() {
    Klick();
    for (int Counter = 0; Counter < 16; Counter++) { //for all midi chanels
      MidiAllSoundsOff(Counter); // mute all!
    }
  }


  //----------------------------------------------------------------------
  // Keyboard Velocity Up
  void HKVelocityUp() {
    Klick();
    if (Keyb->Chanel->Velocity->Get() + Modifier[0] < 127) {
      Keyb->Chanel->Velocity->Set(Keyb->Chanel->Velocity->Get() + Modifier[0]);
    }
    Keyb->Chanel->Velocity->SetExternalValue(NULL);
  }

  //----------------------------------------------------------------------
  // Keyboard Velocity Down
  void HKVelocityDown() {
    Klick();
    if (Keyb->Chanel->Velocity->Get() - Modifier[0] >= 0) {
      Keyb->Chanel->Velocity->Set(Keyb->Chanel->Velocity->Get() - Modifier[0]);
    }
    Keyb->Chanel->Velocity->SetExternalValue(NULL);
  }


  //----------------------------------------------------------------------
  // Keyboard tone Up
  void HKToneUp() {
    Klick();
    if (Keyb->MidiBaseNote + Modifier[0] < 127) {
      Keyb->MidiBaseNote += Modifier[0];
    }
  }

  //----------------------------------------------------------------------
  // Keyboard Tone Down
  void HKToneDown() {
    Klick();
    if (Keyb->MidiBaseNote - Modifier[0] >= 0) {
      Keyb->MidiBaseNote -= Modifier[0];
    }
  }


  //----------------------------------------------------------------------
  // Keyboard Volume Up
  void HKVolumeUp() {
    Klick();
    if (Keyb->Chanel->Volume->Get() + Modifier[0] < 127) {
      Keyb->Chanel->Volume->Set(Keyb->Chanel->Volume->Get() + Modifier[0]);
    }
    Keyb->Chanel->Volume->SetExternalValue(NULL);
  }

  //----------------------------------------------------------------------
  // Keyboard Volume Down
  void HKVolumeDown() {
    Klick();
    if (Keyb->Chanel->Volume->Get() - Modifier[0] >= 0) {
      Keyb->Chanel->Volume->Set(Keyb->Chanel->Volume->Get() - Modifier[0]);
    }
    Keyb->Chanel->Volume->SetExternalValue(NULL);
  }

  //----------------------------------------------------------------------
  // Keyboard Volume Down
  void HKSendMIDI() {
    Klick();
    for(unsigned char Index=0; Modifier[Index]!=0 && Index <= MaxHotkeyMidiLen; Index++){
        SendMidi(Modifier[Index]);
    } 
  }




  //----------------------------------------------------------------------
  // Keyboard Load Preset
  void HKLoadPreset(){
      //EEPROM.write(AddressQuickBoot, 1); //quick boot next time
      DebugPrint("Hot key LoadPreset\r\n");      
      EEPROM.write(AddressStartFile, Modifier[0]);
      NeedLoad = 1;
      //ResetFunc();
      /**/
  }
  
  //----------------------------------------------------------------------
  // Keyboard Load Preset
  void HKLoadRandomPreset(){
      EEPROM.write(AddressStartFile, Modifier[0]);
      NeedLoad = 1;
      //ResetFunc();
  }

  void HKFreezeAnalogIn(){
    //MyKeyb->AnalogInputs[Modifier[0]]->Freeze = !MyKeyb->AnalogInputs[Modifier[0]]->Freeze;
  }
  
  
  //----------------------------------------------------------------------
  //
  void ActionWork() {
    if (Action == 1) {
       HKMuteAll();
    }
    else if (Action == 2) {
       HKVelocityUp();
    }
    else if (Action == 3) {
       HKVelocityDown();
    }
    else if (Action == 4) {
       HKToneUp();
    }
    else if (Action == 5) {
       HKToneDown();
    }
    else if (Action == 6) {
       HKVolumeUp();
    }
    else if (Action == 7) {
       HKVolumeDown();
    }
    else if (Action == 8) {
       HKSendMIDI();
    }
    else if (Action == 9) {
       HKLoadPreset();
    }
    else if (Action == 10) {
       HKLoadRandomPreset();
    }
    else if (Action == 11) {
       HKFreezeAnalogIn();
    }
    
  }

  //----------------------------------------------------------------------
  //the Play Hot key
  void PlayKeys(unsigned char PushedKeys[]) { //a function
    // Send Pushed Buttons
    for (int counter = 0; (PushedKeys[counter] != 0) && (counter < MaxKeysOneTime) ; counter++) { // for all pressed keys
      if ((PushedKeys[counter] == Key) && (Cycle == 0)) { //if Key pressed and timeout not exceed
        ActionWork();       // Work
        Cycle = DelayCycle; // Go timeout
        break;
      }
    }
    if (Cycle > 0) { //decrement timeout counter
      Cycle--;
    }
  }
};



#endif
