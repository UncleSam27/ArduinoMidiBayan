
#ifndef ANALOG_H
#define ANALOG_H

#include "config.h"

#define AnalogCycle 5  //real work evry 5 cycle 
#define AnalogPoints 9  //kurves sets by 9 points

//--------------------------------------------------------------------------------------------------------------
// class AnalogIn
//--------------------------------------------------------------------------------------------------------------



class AnalogIn {
  private:
    bool InternalADC;
    unsigned char  DelayCycle;
    int Cycle;
    int RawInputVal;    
  public:
    unsigned char  Boost;
    unsigned char  CentralBoost; 
    bool Freeze  = false;
    int Correction =0;  
    unsigned char  Value;  
    unsigned char  Input;
    unsigned char  Mode[AnalogPoints]={0x01,0x20,0x40,0x60,0x80,0xA0,0xC0,0xE0,0xFF};

    //----------------------------------------------------------------------
    //the constructor
    AnalogIn(unsigned char  initInput,  unsigned char initMode[], int initCorrection, unsigned char initBoost, unsigned char initCentralBoost) {
      Input = initInput;

      for(uint8_t Counter=0; Counter<AnalogPoints; Counter++){
        Mode[Counter] = initMode[Counter];
      }
      
      DelayCycle = AnalogCycle;
      Cycle = 0;
      Value = 0;
      InternalADC = true;
      Freeze = false;
      Correction = initCorrection;      
      Boost = initBoost;
      CentralBoost = initCentralBoost;       
    }

    //----------------------------------------------------------------------
    // convert raw adc data to value using 
    unsigned char RawToData(int NewVal) {
        int TmpVal = NewVal;
        TmpVal >>= 3;                            //convert ADC 10bit to 7bit

        unsigned char Offset =   TmpVal & 0xF;   //get 4bit Offset
        unsigned char Interval = TmpVal>>4;      //get 3bit Interval

        float Delta = float(float(Mode[Interval+1] - Mode[Interval])/16) * Offset;
        int Result = Mode[Interval] + int(Delta);
        Result>>=1;
        return Result;  
    }


    //----------------------------------------------------------------------
    //Get Current Value
    char GetValue() { //a function
      return Value;
    }


    //----------------------------------------------------------------------
    unsigned int GetRawInputVal(){
          int TmpRawInputVal = RawInputVal;
          TmpRawInputVal += Correction;               //add correction
  
          if(Boost > 0){                           // Boost for low signal
            if(Boost > 5) Boost = 5;
            TmpRawInputVal<<=Boost;
          }
  
          // Boost for low signal with null in center
          if(CentralBoost > 0){                   
            if(CentralBoost > 5) CentralBoost = 5; 
            TmpRawInputVal = ((TmpRawInputVal - 512) * (1 << CentralBoost)) + 512;
          }
          
          if(TmpRawInputVal>1024) TmpRawInputVal=1024;
          if(TmpRawInputVal<0) TmpRawInputVal=0;
      
      return TmpRawInputVal;
    }

    //----------------------------------------------------------------------
    int *SetExternalRawInputVal(){  // disable internal ADC and return pointer to RawInputVal
      InternalADC = false;
      return &RawInputVal;
    }

    //----------------------------------------------------------------------
    void SetRealRawInputVal(){
      InternalADC = true;
    }    

    //----------------------------------------------------------------------
    void FreezeVal(){
      Freeze = true;
    }   

        //----------------------------------------------------------------------
    void UnFreezeVal(){
      Freeze = false;
    }   
    
    //----------------------------------------------------------------------
    //the Play Analog
    void Play() { //a function update a value
      if(!Freeze){ 
        Cycle--;
        if (Cycle < 0) {
          if(InternalADC){
            RawInputVal = analogRead(Input);
          }
          Value=RawToData(GetRawInputVal());
          Cycle = DelayCycle;
        }
      }
    }
};


#endif
