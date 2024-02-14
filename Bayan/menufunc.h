#ifndef MENUFUNC_H
#define MENUFUNC_H


bool GetMenuSubstr(int Num, char InputStr[], char ResultString[]);
void MenuPrint(int Choise, char MenuSttring[]);
int GetButtonState();
int GetButtonStateWait();
int Menu(int DefaultChoise, char MenuSttring[]);
unsigned char MenuGetKey(char header[]);
void MenuDisplaySelect(char header[], int16_t MaxValue, int16_t CurrentValue, char CharBuff[]);
int MenuGetInt(char header[], int StartValue, int MinValue, int MaxValue);
int MenuGetIntHEX(char header[], int StartValue, int MinValue, int MaxValue);
int MenuGetIntAndAnalog(char header[], int StartValue, int MinValue, int MaxValue);
unsigned int ChangeModeAnalogIn(int AnalogIn);
void MenuEditHEXString(char Str[], int MaxLen);
void MenuEditString(char Str[], int MaxLen);
void MsgPrint(char Message[]);
void MsgPrintNoWait(char Message[]);
void MsgPrintInt(int Num);
void WaitButtonPress();
void MsgPrintWait(char Message[]);


#endif
