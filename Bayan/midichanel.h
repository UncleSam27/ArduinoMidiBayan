#ifndef MIDICHANEL_H
#define MIDICHANEL_H

#include "midi.h"



//--------------------------------------------------------------------------------------------------------------
// class DataBox
//--------------------------------------------------------------------------------------------------------------

class DataBox {
  private:
    char Value;
  public:
    char * ExternalValue;
        
  //----------------------------------------------------------------------
  //the constructor
  DataBox (char initValue){
      ExternalValue=&Value;
      Value = initValue;
  }

  //Get current value
  char Get(){
    Value  = *ExternalValue;
    return *ExternalValue;
  }

  // Set current value
  void Set(char NewValue){
    if(&Value == ExternalValue){ //Set Value and setup static
      Value = NewValue;
      ExternalValue = &Value;
    }
  }

  // Get is element use static value
  bool IsStatic(){
    if (&Value == ExternalValue){
      return true;    //Static
    }else {
      return false;   //External
    }
  }
  
  // Get is element changed
  bool IsChanged(){
    if (*ExternalValue != Value){  // if element changed return true
      return true;
    }else {
      return false;
    }
  }

  //----------------------------------------------------------------------
  // Use Analog input or any else external data source
  void SetExternalValue (char *initExternalValue){
    if(initExternalValue ==NULL){           //NULL - Setup static Value
      ExternalValue = &Value;
    }else{                                  //NotNULL - Use Dynamic
      ExternalValue = initExternalValue;  
      Value = *ExternalValue;        
    }
  }
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
  
  //----------------------------------------------------------------------
  //the constructor with LSB
  MidiController (char* initExternalValue, char* initChanel,unsigned char initMSB,unsigned char initLSB){
     Chanel = initChanel;
     Data = new DataBox(0);
     Data->SetExternalValue(initExternalValue);
     MSB = initMSB & 0x7F;
     LSB = initLSB & 0x7F;

     //LSB dont used yet! only 7bit analog values
     //LSB value allways = 0!
     if(initLSB != 0){
       MidiControlChange(*Chanel, LSB, 0);
     }
  }


  //----------------------------------------------------------------------
  // destructor
  ~MidiController (){
    delete Data;
  }

  //----------------------------------------------------------------------
  // Play Note
  void Play(){
    if (Data->IsChanged()){      // if Data is changed
       // Send new Controller value
       MidiControlChange(*Chanel, MSB, Data->Get());

       //LSB dont used yet! only 7bit values
       //if(LSB!=0){
       //       MidiControlChange(*Chanel, LSB, Data->Get()); 
       //}
    }
  }

  //----------------------------------------------------------------------
  unsigned char GetMSB(){
    return MSB;
  }

  //----------------------------------------------------------------------
  void SetMSB(unsigned char initMSB){
    MSB=initMSB & 0x7F;
  }


  //----------------------------------------------------------------------
  unsigned char GetLSB(){
    return LSB;
  }

