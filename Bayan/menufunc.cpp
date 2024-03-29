#include <U8glib.h>
#include <stdlib.h>
#include "config.h"
#include "keyboards.h"
#include "menufunc.h"
#include "beeper.h"
#include "lcd.h"
#include "keybloadsave.h"
#include "control.h"
#include "drummachine.h"




//----------------------------------------------------------------------
// Get Substring Menu
bool GetMenuSubstr(int Num, char InputStr[], char ResultString[]) {

  //Get string start position
  int Start = 0;
  if (Num != 0) {
    int LineCount = 0;
    for (int counter = 0; InputStr[counter] != 0; counter++ ) {
      if (InputStr[counter] == '>') { // count of separator line
        LineCount++;
        if (LineCount == Num) { // if finded separator Num
          Start = counter + 1;
          break;
        }
      }
    }
    if (Start == 0) { // if EOL and start position not finded result - empty string
      ResultString[0] = 0;
      return false;
    }
  } else {  //if need header.
    Start = 0;
  }

  // Copy substring from the begin to the ">" or EOL
  int CounterInput = Start;
  int CounterResult = 0;
  while ((InputStr[CounterInput] != 0) && (InputStr[CounterInput] != '>')) {
    ResultString[CounterResult] = InputStr[CounterInput];
    CounterInput++;
    CounterResult++;
  }
  ResultString[CounterResult] = 0; // put EOL in string
  return true;
}


//----------------------------------------------------------------------
// Random menu
void MenuPrint(int Choise, char MenuSttring[]) {

  char TempString[MaxMenuStringWide];

  int Page = (Choise - 1) / (MaxMenuString - 1); // Get Menu Page

  int CursorPos = Choise % (MaxMenuString - 1); // Get Cursor position on page
  if (CursorPos == 0) CursorPos = MaxMenuString - 1; // yes bad hack

  u8g.firstPage();
  do {
    // Print Header inversed collor
    u8g.setColorIndex(1);
    u8g.drawBox(0, 0, LCD_WIDTH, FontHigh + 2);
    u8g.drawFrame(0, 0, LCD_WIDTH, LCD_HEIGHT);

    u8g.setColorIndex(0);
    u8g.setPrintPos(1, (FontHigh + 1));
    GetMenuSubstr (0, MenuSttring, TempString);
    u8g.print(TempString);


    u8g.setColorIndex(1);

    // Print Menu item
    int PageStartNum = Page * (MaxMenuString - 1);
    for (int StringCount = 1; StringCount < MaxMenuString; StringCount++ ) {

      GetMenuSubstr (PageStartNum + StringCount, MenuSttring, TempString);

      u8g.setPrintPos(6, (FontHigh + 2) * (StringCount + 1));
      u8g.print(TempString);
    }


    if (Choise != 0) {
      //Print Cursor
      u8g.drawBox(1, ((FontHigh + 2)*CursorPos) + 1, 4, FontHigh + 2);
    }
  } while ( u8g.nextPage() );

}

//----------------------------------------------------------------------
// Get the state of buttons do not wait just check
int GetButtonState() {
  Encodr.tick();
  int result = 0;
  if (Encodr.isPress()) {
    result = result | 0b00001;
  }

  if (Encodr.isRight()) {
    result = result | 0b00010;
  }

  if (Encodr.isLeft()) {
    result = result | 0b00100;
  }
  if (digitalRead(BTN_RST) == 0) {
    result = result | 0b01000;
  }
  return  result;
}


//----------------------------------------------------------------------
// Get the state of pressed buttons, wait for press
int GetButtonStateWait() {
  int Keys = 0;
  while (Keys == 0) {
    Keys = GetButtonState();
  }
  return Keys;
}




