#include "drummachine.h"
#include "midi.h"
#include <SD.h>


//--------------------------------------------------------------------------------------------------------------
// class Drum
//--------------------------------------------------------------------------------------------------------------

Drum::Drum(){
    Enabled = false;
    Chanel = 9;
    Note =   80;
    Velocity = 127;  
}

//----------------------------------------------------------------------
void Drum::Enable(bool NewEnabled){
  Enabled = NewEnabled;
}

//----------------------------------------------------------------------
bool Drum::IsEnabled(){
     return (Enabled); 
}

//----------------------------------------------------------------------
void Drum::SetChanel(unsigned char  NewChanel){
  if(NewChanel>=0 && NewChanel <= 15)
     Chanel = NewChanel;
}

//----------------------------------------------------------------------
unsigned char Drum::GetChanel(){
    return (Chanel);
}

//----------------------------------------------------------------------
void Drum::SetNote(unsigned char  NewNote){
  if(NewNote>=0 && NewNote <= 127)
     Note = NewNote;
}

//----------------------------------------------------------------------
unsigned char Drum::GetNote(){
  return (Note);
}
//----------------------------------------------------------------------
void Drum::SetVelocity(unsigned char  NewVelocity){
  if(NewVelocity>=0 && NewVelocity <= 127)
     Velocity = NewVelocity;
}

//----------------------------------------------------------------------
unsigned char Drum::GetVelocity(){
  return (Velocity);
}




//--------------------------------------------------------------------------------------------------------------
// class DrumMachine
//--------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------
DrumMachine::DrumMachine(){
  Enabled = false;
  CurrentTick = NumberOfTicks-1; // in next tick Drums start in 0 tick
  BPM = DefaultBPM;
  DrumDelay = (1000000/DefaultBPM)<<4;
  NextDrumCounter = 0;


  for(unsigned char Counter=0; Counter<NumberOfTicks; Counter++){
    DrumCode[Counter] = 0;
  }

  // test drum code
  for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
    Drums[Counter]= new(Drum);
    if(Counter<4) Drums[Counter]->Enable(true);
  }


  Drums[0]->SetChanel(9);
  Drums[0]->SetNote(45);
  Drums[0]->SetVelocity(125);

  Drums[1]->SetChanel(9);
  Drums[1]->SetNote(53);
  Drums[1]->SetVelocity(125);

    
  SetPlayed(0, 0, true);
  SetPlayed(4, 0, true);
  SetPlayed(8, 0, true);
  SetPlayed(12, 0, true);

  SetPlayed(1, 1, true);
  SetPlayed(2, 1, true);
  SetPlayed(5, 1, true);
  SetPlayed(6, 1, true);
  SetPlayed(9, 1, true);
  SetPlayed(10, 1, true);
  SetPlayed(13, 1, true);
  SetPlayed(14, 1, true);
  
  
 /* for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
    Drums[Counter].Enable(false);
    Drums[Counter].SetChanel(0);
    Drums[Counter].SetNote(0);
    Drums[Counter].SetVelocity(0);
  }
  */


}

//----------------------------------------------------------------------
void DrumMachine::Enable(bool Enable){
  Enabled = Enable;
  if(!Enabled){ //disable all notes
    for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
      MidiClearNote( Drums[Counter]->GetChanel(), Drums[Counter]->GetNote(), 0 );
    }
  }
}


//----------------------------------------------------------------------
bool DrumMachine::IsEnabled(){
  return (Enabled);
}

//----------------------------------------------------------------------
Drum* DrumMachine::GetDrum(unsigned char DrumNumber){
  if(DrumNumber>=0 && DrumNumber < NumberOfDrums)
    return ( Drums[DrumNumber] );
}

//----------------------------------------------------------------------
void DrumMachine::SetDrumCode(unsigned char TickNum, unsigned char Code){
  if(TickNum >= 0 && TickNum < NumberOfTicks)
    DrumCode[TickNum] = Code;
}

//----------------------------------------------------------------------
unsigned char DrumMachine::GetDrumCode(unsigned char TickNum){
  if(TickNum >= 0 && TickNum < NumberOfTicks)
    return ( DrumCode[TickNum] );
}

bool DrumMachine::IsPlayed(unsigned char TickNum, unsigned char DrumNum){
  unsigned char Tick =   DrumCode[TickNum];
  Tick >>= DrumNum;
  Tick &=  1;
  return Tick;
}

void DrumMachine::SetPlayed(unsigned char TickNum, unsigned char DrumNum, bool Enabled){
  unsigned char temp = 1 << DrumNum;
  if(Enabled){
    DrumCode[TickNum] |=  temp;
  }else{
    temp = ~temp;
    DrumCode[TickNum] &=  temp;
  }
}


unsigned char DrumMachine::GetBPM(){
return BPM;
}

void DrumMachine::SetBPM(unsigned char NewBPM){
  BPM=NewBPM;
  DrumDelay = (1000000/NewBPM)<<4;
}

void DrumMachine::PlayImmediately(){
  unsigned char Tick;

  // Turn off all notes sounding in the previous measure
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    if((Tick & 1) && Drums[DrumCounter]->IsEnabled() ){
      MidiClearNote(Drums[DrumCounter]->GetChanel(), Drums[DrumCounter]->GetNote(), 0);
    }
    Tick >>= 1;
  }
  
  // Increment Ticks
  CurrentTick++;
  if(CurrentTick >= NumberOfTicks) CurrentTick = 0;
    
  // Turn on all sounding in this tick notes
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    if((Tick & 1) && Drums[DrumCounter]->IsEnabled() ){
      MidiSendNote(Drums[DrumCounter]->GetChanel(), Drums[DrumCounter]->GetNote(), Drums[DrumCounter]->GetVelocity());
    }
    Tick >>= 1;
  }    
}

//----------------------------------------------------------------------
void DrumMachine::Play(){
  if( Enabled ){
    unsigned long TimeNow = micros();          // get current time in microseconds
    if(TimeNow > NextDrumCounter){             // if now time to work
      NextDrumCounter += DrumDelay;            // calculate new worktime
      if(abs(TimeNow - NextDrumCounter) > (DrumDelay<<1) )  //owerflow protect if error more then 2x DrumDelay
        NextDrumCounter = TimeNow + DrumDelay;              // renew NextDrumCounter

      PlayImmediately();                // play bit
    }
  }
}


void SaveDrumMachine(char FileName[], class DrumMachine* DrumMach){
  File file;
  SD.remove(FileName);//remove if exist
  file = SD.open(FileName, FILE_WRITE);
  file.write((uint8_t*)DrumMach, sizeof(DrumMachine) );
  file.close();
}


void LoadDrumMachine(char FileName[], class DrumMachine* DrumMach){
  File file;
  file = SD.open(FileName, FILE_READ);
  file.read((uint8_t*)DrumMach, sizeof(DrumMachine) );
  file.close();
}
