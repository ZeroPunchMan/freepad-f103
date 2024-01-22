#include "cali.h"
#include "main.h"
#include "cl_log.h"
#include "crc.h"
#include "string.h"
#include "flash_layout.h"
#include "iflash_stm32.h"
#include "cl_event_system.h"
#include "button.h"
#include "led.h"
#include "cl_queue.h"
#include "systime.h"
#include "adc.h"

static CaliParams_t caliParams = {0};

const CaliParams_t *GetCaliParams(void)
{
    return &caliParams;
}

static void PrintParams(CaliParams_t *params)
{
    CL_LOG_INFO("**********");
    CL_LOG_INFO("cali params:");
    CL_LOG_INFO("left x: %d, %d, %d", caliParams.leftX[0], caliParams.leftX[1], caliParams.leftX[2]);
    CL_LOG_INFO("left y: %d, %d, %d", caliParams.leftY[0], caliParams.leftY[1], caliParams.leftY[2]);
    CL_LOG_INFO("left trigger: %d, %d", caliParams.leftTrigger[0], caliParams.leftTrigger[1]);

    CL_LOG_INFO("right x: %d, %d, %d", caliParams.rightX[0], caliParams.rightX[1], caliParams.rightX[2]);
    CL_LOG_INFO("right y: %d, %d, %d", caliParams.rightY[0], caliParams.rightY[1], caliParams.rightY[2]);
    CL_LOG_INFO("right trigger: %d, %d", caliParams.rightTrigger[0], caliParams.rightTrigger[1]);
    CL_LOG_INFO("----------");
}

