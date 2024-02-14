#ifndef MENU_H
#define MENU_H

#include "analog.h"
#include "hotkey.h"
#include "midichanel.h"
#include "keyboard.h"
#include "keyboards.h"
#include "midichanel.h"
#include "keybloadsave.h"

#ifdef DRAM_MACHINE_USED
#include "drummachine.h"
#endif

void        StartLogo();

#ifdef DRAM_MACHINE_USED
void        MenuCahngeDrumMachine(DrumMachine DrumMachin);
#endif
AnalogIn*   MenuGetAnalogInput(AnalogIn* Input);
MidiController* MenuChangeMidiController(MidiController* Controller, MidiChanel *Chanel);
void        MenuMidiControllerSetup(MidiChanel *Chanel);
int         GetProgramFromString(char STR[], unsigned char *Prog, unsigned char *MSB, unsigned char *LSB);
int8_t      CheckInstrumentPKG();
int8_t      SetupProgramFromPKGFile(char header[], MidiChanel *Chanel );
int8_t      MenuMidiProgramSetup(MidiChanel *Chanel);
uint8_t     MenuChangeMidiChanels(MidiChanel *Chanel);
void        MenuMidiChanels();
MidiChanel* MenuGetMidiChanel(MidiChanel* Chanel);
int         MenuChangeKeyboard(Keyboard *Keybrd);
void        MenuKeyboard();
Keyboard*   MenuSelectKeyboard(Keyboard* StartKeyb);
int         MenuChangeHotkeyModifier(Hotkey *Hotk);
uint8_t     MenuChangeHotkey(Hotkey *Hotk);
void        MenuCreateHotkey();
void        MenuHotkey();
void        MenuColorSettings();
void        MenuSettings();
void        MenuTest();
void        MenuInfo(int DisplayCursor);
void        MenuInfoNow();
void        MenuPresets();
void        MenuAnalogIn();
void        MenuMain();

#endif
