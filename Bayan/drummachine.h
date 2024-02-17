#ifndef DRUMMACHINE_H
#define DRUMMACHINE_H

#include "config.h"

#define NumberOfDrums 8
#define NumberOfTicks 16
#define DefaultBPM 60  //real work evry 1000 cycle 

//--------------------------------------------------------------------------------------------------------------
// class Drum
//--------------------------------------------------------------------------------------------------------------

class Drum {
  private:
    bool  Enabled;
    unsigned char  Chanel;
    unsigned char  Note;
    unsigned char  Velocity;  
  public:

    Drum();
    
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
    class Drum     Drums[NumberOfDrums];
    unsigned char  DrumCode[NumberOfTicks]; 
    unsigned long  CurrentTick;      // текущий тик драм машины
    
    unsigned long  DrumDelay;        // задержка в микросекундах между тиками драм машины
    unsigned long  NextDrumCounter;  // следующий тик драммашины

    bool           Enabled;
    bool           SynchroStart;
    unsigned int   BPM;

  public:
    DrumMachine();
    void Enable(bool Enable);
    bool IsEnabled();
    Drum* GetDrum(unsigned char DrumNumber);
    void SetSynchroStart(bool NewSynchroStart);
    bool GetSynchroStart(); 
    void GoSynchroStart();
    void SetDrumCode(unsigned char TickNum,unsigned char Code);
    unsigned char GetDrumCode(unsigned char TickNum);
    bool IsPlayed(unsigned char TickNum, unsigned char DrumNum);
    void SetPlayed(unsigned char TickNum, unsigned char DrumNum, bool Enabled);
    unsigned char GetBPM();
    void SetBPM(unsigned char NewBPM);
    void PlayImmediately();
    void Play();
};

char SaveDrumMachine(char FileName[], class DrumMachine* DrumMach);
char LoadDrumMachine(char FileName[], class DrumMachine* DrumMach);

#endif
