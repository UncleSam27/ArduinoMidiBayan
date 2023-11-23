#ifndef MIDI_H
#define MIDI_H

extern int GlobalMIDIOut;

void MidiInit();
void SendMidi(unsigned char cmd);
void MidiSendNote(unsigned char chanel, unsigned char  tone, unsigned char velocity);
void MidiClearNote(unsigned char  chanel, unsigned char  tone, unsigned char  velocity);
void MidiAllSoundsOff(unsigned char  chanel);
void MidiAllNotesOff(unsigned char  chanel);
void MidiControlChange(unsigned char  chanel, unsigned char  ContNum, unsigned char  Value);
void MidiSetBankMSB(unsigned char  chanel,unsigned char  Value);
void MidiSetBankLSB(unsigned char  chanel,unsigned char  Value);
void MidiProgramChange(unsigned char  chanel, unsigned char  Program);
void MidiSendVolume(unsigned char  chanel,unsigned char  Value);
void MidiSendPressure(unsigned char  chanel,unsigned char  Value);
void MidiSendPitch(unsigned char  chanel,unsigned char  Value);
void SendStr(unsigned char MidiString[]);

#endif