//----------------------------------------------------------------------
// Random menu
int Menu(int DefaultChoise, char MenuSttring[]) {

  int Choise = DefaultChoise;


  int LineCount = 0;
  // get menu lines in menu string
  for (int counter = 0; MenuSttring[counter] != 0; counter++ ) {
    if (MenuSttring[counter] == '>') {
      LineCount++;
    }
  }

  while (1) {
    MenuPrint(Choise, MenuSttring);



    // key scan cycle
    while (1) {
      if (Encodr.isLeft()) {
        Klick();
        if (Choise < LineCount) {
          Choise++;
          break;
        }
      }

      if (Encodr.isRight()) {
        Klick();
        if (Choise > 1) {
          Choise--;
          break;
        }
      }

      if (Encodr.isPress()) {
        Klick();
        return Choise;
      }

      if(NeedLoad != 0){
        LoadConfigFromFlashEEPROM();
        NeedLoad = 0;
      }
      
      //////////////////////////////////
      //Play at Menu
      MyKeyb->Play();
      Encodr.tick();
    }
  }
}


//----------------------------------------------------------------------
//return user Key
unsigned char MenuGetKey(char header[]) {
  while (1) {
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawBox(0, 0, LCD_WIDTH - 1, FontHigh + 2);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
      u8g.setColorIndex(0);
      u8g.setPrintPos(1, (FontHigh + 1));
      u8g.print(header);

      u8g.setColorIndex(1);
      u8g.setPrintPos(10, ((FontHigh + 2) * 4) - 1);
      u8g.print("Press a key");
    } while ( u8g.nextPage() );

    unsigned char Key = MyKeyb->GetKey();

    if (Key != 255) {
      return Key;
    }
  }
}


//----------------------------------------------------------------------
//display user select
void MenuDisplaySelect(char header[], int16_t MaxValue, int16_t CurrentValue, char CharBuff[]){


      //screen refresh
      u8g.firstPage();
      do {
        u8g.setColorIndex(1);
        u8g.drawBox(0, 0, LCD_WIDTH - 1, FontHigh + 2);
        u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        u8g.setColorIndex(0);
        u8g.setPrintPos(1, (FontHigh + 1));
        u8g.print(header);
  
        u8g.setColorIndex(1);
        u8g.setPrintPos(3, (FontHigh+1)*2);
        u8g.print(CurrentValue, DEC);
        u8g.print(" /");    
        u8g.setPrintPos(50,(FontHigh+1)*2);
        u8g.print(MaxValue, DEC);

        u8g.setPrintPos(3, (FontHigh+1)*3);
        u8g.print(CharBuff);
      } while ( u8g.nextPage() );

}



//----------------------------------------------------------------------
//return user select 1-5
int MenuGetInt(char header[], int StartValue, int MinValue, int MaxValue) {
  int CurrentValue = StartValue;

  while (1) {
    //screen refresh
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawBox(0, 0, LCD_WIDTH - 1, FontHigh + 2);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
      u8g.setColorIndex(0);
      u8g.setPrintPos(1, (FontHigh + 1));
      u8g.print(header);

      u8g.setColorIndex(1);

      u8g.drawFrame(40, (FontHigh + 2) * 3, 45, FontHigh + 4);
      u8g.setPrintPos(50, ((FontHigh + 2) * 4));
      u8g.print(CurrentValue, DEC);
    } while ( u8g.nextPage() );

    // key scan cycle
    while (1) {
      Encodr.tick();
      if (Encodr.isLeft()) {
        Klick();
        CurrentValue++;
        if (CurrentValue > MaxValue) {
          CurrentValue = MinValue;
        }
        break;
      }

      if (Encodr.isRight()) {
        Klick();
        CurrentValue--;
        if (CurrentValue < MinValue) {
          CurrentValue = MaxValue;
        }
        break;
      }

      if (Encodr.isPress()) {
        Klick();
        return CurrentValue;
      }
    }
  }
}



