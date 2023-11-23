#include "midi.h"
#include "lcd.h"
#include "keyboards.h"
#include "sdcard.h"
#include "menufunc.h"
#include "tests.h"
#include "control.h"

//----------------------------------------------------------------------
int TestScancode() {
  unsigned char Keys[MaxKeysOneTime];

  while (Encodr.isPress()==0) {
    MyKeyb->ScanKeyMatrix(Keys);

    for (int counter = 0; counter < MaxKeyboards; counter++) {
      if (MyKeyb->Keyboards[counter] != NULL) {
        MyKeyb->Keyboards[counter]->PlayButtons(Keys);
      }
    }


    u8g.firstPage();
    do {
      u8g.setPrintPos(5, FontHigh + 1);
      u8g.print("Pressed keys: ");

      for (int counter = 0; Keys[counter] != 0x00; counter++) {
        u8g.setPrintPos(5 + (20 * counter), (FontHigh + 1) * 2);
        u8g.print(Keys[counter], DEC);

      }
      Encodr.tick();
    } while ( u8g.nextPage() );
  }
  return 0;
}


//----------------------------------------------------------------------
// get Scancode func
int TestScancode3() {
  unsigned int MaskCode[MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart +1];
  unsigned int Code[MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart +1];
  unsigned char ScanCode;

  // Get input mask pin state
  MyKeyb->ScanKeyMatrixLow(MaskCode);
  
   while (Encodr.isPress()==0) {
    ScanCode = 0xFF;
    // Get Keys
    MyKeyb->ScanKeyMatrixLow(Code);
      
    for (int OutCounter = 0; OutCounter < MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart +1; OutCounter++) {
      // XOR MaskCode and Code - delete unchanged pins
      Code[OutCounter] = Code[OutCounter] ^ MaskCode[OutCounter];

      int CodeSym = Code[OutCounter];

      // Convert input to ScanCodes -> KeyCode and push KeyCode in array
      for (int CodeCounter = 0; CodeCounter < MyKeyb->KeyInputPinStop - MyKeyb->KeyInputPinStart +1; CodeCounter++) {
        
        if (CodeSym & 0x01 == 1) { //if bit is changed
          ScanCode = (OutCounter << 4) | CodeCounter;
          break; //get only one key at one cycle
        }
        CodeSym = CodeSym >> 1;
      }
    }

    u8g.firstPage();
    do {
      u8g.setPrintPos(5, FontHigh + 1);
      u8g.print("MASK");
      
      //Draw mask
      unsigned char X = 1;
      unsigned char Y = 40;
      for (unsigned char OutCounter = 0; OutCounter < MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart + 1; OutCounter++) {
        Y = 40; 
        unsigned int CodeByte = MaskCode[OutCounter];
        for (unsigned char ByteNum = 0; ByteNum < MyKeyb->KeyInputPinStop - MyKeyb->KeyInputPinStart + 1; ByteNum++) {
          if(CodeByte & 0x01){
            u8g.drawBox(Y,X,2,2);
          }else{
            u8g.drawPixel(Y,X);
          }
          CodeByte>>=1;
          Y+=3;
        }
        X+=3;
      }
      u8g.drawFrame(39,0,49,25);

      //Draw current data
      u8g.setPrintPos(5, (FontHigh + 1) * 4);
      u8g.print("DATA");
      X = 27;
      for (unsigned char OutCounter = 0; OutCounter < MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart + 1; OutCounter++) {
        Y = 40;        
        unsigned int CodeByte = Code[OutCounter];
        for (unsigned char ByteNum = 0; ByteNum < MyKeyb->KeyInputPinStop - MyKeyb->KeyInputPinStart + 1; ByteNum++) {
          if(CodeByte & 1 == 1){
            u8g.drawBox(Y,X,2,2);
          }else{
            u8g.drawPixel(Y,X);
          }
          CodeByte>>=1;
          Y+=3;
        }
        X+=3;
      } 
      u8g.drawFrame(39,26,49,25);     

      //print scancode
      X = 64;
      u8g.setPrintPos(5, X );
      u8g.print("SCN: ");
      u8g.setPrintPos(35, X);
      u8g.print(ScanCode, HEX);
      u8g.setPrintPos(55, X);
      u8g.print(ScanCode, DEC);

      //print keycode
      u8g.setPrintPos(80, X);
      u8g.print("KEY: ");
      u8g.setPrintPos(110, X);
      u8g.print(MyKeyb->ScanCodeToKeyCode(ScanCode), DEC);
      Encodr.tick();  
    } while ( u8g.nextPage() );

  }

  return 0;

}




