#include <stdlib.h>
#include <EEPROM.h>
//#include <avr/io.h>
#include "keyboards.h"
#include "config.h"
#include "midi.h"
#include "menu.h"
#include "strings.h"
#include "hotkey.h"
#include "analog.h"
#include "midichanel.h"
#include "sdcard.h"
#include "debug.h"

#ifdef BMP280_USED
#include "BMP280.h"
#endif


KeyboardS *MyKeyb;


//--------------------------------------------------------------------------------------------------------------
// KeyboardS
//--------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------
//the constructor
KeyboardS::KeyboardS() {

  ScanMode          = InitScanMode ;
  Normal_Pin_State  = InitNormal_Pin_State;
  KeyInputPinStart  = InitKeyInputPinStart;
  KeyInputPinStop   = InitKeyInputPinStop;
  KeyOutputPinStart = InitKeyOutputPinStart;
  KeyOutputPinStop  = InitKeyOutputPinStop;
  ScanCodeNums      = InitScanCodeNums;

#ifdef DRAM_MACHINE_USED
  DrumMachin = new(DrumMachine);
#endif

  // Set Input pin 22-37 () for KeyBoard matrix In
  for (int counter = KeyInputPinStart; counter <= KeyInputPinStop; counter++) {
    pinMode(counter, INPUT);
    digitalWrite(counter, HIGH);
  }

  // Set output pin 2-9 for KeyBoard matrix out all Inputs at Start
  for (int counter = KeyOutputPinStart; counter <= KeyOutputPinStop; counter++) {
    //Setup high-impedance state
    pinMode(counter, INPUT);
    digitalWrite(counter, LOW);
  }
  //Get input mask (Normal pin state)
  ScanKeyMatrixLow(InputKeysMask);

  // Init Midi Chanels
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    MidiChanels[Counter] = NULL;
  }

  // Clear Keyboards
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    Keyboards[Counter] = NULL;
  }
  // Clear Hotkeys
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    Hotkeys[Counter] = NULL;
  }
  // Clear AnalogIn
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    AnalogInputs[Counter] = NULL;
  }
}


//----------------------------------------------------------------------
//the destructor
KeyboardS::~KeyboardS() {
  DeleteAllHotkeys();
  DeleteAllKeyboard();
  DeleteAllMidiChanel();
  DeleteAllAnalogIn();
}

//----------------------------------------------------------------------
// Convert ScanCode to KeyCode
int KeyboardS::ScanCodeToKeyCode(unsigned char ScanCode) {
  for (int Scan = 1; Scan < ScanCodeNums + 1; Scan++ ) {
    if (KeyCodes[Scan] == ScanCode) {
      return Scan;
    }
  }
  return 0;
}

//----------------------------------------------------------------------
// Output pin to Low
inline void KeyboardS::Pin2Low(unsigned char Pin) {
  //Setup LOW pin state
  pinMode(Pin, OUTPUT);
  digitalWrite(Pin, LOW);
  delay(1);
//  delayMicroseconds(200);

}

//----------------------------------------------------------------------
// Output pin to High
inline void KeyboardS::Pin2High(unsigned char Pin) {
  //Setup LOW pin state
  pinMode(Pin, OUTPUT);
  digitalWrite(Pin, HIGH);
  delay(1);
  //delayMicroseconds(200);
}

//----------------------------------------------------------------------
// Output pin to high-impedance pin state
inline void KeyboardS::Pin2HiZ(unsigned char Pin) {
  //Setup high-impedance pin state
  pinMode(Pin, INPUT);
  digitalWrite(Pin, LOW);
}

