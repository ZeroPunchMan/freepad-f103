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
    XosLedStyle_On,
    XosLedStyle_Off,
    XosLedStyle_Blink,
    XosLedStyle_Breath,
} XosLedStyle_t;

void SetXosLedStyle(XosLedStyle_t style);
