#include "led.h"
#include "board.h"
#include "systime.h"
#include "cl_log.h"
#include "tim.h"

typedef void (*InitFunc)(void);
typedef void (*SwitchFunc)(uint8_t brightness);

static void McuStaLed_Switch(uint8_t brightness);
static void XosLed_Switch(uint8_t brightness);

typedef struct
{
    InitFunc initFunc;
    SwitchFunc switchFunc;
} LedContext_t;

typedef enum
{
    LedIdx_Xos,
    LedIdx_McuStatus,
    LedIdx_Max,
} LedIndex_t;

const LedContext_t ledContext[LedIdx_Max] = {
    [LedIdx_Xos] = {.initFunc = NULL, .switchFunc = XosLed_Switch},
    [LedIdx_McuStatus] = {.initFunc = NULL, .switchFunc = McuStaLed_Switch},
};

static void McuStaLed_Switch(uint8_t brightness)
{
    if (!brightness)
        LL_GPIO_SetOutputPin(STA_LED_PROT, STA_LED_PIN);
    else
        LL_GPIO_ResetOutputPin(STA_LED_PROT, STA_LED_PIN);
}

static void XosLed_Switch(uint8_t brightness)
{
    PwmSetDuty(PwmChan_XosLed, brightness);
}

//----------------xos led-----------------------------
static XosLedStyle_t xosLedStyle = XosLedStyle_On;
static uint8_t xosLedCurBn = 0;
#define XOS_LED_MAX_BN (99)
void XosLedProc(void)
{
    static uint32_t lastTime = 0;
    switch (xosLedStyle)
    {
    case XosLedStyle_On:
    case XosLedStyle_Off:
        break;
    case XosLedStyle_Blink:
        if (SysTimeSpan(lastTime) >= 500)
        {
            lastTime = GetSysTime();

            if (xosLedCurBn)
                xosLedCurBn = 0;
            else
                xosLedCurBn = XOS_LED_MAX_BN;

            ledContext[LedIdx_Xos].switchFunc(xosLedCurBn);
        }
        break;
    case XosLedStyle_Breath:
        if (SysTimeSpan(lastTime) >= 30)
        {
            lastTime = GetSysTime();

            static uint8_t dir = 1;
            if (dir)
            {
                if (xosLedCurBn < 100)
                    xosLedCurBn++;
                if (xosLedCurBn >= 100)
                    dir = 0;
            }
            else
            {
                if (xosLedCurBn > 0)
                    xosLedCurBn--;
                if (xosLedCurBn == 0)
                    dir = 1;
            }
            ledContext[LedIdx_Xos].switchFunc(xosLedCurBn);
        }
        break;
    default:
        break;
    }
}
void SetXosLedStyle(XosLedStyle_t style)
{
    switch (style)
    {
    case XosLedStyle_On:
        xosLedCurBn = XOS_LED_MAX_BN;
        ledContext[LedIdx_Xos].switchFunc(xosLedCurBn);
        break;
    case XosLedStyle_Off:
        xosLedCurBn = 0;
        ledContext[LedIdx_Xos].switchFunc(xosLedCurBn);
        break;
    case XosLedStyle_Blink:
        break;
    case XosLedStyle_Breath:
        break;
    default:
        break;
    }
    xosLedStyle = style;
}
//-----------------status led-------------------------
static McuLedStyle_t mcuLedStyle = McuLedStyle_SlowBlink;

void McuLedProc(void)
{
    uint32_t blinkInterval = 180;

    if (mcuLedStyle == McuLedStyle_NormalBlink)
        blinkInterval = 600;
    else if (mcuLedStyle == McuLedStyle_SlowBlink)
        blinkInterval = 1500;

    static uint32_t lastTime = 0;
    static bool ledOn = false;
    if (SysTimeSpan(lastTime) >= blinkInterval)
    {
        lastTime = GetSysTime();

        ledOn = !ledOn;
        ledContext[LedIdx_McuStatus].switchFunc(ledOn);

        // CL_LOG_LINE("ble led: %d", ledOn);
    }
}

void SetMcuLedStyle(McuLedStyle_t style)
{
    mcuLedStyle = style;
}
//------------------------------------------
void Led_Init(void)
{
    for (int i = 0; i < LedIdx_Max; i++)
    {
        if (ledContext[i].initFunc != NULL)
            ledContext[i].initFunc();
    }
}

void Led_Process(void)
{
    McuLedProc();
}