//----------------------------------------------------------------------
//return user select 1-5
int MenuGetIntHEX(char header[], int StartValue, int MinValue, int MaxValue) {
  int CurrentValue = StartValue;

  while (1) {
    //screen refresh
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawBox(0, 0, LCD_WIDTH - 1, FontHigh + 2);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
      u8g.setColorIndex(0);
      u8g.setPrintPos(1, (FontHigh + 1));
      u8g.print(header);

      u8g.setColorIndex(1);

      u8g.drawFrame(40, (FontHigh + 2) * 3, 45, FontHigh + 4);
      u8g.setPrintPos(50, ((FontHigh + 2) * 4));
      u8g.print(CurrentValue, HEX);
    } while ( u8g.nextPage() );

    // key scan cycle
    while (1) {
      Encodr.tick();
      if (Encodr.isLeft()) {
        Klick();
        CurrentValue++;
        if (CurrentValue > MaxValue) {
          CurrentValue = MinValue;
        }
        break;
      }

      if (Encodr.isRight()) {
        Klick();
        CurrentValue--;
        if (CurrentValue < MinValue) {
          CurrentValue = MaxValue;
        }
        break;
      }

      if (Encodr.isPress()) {
        Klick();
        return CurrentValue;
      }
    }
  }
}

//----------------------------------------------------------------------
//return user select but under zero values displayed as A1,A2 etc
int MenuGetIntAndAnalog(char header[], int StartValue, int MinValue, int MaxValue) {
  int CurrentValue = StartValue;

 
  while (1) {
     char TempStr[10]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
     InsertIntAnalogToStr(TempStr,0, CurrentValue);
          
/*    if(CurrentValue>=0){  //if positive or zero
      itoa(CurrentValue, TempStr, 10);  //value is number
    }
    else{                 // if negative value
      int TempVal=CurrentValue+1; //  make string -1 -> A0, -2 -> A1
      if (TempVal==0){
        SubStrCopy(TempStr,0,"A0"); 
      }else{
        itoa(TempVal, TempStr, 10);  //value is number
        if(TempStr[0]=='-'){
          TempStr[0]='A';
        }
      }         
    }/**/
    //screen refresh
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawBox(0, 0, LCD_WIDTH - 1, FontHigh + 2);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
      u8g.setColorIndex(0);
      u8g.setPrintPos(1, (FontHigh + 1));
      u8g.print(header);

      u8g.setColorIndex(1);

      u8g.drawFrame(40, (FontHigh + 2) * 3, 45, FontHigh + 4);
      u8g.setPrintPos(50, ((FontHigh + 2) * 4));
      u8g.print(TempStr);
    } while ( u8g.nextPage() );

    // key scan cycle
    while (1) {
      Encodr.tick();
      if (Encodr.isLeft()) {
        Klick();
        CurrentValue++;
        if (CurrentValue > MaxValue) {
          CurrentValue = MinValue;
        }
        break;
      }

      if (Encodr.isRight()) {
        Klick();
        CurrentValue--;
        if (CurrentValue < MinValue) {
          CurrentValue = MaxValue;
        }
        break;
      }

      if (Encodr.isPress()) {
        Klick();
        return CurrentValue;
      }
    }
  }
}

