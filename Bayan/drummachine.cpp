#include "drummachine.h"
#include "midi.h"
#include "keybloadsave.h"
#include <SD.h>
#include <EEPROM.h>


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
  MaxTick = NumberOfTicks;
  CurrentTick = MaxTick -1; // in next tick Drums start in 0 tick
  BPM = DefaultBPM;
  DrumDelay = (1000000/DefaultBPM)<<4;
  NextDrumCounter = 0;
  SynchroStart = false;

  for(unsigned char Counter=0; Counter<NumberOfTicks; Counter++){
    DrumCode[Counter] = 0;
  }
}

//----------------------------------------------------------------------
void DrumMachine::Enable(bool Enable){
  Enabled = Enable;
  if(!Enabled){ //disable all notes
    for(unsigned char Counter=0; Counter<NumberOfDrums; Counter++){
      MidiClearNote( Drums[Counter].GetChanel(), Drums[Counter].GetNote(), 0 );
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
    return ( &Drums[DrumNumber] );
}

//----------------------------------------------------------------------
void DrumMachine::SetSynchroStart(bool NewSynchroStart){
  SynchroStart = NewSynchroStart;
}

//----------------------------------------------------------------------
bool DrumMachine::GetSynchroStart(){
  return SynchroStart;
}

//----------------------------------------------------------------------
void DrumMachine::GoSynchroStart(){
  if(SynchroStart){
    CurrentTick = MaxTick-1;
    NextDrumCounter = micros();
    Enabled = true;
  }
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


unsigned char DrumMachine::GetMaxTick(){
  return MaxTick;  
}

void DrumMachine::SetMaxTick(unsigned char NewMaxTick){
  if(NewMaxTick == NumberOfValsTicks)
    MaxTick = NewMaxTick;
  else 
    MaxTick = NumberOfTicks;
}

void DrumMachine::PlayImmediately(){
  unsigned char Tick;

  // Turn off all notes sounding in the previous measure
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    if((Tick & 1) && Drums[DrumCounter].IsEnabled() ){
      MidiClearNote(Drums[DrumCounter].GetChanel(), Drums[DrumCounter].GetNote(), 0);
    }
    Tick >>= 1;
  }
  
  // Increment Ticks
  CurrentTick++;
  if(CurrentTick >= MaxTick) CurrentTick = 0;
    
  // Turn on all sounding in this tick notes
  Tick = DrumCode[CurrentTick];
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    if((Tick & 1) && Drums[DrumCounter].IsEnabled() ){
      MidiSendNote(Drums[DrumCounter].GetChanel(), Drums[DrumCounter].GetNote(), Drums[DrumCounter].GetVelocity());
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



////////////////////////////////////////////////////////////////////////////////////////
char DrumMachine::SaveDrumMachine(char FileName[]){
  File file;
  SD.remove(FileName);//remove if exist
  file = SD.open(FileName, FILE_WRITE);
  if(!file){
    return -1;
  }

  // if the file opened okay, write to it:
  file.println("[Main]");          
  SaveNameValueToSD(&file,"BPM",BPM);
  SaveNameValueToSD(&file,"MaxTick",MaxTick);
  SaveNameValueToSD(&file,"SynchroStart", int(SynchroStart));  
  file.println();

  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
     file.println("[Drum]");
     SaveNameValueToSD(&file,"Num",      DrumCounter);
     SaveNameValueToSD(&file,"Enable",   (unsigned char)Drums[DrumCounter].IsEnabled());     
     SaveNameValueToSD(&file,"Chanel",   Drums[DrumCounter].GetChanel());
     SaveNameValueToSD(&file,"Note",     Drums[DrumCounter].GetNote());
     SaveNameValueToSD(&file,"Velocity", Drums[DrumCounter].GetVelocity());
     file.println();
  }

  file.println("[DrumCode]");
  for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
    for(unsigned char TickCounter=0; TickCounter < NumberOfTicks; TickCounter++){
      if(IsPlayed(TickCounter, DrumCounter))
        file.print("1");
      else
        file.print("0");
    }
    file.println();
  }
  file.println();
 
  file.close();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
char DrumMachine::LoadDrumMachine(char FileName[]){
  File myFile;
  char   CharBuff[MaxFileStringLen];
  
  myFile = SD.open(FileName, FILE_READ);
  if(!myFile){
    return -1;
  }

  Enable(false);
  SetSynchroStart(false);
  
  while (ReadStringFromSD(&myFile,CharBuff) != -1) {
    //Analog Input
    if(strcmp(CharBuff,"[Main]")==0){
      while(ReadStringFromSD(&myFile,CharBuff) >0){
        if(ChkCfgStr("BPM",CharBuff)){
          SetBPM( GetIntCfgStr(CharBuff) );
        }
        if(ChkCfgStr("MaxTick",CharBuff)){
          SetMaxTick( GetIntCfgStr(CharBuff));
        }
        if(ChkCfgStr("SynchroStart",CharBuff)){
          SetSynchroStart( (bool) GetIntCfgStr(CharBuff) );
        }
      }
      continue;
    }

    if(strcmp(CharBuff,"[Drum]")==0){
      unsigned char Num=0;
      while(ReadStringFromSD(&myFile,CharBuff) >0){
        if(ChkCfgStr("Num",CharBuff)){
          Num = GetIntCfgStr(CharBuff);
        }
        if(ChkCfgStr("Enable",CharBuff)){
          Drums[Num].Enable( (bool) GetIntCfgStr(CharBuff) );
        }
        if(ChkCfgStr("Chanel",CharBuff)){
          Drums[Num].SetChanel( GetIntCfgStr(CharBuff) );
        }
        if(ChkCfgStr("Note",CharBuff)){
          Drums[Num].SetNote( GetIntCfgStr(CharBuff) );
        }
        if(ChkCfgStr("Velocity",CharBuff)){
          Drums[Num].SetVelocity( GetIntCfgStr(CharBuff) );
        }
      }
      continue;
    }

    if(strcmp(CharBuff,"[DrumCode]")==0){
      for(unsigned char DrumCounter = 0; DrumCounter < NumberOfDrums; DrumCounter++){ 
        if(ReadStringFromSD(&myFile,CharBuff) >0){
          for(unsigned char TickCounter=0; TickCounter < NumberOfTicks; TickCounter++){
            if ( CharBuff[TickCounter] == '0' )
              SetPlayed(TickCounter, DrumCounter, false);
            else
              SetPlayed(TickCounter, DrumCounter, true);
          }
        }
      }
      continue;        
    }
  }

  myFile.close();
  return 0;
}

void LoadDrumConfigFromFlashEEPROM(class DrumMachine* DrumMachin){
      char fname[20];
      unsigned char Key  = EEPROM.read(AddressStartDrumFile);
      if(Key == 0xFF) return -1;

      itoa(Key, fname, 10);
      strcat(fname, ".drm");
      
      if (SD.exists(fname)) {
        DrumMachin->LoadDrumMachine(fname);
        DrumMachin->Enable(false);
      }
}
