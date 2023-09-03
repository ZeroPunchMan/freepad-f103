#include "dfu.h"
#include "flash_layout.h"
#include "cl_log.h"
#include "systime.h"
#include "crc.h"
#include "mmlib_config.h"
#include "main.h"

void jump_to_addr(uint32_t new_msp, uint32_t addr)
{
    __set_MSP(new_msp);
    ((void (*)(void))addr)();
}

void JumpToApp(void)
{
    jump_to_addr(APP_START_ADDR, APP_START_ADDR + 4); // 跳转到APP
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

CL_Result_t EraseBakSection(void)
{
    return EraseFlash(DFU_BAK_START_ADDR, APP_MAX_SIZE / FLASH_PAGE_SIZE);
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

bool IsDfuBakValid(void)
{
    const AppInfo_t *pInfo = (const AppInfo_t *)DFU_APP_INFO_ADDR;
    if (pInfo->size > APP_MAX_SIZE)
        return false;

    uint32_t hash = Ethernet_CRC32((const uint8_t *)DFU_BAK_START_ADDR, pInfo->size);

    CL_LOG("check bak, size: %u, calc %x, save: %x", pInfo->size, hash, pInfo->hash);
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
    return CL_ResSuccess;
}

CL_Result_t CopyDfuBakToApp(void)
{
    const AppInfo_t *pInfo = (const AppInfo_t *)DFU_APP_INFO_ADDR;
    if (pInfo->size > APP_MAX_SIZE)
        return false;

    EraseAppSection();

    WriteFlash(APP_START_ADDR, (const uint8_t *)DFU_BAK_START_ADDR, pInfo->size);
    return CL_ResSuccess;
}

bool NeedDfu(void)
{

    return false;
}

CL_Result_t UnmarkDfu(void)
{
    return CL_ResSuccess;
}
