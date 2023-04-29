#ifndef BMP280_H
#define BMP280_H

#define BMP280_Calibrate_Try 32

#include <BMP280_DEV.h>
#include "config.h"


class MyBMP280_DEV : public BMP280_DEV{
  public:
  
  int32_t getFastCurrentPres(){  // Get the current pressure without checking the measuring bit and temperature compensation
                                 // Да я сознательно отказываюсь от температурной компенсации, так как это долго и ресурсоемко 
    uint8_t data[6]; // Create a data buffer
    for(uint8_t Counter = 0; Counter <6; Counter++) data[Counter] = 0; //Clear buffer
    
    readBytes(BMP280_PRES_MSB, &data[0], 6);                          // Read the temperature and pressure data
    return (int32_t)data[0] << 12 | (int32_t)data[1] << 4 | (int32_t)data[2] >> 4;
  }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class BMP280_Sensor{
  
  private:
  MyBMP280_DEV bmp280; 
  long NormalPressure;
  long LastPressure;
  int * ExternalSensor;

  long BMP280_AutoAdjust_Pressure;
  uint8_t BMP280_AutoAdjust_Try;
  
  public:  

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Constructor
  BMP280_Sensor(){
    NormalPressure=0;
    ExternalSensor = NULL;
    
    bmp280.begin(BMP280_I2C_ALT_ADDR) ;
    bmp280.setTimeStandby(TIME_STANDBY_62MS);     // Обновляем данные 16 раз в секунду
    bmp280.startNormalConversion();               // BMP280 в режим NORMAL_MODE  
    MeasureNormalPressure();
    BMP280_AutoAdjust_Try = 0;
    BMP280_AutoAdjust_Pressure = 0;
  }

/*  
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  void SaveNormalPresToEEPROM(unsigned int Adress){
    uint32_t Pressure = NormalPressure;
    for(byte Counter = 0; Counter < 4; Counter++){
      EEPROM.write(Adress+Counter, byte(Pressure & 0xFF));
      Pressure >>= 8;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  void LoadNormalPresFromEEPROM(unsigned int Adress){
    uint32_t Pressure = 0;
    for(int Counter = 3; Counter > 0; Counter--){
      Pressure = Pressure | EEPROM.read(Adress+Counter);
      Pressure <<= 8;
    }
    NormalPressure = Pressure;
  } /**/
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Получаем среднее из 8 измерений которое будем использовать как нормальное давление (~0.7 секунды)
  void MeasureNormalPressure(){
      int32_t pressure; 
      NormalPressure = 0;
      delay(300); 
  
      // Получаем среднее
      for(uint8_t Counter = 0; Counter <  BMP280_Calibrate_Try; Counter++){
        pressure = bmp280.getFastCurrentPres();
        NormalPressure += pressure;
        delay(62);
      }

      //NormalPressure = NormalPressure / BMP280_Calibrate_Try;  //div to long
      NormalPressure = NormalPressure >> 5;  //fast div by 32, brood fix
  }

  ///////////////////////////////////////////////////////////////////////////////////
  int32_t GetNormalPressure(){
    return NormalPressure;
  }

  ///////////////////////////////////////////////////////////////////////////////////
  int32_t GetCurrentPressure(){
    return bmp280.getFastCurrentPres();
  }

  ///////////////////////////////////////////////////////////////////////////////////
  int32_t GetPressureDiference(){
    LastPressure = bmp280.getFastCurrentPres();
    return NormalPressure - LastPressure;
  }

  /////////////////////////////////////////////////////////////////////////////
  uint8_t  GetPressureDiference0_127(uint8_t shift){
    int32_t pressure; 
    pressure = GetPressureDiference();
    pressure = pressure >> shift;
    pressure += 64;
    if(pressure > 127) pressure=127;
    if(pressure < 0) pressure=0;    
    return pressure;
  }

  /////////////////////////////////////////////////////////////////////////////
  uint32_t  GetPressureDiference1024(uint8_t shift){
    int32_t pressure; 
    pressure = GetPressureDiference();
    pressure = pressure >> shift;
    pressure += 512;
    if(pressure > 1024) pressure=1024;
    if(pressure < 0) pressure=0;    
    return pressure;
  }  

  
  /////////////////////////////////////////////////////////////////////////////
  //Set pointer to external sensor
  void SetExternalSensor(int * Ptr){
    ExternalSensor = Ptr;
  }

  /////////////////////////////////////////////////////////////////////////////
  //Set pointer to external sensor
  void ClearExternalSensor(){
    ExternalSensor = NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Авто калибровка сенсора на лету (да пока тупой алгоритм, но более умный требует больше памяти)
  inline void AutoAdjust(long TempPressure){
    
    if (abs(NormalPressure - TempPressure) > BMP280_AutoAdjust_Limit){
      BMP280_AutoAdjust_Pressure = 0;
      BMP280_AutoAdjust_Try = 0;
      return;
    }else{
      BMP280_AutoAdjust_Pressure += TempPressure;
      BMP280_AutoAdjust_Try++;
    } 
        
    if(BMP280_AutoAdjust_Try >= BMP280_Calibrate_Try){

      //BMP280_AutoAdjust_Pressure = BMP280_AutoAdjust_Pressure / BMP280_Calibrate_Try;   //div too long
      BMP280_AutoAdjust_Pressure = BMP280_AutoAdjust_Pressure >> 5; //fast div by 32
    
      /*Serial.print("\nBMP280_AutoAdjust_Pressure ");
      Serial.print(BMP280_AutoAdjust_Pressure);
      Serial.print(" Norm pressure ");
      Serial.print(NormalPressure);*/

      unsigned int BMP280_AutoAdjust_Pressure_Dif = abs(NormalPressure - BMP280_AutoAdjust_Pressure);     
      /*Serial.print(" Diff ");
      Serial.print(BMP280_AutoAdjust_Pressure_Dif);*/

      if((BMP280_AutoAdjust_Pressure_Dif < BMP280_AutoAdjust_Limit) && (BMP280_AutoAdjust_Pressure_Dif > BMP280_AutoAdjust_Limit/2)){ // get abs diference
        NormalPressure = BMP280_AutoAdjust_Pressure;
        /*Serial.print("\nNormal changed ");*/
      }       
      BMP280_AutoAdjust_Pressure = 0;
      BMP280_AutoAdjust_Try = 0;
     /*Serial.print("\n"); */       
    }           
  }

  /////////////////////////////////////////////////////////////////////////////
  // Обновление данных.
  void Play(){
    if(ExternalSensor != NULL){
       *ExternalSensor = GetPressureDiference1024(BMP280_Sensitivity);

      if(BMP280_AutoAdjust){ 
        AutoAdjust(LastPressure);
      }
    }
  }

};


// Опять глобальная переменная где не надо. Не забыть переместить.
BMP280_Sensor *Bmp280 = NULL;


#endif
