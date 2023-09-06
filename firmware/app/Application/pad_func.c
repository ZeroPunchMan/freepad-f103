#include "pad_func.h"
#include "main.h"
#include "crc.h"
#include "string.h"
#include "flash_layout.h"
#include "iflash_stm32.h"
#include "cl_log.h"

typedef struct
{
    uint16_t leftX[2], leftY[2];
    uint16_t rightX[2], rightY[2];
    uint16_t leftTrigger[2], rightTrigger[2];
    uint32_t crc;
} CaliParams_t;
static CaliParams_t caliParams = {0};
static void LoadCalibration(void)
{
    memcpy((void *)&caliParams, (void *)PAD_PARAM_ADDR, sizeof(caliParams));
    uint32_t crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    if (crc != caliParams.crc)
    { // use default params  todo
    }
}

static void SaveCalibration(void)
{
    caliParams.crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    HAL_FLASH_Unlock();
    IFlashStm32_ErasePages(PAD_PARAM_ADDR, 1);
    IFlashStm32_Write(PAD_PARAM_ADDR, (const uint8_t *)&caliParams, sizeof(caliParams));
    HAL_FLASH_Lock();
}

void PadFunc_Init(void)
{
    LoadCalibration();
}

void PadFunc_Process(void)
{
}