//----------------------------------------------------------------------
// Scan matrix 8out x 16in max and return bitmap
void KeyboardS::ScanKeyMatrixLow(unsigned int Code[]) {
  for (int OutCounter = 0; OutCounter < KeyOutputPinStop - KeyOutputPinStart + 1; OutCounter++) {
    unsigned char OutPin = OutCounter + KeyOutputPinStart;
    //Setup LOW pin state
    Pin2Low(OutPin);

    Code[OutCounter] = 0; //clear code
    //get all input pin state... Нop read PA and PC more faster, but this solution more portable
    for (int InputPin = KeyInputPinStop; InputPin >= KeyInputPinStart; InputPin--) {
      Code[OutCounter] <<= 1;
      if (digitalRead(InputPin) == HIGH) {
        Code[OutCounter] = Code[OutCounter] | 1;
      }
    }
    //Setup high-impedance pin state
    Pin2HiZ(OutPin);
  }
}


//----------------------------------------------------------------------
// Scan matrix 8out x 16in max
char KeyboardS::ScanKeyMatrix(unsigned char Keys[]) {
  unsigned int Code[8];
  int ScancodesLen = 0;

  ScanKeyMatrixLow(Code); // get keyboard pins state

  for (int OutCounter = 0; OutCounter < KeyOutputPinStop - KeyOutputPinStart + 1; OutCounter++) { //for all output lines
    if ( Normal_Pin_State == 2 ) Code[OutCounter] = Code[OutCounter] ^ InputKeysMask[OutCounter]; //if used auto detect mode - XOR MaskCode and Code - clear unchanged pins

    for (int CodeCounter = 0; CodeCounter < KeyInputPinStop - KeyInputPinStart + 1; CodeCounter++) {
      bool Change = false; //Bit change Flag

      if ((Normal_Pin_State == 0) && (Code[OutCounter] & 1 == 0)) { //normal open mode
        Change = true;
      } else if (Code[OutCounter] & 1 == 1) {                       //normal closed and autodetect mode
        Change = true;
      }
      if (Change) { //if bit is changed convert ScanCodes -> KeyCode and push KeyCode in array
        unsigned char Key = ScanCodeToKeyCode((OutCounter << 4) | CodeCounter);
        if (Key != 0) {
          Keys[ScancodesLen] = Key;
          ScancodesLen++;

          if (ScancodesLen == MaxKeysOneTime - 1) { // проверка на переполнение буфера нажатых кнопок
            Keys[ScancodesLen] = 0x00;      // если буфер переполнен закрываем массив и выходим
            return 0;
          }
        }
      }
      Code[OutCounter] = Code[OutCounter] >> 1;
    }
  }

  // 0x00 after last ScanCode
  Keys[ScancodesLen] = 0x00;
  return 0;
}

//----------------------------------------------------------------------
// Get one fierst key return pressed key, 255-error
unsigned char KeyboardS::GetKey() {
  unsigned char Keys[MaxKeysOneTime];

  while (1) {

    if (digitalRead(ENC_BTN) == 0) {
      delay(DelayNorm);
      return 255;
    }

    ScanKeyMatrix(Keys);
    if (Keys[0] != 0) {
      return Keys[0];
    }
  }
}


//----------------------------------------------------------------------
// Play
void KeyboardS::Play() {
  unsigned char Keys[MaxKeysOneTime]; // Mass for pressed buttons


  // Play AnalogIn
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (AnalogInputs[Counter] != NULL) {
      AnalogInputs[Counter]->Play();
    }
  }

#ifdef BMP280_USED
  // Если используется цифровой датчик BMP820 - обновляем его данные
  if (BMP280_Use == 1 && Bmp280) {
    Bmp280->Play();
  }
#endif
  
  // Если используется драм машина играем
#ifdef DRAM_MACHINE_USED
    DrumMachin->Play();
#endif



  ScanKeyMatrix(Keys);
  //Play Buttons
  for (int counter = 0; counter < MaxKeyboards; counter++) {
    if (Keyboards[counter] != NULL) {
      Keyboards[counter]->Play(Keys);
    }
  }

  // Play Hotkeys
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    if (Hotkeys[Counter] != NULL) {
      Hotkeys[Counter]->PlayKeys(Keys);
    }
  }

  // Send Midi Chanels or controllers change
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MidiChanels[Counter] != NULL) {
      MidiChanels[Counter]->Play();
    }
  }

}