//----------------------------------------------------------------------
// More raw Test scan code
// OLD legacy code
int TestScancode2() {

  int Code = 0;
  int Code1 = 0;
  int Pin;
  char Scancodes[17];
  char Keys[17];
  int ScancodesLen;

  int OutNum = 0;
  while (1) {
    Encodr.tick();
    if (Encodr.isPress()) {
      return 0;
    }

    if (Encodr.isLeft()) {
      OutNum++;
      if (OutNum > MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart +1) {
        OutNum = 0;
      }
    }

    if (Encodr.isRight()) {
      OutNum--;
      if (OutNum < 0) {
        OutNum = MyKeyb->KeyOutputPinStop - MyKeyb->KeyOutputPinStart +1;;
      }
    }

    Pin = OutNum + MyKeyb->KeyOutputPinStart;

    MyKeyb->Pin2Low(Pin);

    // Get input pin state
    Code = 0;
    for (int InputPin = MyKeyb->KeyInputPinStop; InputPin >= MyKeyb->KeyInputPinStart; InputPin--) {
      Code <<= 1;
      if (digitalRead(InputPin)) {
        Code = Code | 1;
      }
    }
    /**/
    Code1 = Code;

    //Setup high-impedance pin state
    MyKeyb->Pin2HiZ(Pin);

    //Clear arrays
    Scancodes[0] = 0;
    Keys[0] = 0;
    ScancodesLen = 0;

    // Convert input to ScanCodes -> KeyCode and push KeyCode in array
    for (int CodeCounter = 0; CodeCounter < MyKeyb->KeyInputPinStop - MyKeyb->KeyInputPinStart +1; CodeCounter++) {
      if (Code & 0x01 == 1) {
        int Scan = (OutNum << 4) | CodeCounter;
        int Key = MyKeyb->ScanCodeToKeyCode(Scan);
        if (Key != 0) {
          Scancodes[ScancodesLen] = Scan;
          Keys[ScancodesLen] = Key;
          ScancodesLen++;
        }
      }
      Code = Code >> 1;
    }
    // 0x00 after last ScanCode
    Scancodes[ScancodesLen] = 0x00;
    Keys[ScancodesLen] = 0x00;

    u8g.firstPage();
    do {
      u8g.setPrintPos(5, FontHigh + 1);
      u8g.print("Output: ");
      u8g.setPrintPos(80, FontHigh + 1);
      u8g.print(OutNum, DEC);

      u8g.setPrintPos(5, (FontHigh + 1) * 2);
      u8g.print("Pin:");
      u8g.setPrintPos(80, (FontHigh + 1) * 2);
      u8g.print(Pin, DEC);


      u8g.setPrintPos(5, (FontHigh + 1) * 3);
      u8g.print("Inputs:");
      u8g.setPrintPos(80, (FontHigh + 1) * 3);
      u8g.print(Code1, HEX);

      for (int counter = 0; Keys[counter] != 0x00; counter++) {
        u8g.setPrintPos(5 + (20 * counter), (FontHigh + 1) * 4);
        u8g.print(Keys[counter], DEC);
      }

      for (int counter = 0; Scancodes[counter] != 0x00; counter++) {
        u8g.setPrintPos(5 + (20 * counter), (FontHigh + 1) * 5);
        u8g.print(Scancodes[counter], HEX);
      }



    } while ( u8g.nextPage() );

  }
}



//----------------------------------------------------------------------
// Test Raw Analog inputs
int TestAnalog() {
  int AnalogIn = 0;
  while (1) {
    if (Encodr.isLeft()) {
      AnalogIn++;
      if (AnalogIn > 15) {
        AnalogIn = 0;
      }
    }

    if (Encodr.isRight()) {
      AnalogIn--;
      if (AnalogIn < 0) {
        AnalogIn = 15;
      }
    }

    if (Encodr.isPress()) {
      return 0;
    }

    unsigned int value = analogRead(AnalogIn);

    u8g.firstPage();
    do {
      u8g.drawBox(4, 4, ((value / 8) + 4) * 0.95, 3);

      u8g.setPrintPos(5, 20);
      u8g.print("Analog chanel: ");
      u8g.setPrintPos(5, 32);
      u8g.print(AnalogIn, DEC);

      u8g.setPrintPos(5, 43);
      u8g.print("Raw value: ");
      u8g.setPrintPos(5, 54);
      u8g.print(value, DEC);
      Encodr.tick();
    } while ( u8g.nextPage() );
  }
}



