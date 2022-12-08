#ifndef SYSTEMCONTROLLERTYPES_H_
#define SYSTEMCONTROLLERTYPES_H_

typedef enum
{
  CONFIG_PT1000       = 0x00,
  CONFIG_NTC          = 0x01,
  CONFIG_VOLTAGE      = 0x02,
  CONFIG_CURRENT      = 0x03,
  CONFIG_RADIO_SUPPLY = 0x04,
  CONFIG_RADIO_OUTPUT = 0x05, 
  CONFIG_DISABLE      = 0x0D
  
} config_t;


typedef enum
{
  PORT_0  = 0,
  PORT_1  = 1
  
} portNo_t;


#endif //SYSTEMCONTROLLER_H_
