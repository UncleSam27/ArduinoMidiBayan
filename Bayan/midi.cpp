#include <arduino.h>
#include "midi.h"

/*
Midi documentation found here :
https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message

Pitch bend:
https://sites.uci.edu/camp2014/2014/04/30/managing-midi-pitchbend-messages/
*/

int GlobalMIDIOut;


// Init Midi Interface
void MidiInit() {
  // Set MIDI baud rate:
  Serial1.begin(31250);
  // Set PC Debug baud rate
  Serial.begin(115200);
  // Communication width ESP-8266 (wireless midi)
  Serial3.begin(115200);
 
}

//------------------------------------------------------------------------------
// Send one MIDI byte
void SendMidi(unsigned char cmd) {
 // MIDI out
 if(GlobalMIDIOut&0x01){ 
  Serial1.write(cmd);
 }
 // USB out
 if(GlobalMIDIOut&0x02){
  Serial.write(cmd);
 } 
}


//------------------------------------------------------------------------------
// Send one MIDI note
void MidiSendNote(unsigned char chanel, unsigned char  tone, unsigned char velocity){
  SendMidi(0x90 | chanel);
  SendMidi(tone);
  SendMidi(velocity);
}

//------------------------------------------------------------------------------
// Clear one MIDI note
void MidiClearNote(unsigned char  chanel, unsigned char  tone, unsigned char  velocity){
  SendMidi(0x80 | chanel);
  SendMidi(tone);
  SendMidi(velocity);
}

//------------------------------------------------------------------------------
// Off All Midi Sound
void MidiAllSoundsOff(unsigned char  chanel){
  SendMidi(0xB0 | chanel);
  SendMidi(120);
  SendMidi(0); 
}

//------------------------------------------------------------------------------
// Off All Midi Notes
void MidiAllNotesOff(unsigned char  chanel){
  SendMidi(0xB0 | chanel);
  SendMidi(123);
  SendMidi(0);   
}

//------------------------------------------------------------------------------
// Change Midi Controller
void MidiControlChange(unsigned char  chanel, unsigned char  ContNum, unsigned char  Value){
  SendMidi(0xB0 | ( chanel & 0x0F));
  SendMidi(ContNum);
  SendMidi(Value);
}

//------------------------------------------------------------------------------
// Set MSB
void MidiSetBankMSB(unsigned char  chanel,unsigned char  Value){
  MidiControlChange(chanel, 0, Value);
}

//------------------------------------------------------------------------------
// Set LSB
void MidiSetBankLSB(unsigned char  chanel,unsigned char  Value){
  MidiControlChange(chanel, 32, Value);
}

//------------------------------------------------------------------------------
// Change Midi Program
void MidiProgramChange(unsigned char  chanel, unsigned char  Program){
  SendMidi(0xC0 | chanel);
  SendMidi(Program);
}

//------------------------------------------------------------------------------
// Set Chanel Volume
void MidiSendVolume(unsigned char  chanel,unsigned char  Value){
  SendMidi(0xB0 | chanel);
  SendMidi(7);
  SendMidi(Value & 0x7f);
}


//------------------------------------------------------------------------------
// Set Chanel  Pressure
void MidiSendPressure(unsigned char  chanel,unsigned char  Value){
  SendMidi(0xD0 | chanel);
  SendMidi(Value & 0x7f);
}


//------------------------------------------------------------------------------
// Set Chanel  Pitch in 1/127 of real pitch value
void MidiSendPitch(unsigned char  chanel,unsigned char  Value){
  SendMidi(0xE0 | chanel);
  SendMidi(0);  
  SendMidi(Value & 0x7f);  
}

//------------------------------------------------------------------------------
// Send a MIDI string
void SendStr(unsigned char MidiString[]) {
  int counter = 0;
  while (MidiString[counter] != 0) {
    SendMidi(MidiString[counter]);
  }
}
