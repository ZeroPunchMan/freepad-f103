#include "hc165scan.h"
#include "board.h"
#include "mmlib_config.h"

#define DELAY_NUM (3)

static inline void Delay(uint8_t x)
{
}

//*****************GPIO操作********************
static inline void ClkPin(uint8_t level)
{
    Mmhl_GpioSetOutput(HC165_CLK_PORT, HC165_CLK_PIN, level);
}

static inline void LoadPin(bool level)
{
    Mmhl_GpioSetOutput(HC165_LOAD_PORT, HC165_LOAD_PIN, level);
}

static inline uint8_t ReadDatPin(void)
{
    return Mmhl_GpioReadInput(HC165_DAT_PORT, HC165_DAT_PIN);
}

void Hc165Scan_Init(void)
{
    Mmhl_GpioInit(HC165_CLK_PORT, HC165_CLK_PIN, LL_GPIO_MODE_OUTPUT);
    Mmhl_GpioInit(HC165_LOAD_PORT, HC165_LOAD_PIN, LL_GPIO_MODE_OUTPUT);
    Mmhl_GpioInit(HC165_DAT_PORT, HC165_DAT_PIN, LL_GPIO_MODE_INPUT);

    ClkPin(1);
    LoadPin(1);
}

void Hc165Scan(uint16_t numOfBits, uint8_t *buff)
{
    LoadPin(0);
    Delay(DELAY_NUM);
    LoadPin(1);
    Delay(DELAY_NUM);

    for (uint16_t i = 0; i < (numOfBits + 7) / 8; i++)
    {
        buff[i] = 0;
    }

    for (uint16_t i = 0; i < numOfBits; i++)
    {
        uint8_t level = ReadDatPin();

        uint16_t byteOffset = i / 8;
        uint16_t bitOffset = i % 8;
        buff[byteOffset] |= level << bitOffset;
        
        ClkPin(0);
        Delay(DELAY_NUM);
        ClkPin(1);
        Delay(DELAY_NUM);
    }
}