//----------------------------------------------------------------------
// Change mode for Analog inputs
unsigned int ChangeModeAnalogIn(int AnalogIn) {

  boolean CursorMode = true;  // true-navigate, false-edit 
  char CursorPos = AnalogPoints;
  unsigned int OldValue =0;
   
  while (1) {
   if(MyKeyb->AnalogInputs[AnalogIn]->Value != OldValue){ //If no change dont update screen
      OldValue = MyKeyb->AnalogInputs[AnalogIn]->Value;
      //screen update
      u8g.firstPage();
      do {
        
        u8g.setPrintPos(5, 11);
        u8g.print("Out: ");
        u8g.setPrintPos(50, 11);
        u8g.print(MyKeyb->AnalogInputs[AnalogIn]->Value, DEC);
        
        if (!CursorMode){
          u8g.setPrintPos(5, 21);
          u8g.print("Val: ");
          u8g.setPrintPos(50, 21);
          u8g.print(MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos], DEC);
        }

  
        //рисуем график кривой
        int OldPos = 63 - MyKeyb->AnalogInputs[AnalogIn]->RawToData(0) / 2;
        int Pos;
        for(int Counter=16;Counter<129;Counter+=16){
          Pos =   63 - MyKeyb->AnalogInputs[AnalogIn]->RawToData(Counter*8) / 2;
          u8g.drawLine(Counter-16,OldPos,Counter,Pos);
          OldPos=Pos;
        
          u8g.setPrintPos(5, 31);
          if( CursorPos == AnalogPoints){
            u8g.print("Exit");
          }else
          if( CursorPos == AnalogPoints+1){
            u8g.print("Freeze");
          }
          else{
            if (CursorMode){
              u8g.print(CursorPos, DEC);                                
              u8g.drawBox((CursorPos*16)-3, 64-(MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos]/4)-4,  6, 8); 
    
            }else{
              u8g.print("Change");           
              u8g.drawBox((CursorPos*16)-3, 64-(MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos]/4)-2,  6, 4);
            }
          }
        }
  
        //рисуем выходное значение
        Pos = MyKeyb->AnalogInputs[AnalogIn]->Value;
        Pos=  63-(Pos/2);
        u8g.drawLine(0,Pos,127,Pos);
  
        //рисуем положение аналогового входа
        Pos=MyKeyb->AnalogInputs[AnalogIn]->GetRawInputVal();
        Pos=Pos/8;
        u8g.drawLine(Pos,0,Pos,63);

      } while ( u8g.nextPage() );
      
   } 

    MyKeyb->Play();// даже тут играем ноты (на самом деле обновлем значение Value)  
    Encodr.tick();
    
    if (Encodr.isLeft()) {
      OldValue = 200; // update screen flag
      Klick();
      if( CursorMode ){ //если в режиме курсора циклически  меняем позицию
        CursorPos++;
        if (CursorPos > AnalogPoints+1) { 
          CursorPos = 0;
        }
      }else{                //если в режиме редактирования правим настройки кривой
        MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos]-=4;
      }  
    }

    if (Encodr.isRight()) {
      OldValue = 200; // update screen flag
      Klick();
      if( CursorMode ){ //если в режиме курсора циклически  меняем позицию
        CursorPos--;
        if (CursorPos < 0) { 
          CursorPos = AnalogPoints+1;
        }
      }else{                //если в режиме редактирования правим настройки кривой
        MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos]+=4;
      }  
    }

    if (Encodr.isPress()) {
      OldValue = 200; // update screen flag
      Klick();
      if(CursorPos == AnalogPoints){
        return 0;
      }else
      if(CursorPos == AnalogPoints+1){
        MyKeyb->AnalogInputs[AnalogIn]->Freeze = (!MyKeyb->AnalogInputs[AnalogIn]->Freeze);
      }
      else{
        if( MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos] == 0){ // zero not allowed
          MyKeyb->AnalogInputs[AnalogIn]->Mode[CursorPos] = 1;
        }
        CursorMode = !CursorMode;
      }
    }

  }
  /**/
}

