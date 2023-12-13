#include <SPI.h>
#include <SD.h>
#include "config.h"
#include "sdcard.h"
#include "debug.h"


// set up variables using the SD utility library functions:
  Sd2Card card;
  SdVolume volume;


////////////////////////////////////////////////////////////////////
//  return string - SD type
int SD_TypeString(char Type[]) {
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      strcpy(Type, "SD1");
      return 1;
    case SD_CARD_TYPE_SD2:
      strcpy(Type, "SD2");
      return 2;
    case SD_CARD_TYPE_SDHC:
      strcpy(Type, "SDHC");
      return 3;
    default:
      strcpy(Type, "Undf");
      return -1;
  }
}


////////////////////////////////////////////////////////////////////
//  return string - type of file system
int SD_FS_TypeString(char Type[]) {

  unsigned int FSType = volume.fatType();
  if (FSType == 16) {
    strcpy(Type, "FAT16");
    return 16;
  }
  if (FSType == 32) {
    strcpy(Type, "FAT32");
    return 32;
  }
  strcpy(Type, "Undf");
  return FSType;
}

////////////////////////////////////////////////////////////////////
//  return string - type of file system
int SD_FS_Size(char SizeStr[]) {
  uint32_t volumesize;
  char TempStr[20];

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes

  if (volumesize < 10240) {
    itoa(volumesize, TempStr, 10);
    strcat( TempStr, "B" );
  }

  volumesize /= 1024;
  if (volumesize < 10240) {

    itoa(volumesize, TempStr, 10);
    strcat( TempStr, "KB" );
    strcpy(SizeStr, TempStr);
    return 0;
  }

  volumesize /= 1024;
  if (volumesize < 10240) {
    itoa(volumesize, TempStr, 10);
    strcat( TempStr, "MB" );
    strcpy(SizeStr, TempStr);
    return 0;
  }

  volumesize /= 1024;
  if (volumesize < 10240) {
    itoa(volumesize, TempStr, 10);
    strcat( TempStr, "GB" );
    strcpy(SizeStr, TempStr);
    return 0;
  }
}



///////////// Debug for SD Card /////////////////
//  init of SD Card 0 - succsess, 1 - init fail 2 - File system not found
int SD_Debug(){
  char TempStr[20];

  do{
    // try use full speed  
    DebugPrintLn("Try SD Init on full speed");
    if(card.init(SPI_FULL_SPEED, SD_CS)){
      DebugPrintLn("SD Init on full speed- complete");
      break;
    } 
    else DebugPrintLn("Fail SD Init on full speed");
  
    // try use half speed
    DebugPrintLn("Try SD Init on half speed");
    if (card.init(SPI_HALF_SPEED, SD_CS)) { 
      DebugPrintLn("SD Init on half speed- complete");
      break;
    }
    else DebugPrintLn("Fail SD Init on half speed");
   
    // try use quater speed
    DebugPrint("Try SD Init on quarter speed\r\n");
    if (card.init(SPI_QUARTER_SPEED, SD_CS)) { 
      DebugPrintLn("SD Init on quarter speed- complete");
      break;
    }  
    else DebugPrintLn("Fail SD Init on quarter speed");

   
    return 1;
  }while(0);


  // print the type of card
  SD_TypeString(TempStr);
  DebugPrint("Card type: ");
  DebugPrintLn(TempStr);
  
  DebugPrintLn("SD Init success, try read file system..."); 
  if (volume.init(card)) {
    DebugPrintLn("Finded FAT16/FAT32 partition");
    //Make sure you've formatted the card");

    SD_FS_TypeString(TempStr);
    DebugPrint("File system type: ");
    DebugPrintLn(TempStr);
    
    SD_FS_Size(TempStr);
    DebugPrint("Volume size: ");
    DebugPrintLn(TempStr);

    return 0;
  } else{
    DebugPrint("Could not find FAT16/FAT32 partition.\r\n");
    return 2;
  }
 
  return 1;
}



////////////////////////////////////////////////////////////////////
bool SD_inserted(){
  if(digitalRead(SD_DET)) return 0;
  return 1;
}



////////////////////////////////////////////////////////////////////
//  init of SD Card 0 - succsess, 1 - init fail
int SD_Init() {
  //SD chip select configure
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // SD Card detection Setup. 
  pinMode(SD_DET, INPUT);
  digitalWrite(SD_DET, HIGH);
  
  // Full Check Card if debug enabled
#ifdef Debug
  SD_Debug();
#endif

    
  // try use full speed  
  DebugPrint("Try Normal SD Init on full speed\r\n");
  if(SD.begin(SPI_FULL_SPEED, SD_CS)){
     DebugPrint("SD Init on full speed- complete\r\n");
     return 0;
  } 
  else {
    DebugPrint("Normal SD Init filed...\r\n"); 
    return 1;
  }
}







////////////////////////////////////////////////////////////////////
//  write string to File
int SD_WriteFile(char FileName[], char String[]) {
  File myFile;
  if (!SD_Init()) {
    return 1;
  }

  myFile = SD.open(FileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(String);
    myFile.close();
    return 0;
  } else {
    return 1;
  }
}

////////////////////////////////////////////////////////////////////
//  read string from File
int SD_ReadFile(char FileName[], char String[], unsigned int Length) {
  File myFile;
  if (!SD_Init()) {
    return 1;
  }

  myFile = SD.open(FileName);
  String[0]=0;
  // if the file opened okay, write to it:
  if (myFile) {
    
    int Counter = 0;
    while ((Counter < Length) && myFile.available()) {
      String[Counter]=myFile.read();
      Counter++;
    }
    String[Counter] = 0;
    return Counter;
  } else {
    return -1;
  }
}
