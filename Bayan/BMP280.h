#ifndef BMP280_H
#define BMP280_H

#define BMP280_Calibrate_Try 32

#include <BMP280_DEV.h>
#include "config.h"

extern unsigned char  BMP280_Use;
extern unsigned char  BMP280_Sensitivity;
extern unsigned char  BMP280_Chanel;
extern unsigned char  BMP280_AutoAdjust;
extern unsigned int   BMP280_AutoAdjust_Limit;

class MyBMP280_DEV : public BMP280_DEV{
  public:
  int32_t getFastCurrentPres();
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
    BMP280_Sensor();
    void MeasureNormalPressure();
    int32_t GetNormalPressure();
    int32_t GetCurrentPressure();
    int32_t GetPressureDiference();
    uint8_t  GetPressureDiference0_127(uint8_t shift);
    uint32_t  GetPressureDiference1024(uint8_t shift);
    void SetExternalSensor(int * Ptr);
    void ClearExternalSensor();
    inline void AutoAdjust(long TempPressure);
    void Play();
};

extern BMP280_Sensor *Bmp280;

#endif
