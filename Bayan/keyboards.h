#ifndef KEYB_H
#define KEYB_H

#include <stdlib.h>
#include "config.h"
#include "midi.h"
#include "menu.h"
#include "strings.h"
#include <avr/io.h>
#include <EEPROM.h>
#include "keyboard.h"
#include "hotkey.h"
#include "analog.h"
#include "midichanel.h"
#include "comfunc.h"
#include "sdcard.h"
#include "BMP280.h"


//--------------------------------------------------------------------------------------------------------------
// KeyboardS
//--------------------------------------------------------------------------------------------------------------
class KeyboardS {
  private:
    
  public: //public 
    Keyboard* Keyboards[MaxKeyboards];
    Hotkey* Hotkeys[MaxHotkeys];
    AnalogIn* AnalogInputs[MaxAnalogInputs];    
    MidiChanel* MidiChanels[MaxMidiChanels];
    unsigned int InputKeysMask[MaxOutCount];

    //----------------------------------------------------------------------
    //the constructor
    KeyboardS() {
      
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
      //Get input mask
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
    ~KeyboardS() {
      DeleteAllHotkeys();
      DeleteAllKeyboard();      
      DeleteAllMidiChanel();
      DeleteAllAnalogIn();
    }

    //----------------------------------------------------------------------
    // Convert ScanCode to KeyCode
    int ScanCodeToKeyCode(unsigned char ScanCode, unsigned  char KeyCodes[]) {
      for (int Scan = 1; Scan < ScanCodeNums + 1; Scan++ ) {
        if (KeyCodes[Scan] == ScanCode) {
          return Scan;
        }
      }
      return 0;
    }

    //----------------------------------------------------------------------
    // Output pin to Low
    inline void Pin2Low(unsigned char Pin) {
      //Setup LOW pin state
      pinMode(Pin, OUTPUT);
      digitalWrite(Pin, LOW);
      delay(1);
    }

    //----------------------------------------------------------------------
    // Output pin to High
    inline void Pin2High(unsigned char Pin) {
      //Setup LOW pin state
      pinMode(Pin, OUTPUT);
      digitalWrite(Pin, HIGH);
      delay(1);
    }

    //----------------------------------------------------------------------
    // Output pin to high-impedance pin state
    inline void Pin2HiZ(unsigned char Pin) {
      //Setup high-impedance pin state
      pinMode(Pin, INPUT);
      digitalWrite(Pin, LOW);
    }

    //----------------------------------------------------------------------
    // Scan matrix 8out x 16in max and return bitmap 
    void ScanKeyMatrixLow(unsigned int Code[]) {
      for (int OutCounter = 0; OutCounter < KeyOutputPinStop - KeyOutputPinStart +1; OutCounter++) {
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
    char ScanKeyMatrix(unsigned char Keys[]) {
      unsigned int Code[8];
      int ScancodesLen = 0;
      
      ScanKeyMatrixLow(Code); // get keyboard pins state
      
      for (int OutCounter = 0; OutCounter < KeyOutputPinStop - KeyOutputPinStart +1; OutCounter++) {  //for all output lines
        if ( Normal_Pin_State == 2 ) Code[OutCounter] = Code[OutCounter] ^ InputKeysMask[OutCounter]; //if used auto detect mode - XOR MaskCode and Code - clear unchanged pins

        for (int CodeCounter = 0; CodeCounter < KeyInputPinStop - KeyInputPinStart +1; CodeCounter++) {
          bool Change = false; //Bit change Flag
          
          if ((Normal_Pin_State == 0) && (Code[OutCounter] & 1 == 0)){ //normal open mode
            Change = true;
          }else if(Code[OutCounter] & 1 == 1){                         //normal closed and autodetect mode
            Change = true;
          }
          if (Change) { //if bit is changed convert ScanCodes -> KeyCode and push KeyCode in array
            unsigned char Key = ScanCodeToKeyCode((OutCounter << 4) | CodeCounter, KeyCodes);
            if (Key != 0) {
              Keys[ScancodesLen] = Key;
              ScancodesLen++;
                               
              if(ScancodesLen == MaxKeysOneTime-1){ // проверка на переполнение буфера нажатых кнопок
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
    unsigned char GetKey() {
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
    void Play() {
      unsigned char Keys[MaxKeysOneTime]; // Mass for pressed buttons

      
      // Play AnalogIn
      for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if(AnalogInputs[Counter] != NULL){
          AnalogInputs[Counter]->Play();
        }
      }
      
      // Если используется цифровой датчик BMP820 - обновляем его данные 
      if(BMP280_Use == 1 && Bmp280) {
        Bmp280->Play(); 
      }
      
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
        if(MidiChanels[Counter]!=NULL){
          MidiChanels[Counter]->Play();
        }  
      }
     
    }

    //##################### MidiChanels ######################################
    //----------------------------------------------------------------------
    // Add New MidiChanel
    MidiChanel* AddNewMidiChanel(unsigned char Chanel,unsigned char Volume, unsigned char Pressure, unsigned char Pitch, unsigned char Velocity, unsigned char Prog, unsigned char MSB, unsigned char LSB) {
      //Search Fierst Empty Slot
      for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
        if (MidiChanels[Counter] == NULL) {  // If founded empty slot - add new keyboard
          MidiChanels[Counter] = new MidiChanel(Chanel,Volume, Pressure, Pitch, Velocity,Prog,MSB,LSB);
          if (MidiChanels[Counter] == NULL) DebugPrintLn("CantCreate Midi Chanel!"); 
          return MidiChanels[Counter];
        }
      }
      return NULL;
    }


    //----------------------------------------------------------------------
    // Delete MidiChanel   return -1 if error,  0 if success
    int DeleteMidiChanel(MidiChanel* Chan) {
      // search keyboards use this chanel
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if (Keyboards[Counter] != NULL) {  
          if(Keyboards[Counter]->Chanel == Chan)
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
    void DeleteAllMidiChanel() {

      for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
        // If founded delete from array
        if (MidiChanels[Counter] != NULL) {
          MidiAllSoundsOff(MidiChanels[Counter]->Chanel);
          delete MidiChanels[Counter];
          MidiChanels[Counter] = NULL;
        }
      }
    }


    //----------------------------------------------------------------------
    // GetMidiChanelCounts
     unsigned int GetMidiChanelCount() {
      unsigned int Count=0;
      for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
        if (MidiChanels[Counter] != NULL) { //if keyboard not empty
          Count++;
        }
      }
      return Count;
    }

    //----------------------------------------------------------------------
    // Get the address of MidiChanel by Num
    MidiChanel* GetMidiChanelByNum(unsigned char Chan) {
      for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
        if (MidiChanels[Counter] !=NULL) {
          if(MidiChanels[Counter]->Chanel == Chan)
            return MidiChanels[Counter];
        }
      }
      return NULL;
    }


    //----------------------------------------------------------------------
    // Setup All Midi Chanel Programs
    void SetupAllPrograms(unsigned char ProgramChangeMode) {
      for (unsigned char counter = 0; counter < 16; counter++) {
        if (MidiChanels[counter]!=NULL){
          MidiChanels[counter]->SetupChanelProgram(ProgramChangeMode);
        }
      }
    }
    
    //##################### KEYBOARDS ######################################
    //----------------------------------------------------------------------
    // Add New Virtual Keyboard
    Keyboard* AddNewKeyboard(char Name[], unsigned char FierstKey, unsigned char LastKey, MidiChanel* Chanel, unsigned char MidiBaseNote, unsigned char  Increment ) {
      //Search Fierst Empty Slot
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if (Keyboards[Counter] == NULL) {  // If founded empty slot - add new keyboard
          Keyboards[Counter] = new Keyboard(Name, FierstKey, LastKey, Chanel, MidiBaseNote, Increment);
          if (Keyboards[Counter] == NULL)  DebugPrintLn("Cant create new Keyboard!"); 
          return Keyboards[Counter];
        }
      }
      return NULL;
    }


    //----------------------------------------------------------------------
    // Delete Virtual Keyboard return -1 if error,  0 if success
    int DeleteKeyboard(Keyboard* Keyb) {
      // Check Hotkes use this keyboard
      for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
        // If founded delete from array
        if (Hotkeys[Counter] != NULL) {
          if(Hotkeys[Counter]->Keyb == Keyb)
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
    void DeleteAllKeyboard() {

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
     unsigned int GetKeyboardsCount(){
      unsigned int Count=0;
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if (Keyboards[Counter] != NULL) { //if keyboard not empty
          Count++;
        }
      }
      return Count;
    }

    
    //----------------------------------------------------------------------
    // Get the index of Keyboard in array
    int GetKeyboardIndex(Keyboard* Keyb) {
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if (Keyboards[Counter] == Keyb) {
          return Counter;
        }
      }
      return -1;
    }


    //----------------------------------------------------------------------
    // Get the keyboard ptr by Name
    Keyboard* GetKeyboardPtrByName(char Name[]) {
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if(Keyboards[Counter]!=NULL){
          if (strcmp(Keyboards[Counter]->Name,Name)==0) {
            return Keyboards[Counter];
         }
       } 
      }
      return NULL;
    }

    //----------------------------------------------------------------------
    // Get the Pointer by index of Keyboard array
    Keyboard* GetKeyboardPtr(int Index) {
      return Keyboards[Index];
    }
    

    //##################### Hot KEYS ######################################
    //----------------------------------------------------------------------
    // Add New Hotkey
    Hotkey* AddNewHotkey(char Name[], Keyboard * Keyb,  unsigned char Key,  int Modifier, unsigned char Action) {
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
    int DeleteHotkey(Hotkey* Key) {
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
          Hotkeys[MaxHotkeys -1] = NULL;
          return 0;
        }
      }
      return -1;
    }


    //----------------------------------------------------------------------
    // Delete All Hotkeys
    void DeleteAllHotkeys() {
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
    unsigned int GetHotkeyCount(){
     unsigned int NumOfKeys=0;
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
    AnalogIn* AddNewAnalogIn(unsigned char  AnalogInputPin,  unsigned char Mode[], int Correction, unsigned char Boost, unsigned char CentralBoost){
      //Search Fierst Empty Slot
      for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        // If founded - add new AnalogIn
        if (AnalogInputs[Counter] == NULL) {
          AnalogInputs[Counter] = new AnalogIn(AnalogInputPin, Mode, Correction, Boost, CentralBoost);
          if(AnalogInputs[Counter] == NULL) DebugPrintLn("Cant create Analog IN"); 
          return AnalogInputs[Counter];
        }
      }
      return NULL;
    }    
    
    //----------------------------------------------------------------------
    // Delete Analog In return -1 if error,  0 if success
    int DeleteAnalogIn(AnalogIn* AnalogInput) {
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
    void DeleteAllAnalogIn(){
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
    unsigned int GetAnalogInCount(){
     unsigned int NumOfInputs=0;
     for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (AnalogInputs[Counter] != NULL) {
          NumOfInputs++;
        }
      } 
      return NumOfInputs;   
    }


    //----------------------------------------------------------------------
    // Get Analog Input Index by Address
    unsigned int GetAnalogInIndex(AnalogIn* Input){
     for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (AnalogInputs[Counter] == Input) {
          return Counter;             
        }
      } 
      return 255;   
    }


    //----------------------------------------------------------------------
    // Get Analog Input Index by Input
    unsigned int GetAnalogInByInput(uint8_t Input){
     for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (AnalogInputs[Counter] != NULL) {
          if (AnalogInputs[Counter]->Input == Input){
            return Counter;    
          }  
        }
     }
      return 255;   
    }

    //----------------------------------------------------------------------
    // Get Analog Input by Address of value
    unsigned int GetAnalogInByValAddr(unsigned char * Input){
     for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if (&AnalogInputs[Counter]->Value == Input) {
          return AnalogInputs[Counter]->Input;             
        }
      } 
      return 255;   
    }


    // ----------------------------------------------------------------------
    // Convert DataBox to int Value for Menu   // перенеcти функцию для меню в другое место
    int GetBoxMenuValue(DataBox * Value){
      int Tmp=0;
      if (Value->IsStatic()){
        Tmp=Value->Get();
      }else{
        //Analog In Value
        Tmp=~GetAnalogInByValAddr(Value->ExternalValue);
      }
      return Tmp;
    }
    
}; // undocumented semicolon otherwise error:




// Глобальная переменная здесь это пиздец. Не забыть переместить.
KeyboardS  *MyKeyb;



/////////////////////////////////////////////////////////////////////////////////////
//Загрузка/сохранение  сильно разрослась. Перенести потом в отдельный файл.
///////////////////////////////////////////////////////////////////////////////////

// Сохранение в указанный файл пары имя - значение для числа
void SaveNameValueToSD(File * myFile,char Name[],int Value){
          myFile->print(Name);
          myFile->print(" ");
          myFile->print(Value,DEC);
          myFile->println();          
}

// Сохранение в указанный файл пары имя - значение для строки
void SaveNameValueToSD(File * myFile,char Name[],char Value[]){
          myFile->print(Name);
          myFile->print(" ");
          myFile->print(Value);
          myFile->println();          
}


// Сохранение в указанный файл пары имя - значение для массива (значения сохранит как hex разделенные пробелом)
void SaveNameValueToSD(File * myFile,char Name[], unsigned char Value[]){
          myFile->print(Name);
          myFile->print(" ");
          
          for(unsigned char Index=0; Value[Index]!=0; Index++){
            myFile->print(Value[Index],HEX);
            myFile->print(" ");
          }  
          myFile->println();                  
}


// Сохранение в указанный файл пары имя - значение для массива фиксированной длины (значения сохранит как hex разделенные пробелом)
void SaveNameValueToSD(File * myFile,char Name[], unsigned char Value[], unsigned char Count){
          myFile->print(Name);
          myFile->print(" ");
          
          for(unsigned char Index=0; Index < Count; Index++){
            myFile->print(Value[Index],HEX);
            myFile->print(" ");
          }  
          myFile->println();                  
}

// Сохранение в указанный файл пары имя - значение для значений типа DataBox (значение сохранит как число или как номер аналогового входа)
void SaveNameValueToSD(File * myFile,char Name[], DataBox * Value){
          myFile->print(Name);
          myFile->print(" ");

           int Tmp = MyKeyb->GetBoxMenuValue(Value);
          
          if (Tmp >= 0){  
            // just print value
            myFile->print(Tmp,DEC);
          } else{ 
            //if value analog in - print number of analog in
            myFile->print("A"); 
            myFile->print(~Tmp, DEC); //тут обратный дополненный код, поэтому чтобы превратить А0 в -1 достаточно инвертировать число
          }
          myFile->println();          
}

//Считывает одну строку из указанного файла
int ReadStringFromSD(File * myFile, char Str[]){
  String StrBuff;
 
  if (myFile->available()){
      StrBuff = myFile->readStringUntil('\n');
      strcpy (Str, StrBuff.c_str());
      for(int Counter=0;Str[Counter]!=0 ;Counter++){
        if ((Str[Counter]=='\n') ||  (Str[Counter]=='\r')){
          Str[Counter]=0;
          break;
        }
      }
      return (strlen(Str));  
  }else{
    return -1;  
  }
}


// Возвращает ссылку на строку после имени параметра и пробела
char * GetStartCfgStr (char Str[]){
  int Counter;
  char * Num;

  for(Counter=0;Str[Counter]!=0;Counter++){
    if (Str[Counter]==' '){
      Num = &Str[Counter]+1;
      return Num;
    }
  }
}

// Функция для разбора конфигурации получает строку с именем и десятичным значением, возвращает десятичное значение
int GetIntCfgStr (char Str[]){
  return atoi(GetStartCfgStr(Str));
}

// Функция для разбора конфигурации получает строку с именем и Аналоговым входом, возвращает десятичное значение входа
int GetAnalogCfgStr (char Str[]){
  char * CharTemp;
  
  CharTemp = GetStartCfgStr(Str);
  if(CharTemp[0]=='A'){//
    CharTemp = &CharTemp[1];
    return atoi(CharTemp);
  }else{
    return 0;
  }
}

// Функция для разбора конфигурации получает строку с именем и десятичным значением, возвращает десятичное значение
int GetAnalogIntCfgStr (char Str[]){
  char * Start = GetStartCfgStr(Str);
  if(Start[0]=='A'){
    Start+=1;
    return ~atoi(Start);
  }else{
    return atoi(Start);
  }
  return 0;
}


// Функция для разбора конфигурации получает строку с именем и набором hex значений разделенную пробелами, массив заполняет полученными значениями
int GetHexCfgStr (char Str[],unsigned char OutStr[]){
  int Counter;
  char * Num;

  // Пропускаем имя параметра, разбираем подстроку  
  for(Counter=0;Str[Counter]!=0;Counter++){
    if (Str[Counter]==' '){
      Num = &Str[Counter]+1;
      break;
    }
  }  

  unsigned char Index=0;
  while(1){
    OutStr[Index] = strtoul(Num,&Num,16); // функция преобразует в hex часть строки до не цифрового символа (пробела, табуляции, перевода строки и.т.д.)
    if(Num[0]==0) break; // если достигнут конец строки выходим 
    Num = &Num[1];  //если это не конец строки - пропускаем символ и пробуем опять
    Index++;
  }
  
}



//Check String 
bool ChkCfgStr(char Templ[],char Str[]){
  int Counter;
  bool Result=false;

  for(Counter=0;Str[Counter]!=0;Counter++){
    if (Str[Counter]==' '){
      Str[Counter]=0;
       if(strcmp (Templ, Str)==0){
         Result=true;
       }
      Str[Counter]=' ';
      break;
    }
  }
  return Result;
}



//#########################################################################################

int LoadConfigFromFlash(char FName[]){
  char   CharBuff[30];
  char   Mode = 0;
  File myFile;
  MidiChanel* LastChanel;
 
  myFile = SD.open(FName, FILE_READ);
  DebugPrint("Try read preset file ");  
  DebugPrintLn(FName);
        
  // if the file opened okay, read line by line
  if (myFile) {
    DebugPrintLn("File open success, start parsing preset"); 

    /*
    /////kill em all! (easy way to kill all with destructor) //Сука в ардуине память реально не освобождается!!!
    if (MyKeyb != NULL) delete MyKeyb;
    MyKeyb = NULL;
    // and create new!
    MyKeyb = new KeyboardS();
    
    if (MyKeyb==NULL){
       DebugPrintLn("ERROR: cant create object keyboard");
    }  else {
       DebugPrintLn("Create object keyboard");      
    }/**/
    
    if(MyKeyb == NULL){
      MyKeyb = new KeyboardS();
      DebugPrintLn("Create object keyboard");
    } else{
      DebugPrintLn("Kill All objects");
      MyKeyb->DeleteAllHotkeys();
      MyKeyb->DeleteAllKeyboard();      
      MyKeyb->DeleteAllMidiChanel();
      MyKeyb->DeleteAllAnalogIn();
    }
    
    if(Bmp280 != NULL){
      Bmp280->ClearExternalSensor();
      DebugPrintLn("Deactivate BMP280 sensor");
    }

    while (ReadStringFromSD(&myFile,CharBuff) != -1) {
      //Analog Input
      if(strcmp(CharBuff,"[analogin]")==0){
        unsigned char Input=0;  
        unsigned char Mode[AnalogPoints];
        int Correction=0;
        unsigned char Boost = 0;
        unsigned char CentralBoost = 0;  
        DebugPrintLn("Create analogin");  
        while(ReadStringFromSD(&myFile,CharBuff) >0){
      
          if(ChkCfgStr("Input",CharBuff)){
            Input = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Mode",CharBuff)){
            GetHexCfgStr(CharBuff, Mode);
          }
          if(ChkCfgStr("Correction",CharBuff)){
            Correction=GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Boost",CharBuff)){
            Boost=GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("CentralBoost",CharBuff)){
            CentralBoost=GetIntCfgStr(CharBuff);
          }
        }
        MyKeyb->AddNewAnalogIn(Input,Mode,Correction,Boost,CentralBoost); 

        continue;
      }

      

      //Midi Chanel
      if(strcmp(CharBuff,"[chanel]")==0){
        DebugPrintLn("Create midichanel");
        unsigned char Num=0;
        int Volume=127;
        int Pressure=127;
        int Pitch=64;
        int Velocity=127;
        unsigned char MSB=0;
        unsigned char LSB=0;
        unsigned char Prog=0;
        while(ReadStringFromSD(&myFile,CharBuff) >0){
          if(ChkCfgStr("Num",CharBuff)){
            Num = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Volume",CharBuff)){
            Volume = GetAnalogIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Pressure",CharBuff)){
            Pressure = GetAnalogIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Pitch",CharBuff)){
            Pitch = GetAnalogIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Velocity",CharBuff)){
            Velocity = GetAnalogIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Prog",CharBuff)){
            Prog = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("MSB",CharBuff)){
            MSB = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("LSB",CharBuff)){
            LSB = GetIntCfgStr(CharBuff);
          }
        }
        
        LastChanel = MyKeyb->AddNewMidiChanel(Num,abs(Volume),abs(Pressure),abs(Pitch),abs(Velocity),Prog, MSB, LSB);

        if(Volume <0){
          LastChanel->Volume->SetExternalValue(&MyKeyb->AnalogInputs[~Volume]->Value);
        }
        if(Pressure <0){
          LastChanel->Pressure->SetExternalValue(&MyKeyb->AnalogInputs[~Pressure]->Value);
        }
        if(Pitch <0){
          LastChanel->Pitch->SetExternalValue(&MyKeyb->AnalogInputs[~Pitch]->Value);
        }
        if(Velocity <0){
          LastChanel->Velocity->SetExternalValue(&MyKeyb->AnalogInputs[~Velocity]->Value);
        }    
        continue;
      }
      
      if(strcmp(CharBuff,"<controller>")==0){
        unsigned char Input=0;
        unsigned char MSB=127;
        unsigned char LSB=127;
        
        DebugPrintLn("Create controller");                
        while(ReadStringFromSD(&myFile,CharBuff) >0){
          if(ChkCfgStr("Input",CharBuff)){
            Input = GetAnalogCfgStr(CharBuff);
          }
          if(ChkCfgStr("MSB",CharBuff)){
            MSB = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("LSB",CharBuff)){
            LSB = GetIntCfgStr(CharBuff);
          }
       }
       if(LastChanel != NULL){
          LastChanel->AddNewController(&MyKeyb->AnalogInputs[Input]->Value, MSB,LSB);
       }
      }

      if(strcmp(CharBuff,"[keyboard]")==0){
        char * TmpStr;
        char Name[20];
        unsigned char ChanelNum=0;
        MidiChanel *  Chan=NULL;
        unsigned char BeginKey=1;
        unsigned char EndKey=1;
        unsigned char BaseNote=1;
        unsigned char Increment=1;
       
        while(ReadStringFromSD(&myFile,CharBuff) >0){

          if(ChkCfgStr("Name",CharBuff)){
            strcpy(Name,GetStartCfgStr(CharBuff));
          }

          if(ChkCfgStr("ChanelNum",CharBuff)){
            ChanelNum = GetIntCfgStr(CharBuff);
          }
          Chan = MyKeyb->GetMidiChanelByNum(ChanelNum);

          if(ChkCfgStr("BeginKey",CharBuff)){
            BeginKey = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("EndKey",CharBuff)){
            EndKey = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("BaseNote",CharBuff)){
            BaseNote = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Increment",CharBuff)){
            Increment = GetIntCfgStr(CharBuff);
          }
       }
       if(Chan!=NULL){
         MyKeyb->AddNewKeyboard(Name, BeginKey, EndKey, Chan, BaseNote, Increment);
       }
      }
      
      if(strcmp(CharBuff,"[hotkey]")==0){
        char Name[20] = "Unnamed";
        Keyboard * Kbd = NULL;
        unsigned char Key = 255;
        unsigned char Action = 255;
        unsigned char Modifier[MaxHotkeyMidiLen+1];
        Modifier[0] = 0; //Empty modifier string

        DebugPrintLn("Create hotkey");                
        while(ReadStringFromSD(&myFile,CharBuff) >0){
          if(ChkCfgStr("Name",CharBuff)){
            strcpy(Name,GetStartCfgStr(CharBuff));
          }

          if(ChkCfgStr("Keyboard",CharBuff)){
            Kbd = MyKeyb->GetKeyboardPtrByName(GetStartCfgStr(CharBuff));
          }

          if(ChkCfgStr("Key",CharBuff)){
            Key = GetIntCfgStr(CharBuff);
          }
          
          if(ChkCfgStr("Action",CharBuff)){
            Action = GetIntCfgStr(CharBuff);
          }
          
          if(ChkCfgStr("Modifier",CharBuff)){
            if(Action == 8){   
              GetHexCfgStr(CharBuff, Modifier); //if action = send midi - get hex string
            }else{
              Modifier[0] = GetIntCfgStr(CharBuff); // else action = get dec int 
              Modifier[1] = 0;
            }
            
          }
          
       }
       if(Action != 255 && Key != 255) {
         MyKeyb->AddNewHotkey(Name, Kbd, Key, Modifier, Action);
       }
      }
      
      if(strcmp(CharBuff,"[Colors]")==0){
        while(ReadStringFromSD(&myFile,CharBuff) >0){
          if(ChkCfgStr("ColorScreenRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorScreenRGB);
          }
          if(ChkCfgStr("ColorButtonRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorButtonRGB);
          }
          if(ChkCfgStr("ColorCaseRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorCaseRGB);
          }
        }
      }

      if(strcmp(CharBuff,"[Global]")==0){
        while(ReadStringFromSD(&myFile,CharBuff) >0){
            if(ChkCfgStr("FileProgram",CharBuff)){
            strcpy(FileProgram,GetStartCfgStr(CharBuff));
          }
        }
      }
    }
    myFile.close();

   ColorSetup();
   MyKeyb->SetupAllPrograms(GlobalProgramChangeMode);

   // Если используется цифровой датчик давления прикручиваем его к указанному аналоговому входу
   if(BMP280_Use == 1){
    if(Bmp280 == NULL){
      DebugPrintLn("BMP280_Used But not Ready!\nTry Init BMP280"); 
      Bmp280 = new BMP280_Sensor();    
      if(Bmp280 != NULL){
        DebugPrintLn("INIT BMP280 - Done"); 
      } else {
        DebugPrintLn("INIT BMP280 - Error!");
      }
    }
    if(MyKeyb->AnalogInputs[BMP280_Chanel] != NULL   && Bmp280 != NULL){            
      Bmp280->SetExternalSensor(MyKeyb->AnalogInputs[BMP280_Chanel]->SetExternalRawInputVal());
      DebugPrintLn("Activate BMP280 Sensor");
    }
   } 
    return 0; 
  } else {
      //MsgPrintWait("ERR Write!");
      return -1;
  }
}


//----------------------------------------------------------------------
// Load Config From SD card in number
int LoadConfigFromFlashNum(unsigned char Num){
      char FName[20];
      itoa(Num,FName,10);
      strcat(FName,".cfg");
      return LoadConfigFromFlash(FName);
}


//----------------------------------------------------------------------
// Load Config From SD card in EEPROM stored file number
int LoadConfigFromFlashEEPROM(){
      return LoadConfigFromFlashNum(EEPROM.read(AddressStartFile));
}


//----------------------------------------------------------------------
// Save Config To Flash SD card
int SaveConfigToFlash(char FName[]){

  File myFile;

  // Clear File
  SD.remove(FName);
  // Create/Open file
  myFile = SD.open(FName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {

      // Save AnalogIn
      for (int Counter = 0; Counter < MaxAnalogInputs; Counter++) {
        if(MyKeyb->AnalogInputs[Counter] != NULL){
          myFile.println("[analogin]");          
          SaveNameValueToSD(&myFile,"Input",MyKeyb->AnalogInputs[Counter]->Input);
          SaveNameValueToSD(&myFile,"Mode", MyKeyb->AnalogInputs[Counter]->Mode);
          if(MyKeyb->AnalogInputs[Counter]->Correction != 0) SaveNameValueToSD(&myFile,"Correction", MyKeyb->AnalogInputs[Counter]->Correction);
          if(MyKeyb->AnalogInputs[Counter]->Boost != 0) SaveNameValueToSD(&myFile,"Boost", MyKeyb->AnalogInputs[Counter]->Boost);
          if(MyKeyb->AnalogInputs[Counter]->CentralBoost != 0) SaveNameValueToSD(&myFile,"CentralBoost", MyKeyb->AnalogInputs[Counter]->CentralBoost);                   
          myFile.println();
        }
      }      
  
      // Save Midi Chanels
      for (int Counter = 0; Counter < MaxMidiChanels; Counter++) {
        if(MyKeyb->MidiChanels[Counter] != NULL){
          myFile.println("[chanel]");          
          SaveNameValueToSD(&myFile,"Num",MyKeyb->MidiChanels[Counter]->Chanel);
          SaveNameValueToSD(&myFile,"Volume",MyKeyb->MidiChanels[Counter]->Volume);
          SaveNameValueToSD(&myFile,"Pressure",MyKeyb->MidiChanels[Counter]->Pressure);
          SaveNameValueToSD(&myFile,"Pitch",MyKeyb->MidiChanels[Counter]->Pitch);
          SaveNameValueToSD(&myFile,"Velocity",MyKeyb->MidiChanels[Counter]->Velocity);
          SaveNameValueToSD(&myFile,"MSB",MyKeyb->MidiChanels[Counter]->MSB);
          SaveNameValueToSD(&myFile,"LSB",MyKeyb->MidiChanels[Counter]->LSB);
          SaveNameValueToSD(&myFile,"Prog",MyKeyb->MidiChanels[Counter]->Prog);
          myFile.println();

          int MCC=MyKeyb->MidiChanels[Counter]->GetMidiControllersCount();
          for (int Cntr = 0; Cntr < MCC; Cntr++) {
            myFile.println("<controller>");
            SaveNameValueToSD(&myFile,"Input",MyKeyb->MidiChanels[Counter]->Controllers[Cntr]->Data);
            //SaveNameValueToSD(&myFile,"Input",MyKeyb->GetAnalogInByAddr(MyKeyb->MidiChanels[Counter]->Controllers[Cntr]->Data->ExternalValue));            
            SaveNameValueToSD(&myFile,"MSB",MyKeyb->MidiChanels[Counter]->Controllers[Cntr]->GetMSB());
            SaveNameValueToSD(&myFile,"LSB",MyKeyb->MidiChanels[Counter]->Controllers[Cntr]->GetLSB());
            myFile.println();
          }
        }
      }

      // Save MyKeyb->Keyboards
      for (int Counter = 0; Counter < MaxKeyboards; Counter++) {
        if(MyKeyb->Keyboards[Counter] != NULL){
          myFile.println("[keyboard]");          
          SaveNameValueToSD(&myFile,"Name",MyKeyb->Keyboards[Counter]->Name);
          SaveNameValueToSD(&myFile,"ChanelNum",MyKeyb->Keyboards[Counter]->Chanel->Chanel);
          SaveNameValueToSD(&myFile,"BeginKey",MyKeyb->Keyboards[Counter]->KeyBegin);
          SaveNameValueToSD(&myFile,"EndKey",MyKeyb->Keyboards[Counter]->KeyEnd);
          SaveNameValueToSD(&myFile,"BaseNote",MyKeyb->Keyboards[Counter]->MidiBaseNote);
          SaveNameValueToSD(&myFile,"Increment",MyKeyb->Keyboards[Counter]->Increment);
          myFile.println();
        }
      }
      // Save Hotkeys
      for (int Counter = 0; Counter < MaxHotkeys; Counter++) {
        if(MyKeyb->Hotkeys[Counter] != NULL){
          myFile.println("[hotkey]");          
          SaveNameValueToSD(&myFile,"Name",MyKeyb->Hotkeys[Counter]->Name);
          if(MyKeyb->Hotkeys[Counter]->Keyb != NULL){  // save keyboard if keyboard exist
            SaveNameValueToSD(&myFile,"Keyboard",MyKeyb->Hotkeys[Counter]->Keyb->Name);
          }
          SaveNameValueToSD(&myFile,"Key", MyKeyb->Hotkeys[Counter]->Key);
          SaveNameValueToSD(&myFile,"Action", MyKeyb->Hotkeys[Counter]->Action);
          if(MyKeyb->Hotkeys[Counter]->Action == 8){ 
            SaveNameValueToSD(&myFile,"Modifier", MyKeyb->Hotkeys[Counter]->Modifier); // if action == send midi - write hex string
          }else{
            SaveNameValueToSD(&myFile,"Modifier", MyKeyb->Hotkeys[Counter]->Modifier[0]); // else - write dec value
          }
          myFile.println();
        }
      }


//######################################################################################
      // Save Colors
      myFile.println("[Colors]");
      SaveNameValueToSD(&myFile,"ColorScreenRGB", ColorScreenRGB, 3);
      SaveNameValueToSD(&myFile,"ColorButtonRGB", ColorButtonRGB, 3);
      SaveNameValueToSD(&myFile,"ColorCaseRGB", ColorCaseRGB, 3);
      myFile.println();
      
      // Save Global varibles
      myFile.println("[Global]");
      SaveNameValueToSD(&myFile,"FileProgram", FileProgram);
      myFile.println();
      
      myFile.close();
      return 0;  
  } else {
    //MsgPrintWait("ERR Write!");
      return -1;
  }
}


// Save to SD card system Keyboard Settings
int SaveKeybToFlash(char FName[]){

  File myFile;
  
  // Clear File
  SD.remove(FName);
  // Create/Open file
  myFile = SD.open(FName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
      myFile.println("[keybcfg]");
      SaveNameValueToSD(&myFile,"KeyInputPinStart",  KeyInputPinStart);
      SaveNameValueToSD(&myFile,"KeyInputPinStop",   KeyInputPinStop);
      SaveNameValueToSD(&myFile,"KeyOutputPinStart", KeyOutputPinStart);
      SaveNameValueToSD(&myFile,"KeyOutputPinStop",  KeyOutputPinStop);
      SaveNameValueToSD(&myFile,"Normal_Pin_State",  Normal_Pin_State);
      SaveNameValueToSD(&myFile,"ScanCodeNums",      ScanCodeNums);      
      myFile.println();
      
      myFile.println("[scancodes]"); 
      for (int Counter = 1; Counter <= ScanCodeNums; Counter++) {
          SaveNameValueToSD(&myFile,"Key", KeyCodes[Counter]);
      }
      myFile.println();
      myFile.close();
      return 0; 
  } else {
      //MsgPrintWait("ERR Write!");
      return -1;
  }
}


int LoadKeybFromFlash(char FName[]){
  char   CharBuff[40];
  File myFile;

  myFile = SD.open(FName, FILE_READ);
        
  // if the file opened okay, read line by line
  if (myFile) {
    DebugPrintLn("Keyboard config file opened");
    while (ReadStringFromSD(&myFile,CharBuff) != -1) {
      //Analog Input
      if(strcmp(CharBuff,"[keybcfg]")==0){
        while(ReadStringFromSD(&myFile,CharBuff) >0){
      
          if(ChkCfgStr("KeyInputPinStart",CharBuff)){
            KeyInputPinStart = GetIntCfgStr(CharBuff);
            DebugPrint("KeyInputPinStart ");
            DebugPrintLn(KeyInputPinStart);
          }
          if(ChkCfgStr("KeyInputPinStop",CharBuff)){
            KeyInputPinStop = GetIntCfgStr(CharBuff);
            DebugPrint("KeyInputPinStop ");
            DebugPrintLn(KeyInputPinStop);
          }
          if(ChkCfgStr("KeyOutputPinStart",CharBuff)){
            KeyOutputPinStart = GetIntCfgStr(CharBuff);
            DebugPrint("KeyOutputPinStart ");
            DebugPrintLn(KeyOutputPinStart);            
          }
          if(ChkCfgStr("KeyOutputPinStop",CharBuff)){
            KeyOutputPinStop = GetIntCfgStr(CharBuff);
            DebugPrint("KeyOutputPinStop ");
            DebugPrintLn(KeyOutputPinStop);              
          }
          if(ChkCfgStr("Normal_Pin_State",CharBuff)){
            Normal_Pin_State = GetIntCfgStr(CharBuff);
            DebugPrint("Normal_Pin_State ");
            DebugPrintLn(Normal_Pin_State);  
          }
          if(ChkCfgStr("ScanCodeNums",CharBuff)){
            ScanCodeNums = GetIntCfgStr(CharBuff);
            DebugPrint("ScanCodeNums ");
            DebugPrintLn(ScanCodeNums);  

          }
          if(ChkCfgStr("ColorScreenRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorScreenRGB);
          }
          if(ChkCfgStr("ColorButtonRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorButtonRGB);
          }
          if(ChkCfgStr("ColorCaseRGB",CharBuff)){
            GetHexCfgStr(CharBuff, ColorCaseRGB);
          }
          if(ChkCfgStr("BMP280_Use",CharBuff)){
            BMP280_Use = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("BMP280_Sensitivity",CharBuff)){
            BMP280_Sensitivity = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("BMP280_Chanel",CharBuff)){
            BMP280_Chanel = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("BMP280_AutoAdjust",CharBuff)){
            BMP280_AutoAdjust = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("BMP280_AutoAdjust_Limit",CharBuff)){
            BMP280_AutoAdjust_Limit = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Enc_Type",CharBuff)){
            Enc_Type = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("Enc_Direction",CharBuff)){
            Enc_Direction = GetIntCfgStr(CharBuff);
          }
          if(ChkCfgStr("LCD_rotation",CharBuff)){
            LCD_rotation = GetIntCfgStr(CharBuff);
          }
        }
        continue;
      }
      if(strcmp(CharBuff,"[scancodes]")==0){
        unsigned int Counter = 1;
        while(ReadStringFromSD(&myFile,CharBuff) >0){
          if(ChkCfgStr("Key",CharBuff)){
            KeyCodes[Counter] = GetIntCfgStr(CharBuff);
            DebugPrint("Add key num: ");
            DebugPrintLn(KeyCodes[Counter]);             
            Counter++;
          }
        }
      }
    }
    myFile.close();
    return 0; 
  } else {
      //MsgPrintWait("ERR Write!");
      return -1;
  }
}


//////////////////////////////////////////////////////////////////////////
// Start with default example configuration
int StartWithDefaultCfg(){
  DebugPrintLn("Load with default config"); 
  
  if(MyKeyb != NULL){
    delete(MyKeyb);
    MyKeyb = NULL;
  }
  MyKeyb = new KeyboardS();
  
  //Default test config  
  //Test bayan defaults
  unsigned char  Mode[10]={0x01,0x20,0x40,0x60,0x80,0xA0,0xC0,0xE0,0xFF,0}; //flat line config
  //MyKeyb->AddNewAnalogIn(Input,Mode,Correction,Boost,CentralBoost);
  MyKeyb->AddNewAnalogIn(0,Mode,0,0,0); // Flat Line
  MyKeyb->AddNewAnalogIn(1,Mode,0,0,0); // Flat Line 
  MyKeyb->AddNewAnalogIn(2,Mode,0,0,0); // Flat Line
  
  //AddNewMidiChanel(unsigned char Chanel,unsigned char Volume, unsigned char Pressure, unsigned char Pitch, unsigned char Velocity, unsigned char Prog, unsigned char MSB, unsigned char LSB);  
  MyKeyb->AddNewMidiChanel(0,127,127,64,127,0,0,0);  
  MyKeyb->AddNewMidiChanel(1,127,127,64,127,0,0,0);  
  MyKeyb->AddNewMidiChanel(2,127,127,64,127,0,0,0);  
  
  //Add new Controller AddNewController(LinkToExternalValue, unsigned char initMSB, unsigned char initLSB)
  MyKeyb->MidiChanels[0]->AddNewController(&MyKeyb->AnalogInputs[0]->Value, 1,33);
  
  //AddNewKeyboard(char Name[], unsigned char FierstKey, unsigned char LastKey, unsigned char MidiChanel, unsigned char MidiBaseNote, unsigned char  MidiVelocity, unsigned char  Increment )
  MyKeyb->AddNewKeyboard("Right",       1, 52, MyKeyb->MidiChanels[0], 46, 1);
  MyKeyb->AddNewKeyboard("Bass",       53, 64, MyKeyb->MidiChanels[1], 41, 1);
  MyKeyb->AddNewKeyboard("Chords",     65, 76, MyKeyb->MidiChanels[2], 55, 1);
  
  //Hotkey(char initName[], Keyboard * initKeyb,  unsigned char initKey, unsigned char initModifier[], unsigned char  initAction)
  unsigned char Modifier[2] = {1,0};
  MyKeyb->AddNewHotkey("AllMute",    MyKeyb->Keyboards[0], 77, Modifier,  1);
}










#endif
