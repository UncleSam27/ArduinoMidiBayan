#ifndef KEYBOARDS_H
#define KEYBOARDS_H

#include <stdlib.h>
#include "config.h"
#include "midi.h"
#include "strings.h"
#include "keyboard.h"
#include "hotkey.h"
#include "analog.h"
#include "midichanel.h"

//--------------------------------------------------------------------------------------------------------------
// KeyboardS
//--------------------------------------------------------------------------------------------------------------
class KeyboardS {
  private:
    
  public: //public 
    //varibles
    unsigned char  ScanMode;           // 0-scan in passive mode (gerkon switches)    1-scan in active mode (hall sensor)
    unsigned char  Normal_Pin_State;  // 0-"normal opened contacts"  1-"normal closed contacts"  2-"auto detect"
    unsigned char  KeyInputPinStart;  
    unsigned char  KeyInputPinStop;
    unsigned char  KeyOutputPinStart;
    unsigned char  KeyOutputPinStop;

    unsigned char  ScanCodeNums;
    unsigned char  KeyCodes[KeyCodesCount];
    
    Keyboard* Keyboards[MaxKeyboards];
    Hotkey* Hotkeys[MaxHotkeys];
    AnalogIn* AnalogInputs[MaxAnalogInputs];    
    MidiChanel* MidiChanels[MaxMidiChanels];
    unsigned int InputKeysMask[MaxOutCount];

    //functions
    KeyboardS();
    ~KeyboardS();
    int ScanCodeToKeyCode(unsigned char ScanCode);
    inline void Pin2Low(unsigned char Pin);
    inline void Pin2High(unsigned char Pin);
    inline void Pin2HiZ(unsigned char Pin);
    void ScanKeyMatrixLow(unsigned int Code[]);
    char ScanKeyMatrix(unsigned char Keys[]);
    unsigned char GetKey();
    void Play();
    MidiChanel* AddNewMidiChanel( unsigned char Chanel
                                , unsigned char Volume
                                , unsigned char Pressure
                                , unsigned char Pitch
                                , unsigned char Velocity
                                , unsigned char Prog
                                , unsigned char MSB
                                , unsigned char LSB
                );
    int DeleteMidiChanel(MidiChanel* Chan);
    void DeleteAllMidiChanel();
    unsigned int GetMidiChanelCount();
    MidiChanel* GetMidiChanelByNum(unsigned char Chan);
    void SetupAllPrograms(unsigned char ProgramChangeMode);
    
    Keyboard* AddNewKeyboard( char Name[]
                            , unsigned char FierstKey
                            , unsigned char LastKey
                            , MidiChanel* Chanel
                            , unsigned char MidiBaseNote
                            , unsigned char  Increment 
                );
    int DeleteKeyboard(Keyboard* Keyb);
    void DeleteAllKeyboard();
     unsigned int GetKeyboardsCount();
    int GetKeyboardIndex(Keyboard* Keyb);
    Keyboard* GetKeyboardPtrByName(char Name[]);
    Keyboard* GetKeyboardPtr(int Index);
    
    Hotkey* AddNewHotkey(char Name[], Keyboard * Keyb,  unsigned char Key,  int Modifier, unsigned char Action);
    int DeleteHotkey(Hotkey* Key);
    void DeleteAllHotkeys();
    unsigned int GetHotkeyCount();
    
    AnalogIn* AddNewAnalogIn(unsigned char  AnalogInputPin,  unsigned char Mode[], int Correction, unsigned char Boost, unsigned char CentralBoost);
    int DeleteAnalogIn(AnalogIn* AnalogInput);
    void DeleteAllAnalogIn();
    unsigned int GetAnalogInCount();
    unsigned int GetAnalogInIndex(AnalogIn* Input);
    unsigned int GetAnalogInByInput(uint8_t Input);
    unsigned int GetAnalogInByValAddr(unsigned char * Input);
    int GetBoxMenuValue(DataBox * Value);
    
}; // class KeyboardS



// Глобальный объект, ну такое... возможно надо переделать
extern KeyboardS  *MyKeyb;


KeyboardS *InitKeyboard(KeyboardS *Keybs);

#endif
