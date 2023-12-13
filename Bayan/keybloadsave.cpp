#include <EEPROM.h>
#include <SD.h>
#include "config.h"
#include "keyboards.h"
#include "control.h"
#include "debug.h"
#include "lcd.h"


// Сохранение в указанный файл пары имя - значение для числа
void SaveNameValueToSD(File *myFile, char Name[], int Value){
          myFile->print(Name);
          myFile->print(" ");
          myFile->print(Value,DEC);
          myFile->println();          
}

// Сохранение в указанный файл пары имя - значение для строки
void SaveNameValueToSD(File *myFile, char Name[], char Value[]){
          myFile->print(Name);
          myFile->print(" ");
          myFile->print(Value);
          myFile->println();          
}


// Сохранение в указанный файл пары имя - значение для массива (значения сохранит как hex разделенные пробелом)
void SaveNameValueToSD(File *myFile, char Name[], unsigned char Value[]){
          myFile->print(Name);
          myFile->print(" ");
          
          for(unsigned char Index=0; Value[Index]!=0; Index++){
            myFile->print(Value[Index],HEX);
            myFile->print(" ");
          }  
          myFile->println();                  
}


// Сохранение в указанный файл пары имя - значение для массива фиксированной длины (значения сохранит как hex разделенные пробелом)
void SaveNameValueToSD(File *myFile, char Name[], unsigned char Value[], unsigned char Count){
          myFile->print(Name);
          myFile->print(" ");
          
          for(unsigned char Index=0; Index < Count; Index++){
            myFile->print(Value[Index],HEX);
            myFile->print(" ");
          }  
          myFile->println();                  
}

// Сохранение в указанный файл пары имя - значение для значений типа DataBox (значение сохранит как число или как номер аналогового входа)
void SaveNameValueToSD(File *myFile, char Name[], DataBox *Value){
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

//Считывает одну строку из указанного файла, внимание, функция не проверяет выход за пределы строки! 
int ReadStringFromSD(File *myFile, char Str[]){
    int Counter = 0; 
    char ch;
    
    Str[0]=0; //clear string

    if(!myFile){
      return -1;
    }

    //check 
    if(!myFile->available()){
      DebugPrintLn("End of file");
      return -1;
    }
 
    while(myFile->available()){
        ch = myFile->read();
        if (ch=='\n') break;
        if (ch=='\r') continue;
        Str[Counter]=ch;
        Counter++;
        
    }
    Str[Counter]=0;

    return (strlen(Str));  
}


// Возвращает ссылку на строку после имени параметра и пробела
char *GetStartCfgStr (char Str[]){
  int Counter;
  char *Num;

  for(Counter=0;Str[Counter]!=0;Counter++){
    if (Str[Counter]==' '){
      return &Str[Counter]+1;
    }
  }
}

// Функция для разбора конфигурации получает строку с именем и десятичным значением, возвращает десятичное значение
int GetIntCfgStr(char *Str){
  int Result;
  char *Num;

  //search start number
  for(int Counter=0;Str[Counter]!=0 && Counter<MaxFileStringLen;Counter++){
    if (Str[Counter]==' '){
      Num=Str+Counter+1;
      break;
    }
  }

  Result = atoi(GetStartCfgStr(Num));
  return Result;
}


// Функция для разбора конфигурации получает строку с именем и десятичным значением, возвращает десятичное значение
unsigned char GetInt8CfgStr (char Str[]){
  unsigned char Result;
  Result = atoi(GetStartCfgStr(Str));
  return Result;
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
  char *Start = GetStartCfgStr(Str);
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
  char   CharBuff[MaxFileStringLen];
  char   Mode = 0;
  File myFile;
  MidiChanel* LastChanel;
 
  myFile = SD.open(FName, FILE_READ);
  DebugPrint("Try read preset file ");  
  DebugPrintLn(FName);
        
  // if the file opened okay, read line by line
  if (myFile) {
    DebugPrintLn("File open success, start parsing preset"); 
  
    if(MyKeyb == NULL){
      MyKeyb = InitKeyboard(MyKeyb);
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
      DebugPrintLn("Can't open file!");
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
      SaveNameValueToSD(&myFile,"KeyInputPinStart",  MyKeyb->KeyInputPinStart);
      SaveNameValueToSD(&myFile,"KeyInputPinStop",   MyKeyb->KeyInputPinStop);
      SaveNameValueToSD(&myFile,"KeyOutputPinStart", MyKeyb->KeyOutputPinStart);
      SaveNameValueToSD(&myFile,"KeyOutputPinStop",  MyKeyb->KeyOutputPinStop);
      SaveNameValueToSD(&myFile,"Normal_Pin_State",  MyKeyb->Normal_Pin_State);
      SaveNameValueToSD(&myFile,"ScanCodeNums",      MyKeyb->ScanCodeNums);      
      myFile.println();
      
      myFile.println("[scancodes]"); 
      for (int Counter = 1; Counter <= MyKeyb->ScanCodeNums; Counter++) {
          SaveNameValueToSD(&myFile,"Key", MyKeyb->KeyCodes[Counter]);
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
  char   CharBuff[MaxFileStringLen];
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
            MyKeyb->KeyInputPinStart = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("KeyInputPinStart ");
            DebugPrintLn(MyKeyb->KeyInputPinStart);
          }
          if(ChkCfgStr("KeyInputPinStop",CharBuff)){
            MyKeyb->KeyInputPinStop = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("KeyInputPinStop ");
            DebugPrintLn(MyKeyb->KeyInputPinStop);
          }
          if(ChkCfgStr("KeyOutputPinStart",CharBuff)){
            MyKeyb->KeyOutputPinStart = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("KeyOutputPinStart ");
            DebugPrintLn(MyKeyb->KeyOutputPinStart);            
          }
          if(ChkCfgStr("KeyOutputPinStop",CharBuff)){
            MyKeyb->KeyOutputPinStop = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("KeyOutputPinStop ");
            DebugPrintLn(MyKeyb->KeyOutputPinStop);              
          }
          if(ChkCfgStr("Normal_Pin_State",CharBuff)){
            MyKeyb->Normal_Pin_State = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("Normal_Pin_State ");
            DebugPrintLn(MyKeyb->Normal_Pin_State);  
          }
          if(ChkCfgStr("ScanCodeNums",CharBuff)){
            MyKeyb->ScanCodeNums = (unsigned char)GetIntCfgStr(CharBuff);
            DebugPrint("ScanCodeNums ");
            DebugPrintLn(MyKeyb->ScanCodeNums);  

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
            BMP280_Use = (unsigned char) GetIntCfgStr(CharBuff);
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
            MyKeyb->KeyCodes[Counter] = (unsigned char) GetIntCfgStr(CharBuff);
            DebugPrint("Add key num: ");
            DebugPrintLn(MyKeyb->KeyCodes[Counter]);             
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
