#include "dfu.h"
#include "flash_layout.h"
#include "cl_log.h"
#include "systime.h"
#include "crc.h"
#include "mmlib_config.h"
#include "board.h"
#include "iflash_stm32.h"

typedef void (*pFunction)(void);
pFunction JumpToAddrFunc;

void JumpToApp(void)
{
    uint32_t JumpAddress; // 跳转地址

    JumpAddress = *(volatile uint32_t *)(APP_START_ADDR + 4); // 获取复位地址
    JumpToAddrFunc = (pFunction)JumpAddress;                  // 函数指针指向复位地址
    __set_MSP(*(volatile uint32_t *)APP_START_ADDR);          // 设置主堆栈指针MSP指向升级机制IAP_ADDR
    JumpToAddrFunc();                                         // 跳转到升级代码处
}

//********************************flash操作******************************************
typedef struct
{
    uint32_t size;
    uint32_t hash;
} AppInfo_t;

CL_Result_t EraseAppSection(void)
{
    return EraseFlash(APP_START_ADDR, APP_MAX_SIZE / FLASH_PAGE_SIZE);
}

CL_Result_t SaveAppInfo(uint32_t addr, uint32_t size)
{
    AppInfo_t info;
    info.size = size;
    info.hash = Ethernet_CRC32((const uint8_t *)addr, size);
    EraseFlash(DFU_APP_INFO_ADDR, 1);
    WriteFlash(DFU_APP_INFO_ADDR, (const uint8_t *)&info, sizeof(info));

    return CL_ResSuccess;
}

bool IsAppValid(void)
{
    const AppInfo_t *pInfo = (const AppInfo_t *)DFU_APP_INFO_ADDR;

    if (pInfo->size > APP_MAX_SIZE)
        return false;

    uint32_t hash = Ethernet_CRC32((const uint8_t *)APP_START_ADDR, pInfo->size);

    CL_LOG_INFO("check app, size: %u, calc %x, save: %x", pInfo->size, hash, pInfo->hash);
    return hash == pInfo->hash;
}

CL_Result_t EraseFlash(uint32_t addr, uint32_t pages)
{
    return IFlashStm32_ErasePages(addr, pages);
}

CL_Result_t WriteFlash(uint32_t addr, const uint8_t *buff, uint32_t length)
{
    return IFlashStm32_Write(addr, buff, length);
}

bool NeedDfu(void)
{
    LL_APB2_GRP1_EnableClock(GPIO_APB);
    Mmhl_GpioInit(BTN_PAIR_PORT, BTN_PAIR_PIN, LL_GPIO_MODE_INPUT, LL_GPIO_PULL_DOWN);
    if (Mmhl_GpioReadInput(BTN_PAIR_PORT, BTN_PAIR_PIN) == 1)
        return true;

    return false;
}

CL_Result_t UnmarkDfu(void)
{
    return CL_ResSuccess;
}
