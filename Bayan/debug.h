#ifndef DEBUG_H
#define DEBUG_H

#ifdef Debug
#  define DebugPrint(x) __DebugPrint(x)
#  define DebugPrintLn(x) __DebugPrintLn(x)
#else
#  define DebugPrint(x) do {} while (0)
#  define DebugPrintLn(x) do {} while (0)
#endif

void  __DebugPrint(char Str[]);
void  __DebugPrintLn(char Str[]);

void  __DebugPrint(int Num);
void  __DebugPrintLn(int Num);

void  __DebugPrint(unsigned char Num);
void  __DebugPrintLn(unsigned char Num);

int   print_array(int* x, int len);

#endif // DEBUG_H
