#include "pad_func.h"
#include "main.h"
#include "cl_log.h"
#include "adc.h"
#include "systime.h"
#include "hc165scan.h"
#include "cl_serialize.h"
#include "tim.h"
#include "led.h"
#include "cali.h"
#include "usb_device.h"
#include "usbd_hid.h"

static XosHidReport_t xosReport = {
    .leftX = 0, // -32767 ~ 32767
    .leftY = 0,

    .rightX = 0,
    .rightY = 0,

    .leftTrigger = 0, // 0 ~ 255
    .rightTrigger = 0,

    .button[0] = 0,
    .button[1] = 0,
};

void PadFunc_Init(void)
{
    Hc165Scan_Init();
    Cali_Init();
    SetXosLedStyle(XosLedStyle_On);
}

// button[0]: R3 L3 LM RM 右 左 下 上
// button[1]: Y X B A RES XBOX RB LB
static const uint8_t xosBtn0Offset[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static const uint8_t xosBtn1Offset[8] = {8, 9, 10, 11, 12, 13, 14, 15};

static uint16_t StickAdcToHid(uint16_t adc, uint16_t min, uint16_t middle, uint16_t max)
{ // int16_t
    if (adc < min)
        return INT16_MIN;
    else if (adc < middle)
        return (middle - adc) * INT16_MIN / (middle - min);
    else if (adc < max)
        return (adc - middle) * INT16_MAX / (max - middle);
    else
        return INT16_MAX;
}

static uint16_t HallAdcToHid(uint16_t adc, uint16_t min, uint16_t max)
{ // uint8_t
    if (adc < min)
        return 0;
    else if (adc < max)
        return (adc - min) * 255 / (max - min);
    else
        return 255;
}

void PadFunc_Process(void)
{ // dmaCount
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) > 1000) // todo interval
    {
        lastTime = GetSysTime();

        uint8_t temp[2];
        Hc165Scan(16, temp);
        uint16_t btnValue = CL_BytesToUint16(temp, CL_LittleEndian);

        // buttons
        xosReport.button[0] = 0;
        xosReport.button[1] = 0;
        for (int i = 0; i < 8; i++)
        {
            if (btnValue & (1 << xosBtn0Offset[i]))
            {
                xosReport.button[0] |= 1 << i;
            }

            if (btnValue & (1 << xosBtn1Offset[i]))
            {
                xosReport.button[1] |= 1 << i;
            }
        }
        xosReport.button[1] &= ~(1 << 3);

        const CaliParams_t *caliParams = GetCaliParams();

        // sticks
        xosReport.leftX = StickAdcToHid(GetAdcResult(AdcChan_LeftX),
                                        caliParams->leftX[0],
                                        caliParams->leftX[1],
                                        caliParams->leftX[2]);

        xosReport.leftY = StickAdcToHid(GetAdcResult(AdcChan_LeftY),
                                        caliParams->leftY[0],
                                        caliParams->leftY[1],
                                        caliParams->leftY[2]);

        xosReport.rightX = StickAdcToHid(GetAdcResult(AdcChan_RightX),
                                         caliParams->rightX[0],
                                         caliParams->rightX[1],
                                         caliParams->rightX[2]);

        xosReport.rightY = StickAdcToHid(GetAdcResult(AdcChan_RightY),
                                         caliParams->rightY[0],
                                         caliParams->rightY[1],
                                         caliParams->rightY[2]);
        // hall
        xosReport.leftTrigger = HallAdcToHid(GetAdcResult(AdcChan_LeftHall),
                                             caliParams->leftTrigger[0], caliParams->leftTrigger[1]);
        xosReport.rightTrigger = HallAdcToHid(GetAdcResult(AdcChan_RightHall),
                                              caliParams->rightTrigger[0], caliParams->rightTrigger[1]);

        //**************simulation************
        // static bool press = false;
        // if(press)
        // {
        //     xosReport.button[0] = 0xf6;
        //     xosReport.button[1] = 0xf3;
        //     xosReport.leftTrigger = 0xff;
        //     xosReport.rightTrigger = 0;

        //     xosReport.leftX = INT16_MAX;
        //     xosReport.leftY = INT16_MAX;

        //     xosReport.rightX = INT16_MAX;
        //     xosReport.rightY = INT16_MAX;
        // }
        // else
        // {
        //     xosReport.button[0] = 0x09;
        //     xosReport.button[1] = 0;
        //     xosReport.leftTrigger = 0;
        //     xosReport.rightTrigger = 0xff;

        //     xosReport.leftX = INT16_MIN;
        //     xosReport.leftY = INT16_MIN;

        //     xosReport.rightX = INT16_MIN;
        //     xosReport.rightY = INT16_MIN;
        // }
        // press = !press;

        USBD_SendXosReport(&hUsbDeviceFS, &xosReport);
    }

    Cali_Process();
}

bool IsButtonPress(XosBtnIdx_t idx)
{
    switch (idx)
    {
    case XosBtnIdx_Pair:
        return (xosReport.button[0] & (1 << 1)) != 0;
    case XosBtnIdx_A:
        return (xosReport.button[1] & (1 << 4)) != 0;
    }
    return false;
}