//----------------------------------------------------------------------
// Edit string on screen
void MenuEditHEXString(char Str[], int MaxLen) {

  char *TempStr = NULL;  // new pointer
  TempStr = new char [MaxLen + 1]; // get mem

  TempStr[0] = 0;
  strcat(TempStr, Str);

  //add space symb to TempStr 
  for (int Counter = strlen(TempStr);Counter<MaxLen;Counter++){
    TempStr[Counter]=' ';
  }
  TempStr[MaxLen]=0;

  int       XStartPos = 3;
  int       YStartPos = (LCD_HEIGHT / 2) + (FontHigh / 2);
  int       CursorPos = 0;
  boolean   CursorMode = false;  // false-navigate true-Edit

  while (1) {
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

      u8g.setPrintPos(XStartPos, YStartPos);
      u8g.print(TempStr);


      if (CursorPos == MaxLen) {
        u8g.setPrintPos(0, FontHigh+2);
        u8g.print("Save?");
      }
      else {
        if (!CursorMode) {
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos - FontHigh - 3 , FontWidth - 1, 3);
        }
        else {
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos - FontHigh - 3 , FontWidth - 1, 3);
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos + 1,             FontWidth - 1, 3);
        }
      }

    } while ( u8g.nextPage() );


    int Keys = GetButtonStateWait();


    if (!CursorMode) { // if in "navigate" mode

      //Pressed Left
      if (Keys & 0b00010) {
        CursorPos--;
        if (CursorPos < 0) {
          CursorPos = MaxLen;
        }
      }

      //Pressed Right
      if (Keys & 0b00100) {
        CursorPos++;
        if (CursorPos > (MaxLen)) {
          CursorPos = 0;
        }
      }

      //Pressed Ok
      if (Keys & 0b00001) {
        if (CursorPos == MaxLen) {
          
          //trunk spaces at the end
          int Counter = MaxLen-1;
          while(Counter>=0){
            if (TempStr[Counter]==' '){ //!!!!!!!!!!!!!!!!!!!!!!!
              TempStr[Counter]=0;
            }
            Counter--;
          }

          //Copy new string to Result
          Str[0] = 0;
          strcat(Str, TempStr);
          
          //Clear memory
          delete [] TempStr;    // Free memory
          TempStr = NULL;       // указатель заменяется на нуль-указатель
          break;
        }
        else {
          CursorMode = true; // Go in "edit" Mode
        }
      }
    }
    else { //if in "edit" mode
      //Pressed Right
      if (Keys & 0b00100) {
        if(TempStr[CursorPos]=='9'){
          TempStr[CursorPos]='A';
        }
        else if(TempStr[CursorPos]=='F'){
          TempStr[CursorPos]=' ';
        }
        else if(TempStr[CursorPos]==' '){
          TempStr[CursorPos]='0';
        }
        else{
          TempStr[CursorPos]++;
        }
      }

      //Pressed Left
      if (Keys & 0b00010) {
        if(TempStr[CursorPos]=='A'){
          TempStr[CursorPos]='9';
        }
        else if(TempStr[CursorPos]=='0'){
          TempStr[CursorPos]=' ';
        }
        else if(TempStr[CursorPos]==' '){
          TempStr[CursorPos]='F';
        }
        else{
          TempStr[CursorPos]--;
        }
      }

      //Pressed Ok
      if (Keys & 0b00001) {
        CursorMode = false; // Go in "navigate" Mode
      }
    }
  }
}

