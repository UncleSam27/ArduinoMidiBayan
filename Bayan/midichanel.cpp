#include <arduino.h>
#include "midichanel.h"
#include "debug.h"

//--------------------------------------------------------------------------------------------------------------
// class DataBox
//--------------------------------------------------------------------------------------------------------------

//the constructor
DataBox::DataBox (char initValue){
    ExternalValue=&Value;
    Value = initValue;
}

//Get current value
char DataBox::Get(){
  Value  = *ExternalValue;
  return *ExternalValue;
}

// Set current value
void DataBox::Set(char NewValue){
  if(&Value == ExternalValue){ //Set Value and setup static
    Value = NewValue;
    ExternalValue = &Value;
  }
}

// Get is element use static value
bool DataBox::IsStatic(){
  if (&Value == ExternalValue){
    return true;    //Static
  }else {
    return false;   //External
  }
}

// Get is element changed
bool DataBox::IsChanged(){
  if (*ExternalValue != Value){  // if element changed return true
    return true;
  }else {
    return false;
  }
}

// Use Analog input or any else external data source
void DataBox::SetExternalValue (char *initExternalValue){
  if(initExternalValue ==NULL){           //NULL - Setup static Value
    ExternalValue = &Value;
  }else{                                  //NotNULL - Use Dynamic
    ExternalValue = initExternalValue;  
    Value = *ExternalValue;        
  }
}


// end class DataBox 



//--------------------------------------------------------------------------------------------------------------
// class MidiController
//--------------------------------------------------------------------------------------------------------------

MidiController::MidiController (char* initExternalValue, char* initChanel,unsigned char initMSB,unsigned char initLSB){
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


// destructor
MidiController::~MidiController (){
  delete Data;
}

// Play Note
void MidiController::Play(){
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
unsigned char MidiController::GetMSB(){
  return MSB;
}

//----------------------------------------------------------------------
void MidiController::SetMSB(unsigned char initMSB){
  MSB=initMSB & 0x7F;
}


//----------------------------------------------------------------------
unsigned char MidiController::GetLSB(){
  return LSB;
}

//----------------------------------------------------------------------
void MidiController::SetLSB(unsigned char initLSB){
  LSB=initLSB & 0x7F;
}

// end class MidiController




//--------------------------------------------------------------------------------------------------------------
// class MidiChanel
//--------------------------------------------------------------------------------------------------------------

//the constructor
MidiChanel::MidiChanel(unsigned char initChanel,unsigned char initVolume, unsigned char initPressure, unsigned char initPitch, unsigned char initVelocity, unsigned char initProg, unsigned char initMSB, unsigned char initLSB) {
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
MidiChanel::~MidiChanel(){
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
void MidiChanel::SetupChanelProgram(unsigned char ProgramChangeMode) {

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
void MidiChanel::SendNote(char Note){
  MidiSendNote(Chanel, Note, Velocity->Get());
}

//----------------------------------------------------------------------
// ClearNote (used in keyboard)
void MidiChanel::ClearNote(char Note){
  MidiClearNote(Chanel, Note, Velocity->Get());
}

void MidiChanel::AllNotesOff(){
  MidiAllSoundsOff(Chanel);
}


//----------------------------------------------------------------------
// Play (Used in main cycle)
void MidiChanel::Play(){
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
MidiController* MidiChanel::AddNewController(char* initExternalValue, unsigned char initMSB, unsigned char initLSB) {
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
int MidiChanel::DeleteMidiController(MidiController* Chan) {
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
void MidiChanel::DeleteAllMidiControllers() {
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
 unsigned int MidiChanel::GetMidiControllersCount() {
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
MidiController* MidiChanel::GetMidiControllersByMSB(unsigned char Chan) {
  for (int Counter = 0; Counter < MaxMidiControllers; Counter++) {
    if (Controllers[Counter] !=NULL) {
      if(Controllers[Counter]->GetMSB() == Chan)
        return Controllers[Counter];
    }
  }
  return NULL;
}

// end MidiChanel class