//----------------------------------------------------------------------
// Test Graph Analog inputs
char GraphAnalogAlgoritm() {
  IRQEnable();
  int AnalogInCount = MyKeyb->GetAnalogInCount();
  AnalogInCount--; // get MaxNumber of input

  int AnalogIn = 0;
  while (1) {
    if (Encodr.isLeft()) {
      AnalogIn++;
      if (AnalogIn > AnalogInCount) {
        AnalogIn = 0;
      }
    }

    if (Encodr.isRight()) {
      AnalogIn--;
      if (AnalogIn < 0) {
        AnalogIn = AnalogInCount;
      }
    }

    if (Encodr.isPress()) {
      return AnalogIn;
    }

    u8g.firstPage();
    do {
      u8g.setPrintPos(5, 10);
      u8g.print("Input: ");
      u8g.setPrintPos(50, 10);
      u8g.print(MyKeyb->AnalogInputs[AnalogIn]->Input, DEC);

      u8g.setPrintPos(5, 20);
      u8g.print("Out: ");
      u8g.setPrintPos(50, 20);
      u8g.print(MyKeyb->AnalogInputs[AnalogIn]->Value, DEC);

      int OldPos = 63 - (MyKeyb->AnalogInputs[AnalogIn]->RawToData(0) / 2);
      int Pos;
      for (int Counter = 0; Counter < 128; Counter++) {
        Pos = MyKeyb->AnalogInputs[AnalogIn]->RawToData(Counter * 8);
        Pos = 63 - (Pos / 2);

        u8g.drawLine(Counter, OldPos, Counter, Pos); //нарисовать линию (начало x,y - конец x,y)
        OldPos = Pos;
        Encodr.tick();
      }

      Pos = MyKeyb->AnalogInputs[AnalogIn]->Value;
      Pos = 63 - (Pos / 2);
      u8g.drawLine(0, Pos, 127, Pos);

      Pos = analogRead(MyKeyb->AnalogInputs[AnalogIn]->Input);
      Pos = Pos / 8;
      u8g.drawLine(Pos, 0, Pos, 63);

      MyKeyb->Play();
    } while ( u8g.nextPage() );
  }
  IRQDisable();
}


//----------------------------------------------------------------------
// Test Midi Devices
int TestMidi() {
  // play notes from F#-0 (0x1E) to F#-5 (0x5A):
#define StartTestNote 0x1E
#define StopTestNote 0x5A

  IRQEnable();
  // for chanel 0 - 16
  for (int chanel = 0; chanel < 16; chanel++) {
    for (int pitch = StartTestNote; pitch < StopTestNote; pitch++) {
      //Get Buttons
      if (Encodr.isLeft()) {
        chanel++;
        if (chanel > 15) {
          chanel = 0;
        }
      }
      if (Encodr.isRight()) {
        chanel--;
        if (chanel < 0) {
          chanel = 15;
        }
      }
      if (Encodr.isPress()) {
        return 0;
      }

      // Send Midi
      MidiSendNote(chanel, pitch, 127);

      u8g.firstPage();
      do {
        u8g.drawBox(4, 4, (pitch - StartTestNote) * (LCD_WIDTH - 8) / (StopTestNote - StartTestNote), 4);

        u8g.setPrintPos(5, 20);
        u8g.print("Midi chanel: ");
        u8g.setPrintPos(5, 32);
        u8g.print(chanel, DEC);

        u8g.setPrintPos(5, 43);
        u8g.print("Send note: ");
        u8g.setPrintPos(5, 54);
        u8g.print(pitch, DEC);
        Encodr.tick();
      } while ( u8g.nextPage() );
      MidiClearNote(chanel, pitch, 0);
    }
  }
  IRQDisable();
}


