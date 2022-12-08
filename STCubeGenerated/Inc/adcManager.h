#ifndef ADCMANAGER_H_
#define ADCMANAGER_H_

#include "stm32f0xx_hal.h"
#include "systemControllerTypes.h"
#include <stdbool.h>

typedef enum
{
  AINp_AIN0_AND_AINn_AIN1 = 0,
  AINp_AIN0_AND_AINn_AIN3,
  AINp_AIN1_AND_AINn_AIN3,
  AINp_AIN2_AND_AINn_AIN3,
  AINp_AIN0_AND_AINn_GND,
  AINp_AIN1_AND_AINn_GND,
  AINp_AIN2_AND_AINn_GND,
  AINp_AIN3_AND_AINn_GND
  
} inputMuxConfig_t;


typedef enum
{
  FSR_6144mV = 0,
  FSR_4096mV,
  FSR_2048mV,
  FSR_1024mV,
  FSR_0512mV,
  FSR_0256mV
    
} pgaConfig_t;

typedef enum
{
  CONTINUOS_CONVERSION = 0,
  SINGLE_SHOT
  
} opMode_t;

typedef enum
{
  SPS_8 = 0,
  SPS_16,
  SPS_32,
  SPS_64,
  SPS_128,
  SPS_250,
  SPS_475,
  SPS_860
  
} dataRate_t;

typedef struct
{
  inputMuxConfig_t inputMuxConfig;
  pgaConfig_t      pgaConfig;
  opMode_t         opMode;
  dataRate_t       dataRate;
  
} adcConfig_t;


void adcManager_init(I2C_HandleTypeDef *i2cInstance);
void adcManager_tick(void);

void adcManager_startSingleShotConversion(void);

void adcManager_setConfig(config_t config, portNo_t portNo);
void adcManager_requestLastConversion(void);

void adcManager_onConfigWritten(bool success);
void adcManager_onConversionReceived(uint16_t value);

#endif //ADCMANAGER_H_
