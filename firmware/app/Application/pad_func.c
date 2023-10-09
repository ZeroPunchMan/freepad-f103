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
#include "math.h"

static PadReport_t padReport = {
    .leftX = 0, // -32767 ~ 32767
    .leftY = 0,

    .rightX = 0,
    .rightY = 0,

    .leftTrigger = 0, // 0 ~ 255
    .rightTrigger = 0,

    .button[0] = 0,
    .button[1] = 0,
};

uint8_t vibration[PadVbrtIdx_Max] = {0};

void PadFunc_Init(void)
{
    Hc165Scan_Init();
    Cali_Init();
}

// button[0]: R3 L3 LM RM 右 左 下 上 bit7~bit0
// button[1]: Y X B A PAIR XBOX RB LB
static const uint8_t padBtn0Offset[8] = {11, 8, 10, 9, 6, 14, 12, 13};
static const uint8_t padBtn1Offset[8] = {15, 7, 5, 4, 1, 0, 2, 3};

static int16_t StickAdcToHid(uint16_t adc, uint16_t min, uint16_t middle, uint16_t max)
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

static uint8_t HallAdcToHid(uint16_t adc, uint16_t min, uint16_t max)
{ // uint8_t
    if (adc < min)
        return 0;
    else if (adc < max)
        return (adc - min) * 255 / (max - min);
    else
        return 255;
}

void PadFunc_Process(void)
{
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) > 1) // todo interval
    {
        lastTime = GetSysTime();

        uint8_t temp[2];
        Hc165Scan(16, temp);
        uint16_t btnValue = CL_BytesToUint16(temp, CL_LittleEndian);

        static uint8_t testOffset = 0;
        testOffset++;
        if (testOffset >= 16)
            testOffset = 0;

        // buttons
        padReport.button[0] = 0;
        padReport.button[1] = 0;
        for (int i = 0; i < 8; i++)
        {
            if (btnValue & (1 << padBtn0Offset[i]))
            {
                padReport.button[0] |= 1 << i;
            }

            if (btnValue & (1 << padBtn1Offset[i]))
            {
                padReport.button[1] |= 1 << i;
            }
        }

        // CL_LOG_LINE("button %04x: %02x, %02x", btnValue, padReport.button[0], padReport.button[1]);

        const CaliParams_t *caliParams = GetCaliParams();

        // CL_LOG_LINE("adc left: %d,%d; right: %d,%d; lt: %d; rt: %d;",
        //             GetAdcResult(AdcChan_LeftX),
        //             GetAdcResult(AdcChan_LeftY),
        //             GetAdcResult(AdcChan_RightX),
        //             GetAdcResult(AdcChan_RightY),
        //             GetAdcResult(AdcChan_LeftHall),
        //             GetAdcResult(AdcChan_RightHall));

        // sticks
        padReport.leftX = -StickAdcToHid(GetAdcResult(AdcChan_LeftX),
                                        caliParams->leftX[0],
                                        caliParams->leftX[1],
                                        caliParams->leftX[2]);

        padReport.leftY = -StickAdcToHid(GetAdcResult(AdcChan_LeftY),
                                        caliParams->leftY[0],
                                        caliParams->leftY[1],
                                        caliParams->leftY[2]);

        padReport.rightX = -StickAdcToHid(GetAdcResult(AdcChan_RightX),
                                         caliParams->rightX[0],
                                         caliParams->rightX[1],
                                         caliParams->rightX[2]);

        padReport.rightY = -StickAdcToHid(GetAdcResult(AdcChan_RightY),
                                         caliParams->rightY[0],
                                         caliParams->rightY[1],
                                         caliParams->rightY[2]);
        // hall
        padReport.leftTrigger = HallAdcToHid(GetAdcResult(AdcChan_LeftHall),
                                             caliParams->leftTrigger[0], caliParams->leftTrigger[1]);
        padReport.rightTrigger = HallAdcToHid(GetAdcResult(AdcChan_RightHall),
                                              caliParams->rightTrigger[0], caliParams->rightTrigger[1]);

        // CL_LOG_LINE("left: %d,%d; right: %d,%d; lt: %d; rt: %d;",
        //             padReport.leftX,
        //             padReport.leftY,
        //             padReport.rightX,
        //             padReport.rightY,
        //             padReport.leftTrigger,
        //             padReport.rightTrigger);

        //**************simulation************
        // static bool press = false;
        // if(press)
        // {
        //     padReport.button[0] = 0xf6;
        //     padReport.button[1] = 0xf3;
        //     padReport.leftTrigger = 0xff;
        //     padReport.rightTrigger = 0;

        //     padReport.leftX = INT16_MAX;
        //     padReport.leftY = INT16_MAX;

        //     padReport.rightX = INT16_MAX;
        //     padReport.rightY = INT16_MAX;
        // }
        // else
        // {
        //     padReport.button[0] = 0x09;
        //     padReport.button[1] = 0;
        //     padReport.leftTrigger = 0;
        //     padReport.rightTrigger = 0xff;

        //     padReport.leftX = INT16_MIN;
        //     padReport.leftY = INT16_MIN;

        //     padReport.rightX = INT16_MIN;
        //     padReport.rightY = INT16_MIN;
        // }
        // press = !press;

        USBD_SendPadReport(&hUsbDeviceFS, &padReport);

        PwmSetDuty(PwmChan_MotorLeftBottom, vibration[PadVbrtIdx_LeftBottom] / 5);
        PwmSetDuty(PwmChan_MotorRightBottom, vibration[PadVbrtIdx_RightBottom] / 5);
    }

    Cali_Process();
}

void SetPadVibration(PadVbrtIdx_t idx, uint8_t vbrt)
{
    vibration[idx] = vbrt;
}

bool IsButtonPress(PadBtnIdx_t idx)
{
    switch (idx)
    {
    case PadBtnIdx_Pair:
        return (padReport.button[1] & (1 << 3)) != 0;
    case PadBtnIdx_A:
        return (padReport.button[1] & (1 << 4)) != 0;
    }
    return false;
}
