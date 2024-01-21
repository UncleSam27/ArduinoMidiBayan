#ifndef DRUMMACHINE_H
#define DRUMMACHINE_H

#include "config.h"

#define NumberOfDrums 8
#define NumberOfTicks 16
#define DefaultDrumDelay 1000  //real work evry 1000 cycle 

//--------------------------------------------------------------------------------------------------------------
// class Drum
//--------------------------------------------------------------------------------------------------------------

class Drum {
  private:
    unsigned char  Enabled;
    unsigned char  Chanel;
    unsigned char  Note;
    unsigned char  Velocity;  
  public:
    void Enable(bool Enable);
    bool IsEnabled();

    void SetChanel(unsigned char  NewChanel);
    unsigned char GetChanel();

    void SetNote(unsigned char  NewNote);
    unsigned char GetNote();
    
    void SetVelocity(unsigned char  NewVelocity);
    unsigned char GetVelocity();    
};



//--------------------------------------------------------------------------------------------------------------
// class DrumMachine
//--------------------------------------------------------------------------------------------------------------
class DrumMachine {
  private:
    Drum           Drums[NumberOfDrums];
    unsigned char  DrumCode[NumberOfTicks]; 
    unsigned int   DrumDelay;
    unsigned int   DrumCounter;
    unsigned int   CurrentTick;
    bool           Enabled;

  public:
    DrumMachine();
    void EnableMachine(bool Enable);
    Drum* GetDrum(unsigned char DrumNumber);
    void SetDrumDelay(unsigned char NewDrumDelay);
    unsigned char GetDrumDelay();
    void SetDrumCode(unsigned char TickNum,unsigned char Code);
    unsigned char GetDrumCode(unsigned char TickNum);
    void PlayImmediately();
    void Play();
};

#endif
