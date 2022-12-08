#ifndef SYSTEMCONTROLLER_H_
#define SYSTEMCONTROLLER_H_

#include "stm32f0xx_hal.h"
#include <stdbool.h>

void systemController_init(I2C_HandleTypeDef *i2cHdl, UART_HandleTypeDef *uartHdl);
void systemController_tick(void);

#endif //SYSTEMCONTROLLER_H_
