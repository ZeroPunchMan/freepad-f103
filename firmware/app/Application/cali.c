#include "cali.h"
#include "main.h"
#include "cl_log.h"
#include "crc.h"
#include "string.h"
#include "flash_layout.h"
#include "iflash_stm32.h"
#include "cl_event_system.h"
#include "button.h"

static CaliParams_t caliParams = {0};

const CaliParams_t *GetCaliParams(void)
{
    return &caliParams;
}

static void PrintParams(CaliParams_t *params)
{ 
    CL_LOG_LINE("**********");
    CL_LOG_LINE("cali params:");
    CL_LOG_LINE("left x: %d, %d, %d", caliParams.leftX[0], caliParams.leftX[1], caliParams.leftX[2]);
    CL_LOG_LINE("left y: %d, %d, %d", caliParams.leftY[0], caliParams.leftY[1], caliParams.leftY[2]);
    CL_LOG_LINE("left trigger: %d, %d", caliParams.leftTrigger[0], caliParams.leftTrigger[1]);
    
    CL_LOG_LINE("right x: %d, %d, %d", caliParams.rightX[0], caliParams.rightX[1], caliParams.rightX[2]);
    CL_LOG_LINE("right y: %d, %d, %d", caliParams.rightY[0], caliParams.rightY[1], caliParams.rightY[2]);
    CL_LOG_LINE("right trigger: %d, %d", caliParams.rightTrigger[0], caliParams.rightTrigger[1]);
    CL_LOG_LINE("----------");
}

static void LoadCalibration(void)
{
    memcpy((void *)&caliParams, (void *)PAD_PARAM_ADDR, sizeof(caliParams));
    uint32_t crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    if (crc != caliParams.crc)
    { // use default params
        CL_LOG_LINE("use default params");
        caliParams.leftX[0] = 0;
        caliParams.leftX[1] = 2048;
        caliParams.leftX[2] = 4096;

        caliParams.leftY[0] = 0;
        caliParams.leftY[1] = 2048;
        caliParams.leftY[2] = 4096;

        caliParams.rightX[0] = 0;
        caliParams.rightX[1] = 2048;
        caliParams.rightX[2] = 4096;

        caliParams.rightY[0] = 0;
        caliParams.rightY[1] = 2048;
        caliParams.rightY[2] = 4096;

        caliParams.leftTrigger[0] = 0;
        caliParams.leftTrigger[1] = 4096;

        caliParams.rightTrigger[0] = 0;
        caliParams.rightTrigger[1] = 4096;
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

typedef enum
{
    CaliSta_None,   // 正常模式
    CaliSta_Middle, // 校准中间值
    CaliSta_Margin, // 校准边界值
} CaliStatus_t;
static CaliStatus_t caliStatus = CaliSta_None;

bool OnBtnPairEvent(void *eventArg)
{
    return true;
}

bool OnBtnAEvent(void *eventArg)
{
    return true;
}

void Cali_Init(void)
{
    LoadCalibration();
    CL_EventSysAddListener(OnBtnPairEvent, CL_Event_Button, BtnIdx_Pair);
    CL_EventSysAddListener(OnBtnAEvent, CL_Event_Button, BtnIdx_A);
}

void Cali_Process(void)
{
    switch (caliStatus)
    {
    case CaliSta_None:
        break;
    case CaliSta_Middle:
        break;
    case CaliSta_Margin:
        break;
    }
}