//----------------------------------------------------------------------
// Edit string on screen
void MenuEditString(char Str[], int MaxLen) {

  char *TempStr = NULL;  // new pointer
  TempStr = new char [MaxLen + 1]; // get mem

  TempStr[0] = 0;
  strcat(TempStr, Str);

  //add space symb to TempStr 
  for (int Counter = strlen(TempStr);Counter<MaxLen;Counter++){
    TempStr[Counter]=' ';
  }
  TempStr[MaxLen]=0;

  int XStartPos = 3;
  int YStartPos = (LCD_HEIGHT / 2) + (FontHigh / 2);
  int CursorPos = 0;
  boolean CursorMode = false;  // 0-navigate 1-Edit

  while (1) {
    u8g.firstPage();
    do {
      u8g.setColorIndex(1);
      u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

      u8g.setPrintPos(XStartPos, YStartPos);
      u8g.print(TempStr);


      if (CursorPos == MaxLen) {
        u8g.setPrintPos(0, FontHigh+2);
        u8g.print("Save?");
      }
      else {
        if (!CursorMode) {
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos - FontHigh - 3 , FontWidth - 1, 3);
        }
        else {
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos - FontHigh - 3 , FontWidth - 1, 3);
          u8g.drawBox(XStartPos + (CursorPos * FontWidth) + 1, YStartPos + 1,             FontWidth - 1, 3);
        }
      }

    } while ( u8g.nextPage() );


    int Keys = GetButtonStateWait();


    if (!CursorMode) { // if in "navigate" mode

      //Pressed Left
      if (Keys & 0b00010) {
        CursorPos--;
        if (CursorPos < 0) {
          CursorPos = MaxLen;
        }
      }

      //Pressed Right
      if (Keys & 0b00100) {
        CursorPos++;
        if (CursorPos > (MaxLen)) {
          CursorPos = 0;
        }
      }

      //Pressed Ok
      if (Keys & 0b00001) {
        if (CursorPos == MaxLen) {
          
          //trunk spaces at the end
          int Counter = MaxLen-1;
          while(Counter>=0){
            if (TempStr[Counter]!=' '){
              TempStr[Counter+1]=0;
              break;
            }
            Counter--;
          }

          //Copy new string to Result
          Str[0] = 0;
          strcat(Str, TempStr);
          
          //Clear memory
          delete [] TempStr;    // Free memory
          TempStr = NULL;       // указатель заменяется на нуль-указатель
          break;
        }
        else {
          CursorMode = true; // Go in "edit" Mode
        }
      }
    }
    else { //if in "edit" mode



      //Pressed Right
      if (Keys & 0b00100) {
        if(TempStr[CursorPos]=='9'){
          TempStr[CursorPos]='A';
        }
        else if(TempStr[CursorPos]=='Z'){
          TempStr[CursorPos]='a';
        }
        else if(TempStr[CursorPos]=='z'){
          TempStr[CursorPos]=' ';
        }
        else if(TempStr[CursorPos]==' '){
          TempStr[CursorPos]='0';
        }
        else{
          TempStr[CursorPos]++;
        }
      }

      //Pressed Left
      if (Keys & 0b00010) {
        if(TempStr[CursorPos]=='A'){
          TempStr[CursorPos]='9';
        }
        else if(TempStr[CursorPos]=='a'){
          TempStr[CursorPos]='Z';
        }
        else if(TempStr[CursorPos]=='0'){
          TempStr[CursorPos]=' ';
        }
        else if(TempStr[CursorPos]==' '){
          TempStr[CursorPos]='z';
        }
        else{
          TempStr[CursorPos]--;
        }
        
      }

      //Pressed Ok
      if (Keys & 0b00001) {
        CursorMode = false; // Go in "navigate" Mode
      }
    }
  }
}




//----------------------------------------------------------------------
// MSG Print on screen
void MsgPrint(char Message[]) {

  int XStartPos = (LCD_WIDTH - (strlen(Message) * FontWidth)) / 2; // Get Width of string in pixels and Calculate start position of X

  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    u8g.drawBox(10, 20, 108, 25);

    u8g.setColorIndex(0);
    u8g.setPrintPos(XStartPos, 35);
    u8g.print(Message);
    u8g.setPrintPos(XStartPos + 1, 35);
    u8g.print(Message);
    u8g.setColorIndex(1);

  } while ( u8g.nextPage() );
  delay(DelayNorm);
}


void MsgPrintNoWait(char Message[]) {

  int XStartPos = (LCD_WIDTH - (strlen(Message) * FontWidth)) / 2; // Get Width of string in pixels and Calculate start position of X

  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.drawFrame(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    u8g.drawBox(10, 20, 108, 25);

    u8g.setColorIndex(0);
    u8g.setPrintPos(XStartPos, 35);
    u8g.print(Message);
    u8g.setPrintPos(XStartPos + 1, 35);
    u8g.print(Message);
    u8g.setColorIndex(1);

  } while ( u8g.nextPage() );
}

//----------------------------------------------------------------------
// MSG The number
void MsgPrintInt(int Num) {
  char TempStr[10];
  itoa(Num, TempStr, 10);
  MsgPrint(TempStr);
}


void WaitButtonPress(){
    while (1){
      Encodr.tick();
      if(Encodr.isPress()) break;
    }
}

//----------------------------------------------------------------------
// MSG And Wait
void MsgPrintWait(char Message[]) {
  MsgPrint(Message);
  WaitButtonPress();
}

void(* resetFunc) (void) = 0;//declare reset function at address 0