//##################### MidiChanels ######################################
//----------------------------------------------------------------------
// Add New MidiChanel
MidiChanel* KeyboardS::AddNewMidiChanel(unsigned char Chanel, unsigned char Volume, unsigned char Pressure, unsigned char Pitch, unsigned char Velocity, unsigned char Prog, unsigned char MSB, unsigned char LSB) {
  //Search Fierst Empty Slot
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MidiChanels[Counter] == NULL) {  // If founded empty slot - add new keyboard
      MidiChanels[Counter] = new MidiChanel(Chanel, Volume, Pressure, Pitch, Velocity, Prog, MSB, LSB);
      if (MidiChanels[Counter] == NULL) DebugPrintLn("CantCreate Midi Chanel!");
      return MidiChanels[Counter];
    }
  }
  return NULL;
}


//----------------------------------------------------------------------
// Delete MidiChanel   return -1 if error,  0 if success
int KeyboardS::DeleteMidiChanel(MidiChanel* Chan) {
  // search keyboards use this chanel
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    if (Keyboards[Counter] != NULL) {
      if (Keyboards[Counter]->Chanel == Chan)
        return -1;
    }
  }

  // try delete chanel
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    // If founded delete from array
    if (MidiChanels[Counter] == Chan) {
      MidiChanels[Counter] = NULL;
      delete Chan; //Free memory
      //shift data in array
      while (Counter < MaxMidiChanels - 1) {
        MidiChanels[Counter] = MidiChanels[Counter + 1];
        Counter++;
      }
      MidiChanels[MaxMidiChanels - 1] = NULL;
      return 0;
    }
  }
  return -1;
}


//----------------------------------------------------------------------
// Delete All MidiChanel
void KeyboardS::DeleteAllMidiChanel() {

  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    // If founded delete from array
    if (MidiChanels[Counter] != NULL) {
      MidiAllSoundsOff(MidiChanels[Counter]->Chanel);  //delete in test for Mikhail
      delete MidiChanels[Counter];
      MidiChanels[Counter] = NULL;
    }
  }
}


//----------------------------------------------------------------------
// GetMidiChanelCounts
unsigned int KeyboardS::GetMidiChanelCount() {
  unsigned int Count = 0;
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MidiChanels[Counter] != NULL) { //if keyboard not empty
      Count++;
    }
  }
  return Count;
}

//----------------------------------------------------------------------
// Get the address of MidiChanel by Num
MidiChanel* KeyboardS::GetMidiChanelByNum(unsigned char Chan) {
  for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
    if (MidiChanels[Counter] != NULL) {
      if (MidiChanels[Counter]->Chanel == Chan)
        return MidiChanels[Counter];
    }
  }
  return NULL;
}


//----------------------------------------------------------------------
// Setup All Midi Chanel Programs
void KeyboardS::SetupAllPrograms(unsigned char ProgramChangeMode) {
  for (unsigned char counter = 0; counter < 16; counter++) {
    if (MidiChanels[counter] != NULL) {
      MidiChanels[counter]->SetupChanelProgram(ProgramChangeMode);
    }
  }
}

//##################### KEYBOARDS ######################################
//----------------------------------------------------------------------
// Add New Virtual Keyboard
Keyboard* KeyboardS::AddNewKeyboard(char Name[], unsigned char FierstKey, unsigned char LastKey, MidiChanel* Chanel, unsigned char MidiBaseNote, unsigned char  Increment, unsigned char  SynchroStart ) {
  //Search Fierst Empty Slot
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    if (Keyboards[Counter] == NULL) {  // If founded empty slot - add new keyboard
      Keyboards[Counter] = new Keyboard(Name, FierstKey, LastKey, Chanel, MidiBaseNote, Increment, SynchroStart);
      if (Keyboards[Counter] == NULL)  DebugPrintLn("Cant create new Keyboard!");
      return Keyboards[Counter];
    }
  }
  return NULL;
}


