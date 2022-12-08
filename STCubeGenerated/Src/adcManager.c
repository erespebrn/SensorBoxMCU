#include "adcManager.h"

#define CONFIG_REG_ADDR    0x01
#define ADC_I2C_READ_ADDR  ((0x48 << 1) & 0xFE)
#define ADC_I2C_WRITE_ADDR ((0x48 << 1) | 0x01)

#define ADC_CONFIG_REG_ADDR  0x01
#define ADC_CONVERT_REG_ADDR 0x00

#define ADC_CONFIG_START_CONV_MASK (1 << 15)

#define COMP_QUE_DEFAULT  0x03
#define COMP_LAT_DEFAULT  0x00
#define COMP_POL_DEFAULT  0x00
#define COMP_MODE_DEFAULT 0x00

#define ADC_BASIC_CONFIG   \
{                          \
  AINp_AIN0_AND_AINn_GND,  \
  FSR_4096mV,              \
  SINGLE_SHOT,             \
  SPS_128                  \
}

typedef enum
{
  IDLE,
  WRITE_CONFIG,
  READ_CONFIG,
  WAIT_FOR_START_CONVERSION,
  CHECK_CONVERSION_STATUS,
  READ_CONVERSION,
  
} transferType_t;

typedef enum
{
  COMP_QUE_OFFSET  = 0,
  COMP_LAT_OFFSET  = 2,
  COMP_POL_OFFSET  = 3,
  COMP_MODE_OFFSET = 4, 
  DR_OFFSET        = 5,
  MODE_OFFSET      = 8,
  PGA_OFFSET       = 9,
  MUX_OFFSET       = 12,
  OS_OFFSET        = 15
  
} configRegOffsets_t;

static uint8_t readBuff[4];
static uint8_t writeBuff[4];
static uint16_t configReg = 0;

static transferType_t transType = IDLE;

static I2C_HandleTypeDef *i2cHandle = NULL;
static uint16_t updateConfigRegValue(adcConfig_t * adcConfig);
static void setConfigRegister(adcConfig_t * adcConfig);

void adcManager_init(I2C_HandleTypeDef *i2cInstance)
{
  i2cHandle = i2cInstance;
}

void adcManager_setConfig(config_t config, portNo_t portNo)
{ 
  adcConfig_t adcConfig = (adcConfig_t)ADC_BASIC_CONFIG;
  
  switch (config)
  {
    case CONFIG_PT1000:
    case CONFIG_NTC:
    case CONFIG_VOLTAGE:
    case CONFIG_DISABLE:
    case CONFIG_CURRENT:
    case CONFIG_RADIO_OUTPUT:
    {
      switch (portNo)
      {
        case PORT_0:
        {
          adcConfig.inputMuxConfig = AINp_AIN1_AND_AINn_GND;
          break;
        }
        case PORT_1:
        {
          adcConfig.inputMuxConfig = AINp_AIN3_AND_AINn_GND;
          break;
        }
      }
      break;
    }
    case CONFIG_RADIO_SUPPLY:
    {
      switch (portNo)
      {
        case PORT_0:
        {
          adcConfig.inputMuxConfig = AINp_AIN0_AND_AINn_GND;
          break;
        }
        case PORT_1:
        {
          adcConfig.inputMuxConfig = AINp_AIN2_AND_AINn_GND;
          break;
        }
      }
     break;     
    }
  }

  setConfigRegister(&adcConfig);
}

void adcManager_startSingleShotConversion(void)
{
  configReg |= ADC_CONFIG_START_CONV_MASK;
  
  writeBuff[0] = CONFIG_REG_ADDR;
  writeBuff[1] = (uint8_t)(configReg >> 8);
  writeBuff[2] = (uint8_t)configReg;
  
  transType = WAIT_FOR_START_CONVERSION;
  
  HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_WRITE_ADDR, writeBuff, 3);
}

void adcManager_requestLastConversion(void)
{
  writeBuff[0] = ADC_CONVERT_REG_ADDR;
  transType = READ_CONVERSION;
  
  HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_READ_ADDR, writeBuff, 1);
}

__weak void adcManager_onConfigWritten(bool success)
{
  UNUSED(success);
}

__weak void adcManager_onConversionReceived(uint16_t value)
{
  
}

/**************************************************************************************************
I2C Callback
**************************************************************************************************/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  switch (transType)
  {
    default:
    case IDLE:
    {
      break;
    }
    case WRITE_CONFIG:
    {
      writeBuff[0] = CONFIG_REG_ADDR;
      transType = READ_CONFIG;
      HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_READ_ADDR, writeBuff, 1);
      break;
    }
    case READ_CONFIG:
    {
      HAL_I2C_Master_Receive_DMA(i2cHandle, ADC_I2C_READ_ADDR, readBuff, 2);
      break;
    }
    case READ_CONVERSION:
    {
      HAL_I2C_Master_Receive_DMA(i2cHandle, ADC_I2C_READ_ADDR, readBuff, 2);
      break;
    }
    case WAIT_FOR_START_CONVERSION:
    {
      writeBuff[0] = CONFIG_REG_ADDR;
      transType = CHECK_CONVERSION_STATUS;
      HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_READ_ADDR, writeBuff, 1);
    }
    case CHECK_CONVERSION_STATUS:
    {
      HAL_I2C_Master_Receive_DMA(i2cHandle, ADC_I2C_READ_ADDR, readBuff, 2);
      break;
    }

  }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  uint16_t rcvdConfig = 0;
    
  if (transType == READ_CONFIG)
  {
    rcvdConfig = ((readBuff[0] << 8) | readBuff[1]) & 0x7F;
    adcManager_onConfigWritten(rcvdConfig == (configReg & 0x7F));
  }
  else if (transType == READ_CONVERSION)
  {
    uint16_t conVal = (readBuff[0] << 8) | readBuff[1];
    adcManager_onConversionReceived(conVal);
  }
  else if (transType == CHECK_CONVERSION_STATUS)
  {
    rcvdConfig = ((readBuff[0] << 8) | readBuff[1]);
    
    if (rcvdConfig & ADC_CONFIG_START_CONV_MASK)
    {
      adcManager_requestLastConversion();
    }
    else
    {
      writeBuff[0] = CONFIG_REG_ADDR;
      transType = CHECK_CONVERSION_STATUS;
      HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_READ_ADDR, writeBuff, 1);
    }
  }
  
}

/**************************************************************************************************
Static local functions
**************************************************************************************************/
static void setConfigRegister(adcConfig_t * adcConfig)
{
  configReg = updateConfigRegValue(adcConfig);
  
  writeBuff[0] = CONFIG_REG_ADDR;
  writeBuff[1] = (uint8_t)(configReg >> 8);
  writeBuff[2] = (uint8_t)configReg;
  
  transType = WRITE_CONFIG;
  
  HAL_I2C_Master_Transmit_DMA(i2cHandle, ADC_I2C_WRITE_ADDR, writeBuff, 3);
}

static uint16_t updateConfigRegValue(adcConfig_t *adcConfig)
{
  return adcConfig->dataRate << DR_OFFSET       | adcConfig->inputMuxConfig << MUX_OFFSET      |
         adcConfig->opMode   << MODE_OFFSET     | adcConfig->pgaConfig      << PGA_OFFSET      |
         COMP_QUE_DEFAULT    << COMP_QUE_OFFSET | COMP_LAT_DEFAULT          << COMP_LAT_OFFSET |
         COMP_POL_DEFAULT    << COMP_POL_OFFSET | COMP_MODE_DEFAULT         << COMP_MODE_OFFSET;
}
