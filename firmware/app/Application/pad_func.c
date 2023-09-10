#include "pad_func.h"
#include "main.h"
#include "crc.h"
#include "string.h"
#include "flash_layout.h"
#include "iflash_stm32.h"
#include "cl_log.h"
#include "adc.h"
#include "systime.h"
#include "hc165scan.h"

typedef struct
{
    uint16_t leftX[2], leftY[2];
    uint16_t rightX[2], rightY[2];
    uint16_t leftTrigger[2], rightTrigger[2];
    uint32_t crc;
} CaliParams_t;
static CaliParams_t caliParams = {0};

static void PrintParams(CaliParams_t *params)
{
}

static void LoadCalibration(void)
{
    memcpy((void *)&caliParams, (void *)PAD_PARAM_ADDR, sizeof(caliParams));
    uint32_t crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    if (crc != caliParams.crc)
    { // use default params  todo
        CL_LOG_LINE("use default params");
    }
    else
    {
        CL_LOG_LINE("use saved params");
        PrintParams(&caliParams);
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
extern volatile uint16_t adcResult[6];
void PadFunc_Process(void)
{ // dmaCount
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) > 1000)
    {
        lastTime = GetSysTime();

        uint8_t button[2];
        Hc165Scan(16, button);
        CL_LOG_LINE("button: %02x %02x", button[0], button[1]);
        CL_LOG_LINE("adc: %d, %d, %d, %d, %d, %d", adcResult[0], adcResult[1], adcResult[2], adcResult[3], adcResult[4], adcResult[5]);
    }
}
