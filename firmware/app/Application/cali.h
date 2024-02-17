#pragma once

#include "cl_common.h"
#include "vector2.h"

typedef enum
{
    CaliSta_None,   // 正常模式
    CaliSta_Middle, // 校准中间值
    CaliSta_Margin, // 校准边界值
} CaliStatus_t;

typedef struct
{
    uint16_t leftMag[30], leftMidX, leftMidY;    // 30个角度长度; 中间值
    uint16_t rightMag[30], rightMidX, rightMidY; // 30个角度长度; 中间值
    uint16_t leftTrigger[2], rightTrigger[2];       // min,max
    uint32_t crc;
} CaliParams_t;

void Cali_Init(void);
void Cali_Process(void);
const CaliParams_t *GetCaliParams(void);

CaliStatus_t GetCaliStatus(void);
void StickCorrect(Vector2 *stick, bool left);

// 校准流程
// 1.长按pair键,进入校准中间值状态,led改为呼吸灯效果
// 2.松开摇杆和扳机,过几秒后,自动记录中间值,并进入校准边界值黄台,led改为0.5s闪烁效果
// 3,摇杆搓圈,按住扳机,过几秒后,按A结束校准,led改为常亮
