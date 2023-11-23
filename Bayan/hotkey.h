#ifndef HOTKEY_H
#define HOTKEY_H

#include <string.h>
#include "config.h"
#include "keyboard.h"
#include "BMP280.h"

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
    Hotkey(char initName[], Keyboard * initKeyb,  unsigned char initKey, unsigned char initModifier[], unsigned char  initAction);
    void HKMuteAll();
    void HKVelocityUp();
    void HKVelocityDown();
    void HKToneUp();
    void HKToneDown();
    void HKVolumeUp();
    void HKVolumeDown();
    void HKSendMIDI();
    void HKLoadPreset();
    void HKLoadRandomPreset();
    void HKFreezeAnalogIn();
    void ActionWork();
    void PlayKeys(unsigned char PushedKeys[]);
};



#endif
