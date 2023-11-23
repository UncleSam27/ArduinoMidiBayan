#ifndef MIDICHANEL_H
#define MIDICHANEL_H

#include "midi.h"
#include "config.h"

//--------------------------------------------------------------------------------------------------------------
// class DataBox
//--------------------------------------------------------------------------------------------------------------

class DataBox {
  private:
    char Value;
    
  public:
    char * ExternalValue;
    
    DataBox (char initValue);
    char Get();
    void Set(char NewValue);
    bool IsStatic();
    bool IsChanged();
    void SetExternalValue (char *initExternalValue);
}; // class DataBox 



//--------------------------------------------------------------------------------------------------------------
// class MidiController
//--------------------------------------------------------------------------------------------------------------

class MidiController {
  private:
    char*    Chanel;
    unsigned char MSB;
    unsigned char LSB;
    
  public:
    DataBox* Data;
  
    MidiController (char* initExternalValue, char* initChanel,unsigned char initMSB,unsigned char initLSB);
    ~MidiController ();
    void Play();
    unsigned char GetMSB();
    void SetMSB(unsigned char initMSB);
    unsigned char GetLSB();
    void SetLSB(unsigned char initLSB);
}; // class MidiController


//--------------------------------------------------------------------------------------------------------------
// class MidiChanel
//--------------------------------------------------------------------------------------------------------------
class MidiChanel {
  private:
   
  public: //public otherwise error: test::test private
    MidiController* Controllers[MaxMidiControllers];
    
    DataBox* Volume;      
    DataBox* Pressure;
    DataBox* Pitch;
    DataBox* Velocity;

    unsigned char Chanel;
    unsigned char MSB;
    unsigned char LSB;
    unsigned char Prog;

    //----------------------------------------------------------------------
    //the constructor
    MidiChanel( unsigned char initChanel
              , unsigned char initVolume
              , unsigned char initPressure
              , unsigned char initPitch
              , unsigned char initVelocity
              , unsigned char initProg
              , unsigned char initMSB
              , unsigned char initLSB
     );
    ~MidiChanel();
    void SetupChanelProgram(unsigned char ProgramChangeMode);
    void SendNote(char Note);
    void ClearNote(char Note);
    void Play();
    MidiController* AddNewController(char* initExternalValue, unsigned char initMSB, unsigned char initLSB);
    int DeleteMidiController(MidiController* Chan);
    void DeleteAllMidiControllers();
    unsigned int GetMidiControllersCount();
    MidiController* GetMidiControllersByMSB(unsigned char Chan);
    void AllNotesOff();
}; // class MidiController




#endif
