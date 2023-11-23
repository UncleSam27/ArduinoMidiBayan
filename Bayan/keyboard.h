#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "midichanel.h"

//--------------------------------------------------------------------------------------------------------------
// class Keyboard
//--------------------------------------------------------------------------------------------------------------
class Keyboard {
  private:
    unsigned char SendedKeys[MaxKeysOneTime + 1];

  public: //public otherwise error: test::test private
    MidiChanel*   Chanel;
    char          Name[MaxNameLen + 1];
    unsigned char KeyBegin; // Fierst Button
    unsigned char KeyEnd; // Last Button
    unsigned char MidiBaseNote;
    unsigned char Increment;
    unsigned char ReNew; // Flag of external Renew

    //----------------------------------------------------------------------
    //the constructor
    Keyboard(char initName[]
           , unsigned char initKeyBegin
           , unsigned char initKeyEnd
           , MidiChanel * initMidiChanel
           , unsigned char initMidiBaseNote
           , unsigned char  initIncrement);
    bool KeyIsSended(unsigned char Key);
    bool KeyIsReleased(unsigned char Key, unsigned char PushedKeys[]);
    bool AddKeyToSendedKeys(unsigned char Key);
    bool RemoveKeyFromSendedKeys(unsigned char Key);
    unsigned char GetNoteFromKey(unsigned char Key);
    void PlayButtons(unsigned char PushedKeys[]);
    void PlayButtonsChords(unsigned char PushedKeys[]);
    void Play(unsigned char PushedKeys[]);
    
}; // class Keyboard 



#endif
