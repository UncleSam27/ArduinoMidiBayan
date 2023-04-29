
#ifndef STRINGS_H
#define STRINGS_H


#include <stdlib.h>


//----------------------------------------------------------------------
// Copy PastedStr into Str widthout "0" at the end
void SubStrCopy(char Str[],int Start,char PastedStr[]){
  int CounterStr=Start;
  int CounterPst=0;
  while((Str[CounterStr]!=0)&& (PastedStr[CounterPst]!=0)){
    Str[CounterStr]=PastedStr[CounterPst];
    CounterStr++;
    CounterPst++;
  }
}

//----------------------------------------------------------------------
// Copy PastedStr into Str width "0" at the end
void StrCopyToAlign(char Str[],int Start,char PastedStr[]){
  int CounterStr=Start;
  int CounterPst=0;
  while(PastedStr[CounterPst]!=0){
    Str[CounterStr]=PastedStr[CounterPst];
    CounterStr++;
    CounterPst++;
  }
  Str[CounterStr]=0;
}


//----------------------------------------------------------------------
// Copy PastedStr from Start aligment into Str width "0" at the end
void StrCopyFromAlign(char Str[],int Start,char PastedStr[]){
  int CounterStr=0;
  int CounterPst=Start;
  while(PastedStr[CounterPst]!=0){
    Str[CounterStr]=PastedStr[CounterPst];
    CounterStr++;
    CounterPst++;
  }
  Str[CounterStr]=0;
}


//----------------------------------------------------------------------
// Copy PastedStr from Start aligment into Str width "0" at the end
void StrCopyFromAlign(char Str[],int Start,char PastedStr[],int Count){
  int CounterStr=0;
  int CounterPst=Start;
  while(PastedStr[CounterPst]!=0){
    Str[CounterStr]=PastedStr[CounterPst];
    CounterStr++;
    CounterPst++;
    if (CounterStr> Count){
      break;
    }
  }
  Str[CounterStr]=0;
}

//----------------------------------------------------------------------
// Copy integer as DEC into Str
void InsertIntToStr(char Str[],int Start, int Val){
    char Buffer[7];         //the ASCII of the integer will be stored in this char array
    itoa(Val,Buffer,10); //(integer, yourBuffer, base)
    SubStrCopy(Str,Start,Buffer);
}

// Copy integer as DEC into Str
void InsertIntAnalogToStr(char Str[],int Start, int Val){
    char Buffer[7];         //the ASCII of the integer will be stored in this char array
    if(Val>=0){
      itoa(Val,Buffer,10); //(integer, yourBuffer, base)
      SubStrCopy(Str,Start,Buffer);
    }else{
      Val++;
      Val=abs(Val);
      itoa(Val,Buffer,10); //(integer, yourBuffer, base)
      Str[Start]='A';
      Start++;
      SubStrCopy(Str,Start,Buffer);    
    }
}


// Copy integer as DEC into Str
void InsertArrayToStrAsHex(char Str[],int Start, unsigned char Mass[],unsigned char Lenght){
    int Addr=Start;
    for(unsigned char Counter=0; Counter<Lenght; Counter++){
      char Buffer[3];         //the ASCII of the HEX Byte will be stored in this char array
      sprintf(Buffer,"%02X", Mass[Counter]);
      SubStrCopy(Str,Addr,Buffer);
      Addr+=2;
    }
    
}

//----------------------------------------------------------------------
// Crop substring from Str to OutStr, 
void SubStrCrop(char Str[],char OutStr[],int Start, int Len){
  //Copy Substr to OutStr
  int CounterStr=Start;
  int CounterOut=0;
  while((Str[CounterStr]!=0)&& (CounterOut<Len)){
    OutStr[CounterOut]=Str[CounterStr];
    CounterStr++;
    CounterOut++;
  }
  OutStr[CounterOut]="0";
  
  //shift string to begin
  CounterOut=0;  
  while((Str[CounterStr]!=0)){  
    Str[CounterOut]=Str[CounterStr];
    CounterStr++;
    CounterOut++;
  }
  Str[CounterOut]=0;
}



//Convert Mass of unsigned char to hex strings divided by separators  "0"- end of Mass
unsigned int MassToHexStr(unsigned char Mass[], char Str[], char Separator[]){
  char TempStr[3];
  for(unsigned char Index = 0 ; Mass[Index]!=0; Index++){
    //itoa(Mass[Index], TempStr, 16);
    sprintf(TempStr, "%.2X",Mass[Index]);
    strcat(Str,TempStr);
    strcat(Str,Separator);
  }
  return 0;
}

//Convert hex string to Mass of unsigned char
unsigned int HexStrToMass(char Str[], unsigned char Mass[]){
  char TempStr[3];
  unsigned char Index = 0;
  while (strlen(Str)>0){
    SubStrCrop(Str,TempStr,0, 2); //Copy to TempStr 2 symbol
    Mass[Index] = strtoul(TempStr,NULL,16);
    if(Mass[Index]==0){
      break;
    }
    Index++;
  }
  Mass[Index] = 0;
  return 0;
}

#endif