//----------------------------------------------------------------------
// Delete Virtual Keyboard return -1 if error,  0 if success
int KeyboardS::DeleteKeyboard(Keyboard* Keyb) {
  // Check Hotkes use this keyboard
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    // If founded delete from array
    if (Hotkeys[Counter] != NULL) {
      if (Hotkeys[Counter]->Keyb == Keyb)
        return -1;
    }
  }

  //Search keyboard
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    // If founded delete from array
    if (Keyboards[Counter] == Keyb) {
      delete Keyb; //Free memory
      Keyboards[Counter] = NULL;
      //shift data in array
      while (Counter < MaxKeyboards - 1) {
        Keyboards[Counter] = Keyboards[Counter + 1];
        Counter++;
      }
      Keyboards[MaxKeyboards - 1] = NULL;
      return 0;
    }
  }
  return -1;
}


//----------------------------------------------------------------------
// Delete All Virtual Keyboards
void KeyboardS::DeleteAllKeyboard() {

  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    // If founded delete from array
    if (Keyboards[Counter] != NULL) {
      MidiAllSoundsOff(Keyboards[Counter]->Chanel->Chanel);
      delete Keyboards[Counter];
      Keyboards[Counter] = NULL;
    }
  }
}

//----------------------------------------------------------------------
// Get Virtual Keyboard Count
unsigned int KeyboardS::GetKeyboardsCount() {
  unsigned int Count = 0;
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    if (Keyboards[Counter] != NULL) { //if keyboard not empty
      Count++;
    }
  }
  return Count;
}


//----------------------------------------------------------------------
// Get the index of Keyboard in array
int KeyboardS::GetKeyboardIndex(Keyboard* Keyb) {
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    if (Keyboards[Counter] == Keyb) {
      return Counter;
    }
  }
  return -1;
}


//----------------------------------------------------------------------
// Get the keyboard ptr by Name
Keyboard* KeyboardS::GetKeyboardPtrByName(char Name[]) {
  for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
    if (Keyboards[Counter] != NULL) {
      if (strcmp(Keyboards[Counter]->Name, Name) == 0) {
        return Keyboards[Counter];
      }
    }
  }
  return NULL;
}

//----------------------------------------------------------------------
// Get the Pointer by index of Keyboard array
Keyboard* KeyboardS::GetKeyboardPtr(int Index) {
  return Keyboards[Index];
}


//##################### Hot KEYS ######################################
//----------------------------------------------------------------------
// Add New Hotkey
Hotkey* KeyboardS::AddNewHotkey(char Name[], Keyboard * Keyb,  unsigned char Key,  int Modifier, unsigned char Action) {
  //Search Fierst Empty Slot
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    // If founded - add new keyboard
    if (Hotkeys[Counter] == NULL) {
      Hotkeys[Counter] = new Hotkey(Name, Keyb, Key, Modifier, Action);
      if (Hotkeys[Counter] == NULL)  DebugPrintLn("Cant create new HotKey!");
      return Hotkeys[Counter];
    }
  }
  return NULL;
}


//----------------------------------------------------------------------
// Delete Hotkey return -1 if error,  0 if success
int KeyboardS::DeleteHotkey(Hotkey* Key) {
  int Counter;
  //Search Hotkey
  for (Counter = 0; Counter < MaxHotkeys; Counter++) {
    // If founded delete from array
    if (Hotkeys[Counter] == Key) {
      delete Key; //Free memory
      Hotkeys[Counter] = NULL;
      while (Counter < MaxHotkeys - 1) {
        Hotkeys[Counter] = Hotkeys[Counter + 1];
        Counter++;
      }
      Hotkeys[MaxHotkeys - 1] = NULL;
      return 0;
    }
  }
  return -1;
}


//----------------------------------------------------------------------
// Delete All Hotkeys
void KeyboardS::DeleteAllHotkeys() {
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    // If founded delete from array
    if (Hotkeys[Counter] != NULL) {
      delete Hotkeys[Counter];
      Hotkeys[Counter] = NULL;
    }
  }
}

//----------------------------------------------------------------------
// Get Hotkey Count
unsigned int KeyboardS::GetHotkeyCount() {
  unsigned int NumOfKeys = 0;
  for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
    if (Hotkeys[Counter] != NULL) {
      NumOfKeys++;
    }
  }
  return NumOfKeys;
}

