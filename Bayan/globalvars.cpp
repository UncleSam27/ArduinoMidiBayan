#include <GyverEncoder.h>
#include "config.h"

//############################## Global Varibles ###################################

unsigned char  NeedLoad = 0;

char FileProgram[20]              = "INSTR.CFG";
char FileProgramPKG[20]           = "INSTR.PKG";

// Format of scancode
// D7 D6 D5 D4 D3 D2 D1 D0
//  |  |  |  |  |  |  |  | 
//  |  |  |  |  +--+--+--+------ D0-D3 - Number of input Pin  
//  +--+--+--+------------------ D4-D7 - Number of output Pin


/**/
//Global Program Change Mode
unsigned char GlobalProgramChangeMode; //1-Default only   2-Roland MSB+LSB

// All pressed keys there
//unsigned char PressedKeys[MaxKeysOneTime+1];
