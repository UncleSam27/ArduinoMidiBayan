#include "drummachine.h"
#include "midi.h"


//--------------------------------------------------------------------------------------------------------------
// class Drum
//--------------------------------------------------------------------------------------------------------------

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
  if(NewChanel>0 && NewChanel < 16)
     Note = NewChanel;
}

//----------------------------------------------------------------------
unsigned char Drum::GetChanel(){
    return (Chanel);
}

//----------------------------------------------------------------------
void Drum::SetNote(unsigned char  NewNote){
  if(NewNote>0 && NewNote < 127)
     Note = NewNote;
}

//----------------------------------------------------------------------
unsigned char Drum::GetNote(){
  return (Note);
}
//----------------------------------------------------------------------
void Drum::SetVelocity(unsigned char  NewVelocity){
  if(NewVelocity>0 && NewVelocity < 127)
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
  DrumDelay = DefaultDrumDelay;
  DrumCounter = 0;
  CurrentTick = NumberOfTicks-1; // in next tick Drums start in 0 tick
  
  for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
    Drums[Counter].Enable(false);
    Drums[Counter].SetChanel(0);
    Drums[Counter].SetNote(0);
    Drums[Counter].SetVelocity(0);
  }

  for(unsigned char Counter=0; Counter<NumberOfTicks; Counter++){
    DrumCode[Counter] = 0;
  }
}

//----------------------------------------------------------------------
void DrumMachine::EnableMachine(bool Enable){
  Enabled = Enable;
  if(!Enabled){ //disable all notes
    for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
      MidiClearNote( Drums[Counter].GetChanel(), Drums[Counter].GetNote(), 0 );
    }
  }
}

//----------------------------------------------------------------------
Drum* DrumMachine::GetDrum(unsigned char DrumNumber){
  if(DrumNumber>0 && DrumNumber < NumberOfDrums)
    return ( &Drums[DrumNumber] );
}

//----------------------------------------------------------------------
void DrumMachine::SetDrumDelay(unsigned char NewDrumDelay){
   DrumDelay = NewDrumDelay;
}

//----------------------------------------------------------------------
unsigned char DrumMachine::GetDrumDelay(){
  return ( DrumDelay );
}

//----------------------------------------------------------------------
void DrumMachine::SetDrumCode(unsigned char TickNum, unsigned char Code){
  if(TickNum > 0 && TickNum < NumberOfTicks)
    DrumCode[TickNum] = Code;
}

//----------------------------------------------------------------------
unsigned char DrumMachine::GetDrumCode(unsigned char TickNum){
  if(TickNum > 0 && TickNum < NumberOfTicks)
    return ( DrumCode[TickNum] );
}

void DrumMachine::PlayImmediately(){
  unsigned char Tick;

  // Turn off all notes sounding in the previous measure
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter<NumberOfDrums; DrumCounter++){ 
    if(Tick & 1){
      MidiClearNote(Drums[DrumCounter].GetChanel(), Drums[DrumCounter].GetNote(), 0);
    }
    Tick >>= 1;
  }
    // Increment Ticks
    CurrentTick++;
    if(CurrentTick >= NumberOfTicks){
      CurrentTick = 0;
    }
    
  // Turn on all sounding in this tick notes
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    if(Tick & 1){
      MidiSendNote(Drums[DrumCounter].GetChanel(), Drums[DrumCounter].GetNote(), Drums[DrumCounter].GetVelocity());
    }
    Tick >>= 1;
  }    
}

//----------------------------------------------------------------------
void DrumMachine::Play(){
  if( Enabled ){
    DrumCounter++;
    if(DrumCounter > DrumDelay){
      DrumCounter=0;
      PlayImmediately();
    }
  }
}
