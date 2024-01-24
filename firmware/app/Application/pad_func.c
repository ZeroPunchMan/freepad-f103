#include "pad_func.h"
#include "main.h"
#include "cl_log.h"
#include "adc.h"
#include "systime.h"
#include "cl_serialize.h"
#include "tim.h"
#include "led.h"
#include "cali.h"
#include "usb_device.h"
#include "usbd_hid.h"
#include "math.h"
#include "board.h"

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
    Cali_Init();
}

typedef struct
{
    GPIO_TypeDef *port;
    uint32_t pin;
} BtnPinDef_t;
// button[0]: R3 L3 LM RM 右 左 下 上 bit7~bit0
BtnPinDef_t btn0PinDef[8] = {
    [7] = {BTN_RSTICK_PORT, BTN_RSTICK_PIN},
    [6] = {BTN_LSTICK_PORT, BTN_LSTICK_PIN},
    [5] = {BTN_LMENU_PORT, BTN_LMENU_PIN},
    [4] = {BTN_RMENU_PORT, BTN_RMENU_PIN},
    [3] = {BTN_RIGHT_PORT, BTN_RIGHT_PIN},
    [2] = {BTN_LEFT_PORT, BTN_LEFT_PIN},
    [1] = {BTN_DOWN_PORT, BTN_DOWN_PIN},
    [0] = {BTN_UP_PORT, BTN_UP_PIN},
};
// button[1]: Y X B A PAIR XBOX RB LB
BtnPinDef_t btn1PinDef[8] = {
    [7] = {BTN_Y_PORT, BTN_Y_PIN},
    [6] = {BTN_X_PORT, BTN_X_PIN},
    [5] = {BTN_B_PORT, BTN_B_PIN},
    [4] = {BTN_A_PORT, BTN_A_PIN},
    [3] = {BTN_PAIR_PORT, BTN_PAIR_PIN},
    [2] = {BTN_XBOX_PORT, BTN_XBOX_PIN},
    [1] = {BTN_RB_PORT, BTN_RB_PIN},
    [0] = {BTN_LB_PORT, BTN_LB_PIN},
};

static inline bool IsButtonPressed(GPIO_TypeDef *port, uint32_t pin)
{
    return LL_GPIO_IsInputPinSet(port, pin);
}

static int16_t StickAdcToHid(uint16_t adc, uint16_t min, uint16_t middle, uint16_t max)
{ // int16_t
    if (adc < min)
    {
        return INT16_MIN;
    }
    else if (adc < middle)
    {
        float ratio = (float)(middle - adc) / (middle - min);
        ratio = sqrt(ratio);
        // ratio = cbrt(ratio);
        return ratio * INT16_MIN;
    }
    else if (adc < max)
    {
        float ratio = (float)(adc - middle) / (max - middle);
        ratio = sqrt(ratio);
        // ratio = cbrt(ratio);
        return ratio * INT16_MAX;
    }
    else
    {
        return INT16_MAX;
    }
}

static uint8_t HallAdcToHid(uint16_t adc, uint16_t min, uint16_t max)
{ // uint8_t
    if (adc < min)
    {
        return 0;
    }
    else if (adc < max)
    { // 三次方插值
        uint16_t total = max - min;
        float ratio = (float)(adc - min) / total;
        ratio = sqrt(ratio);
        // ratio = cbrt(ratio);
        return ratio * 255;
    }
    else
    {
        return 255;
    }
}

void PadFunc_Process(void)
{
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) >= 1)
    {
        lastTime = GetSysTime();

        // button0
        padReport.button[0] = 0;
        for (int i = 0; i < 8; i++)
        {
            if (IsButtonPressed(btn0PinDef[i].port, btn0PinDef[i].pin))
                padReport.button[0] |= 1 << i;
        }

        // button1
        padReport.button[1] = 0;
        for (int i = 0; i < 8; i++)
        {
            if (IsButtonPressed(btn1PinDef[i].port, btn1PinDef[i].pin))
                padReport.button[1] |= 1 << i;
        }

        // CL_LOG_INFO("button: %02x, %02x", padReport.button[0], padReport.button[1]);

        if (GetCaliStatus() == CaliSta_None)
        {
            const CaliParams_t *caliParams = GetCaliParams();
            // sticks
            padReport.leftX = StickAdcToHid(GetAdcResult(AdcChan_LeftX),
                                            caliParams->leftX[0],
                                            caliParams->leftX[1],
                                            caliParams->leftX[2]);

            padReport.leftY = StickAdcToHid(GetAdcResult(AdcChan_LeftY),
                                            caliParams->leftY[0],
                                            caliParams->leftY[1],
                                            caliParams->leftY[2]);

            padReport.rightX = StickAdcToHid(GetAdcResult(AdcChan_RightX),
                                             caliParams->rightX[0],
                                             caliParams->rightX[1],
                                             caliParams->rightX[2]);

            padReport.rightY = StickAdcToHid(GetAdcResult(AdcChan_RightY),
                                             caliParams->rightY[0],
                                             caliParams->rightY[1],
                                             caliParams->rightY[2]);
            // hall
            padReport.leftTrigger = HallAdcToHid(GetAdcResult(AdcChan_LeftHall),
                                                 caliParams->leftTrigger[0], caliParams->leftTrigger[1]);
            padReport.rightTrigger = HallAdcToHid(GetAdcResult(AdcChan_RightHall),
                                                  caliParams->rightTrigger[0], caliParams->rightTrigger[1]);
        }
        else
        {
            padReport.leftX = 0;
            padReport.leftY = 0;
            padReport.rightX = 0;
            padReport.rightY = 0;
            padReport.leftTrigger = 0;
            padReport.rightTrigger = 0;
        }

        USBD_SendPadReport(&hUsbDeviceFS, &padReport);

        PwmSetDuty(PwmChan_MotorLeft, vibration[PadVbrtIdx_LeftBottom]);
        PwmSetDuty(PwmChan_MotorRight, vibration[PadVbrtIdx_RightBottom]);
    }

    Cali_Process();
}

void SetPadVibration(PadVbrtIdx_t idx, uint8_t vbrt)
{
    // 原始值0~255,不要超100
    if (vbrt > 0)
    {
        vbrt = CL_CLAMP(vbrt, 0, 250);
        vibration[idx] = vbrt / 5 + 50;
    }
    else
    {
        vibration[idx] = 0;
    }
}
