#include "dfu.h"
#include "flash_layout.h"
#include "cl_log.h"
#include "systime.h"
#include "crc.h"
#include "mmlib_config.h"
#include "board.h"

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

    CL_LOG("check app, size: %u, calc %x, save: %x", pInfo->size, hash, pInfo->hash);
    return hash == pInfo->hash;
}

CL_Result_t EraseFlash(uint32_t addr, uint32_t pages)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = pages;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
        return CL_ResFailed;

    return CL_ResSuccess;
}

CL_Result_t WriteFlash(uint32_t addr, const uint8_t *buff, uint32_t length)
{
    uint32_t writeAddr, offset;
    uint32_t data;
    HAL_StatusTypeDef status;

    /* Clear All pending flags */
    // __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR | FLASH_FLAG_PROGERR);

    // little endian [0-0][0-1][0-2][0-3][1-0][1-1][1-2][1-3] -> 3210
    offset = 0;
    writeAddr = addr;
    while (offset < length)
    {
        data = 0;
        for (int i = 0; i < 4; i++)
        {
            if (offset < length)
                data |= (uint32_t)buff[offset++] << (i * 8);
            else
                break;
        }
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, writeAddr, data);
        if (status != HAL_OK)
            return CL_ResFailed;

        writeAddr += 4;
    }
    return CL_ResSuccess;
}

bool NeedDfu(void)
{
    LL_APB2_GRP1_EnableClock(GPIO_APB);
    Mmhl_GpioInit(DFU_BTN_PORT, DFU_BTN_PIN, LL_GPIO_MODE_INPUT);
    if (Mmhl_GpioReadInput(DFU_BTN_PORT, DFU_BTN_PIN) == 0)
        return true;

    return false;
}

CL_Result_t UnmarkDfu(void)
{
    return CL_ResSuccess;
}