//----------------------------------------------------------------------
// Test Midi Devices
int TestMidiInstruments() {

#define ToneCount 7
#define MidiCh 15

unsigned char GammaCmaj[ToneCount] = {60,62,64,65,67,69,71};
unsigned int MSB =0;
unsigned int LSB =0;

  IRQEnable();
  
  // for prog 0 - 127
  for (int Prog = 0; Prog < 127; Prog++) {
    //Setup program
    MidiSetBankMSB(MidiCh, MSB);
    MidiSetBankLSB(MidiCh, LSB);
    MidiProgramChange(MidiCh, Prog);
    
    for (int Tone = 0; Tone < ToneCount; Tone++) {

      delay(50);
      // Send Midi
      MidiSendNote(MidiCh, GammaCmaj[Tone], 127);

      u8g.firstPage();
      do {
        u8g.drawBox(4, 4, (((LCD_WIDTH-8) / (ToneCount)) * Tone+1) + 4 , 4);

        u8g.setPrintPos(5, 20);
        u8g.print("Program: ");
        u8g.setPrintPos(80, 20);
        u8g.print(Prog, DEC);

        u8g.setPrintPos(5, 32);
        u8g.print("MSB: ");
        u8g.setPrintPos(80, 32);
        u8g.print(MSB, DEC);

        u8g.setPrintPos(5, 43);
        u8g.print("LSB: ");
        u8g.setPrintPos(80, 43);
        u8g.print(LSB, DEC);

        u8g.setPrintPos(5, 54);
        u8g.print("Note: ");
        u8g.setPrintPos(80, 54);
        u8g.print(GammaCmaj[Tone], DEC);
        Encodr.tick();

        //Get Buttons
        if (Encodr.isLeft()) {
          MSB++;
          if (MSB > 127) {
            MSB = 0;
          }
        }
        if (Encodr.isRight()) {
          MSB--;
          if (MSB < 0) {
            MSB = 15;
          } 
        }
        if (Encodr.isPress()) {
          return 0;
        }

        
      } while ( u8g.nextPage() );
      
      delay(200);
      MidiClearNote(MidiCh, GammaCmaj[Tone], 0);
    }
  }
  IRQDisable();
  MidiAllSoundsOff(MidiCh);
}


//----------------------------------------------------------------------
// Test SD CARDS
int TestSD() {
  char TypeStr[10];
  char TypeFSStr[10];
  char SizeStr[20];

  int Result = card.init(SPI_QUARTER_SPEED, SD_CS);
  
  SD_TypeString(TypeStr);
  SD_FS_TypeString(TypeFSStr);
  SD_FS_Size(SizeStr);
  
  
  u8g.firstPage();
  do {

    u8g.setPrintPos(5, 12);
    u8g.print("SD Init: ");
    u8g.setPrintPos(70, 12);

    if ( Result == 0) {
      u8g.print("InitFail");
    } else if (Result == 1) {
      u8g.print("Success");
    } else if (Result == 2) {
      u8g.print("FS NA");
    }

    u8g.setPrintPos(5, 24);
    u8g.print("SD Card: ");
    u8g.setPrintPos(70, 24);
    u8g.print(TypeStr);

    u8g.setPrintPos(5, 36);
    u8g.print("FS Type: ");
    u8g.setPrintPos(70, 36);
    u8g.print(TypeFSStr);

    u8g.setPrintPos(5, 48);
    u8g.print("FS Size: ");
    u8g.setPrintPos(70, 48);
    u8g.print(SizeStr);


  } while ( u8g.nextPage() );

  WaitButtonPress();

  if (Result !=1) {
    return -1;
  }
  
  // ######## Write File part ####### 
  
  MsgPrintWait("Try write");
  
  File myFile;
  if (!SD.begin(SD_CS)) {
    MsgPrintWait("ERR SD Start");
    return -1;
  }

  // Delete old test
  SD.remove("test.txt");
  // Create/Open file
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    // Write to file
    myFile.print("Test...");
    myFile.close(); // close the file
    MsgPrintWait("Write done!");    
  }
  else {
    MsgPrintWait("ERR Write!");
    return -1;
  }

  
  // ######## Write Read part ####### 
  MsgPrintWait("Try read");
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {

    // read from the file
    char TmpBuff[21];
    int Len=20;
    for(unsigned char Counter =0; Counter < Len; Counter ++) TmpBuff[Counter ]=0;
        
    myFile.read(TmpBuff,Len);
    // close the file:
    myFile.close();
    
    if(strcmp("Test...",TmpBuff)==0){
      MsgPrintWait("SUCCESS!");
      return 0;
    }
    else{
      MsgPrintWait("ERR Content!");
      MsgPrintWait(TmpBuff);
      return -1;
    }
  }else{
    MsgPrintWait("ERR Read!");
    return -1;
  }
  /**/

  
 return 0;
}
