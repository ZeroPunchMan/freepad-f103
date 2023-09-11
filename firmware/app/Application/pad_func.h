#pragma once

#include "cl_common.h"

void PadFunc_Init(void);
void PadFunc_Process(void);

typedef enum
{
    XosBtnIdx_Pair,
    XosBtnIdx_A,
} XosBtnIdx_t;

bool IsButtonPress(XosBtnIdx_t);