//##################### Analog Inputs ######################################
//----------------------------------------------------------------------
// Add New Analog In
AnalogIn* KeyboardS::AddNewAnalogIn(unsigned char  AnalogInputPin,  unsigned char Mode[], int Correction, unsigned char Boost, unsigned char CentralBoost) {
  //Search Fierst Empty Slot
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    // If founded - add new AnalogIn
    if (AnalogInputs[Counter] == NULL) {
      AnalogInputs[Counter] = new AnalogIn(AnalogInputPin, Mode, Correction, Boost, CentralBoost);
      if (AnalogInputs[Counter] == NULL) DebugPrintLn("Cant create Analog IN");
      return AnalogInputs[Counter];
    }
  }
  return NULL;
}

//----------------------------------------------------------------------
// Delete Analog In return -1 if error,  0 if success
int KeyboardS::DeleteAnalogIn(AnalogIn* AnalogInput) {
  int Counter;
  //Search Hotkey
  for (Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    // If founded delete from array
    if (AnalogInputs[Counter] == AnalogInput) {
      delete AnalogInput; //Free memory
      AnalogInputs[Counter] = NULL;
      while (Counter < MaxAnalogInputs - 1) {
        AnalogInputs[Counter] = AnalogInputs[Counter + 1];
        Counter++;
      }
      AnalogInputs[MaxAnalogInputs - 1] = NULL;
      return 0;
    }
  }
  return -1;
}

//----------------------------------------------------------------------
// Delete All Analog Inputs
void KeyboardS::DeleteAllAnalogIn() {
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    // If founded delete from array
    if (AnalogInputs[Counter] != NULL) {
      delete AnalogInputs[Counter];
      AnalogInputs[Counter] = NULL;
    }
  }
}

//----------------------------------------------------------------------
// Get Analog Inputs count
unsigned int KeyboardS::GetAnalogInCount() {
  unsigned int NumOfInputs = 0;
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (AnalogInputs[Counter] != NULL) {
      NumOfInputs++;
    }
  }
  return NumOfInputs;
}


//----------------------------------------------------------------------
// Get Analog Input Index by Address
unsigned int KeyboardS::GetAnalogInIndex(AnalogIn* Input) {
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (AnalogInputs[Counter] == Input) {
      return Counter;
    }
  }
  return 255;
}


//----------------------------------------------------------------------
// Get Analog Input Index by Input
unsigned int KeyboardS::GetAnalogInByInput(uint8_t Input) {
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (AnalogInputs[Counter] != NULL) {
      if (AnalogInputs[Counter]->Input == Input) {
        return Counter;
      }
    }
  }
  return 255;
}

//----------------------------------------------------------------------
// Get Analog Input by Address of value
unsigned int KeyboardS::GetAnalogInByValAddr(unsigned char * Input) {
  for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
    if (&AnalogInputs[Counter]->Value == Input) {
      return AnalogInputs[Counter]->Input;
    }
  }
  return 255;
}


// ----------------------------------------------------------------------
// Convert DataBox to int Value for Menu   // перенеcти функцию для меню в другое место
int KeyboardS::GetBoxMenuValue(DataBox * Value) {
  int Tmp = 0;
  if (Value->IsStatic()) {
    Tmp = Value->Get();
  } else {
    //Analog In Value
    Tmp = ~GetAnalogInByValAddr(Value->ExternalValue);
  }
  return Tmp;
}
// class KeyboardS end






// ----------------------------------------------------------------------
KeyboardS *InitKeyboard(KeyboardS *Keybs){

    /////kill em all! (easy way to kill all with destructor) 
    if (Keybs != NULL) delete MyKeyb;
    Keybs = NULL;
    
    // and create new!
    Keybs = new KeyboardS();
    
    if (Keybs==NULL){
       DebugPrintLn("ERROR: cant create object keyboard");
    }  else {
       DebugPrintLn("Create object keyboard");      
    }/**/

    return Keybs;
}
