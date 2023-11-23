#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>

extern Sd2Card card;
extern SdVolume volume;

int  SD_TypeString(char Type[]);
int  SD_FS_TypeString(char Type[]);
int  SD_FS_Size(char SizeStr[]);
int  SD_Debug();
bool SD_inserted();
int  SD_Init();
int  SD_WriteFile(char FileName[], char String[]);
int  SD_ReadFile(char FileName[], char String[], unsigned int Length);

#endif
