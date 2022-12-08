#ifndef SWITCHMANAGER_H_
#define SWITCHMANAGER_H_

#include <stdbool.h>
#include "stm32f0xx_hal.h"
#include "systemControllerTypes.h"
#include "main.h"
#include <stdbool.h>



void switchManager_setConfig(config_t config, portNo_t portNo);

#endif //SWITCHMANAGER_H_
