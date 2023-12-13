#include "led.h"
#include "board.h"
#include "systime.h"
#include "cl_log.h"
#include "tim.h"

typedef void (*InitFunc)(void);
typedef void (*SwitchFunc)(uint8_t brightness);

static void McuStaLed_Switch(uint8_t brightness);
static void PadLed_Switch(uint8_t brightness);

typedef struct
{
    InitFunc initFunc;
    SwitchFunc switchFunc;
} LedContext_t;

typedef enum
{
    LedIdx_Pad,
    LedIdx_McuStatus,
    LedIdx_Max,
} LedIndex_t;

const LedContext_t ledContext[LedIdx_Max] = {
    [LedIdx_Pad] = {.initFunc = NULL, .switchFunc = PadLed_Switch},
    [LedIdx_McuStatus] = {.initFunc = NULL, .switchFunc = McuStaLed_Switch},
};

static void McuStaLed_Switch(uint8_t brightness)
{
    if (!brightness)
        LL_GPIO_SetOutputPin(STA_LED_PROT, STA_LED_PIN);
    else
        LL_GPIO_ResetOutputPin(STA_LED_PROT, STA_LED_PIN);
}

static void PadLed_Switch(uint8_t brightness)
{
    PwmSetDuty(PwmChan_PadLed, brightness);
}

//----------------pad led-----------------------------
static PadLedStyle_t padLedStyle = PadLedStyle_On;
static uint8_t padLedCurBn = 0;
#define PAD_LED_MAX_BN (80)
void PadLedProc(void)
{
    static uint32_t lastTime = 0;
    switch (padLedStyle)
    {
    case PadLedStyle_On:
    case PadLedStyle_Off:
        break;
    case PadLedStyle_Blink:
        if (SysTimeSpan(lastTime) >= 200)
        {
            lastTime = GetSysTime();

            if (padLedCurBn)
                padLedCurBn = 0;
            else
                padLedCurBn = PAD_LED_MAX_BN;

            ledContext[LedIdx_Pad].switchFunc(padLedCurBn);
        }
        break;
    case PadLedStyle_Breath:
    {
        uint32_t breathDelay = 10;
        if (padLedCurBn < 20)
            breathDelay += 50;
        if (SysTimeSpan(lastTime) >= breathDelay)
        {
            lastTime = GetSysTime();

            static uint8_t dir = 1;
            if (dir)
            {
                if (padLedCurBn < PAD_LED_MAX_BN)
                    padLedCurBn++;
                if (padLedCurBn >= PAD_LED_MAX_BN)
                    dir = 0;
            }
            else
            {
                if (padLedCurBn > 0)
                    padLedCurBn--;
                if (padLedCurBn == 0)
                    dir = 1;
            }
            ledContext[LedIdx_Pad].switchFunc(padLedCurBn);
        }
    }
    break;
    default:
        break;
    }
}
void SetPadLedStyle(PadLedStyle_t style)
{
    switch (style)
    {
    case PadLedStyle_On:
        padLedCurBn = PAD_LED_MAX_BN;
        ledContext[LedIdx_Pad].switchFunc(padLedCurBn);
        break;
    case PadLedStyle_Off:
        padLedCurBn = 0;
        ledContext[LedIdx_Pad].switchFunc(padLedCurBn);
        break;
    case PadLedStyle_Blink:
        break;
    case PadLedStyle_Breath:
        break;
    default:
        break;
    }
    padLedStyle = style;
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

        CL_LOG_INFO(LED, "ble led: %d\r\n", ledOn);
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
    PadLedProc();
}
