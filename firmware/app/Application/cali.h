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

//校准流程
//1.长按pair键,进入校准中间值状态,led为呼吸灯效果
//2.松开摇杆和扳机,过几秒后,按A进入校准边界值状态,led为0.5s闪烁效果
//3,摇杆搓圈,按住扳机,过几秒后,按A结束校准,led改为常亮

