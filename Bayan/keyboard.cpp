#include "keyboard.h"
#include "config.h"
#include "midichanel.h"
#include "keyboards.h"

//--------------------------------------------------------------------------------------------------------------
// class Keyboard
//--------------------------------------------------------------------------------------------------------------
Keyboard::Keyboard(char initName[], unsigned char initKeyBegin, unsigned char initKeyEnd, MidiChanel * initMidiChanel, unsigned char initMidiBaseNote, unsigned char  initIncrement, unsigned char  initSynchroStart) {
    //Init Name
    int Counter;
    for (Counter = 0; (initName[Counter] != 0) && (Counter < MaxNameLen); Counter++) {
      Name[Counter] = initName[Counter];
    }
    Name[Counter] = 0;
    ReNew=0;
    //Init Values
    KeyBegin = initKeyBegin;
    KeyEnd = initKeyEnd;
    MidiBaseNote = initMidiBaseNote;
    Chanel = initMidiChanel;
    Increment = initIncrement;
    SynchroStart = initSynchroStart; 

    for (int counter = 0; counter < MaxKeysOneTime + 1; counter++) {
      SendedKeys[counter] = 0;
    }

}

//----------------------------------------------------------------------
// Возвращает true если звук уже отправлен
bool Keyboard::KeyIsSended(unsigned char Key) {
  for (int counter = 0; (SendedKeys[counter] != 0) && (counter < MaxKeysOneTime); counter++) { // Просматриваем весь массив отправленных
    if (SendedKeys[counter] == Key ) { // если есть среди отправленных 
      return true;                      // возвращаем true
    }
  }
  return false;
}


//----------------------------------------------------------------------
//
bool Keyboard::KeyIsReleased(unsigned char Key, unsigned char PushedKeys[]) {
  for (int counter = 0; (PushedKeys[counter] != 0) && (counter < MaxKeysOneTime) ; counter++) {
    if (PushedKeys[counter] == Key ) {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------
//
bool Keyboard::AddKeyToSendedKeys(unsigned char Key) {
  for (int counter = 0; counter < MaxKeysOneTime; counter++) {
    if (SendedKeys[counter] == 0 ) {
      SendedKeys[counter] = Key;
      SendedKeys[counter + 1] = 0;
      return true;
    }
  }
  return false; // Not enought space
}


//----------------------------------------------------------------------
//
bool Keyboard::RemoveKeyFromSendedKeys(unsigned char Key) {
  for (int counter = 0; (counter < MaxKeysOneTime) && (SendedKeys[counter] != 0); counter++) {
    if (SendedKeys[counter] == 0 ) { // Key not found
      return false;
    }
    if (SendedKeys[counter] == Key) { // Key found
      for (int Index = counter; (Index < MaxKeysOneTime) && (SendedKeys[Index] != 0); Index++) {
        SendedKeys[Index] = SendedKeys[Index + 1];
      }
      return true;
    }
  }
  return false; // Not found -
}


//----------------------------------------------------------------------
// Note num calculate
unsigned char Keyboard::GetNoteFromKey(unsigned char Key) {
  if (Increment == 0 || Increment == 3) {
    return (MidiBaseNote);
  }
  else {
    return (MidiBaseNote + ( Key - KeyBegin));
  }
}

//----------------------------------------------------------------------
//Check for new Button Pushed or Released
void Keyboard::PlayButtons(unsigned char PushedKeys[]) { //a function
  //Mute Released Buttons
  for (int counter = 0; (SendedKeys[counter] != 0) && (counter < MaxKeysOneTime); counter++) {  // for all Sended keys
    if (KeyIsReleased(SendedKeys[counter], PushedKeys) ) {                                      // If Sended Key not found in PushedKeys Array
      unsigned char Note = GetNoteFromKey(SendedKeys[counter]);                                 // Get Note
      Chanel->ClearNote(Note);                                                                  // Mute Note

      RemoveKeyFromSendedKeys(SendedKeys[counter]);      // delete Key from SendedKeys
      counter = 0; // begin at start
    }
  }
  
  // Send Pushed Buttons
  for (int counter = 0; (PushedKeys[counter] != 0) && (counter < MaxKeysOneTime) ; counter++) { // for all pressed keys
    if ((PushedKeys[counter] >= KeyBegin) && (PushedKeys[counter] <= KeyEnd) ) {    //check button in Keyboard
      if (!KeyIsSended(PushedKeys[counter]) ) {                                     // chek is key allready not sended
        if ( AddKeyToSendedKeys(PushedKeys[counter]) ) {                            // if add to Sended Keys succsessfull
          unsigned char  Note = GetNoteFromKey( PushedKeys[counter] );
          Chanel->SendNote(Note);                                                   // Send Note

#ifdef DRAM_MACHINE_USED
          if(SynchroStart == 1)
            if(MyKeyb->DrumMachin->GetSynchroStart() == true)
              if(!MyKeyb->DrumMachin->IsEnabled())
                MyKeyb->DrumMachin->GoSynchroStart();
#endif

        }
      }
    }
  }
}


//----------------------------------------------------------------------
//Check for new Button Pushed or Released
void Keyboard::PlayButtonsChords(unsigned char PushedKeys[]) { //a function
  unsigned char Counter =0;
  for (int counter = 0; (PushedKeys[counter] != 0) && (counter < MaxKeysOneTime) ; counter++) // Count pressed keys
    if ((PushedKeys[counter] >= KeyBegin) && (PushedKeys[counter] <= KeyEnd) ) 
      Counter++;

  if(Counter<3){ // if pressed less then 3 keys
    //Mute All Buttons
    for (int counter = 0; (SendedKeys[counter] != 0) && (counter < MaxKeysOneTime); counter++) {  // for all Sended keys
        unsigned char Note = GetNoteFromKey(SendedKeys[counter]);                                 // Get Note
        Chanel->ClearNote(Note);                                                                  // Mute Note
        RemoveKeyFromSendedKeys(SendedKeys[counter]);      // delete Key from SendedKeys
        counter = 0; // begin at start
    }
  }
  else{
    PlayButtons(PushedKeys);
  }
}


///////////////////////////////////////////////////////////////////////////////////
void Keyboard::Play(unsigned char PushedKeys[]){

  if(Increment < 2 ){    // 0, 1 - single note keyboard
    PlayButtons(PushedKeys);    
  }
  else {                 //2,3 -chord keyboard
    PlayButtonsChords(PushedKeys);
  }
}
