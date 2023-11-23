
#ifndef ANALOG_H
#define ANALOG_H

#include "config.h"

#define AnalogCycle 5  //real work evry 5 cycle 
#define AnalogPoints 9  //kurves sets by 9 points

//--------------------------------------------------------------------------------------------------------------
// class AnalogIn
//--------------------------------------------------------------------------------------------------------------

class AnalogIn {
  private:
    bool InternalADC;
    unsigned char  DelayCycle;
    int Cycle;
    int RawInputVal;    
  public:
    unsigned char  Boost;
    unsigned char  CentralBoost; 
    bool Freeze  = false;
    int Correction =0;  
    unsigned char  Value;  
    unsigned char  Input;
    unsigned char  Mode[AnalogPoints]={0x01,0x20,0x40,0x60,0x80,0xA0,0xC0,0xE0,0xFF};

    AnalogIn(unsigned char  initInput,  unsigned char initMode[], int initCorrection, unsigned char initBoost, unsigned char initCentralBoost);
    unsigned char RawToData(int NewVal);
    char GetValue();
    unsigned int GetRawInputVal();
    int *SetExternalRawInputVal();
    void SetRealRawInputVal();
    void FreezeVal();
    void UnFreezeVal();
    void Play();
};


#endif
