#ifndef KEYBLOADSAVE_H
#define KEYBLOADSAVE_H
#include <SD.h>
#include "midichanel.h"

void SaveNameValueToSD(File *myFile, char Name[], int Value);
void SaveNameValueToSD(File *myFile, char Name[], char Value[]);
void SaveNameValueToSD(File *myFile, char Name[], unsigned char Value[]);
void SaveNameValueToSD(File *myFile, char Name[], unsigned char Value[], unsigned char Count);
void SaveNameValueToSD(File *myFile, char Name[], DataBox *Value);
int ReadStringFromSD(File *myFile, char Str[]);
char *GetStartCfgStr (char Str[]);
int GetIntCfgStr (char Str[]);
int GetAnalogCfgStr (char Str[]);
int GetAnalogIntCfgStr (char Str[]);
int GetHexCfgStr (char Str[],unsigned char OutStr[]);
bool ChkCfgStr(char Templ[],char Str[]);
int LoadConfigFromFlash(char FName[]);
int LoadConfigFromFlashNum(unsigned char Num);
int LoadConfigFromFlashEEPROM();
int SaveConfigToFlash(char FName[]);
int SaveKeybToFlash(char FName[]);
uint8_t LoadKeybFromFlash(char FName[]);
int StartWithDefaultCfg();

#endif // KEYBLOADSAVE_H
