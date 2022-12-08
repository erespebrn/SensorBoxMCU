#include "systemController.h"
#include "adcManager.h"
#include "switchManager.h"

#define UART_FRAME_SIZE   3
#define MAX_PORT_NUMBER   2
#define MAX_CONFIG_NUMBER CONFIG_DISABLE
#define ADC_TIMEOUT_MS    1000

typedef enum
{
  WRITE_CONFIG_FUNC = 0x01,
  WRITE_TIME_FUNC   = 0x02,
  START_MEAS_FUNC   = 0x03,
  STOP_MEAS_FUNC    = 0x04,
  REQ_SAMPLE        = 0x05,
  UPLINK_SAMPLE     = 0x10,
  UPLINK_STOP       = 0x11
  
} funcCode_t;

typedef enum
{
  SYS_OK          = 0xCC,
  SYS_WRONG_CMD   = 0xCD,
  ADC_TIMEOUT_ERR = 0xAD,
  ADC_CONF_ERR    = 0xAE,
  
} errorCode_t;

typedef enum
{
  SET_SWITCH_CONFIG,
  SET_ADC_CONFIG,
  WAIT_FOR_ADC_RESPONSE,
  ADC_RESPONSE_ARRIVED
  
} configState_t;

typedef enum
{
  REQUEST_SAMPLE,
  WAIT_FOR_SAMPLE,
  
} measState_t;

typedef enum
{
  MEAS_UNTIL_STOP,
  MEAS_UNTIL_TIME
  
} measType_t;


static configState_t configState = SET_SWITCH_CONFIG;
static measState_t   measState   = REQUEST_SAMPLE;

static portNo_t portNo     = PORT_0;
static config_t configType = CONFIG_CURRENT;

static bool configActive    = false;
static bool measRunning     = false;
static bool adcConfigScs    = false;
static bool sampleRequested = false;
static bool sampleReceived  = false;

UART_HandleTypeDef *uartHandle = NULL;

static uint32_t commTimeoutMs   = 0;

uint16_t lastADCsample = 0;

static uint8_t uartWrBuff[UART_FRAME_SIZE];
static uint8_t uartReBuff[UART_FRAME_SIZE];

static void adcInit(I2C_HandleTypeDef *i2cHandle);
static void uartWriteConfigHandler(void);
static void uartReplyToGuiOnReceived(errorCode_t error);
static void uartRequestSampleHandler(void);
static bool configStateMachineHandler(void);
static bool measStateMachineHandler(void);

void systemController_init(I2C_HandleTypeDef *i2cHdl, UART_HandleTypeDef *uartHdl)
{
  uartHandle = uartHdl;
  adcInit(i2cHdl);
  HAL_UART_Receive_DMA(uartHandle, uartReBuff, UART_FRAME_SIZE);
}

void systemController_tick(void)
{  
  if (configActive)
  {
    configActive = configStateMachineHandler();
  }
  else if (measRunning)
  {
    measRunning = measStateMachineHandler();
  }
}

void adcManager_onConversionReceived(uint16_t value)
{
  lastADCsample = value;
  
  sampleReceived = true;
  
  if (measRunning && sampleRequested)
  {
    sampleRequested = false;
    
    uartWrBuff[0] = (uint8_t)UPLINK_SAMPLE;
    uartWrBuff[1] = (uint8_t)value;
    uartWrBuff[2] = (uint8_t)(value >> 8);
    
    HAL_UART_Transmit_DMA(uartHandle, uartWrBuff, UART_FRAME_SIZE);

  }
}

void adcManager_onConfigWritten(bool success)
{
  configState = ADC_RESPONSE_ARRIVED;
  adcConfigScs = success;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{  
  funcCode_t funcCode = (funcCode_t)uartReBuff[0];
  
  switch (funcCode)
  {
    case WRITE_CONFIG_FUNC:
    {
      uartWriteConfigHandler();
      break;
    }
    case REQ_SAMPLE:
    {
      uartRequestSampleHandler();
      break;
    }
    case WRITE_TIME_FUNC:
    case START_MEAS_FUNC:
    case STOP_MEAS_FUNC:
    case UPLINK_SAMPLE:
    case UPLINK_STOP:
    default:
    {
      break;
    }
  }
  
  HAL_UART_Receive_DMA(uartHandle, uartReBuff, UART_FRAME_SIZE);
}

/**************************************************************************************************
Static local functions
**************************************************************************************************/
static bool measStateMachineHandler(void)
{
  bool retVal = true;
  
  switch (measState)
  {
    case REQUEST_SAMPLE:
    {
      measState = WAIT_FOR_SAMPLE;
      commTimeoutMs = HAL_GetTick();
      sampleReceived = false;
      adcManager_startSingleShotConversion();
      break;
    }
    case WAIT_FOR_SAMPLE:
    {
      if ((HAL_GetTick() - commTimeoutMs) > ADC_TIMEOUT_MS)
      {
        uartReplyToGuiOnReceived(ADC_TIMEOUT_ERR);
        retVal = false;
      }
      else if (sampleReceived)
      {
        retVal = false;
        measState = REQUEST_SAMPLE;
      }
    }
  }
  
  return retVal;
}

static bool configStateMachineHandler(void)
{
  bool retVal = true;
  
  switch (configState)
  {
    case SET_SWITCH_CONFIG:
    {
      switchManager_setConfig(configType, portNo);
      configState = SET_ADC_CONFIG;
    }
    case SET_ADC_CONFIG:
    {
      adcManager_setConfig(configType, portNo);
      configState = WAIT_FOR_ADC_RESPONSE;
      commTimeoutMs = HAL_GetTick();
      break;
    }
    case WAIT_FOR_ADC_RESPONSE:
    {
      if ((HAL_GetTick() - commTimeoutMs) > ADC_TIMEOUT_MS)
      {
        uartReplyToGuiOnReceived(ADC_TIMEOUT_ERR);
        retVal = false;
        configState = SET_SWITCH_CONFIG;
      }
      break;
    }
    case ADC_RESPONSE_ARRIVED:
    {
      if (adcConfigScs)
      {
        uartReplyToGuiOnReceived(SYS_OK);
      }
      else
      {
        uartReplyToGuiOnReceived(ADC_CONF_ERR);
      }
      
      retVal = false;
      configState = SET_SWITCH_CONFIG;
      
      break;
    } 
  }
  
  return retVal;
}

static void adcInit(I2C_HandleTypeDef *i2cHandle)
{
  adcManager_init(i2cHandle);
}

static void uartWriteConfigHandler(void)
{  
  if (uartReBuff[1] < MAX_PORT_NUMBER && uartReBuff[2] <= MAX_CONFIG_NUMBER)
  {
    portNo       = (portNo_t)uartReBuff[1];
    configType   = (config_t)uartReBuff[2];
    configActive = true;
  }
  else
  {
    uartReplyToGuiOnReceived(SYS_WRONG_CMD);
  }  
}

static void uartRequestSampleHandler(void)
{
  sampleRequested = true;
  measRunning = true;
  measState   = REQUEST_SAMPLE;
}

static void uartReplyToGuiOnReceived(errorCode_t error)
{
  uartWrBuff[0] = (uint8_t)error;
  uartWrBuff[1] = uartWrBuff[2] = 0x00;
  
  HAL_UART_Transmit_DMA(uartHandle, uartWrBuff, UART_FRAME_SIZE);
}
