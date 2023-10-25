#pragma once

#include "cl_common.h"

void PadFunc_Init(void);
void PadFunc_Process(void);

typedef enum
{
    PadVbrtIdx_LeftBottom,
    PadVbrtIdx_RightBottom,
    PadVbrtIdx_Max,
} PadVbrtIdx_t;
void SetPadVibration(PadVbrtIdx_t idx, uint8_t vbrt);


