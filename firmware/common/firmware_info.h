#pragma once

#include "cl_common.h"

#define PRODUCT_BOOT_STR "FREEPADDFU"
#define PRODUCT_APP_STR "FREEPADAPP"

typedef struct 
{
    uint8_t verMajor;
    uint8_t verMinor; 
    uint16_t verPatch;
    uint32_t check;
} FirmwareInfo_t;

#define FIRMWARE_CHECK_VALUE(x0,x1,x2,x3,x4) \
    ((((x0) + (x1) + (x2) + 2333UL) * ((x3) + (x4) + UINT32_MAX)) ^ 0xdbef5328UL)

static inline bool FirmwareCheck(const FirmwareInfo_t* info)
{
    uint32_t checkValue = FIRMWARE_CHECK_VALUE(info->verMajor, info->verMinor, info->verPatch, info->verMinor, info->verPatch);
    return info->check == checkValue;
}