static void ResetParams(void)
{
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

static void LoadCalibration(void)
{
    memcpy((void *)&caliParams, (void *)PAD_PARAM_ADDR, sizeof(caliParams));
    uint32_t crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    if (crc != caliParams.crc)
    { // use default params
        CL_LOG_INFO("use default params");
        ResetParams();
    }
    else
    {
        CL_LOG_INFO("use saved params");
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

static CaliStatus_t caliStatus = CaliSta_None;

static void ToCaliNone(void)
{
    SetPadLedStyle(PadLedStyle_On);
    caliStatus = CaliSta_None;
    CL_LOG_INFO("cali done");
    PrintParams(&caliParams);
}

typedef struct
{
    uint16_t leftX, leftY, rightX, rightY;
    uint16_t leftHall, rightHall;
} MidVal_t;
CL_QUEUE_DEF_INIT(middleQueue, 20, MidVal_t, static);
static void ToCaliMiddle(void)
{
    SetPadLedStyle(PadLedStyle_Breath);
    CL_QueueClear(&middleQueue);
    caliStatus = CaliSta_Middle;
    CL_LOG_INFO("start cali middle");
}

static void ToCaliMargin(void)
{
    caliParams.leftX[0] = UINT16_MAX;
    caliParams.leftX[2] = 0;
    caliParams.leftY[0] = UINT16_MAX;
    caliParams.leftY[2] = 0;

    caliParams.rightX[0] = UINT16_MAX;
    caliParams.rightX[2] = 0;
    caliParams.rightY[0] = UINT16_MAX;
    caliParams.rightY[2] = 0;

    caliParams.leftTrigger[1] = 0;
    caliParams.rightTrigger[1] = 0;

    SetPadLedStyle(PadLedStyle_Blink);
    caliStatus = CaliSta_Margin;
    CL_LOG_INFO("start cali margin");
}

static bool OnBtnPairEvent(void *eventArg)
{ // pair长按,进入中间值校准状态
    ButtonEvent_t *pEvt = (ButtonEvent_t *)eventArg;
    if (pEvt[0] == ButtonEvent_LongPress)
    {
        if (caliStatus == CaliSta_None)
        {
            ToCaliMiddle();
        }
    }
    return true;
}

static bool OnBtnAEvent(void *eventArg)
{ // A短按,切换校准步骤
    ButtonEvent_t *pEvt = (ButtonEvent_t *)eventArg;
    if (pEvt[0] == ButtonEvent_Click)
    {
        if (caliStatus == CaliSta_Margin)
        {
            SaveCalibration();
            ToCaliNone();
        }
    }
    return true;
}

static bool OnBtnYEvent(void *eventArg)
{ // Y长按,参数复位
    ButtonEvent_t *pEvt = (ButtonEvent_t *)eventArg;
    if (pEvt[0] == ButtonEvent_LongPress)
    {
        if (caliStatus == CaliSta_Margin)
        {
            ResetParams();
            SaveCalibration();
            ToCaliNone();
        }
    }
    return true;
}

void Cali_Init(void)
{
    SetPadLedStyle(PadLedStyle_On);
    LoadCalibration();
    CL_EventSysAddListener(OnBtnPairEvent, CL_Event_Button, BtnIdx_Pair);
    CL_EventSysAddListener(OnBtnAEvent, CL_Event_Button, BtnIdx_A);
    CL_EventSysAddListener(OnBtnYEvent, CL_Event_Button, BtnIdx_Y);
}

#define MID_MAX_DIFF (50)
static void MiddleProc(void)
{
    // 摇杆和扳机值稳定后,记录下来作为摇杆的中间值,扳机的最小值
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) >= 100)
    {
        lastTime = GetSysTime();

        MidVal_t temp;
        temp.leftX = GetAdcResult(AdcChan_LeftX);
        temp.leftY = GetAdcResult(AdcChan_LeftY);
        temp.rightX = GetAdcResult(AdcChan_RightX);
        temp.rightY = GetAdcResult(AdcChan_RightY);
        temp.leftHall = GetAdcResult(AdcChan_LeftHall);
        temp.rightHall = GetAdcResult(AdcChan_RightHall);

        if (CL_QueueFull(&middleQueue))
        {
            CL_QueuePoll(&middleQueue, CL_NULL);
        }
        CL_QueueAdd(&middleQueue, &temp);

        if (CL_QueueFull(&middleQueue))
        {
            MidVal_t min, max;
            memset(&min, 0xff, sizeof(MidVal_t));
            memset(&max, 0, sizeof(MidVal_t));

            MidVal_t *pData;
            CL_QUEUE_FOR_EACH(&middleQueue, pData, MidVal_t)
            {
                min.leftX = CL_MIN(min.leftX, pData->leftX);
                min.leftY = CL_MIN(min.leftY, pData->leftY);
                min.rightX = CL_MIN(min.rightX, pData->rightX);
                min.rightY = CL_MIN(min.rightY, pData->rightY);
                min.leftHall = CL_MIN(min.leftHall, pData->leftHall);
                min.rightHall = CL_MIN(min.rightHall, pData->rightHall);

                max.leftX = CL_MAX(max.leftX, pData->leftX);
                max.leftY = CL_MAX(max.leftY, pData->leftY);
                max.rightX = CL_MAX(max.rightX, pData->rightX);
                max.rightY = CL_MAX(max.rightY, pData->rightY);
                max.leftHall = CL_MAX(max.leftHall, pData->leftHall);
                max.rightHall = CL_MAX(max.rightHall, pData->rightHall);
            }

            MidVal_t diff;
            diff.leftX = max.leftX - min.leftX;
            diff.leftY = max.leftY - min.leftY;
            diff.rightX = max.rightX - min.rightX;
            diff.rightY = max.rightY - min.rightY;
            diff.leftHall = max.leftHall - min.leftHall;
            diff.rightHall = max.rightHall - min.rightHall;

            if (diff.leftX < MID_MAX_DIFF &&
                diff.leftY < MID_MAX_DIFF &&
                diff.rightX < MID_MAX_DIFF &&
                diff.rightY < MID_MAX_DIFF &&
                diff.leftHall < MID_MAX_DIFF &&
                diff.rightHall < MID_MAX_DIFF)
            {
                caliParams.leftX[1] = (min.leftX + max.leftX) / 2;
                caliParams.leftY[1] = (min.leftY + max.leftY) / 2;
                caliParams.rightX[1] = (min.rightX + max.rightX) / 2;
                caliParams.rightY[1] = (min.rightY + max.rightY) / 2;
                caliParams.leftTrigger[0] = (min.leftHall + max.leftHall) / 2;
                caliParams.rightTrigger[0] = (min.rightHall + max.rightHall) / 2;

                CL_LOG_INFO("middle: %d, %d, %d, %d, %d, %d",
                            caliParams.leftX[1],
                            caliParams.leftY[1],
                            caliParams.rightX[1],
                            caliParams.rightY[1],
                            caliParams.leftTrigger[0],
                            caliParams.rightTrigger[0]);
                ToCaliMargin();
            }
        }
    }
}

static void MarginProc(void)
{
    // 记录摇杆的最大最小值,扳机的最大值
    caliParams.leftX[0] = CL_MIN(caliParams.leftX[0], GetAdcResult(AdcChan_LeftX));
    caliParams.leftX[2] = CL_MAX(caliParams.leftX[2], GetAdcResult(AdcChan_LeftX));

    caliParams.leftY[0] = CL_MIN(caliParams.leftY[0], GetAdcResult(AdcChan_LeftY));
    caliParams.leftY[2] = CL_MAX(caliParams.leftY[2], GetAdcResult(AdcChan_LeftY));

    caliParams.rightX[0] = CL_MIN(caliParams.rightX[0], GetAdcResult(AdcChan_RightX));
    caliParams.rightX[2] = CL_MAX(caliParams.rightX[2], GetAdcResult(AdcChan_RightX));

    caliParams.rightY[0] = CL_MIN(caliParams.rightY[0], GetAdcResult(AdcChan_RightY));
    caliParams.rightY[2] = CL_MAX(caliParams.rightY[2], GetAdcResult(AdcChan_RightY));

    caliParams.leftTrigger[1] = CL_MAX(caliParams.leftTrigger[1], GetAdcResult(AdcChan_LeftHall));
    caliParams.rightTrigger[1] = CL_MAX(caliParams.rightTrigger[1], GetAdcResult(AdcChan_RightHall));
}

void Cali_Process(void)
{
    switch (caliStatus)
    {
    case CaliSta_None:
        break;
    case CaliSta_Middle:
        MiddleProc();
        break;
    case CaliSta_Margin:
        MarginProc();
        break;
    }
}

CaliStatus_t GetCaliStatus(void)
{
    return caliStatus;
}