  //----------------------------------------------------------------------
  void SetLSB(unsigned char initLSB){
    LSB=initLSB & 0x7F;
  }


  
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
    MidiChanel(unsigned char initChanel,unsigned char initVolume, unsigned char initPressure, unsigned char initPitch, unsigned char initVelocity, unsigned char initProg, unsigned char initMSB, unsigned char initLSB) {
      Chanel = initChanel;

      Volume = new DataBox(initVolume);
      Pressure = new DataBox(initPressure);
      Pitch = new DataBox(initPitch);
      Velocity = new DataBox(initVelocity);
      
      Prog=initProg;
      MSB=initMSB;
      LSB=initLSB;

      // Clear MidiControllers
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        Controllers[Counter] = NULL;
      }      

    }

    //----------------------------------------------------------------------
    //the destructor
    ~MidiChanel(){
      delete Volume;      
      delete Pressure;
      delete Pitch;
      delete Velocity;
      // Delete All MidiControllers
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        if(Controllers[Counter] != NULL){
          delete Controllers[Counter];
          Controllers[Counter] = NULL;
        }
      }  
    }

    //----------------------------------------------------------------------
    // Send in midi device  program change
    void SetupChanelProgram(unsigned char ProgramChangeMode) {

      MidiSendVolume(Chanel, Volume->Get());
      MidiSendPressure(Chanel, Pressure->Get());  
      MidiSendPitch(Chanel, Pitch->Get());

      // old style just send program change
      if (ProgramChangeMode == 1) {
        MidiProgramChange(Chanel, Prog);
      }
      //New Style Change  - select bank and instrument before send program change
      else if (ProgramChangeMode == 2) {
        MidiSetBankMSB   (Chanel, MSB);
        MidiSetBankLSB   (Chanel, LSB);
        MidiProgramChange(Chanel, Prog);
      }else{
        MidiSetBankMSB   (Chanel, MSB);
        MidiSetBankLSB   (Chanel, LSB);
        MidiProgramChange(Chanel, Prog);
        DebugPrint("Error: ivalid ProgramChangeMode in eeprom: ");
        DebugPrintLn(ProgramChangeMode);
      }
    }


    //----------------------------------------------------------------------
    // PlayNote (used in keyboard)
    void SendNote(char Note){
      MidiSendNote(Chanel, Note, Velocity->Get());
    }

    //----------------------------------------------------------------------
    // ClearNote (used in keyboard)
    void ClearNote(char Note){
      MidiClearNote(Chanel, Note, Velocity->Get());
    }

    void AllNotesOff(){
      MidiAllSoundsOff(Chanel);
    }

    
    //----------------------------------------------------------------------
    // Play (Used in main cycle)
    void Play(){
       if (Volume->IsChanged()){      // if Volume is changed
         MidiSendVolume(Chanel, Volume->Get() );  // Send new volume
       }
       
       if(Pressure->IsChanged()){
         MidiSendPressure(Chanel,Pressure->Get());       
       }
       
       if(Pitch->IsChanged()){
         MidiSendPitch(Chanel, Pitch->Get());
       }
       
       for (int Counter = 0; Counter < MaxMidiControllers ; Counter++) {
         if (Controllers[Counter] != NULL) {
           Controllers[Counter]->Play();
         }
       }
    }

    //----------------------------------------------------------------------
    // Add New Controller
    MidiController* AddNewController(char* initExternalValue, unsigned char initMSB, unsigned char initLSB) {
      //Search Fierst Empty Slot
      for (int Counter = 0; Counter < MaxMidiControllers ; Counter++) {
        if (Controllers[Counter] == NULL) {  // If founded empty slot - add new keyboard
          Controllers[Counter] = new MidiController (initExternalValue, &Chanel, initMSB, initLSB);
          return Controllers[Counter];
        }
      }
      return NULL;
    }

    //----------------------------------------------------------------------
    // Delete MidiController
    int DeleteMidiController(MidiController* Chan) {
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        // If founded delete from array
        if (Controllers[Counter] == Chan) {
          delete Chan; //Free memory
          Controllers[Counter]=NULL;
          //shift data in array
          while (Counter < MaxMidiControllers - 1) {
            Controllers[Counter] = Controllers[Counter + 1];
            Counter++;
          }
          Controllers[MaxMidiControllers - 1] = NULL;
          return 0;
        }
      }
      return -1;
    }

    //----------------------------------------------------------------------
    // Delete All MidiController
    void DeleteAllMidiControllers() {
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        // If founded delete from array
        if (Controllers[Counter] != NULL) {
          delete Controllers[Counter];
          Controllers[Counter] = NULL;
        }
      }
    }


    //----------------------------------------------------------------------
    // GetMidiControllersCounts
     unsigned int GetMidiControllersCount() {
      unsigned int Count=0;
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        if (Controllers[Counter] != NULL) { //if keyboard not empty
          Count++;
        }
      }
      return Count;
    }


    //----------------------------------------------------------------------
    // Get the index of Keyboard in array
    MidiController* GetMidiControllersByMSB(unsigned char Chan) {
      for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
        if (Controllers[Counter] !=NULL) {
          if(Controllers[Counter]->GetMSB() == Chan)
            return Controllers[Counter];
        }
      }
      return NULL;
    }

};




#endif
