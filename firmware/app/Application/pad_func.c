#include "pad_func.h"
#include "main.h"
#include "crc.h"
#include "string.h"
#include "flash_layout.h"
#include "iflash_stm32.h"
#include "cl_log.h"
#include "adc.h"
#include "systime.h"
#include "hc165scan.h"
#include "usbd_custom_hid_if.h"
#include "cl_serialize.h"

static XosHidReport_t xosReport =
    {
        .leftX = UINT16_MAX, // max 65535
        .leftY = UINT16_MAX,

        .rightX = UINT16_MAX / 2,
        .rightY = UINT16_MAX / 2,

        .leftTrigger = 0x3ff, // max 0x3ff
        .rightTrigger = 0x3ff,

        .dPad = 0, // 1~8
        .button[0] = 0,
        .button[1] = 0,
        .reserved = 0,
};

typedef struct
{
    uint16_t leftX[3], leftY[3];              // min,middle,max
    uint16_t rightX[3], rightY[3];            // min,middle,max
    uint16_t leftTrigger[2], rightTrigger[2]; // min,max
    uint32_t crc;
} CaliParams_t;
static CaliParams_t caliParams = {0};

static void PrintParams(CaliParams_t *params)
{
}

static void LoadCalibration(void)
{
    memcpy((void *)&caliParams, (void *)PAD_PARAM_ADDR, sizeof(caliParams));
    uint32_t crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    if (crc != caliParams.crc)
    { // use default params
        CL_LOG_LINE("use default params");
        caliParams.leftX[0] = 0;
        caliParams.leftX[1] = 2048;
        caliParams.leftX[2] = 4096;

        caliParams.leftY[0] = 0;
        caliParams.leftY[1] = 2048;
        caliParams.leftY[2] = 4096;

        caliParams.rightX[0] = 0;
        caliParams.rightX[1] = 2048;
        caliParams.rightX[2] = 4096;

        caliParams.rightY[0] = 0;
        caliParams.rightY[1] = 2048;
        caliParams.rightY[2] = 4096;

        caliParams.leftTrigger[0] = 0;
        caliParams.leftTrigger[1] = 4096;

        caliParams.rightTrigger[0] = 0;
        caliParams.rightTrigger[1] = 4096;
    }
    else
    {
        CL_LOG_LINE("use saved params");
        PrintParams(&caliParams);
    }
}

static void SaveCalibration(void)
{
    caliParams.crc = Ethernet_CRC32((const uint8_t *)&caliParams, CL_OFFSET_OF(CaliParams_t, crc));
    HAL_FLASH_Unlock();
    IFlashStm32_ErasePages(PAD_PARAM_ADDR, 1);
    IFlashStm32_Write(PAD_PARAM_ADDR, (const uint8_t *)&caliParams, sizeof(caliParams));
    HAL_FLASH_Lock();
}

void PadFunc_Init(void)
{
    LoadCalibration();
}

static const uint8_t xosBtn0Offset[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static const uint8_t xosBtn1Offset[8] = {8, 9, 10, 11, 12, 13, 14, 15};
static const uint8_t xosDpadOffset[4] = {9, 10, 11, 12};
uint8_t DirToDpad(bool up, bool right, bool down, bool left)
{
    uint8_t bitsFlag = 0;
    bitsFlag |= (up ? 1 : 0) << 3;
    bitsFlag |= (right ? 1 : 0) << 2;
    bitsFlag |= (down ? 1 : 0) << 1;
    bitsFlag |= (left ? 1 : 0) << 0;

    switch (bitsFlag)
    {
    case 0x01:
        return 1;
    case 0x03:
        return 2;
    case 0x02:
        return 3;
    case 0x06:
        return 4;
    case 0x04:
        return 5;
    case 0x0c:
        return 6;
    case 0x08:
        return 7;
    case 0x09:
        return 8;
    default:
        return 0;
    }
}

static uint16_t StickAdcToHid(uint16_t adc, uint16_t min, uint16_t middle, uint16_t max)
{ // 0 ~ 65535
    if (adc < min)
        return 0;
    else if (adc < middle)
        return (adc - min) * 32767ul / (middle - min);
    else if (adc < max)
        return (adc - middle) * 32767ul / (max - middle);
    else
        return UINT16_MAX;
}

static uint16_t HallAdcToHid(uint16_t adc, uint16_t min, uint16_t max)
{ // 0 ~ 1023
    if (adc < min)
        return 0;
    else if (adc < max)
        return (adc - min) * 1023ul / (max - min);
    else
        return 0x3ff;
}

void PadFunc_Process(void)
{ // dmaCount
    static uint32_t lastTime = 0;
    if (SysTimeSpan(lastTime) > 5)
    {
        lastTime = GetSysTime();

        uint8_t temp[2];
        Hc165Scan(16, temp);
        uint16_t btnValue = CL_BytesToUint16(temp, CL_LittleEndian);

        // dpad
        bool up, right, down, left;
        up = (btnValue & (1 << xosDpadOffset[0])) != 0;
        right = (btnValue & (1 << xosDpadOffset[1])) != 0;
        down = (btnValue & (1 << xosDpadOffset[2])) != 0;
        left = (btnValue & (1 << xosDpadOffset[3])) != 0;
        xosReport.dPad = DirToDpad(up, right, down, left);

        // buttons
        xosReport.button[0] = 0;
        xosReport.button[1] = 0;
        for (int i = 0; i < 8; i++)
        {
            if (btnValue & (1 << xosBtn0Offset[i]))
            {
                xosReport.button[0] |= 1 << i;
            }

            if (btnValue & (1 << xosBtn1Offset[i]))
            {
                xosReport.button[1] |= 1 << i;
            }
        }

        // sticks
        xosReport.leftX = StickAdcToHid(GetAdcResult(AdcChan_LeftX),
                                        caliParams.leftX[0],
                                        caliParams.leftX[1],
                                        caliParams.leftX[2]);

        xosReport.leftY = StickAdcToHid(GetAdcResult(AdcChan_LeftY),
                                        caliParams.leftY[0],
                                        caliParams.leftY[1],
                                        caliParams.leftY[2]);

        xosReport.rightX = StickAdcToHid(GetAdcResult(AdcChan_RightX),
                                         caliParams.rightX[0],
                                         caliParams.rightX[1],
                                         caliParams.rightX[2]);

        xosReport.rightY = StickAdcToHid(GetAdcResult(AdcChan_RightY),
                                         caliParams.rightY[0],
                                         caliParams.rightY[1],
                                         caliParams.rightY[2]);
        // hall
        xosReport.leftTrigger = HallAdcToHid(GetAdcResult(AdcChan_LeftHall),
                                             caliParams.leftTrigger[0], caliParams.leftTrigger[1]);
        xosReport.rightTrigger = HallAdcToHid(GetAdcResult(AdcChan_LeftHall),
                                              caliParams.rightTrigger[0], caliParams.rightTrigger[1]);

        USBD_SendXosReport(&xosReport);
    }
}
