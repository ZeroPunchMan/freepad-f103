#pragma once

#include "cl_common.h"

typedef struct
{
    uint16_t leftX[3], leftY[3];              // min,middle,max
    uint16_t rightX[3], rightY[3];            // min,middle,max
    uint16_t leftTrigger[2], rightTrigger[2]; // min,max
    uint32_t crc;
} CaliParams_t;


void Cali_Init(void);
void Cali_Process(void);
const CaliParams_t* GetCaliParams(void);
