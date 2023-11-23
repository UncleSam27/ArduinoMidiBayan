#ifndef STRINGS_H
#define STRINGS_H

void          SubStrCopy(char Str[],int Start,char PastedStr[]);
void          StrCopyToAlign(char Str[],int Start,char PastedStr[]);
void          StrCopyFromAlign(char Str[],int Start,char PastedStr[]);
void          StrCopyFromAlign(char Str[],int Start,char PastedStr[],int Count);
void          InsertIntToStr(char Str[],int Start, int Val);
void          InsertIntAnalogToStr(char Str[],int Start, int Val);
void          InsertArrayToStrAsHex(char Str[],int Start, unsigned char Mass[],unsigned char Lenght);
void          SubStrCrop(char Str[],char OutStr[],int Start, int Len);
unsigned int  MassToHexStr(unsigned char Mass[], char Str[], char Separator[]);
unsigned int  HexStrToMass(char Str[], unsigned char Mass[]);

#endif
