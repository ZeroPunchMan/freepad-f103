#pragma once

#include "cl_common.h"


void Led_Init(void);

void Led_Process(void);


typedef enum
{
    McuLedStyle_FastBlink,
    McuLedStyle_NormalBlink,
    McuLedStyle_SlowBlink,
} McuLedStyle_t;

void SetMcuLedStyle(McuLedStyle_t style);


typedef enum
{
    PadLedStyle_Off,
    PadLedStyle_On,
    PadLedStyle_Blink,
    PadLedStyle_Breath,
} PadLedStyle_t;

void SetPadLedStyle(PadLedStyle_t style);
