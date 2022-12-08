#include "switchManager.h"

typedef struct
{
  bool CS_EN;
  bool Radiometric;
  bool Port_EN;
  bool DIV_EN;
  bool R10k_EN;
  bool R200_EN;
  
} switchConfig_t;

static const switchConfig_t disabled =
{
  .CS_EN       = false,
  .Radiometric = false,
  .Port_EN     = false,
  .DIV_EN      = false,
  .R10k_EN     = false,
  .R200_EN     = false
};

static const switchConfig_t pt1000 =
{
  .CS_EN       = true,
  .Radiometric = false,
  .Port_EN     = true,
  .DIV_EN      = false,
  .R10k_EN     = false,
  .R200_EN     = false
};

static const switchConfig_t ntc =
{
  .CS_EN       = true,
  .Radiometric = false,
  .Port_EN     = true,
  .DIV_EN      = false,
  .R10k_EN     = true,
  .R200_EN     = false
};

static const switchConfig_t voltage =
{
  .CS_EN       = false,
  .Radiometric = false,
  .Port_EN     = false,
  .DIV_EN      = true,
  .R10k_EN     = true,
  .R200_EN     = false
};

static const switchConfig_t current =
{
  .CS_EN       = false,
  .Radiometric = false,
  .Port_EN     = false,
  .DIV_EN      = true,
  .R10k_EN     = true,
  .R200_EN     = true
};

static const switchConfig_t Radiometric =
{
  .CS_EN       = false,
  .Radiometric = true,
  .Port_EN     = false,
  .DIV_EN      = true,
  .R10k_EN     = true,
  .R200_EN     = false
};

static void writeConfig(const switchConfig_t *const config, portNo_t portNo);

void switchManager_setConfig(config_t config, portNo_t portNo)
{
  switch (config)
  {
    case CONFIG_PT1000:
    {
      writeConfig(&pt1000, portNo);
      break;
    }
    case CONFIG_NTC:
    {
      writeConfig(&ntc, portNo);
      break;
    }
    case CONFIG_VOLTAGE:
    {
      writeConfig(&voltage, portNo);
      break;
    }
    case CONFIG_CURRENT:
    {
      writeConfig(&current, portNo);
      break;
    }
    case CONFIG_RADIO_OUTPUT:
    case CONFIG_RADIO_SUPPLY:
    {
      writeConfig(&Radiometric, portNo);
      break;
    }
    case CONFIG_DISABLE:
    {
      writeConfig(&disabled, portNo);
      break;
    }
  }
}

static void writeConfig(const switchConfig_t *const config, portNo_t portNo)
{
  if (portNo == PORT_0)
  {
    HAL_GPIO_WritePin(CS_EN0_GPIO_Port,       CS_EN0_Pin,       config->CS_EN       ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RADIOMETRIC0_GPIO_Port, RADIOMETRIC0_Pin, config->Radiometric ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PORT0_EN_GPIO_Port,     PORT0_EN_Pin,     config->Port_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIV0_EN_GPIO_Port,      DIV0_EN_Pin,      config->DIV_EN      ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(P0_R10K_EN_GPIO_Port,   P0_R10K_EN_Pin,   config->R10k_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(P0_R200_EN_GPIO_Port,   P0_R200_EN_Pin,   config->R200_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
  else if (portNo == PORT_1)
  {
    HAL_GPIO_WritePin(CS_EN1_GPIO_Port,       CS_EN1_Pin,       config->CS_EN       ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RADIOMETRIC1_GPIO_Port, RADIOMETRIC1_Pin, config->Radiometric ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PORT1_EN_GPIO_Port,     PORT1_EN_Pin,     config->Port_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIV1_EN_GPIO_Port,      DIV1_EN_Pin,      config->DIV_EN      ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(P1_R10K_EN_GPIO_Port,   P1_R10K_EN_Pin,   config->R10k_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(P1_R200_EN_GPIO_Port,   P1_R200_EN_Pin,   config->R200_EN     ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
  else
  {
    // Not handled - wrong port number
  }
}

