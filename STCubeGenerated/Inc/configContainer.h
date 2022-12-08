#ifndef CONFIGCONTAINER_H_
#define CONFIGCONTAINER_H_

#include <stdbool.h>

typedef struct
{
  bool csEn;
  bool muxA;
  bool muxB;
  bool muxC;
  bool r11kEn;
  bool r200En;
  
} switchConfig_t;


static const switchConfig_t pt1000 =
{
  .csEn = true,
  .muxA = true,
  .muxB = false,
  .muxC = false,
  .r11kEn = false,
  .r200En = false
};

static const switchConfig_t ntc =
{
  .csEn = true,
  .muxA = true,
  .muxB = false,
  .muxC = false,
  .r11kEn = true,
  .r200En = false
};

static const switchConfig_t current =
{
  .csEn = false,
  .muxA = true,
  .muxB = false,
  .muxC = false,
  .r11kEn = false,
  .r200En = true
};

static const switchConfig_t voltage =
{
  .csEn = false,
  .muxA = false,
  .muxB = true,
  .muxC = false,
  .r11kEn = true,
  .r200En = false
};

static const switchConfig_t radiometric =
{
  .csEn = false,
  .muxA = false,
  .muxB = true,
  .muxC = true,
  .r11kEn = true,
  .r200En = false
};



#endif // CONFIGCONTAINER_H_
