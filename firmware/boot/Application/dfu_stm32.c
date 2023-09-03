#include "dfu.h"
#include "flash_layout.h"
#include "cl_log.h"
#include "systime.h"
#include "crc.h"
#include "mmlib_config.h"


void jump_to_addr(uint32_t new_msp, uint32_t addr)
{
    __set_MSP(new_msp);
    ((void (*)(void))addr)();
}

void JumpToApp(void)
{
    
}

//********************************flash操作******************************************
typedef struct
{
    uint32_t size;
    uint32_t hash;
} AppInfo_t;

CL_Result_t EraseAppSection(void)
{

    return CL_ResSuccess;
}

CL_Result_t EraseBakSection(void)
{

    return CL_ResSuccess;
}

CL_Result_t SaveAppInfo(uint32_t addr, uint32_t size)
{
    AppInfo_t info;
    info.size = size;
    info.hash = Ethernet_CRC32((const uint8_t *)addr, size);
    WriteFlash(DFU_APP_INFO_ADDR, (const uint8_t *)&info, sizeof(info));

    return CL_ResSuccess;
}

bool IsAppValid(void)
{
    const AppInfo_t *pInfo = (const AppInfo_t *)DFU_APP_INFO_ADDR;
    if (pInfo->size > APP_MAX_SIZE)
        return false;

    uint32_t hash = Ethernet_CRC32((const uint8_t *)APP_START_ADDR, pInfo->size);

    CL_LOG("check app, size: %lu, calc %lx, save: %lx", pInfo->size, hash, pInfo->hash);
    return hash == pInfo->hash;
}

bool IsDfuBakValid(void)
{
    const AppInfo_t *pInfo = (const AppInfo_t *)DFU_APP_INFO_ADDR;
    if (pInfo->size > APP_MAX_SIZE)
        return false;

    uint32_t hash = Ethernet_CRC32((const uint8_t *)DFU_BAK_START_ADDR, pInfo->size);

    CL_LOG("check bak, size: %lu, calc %lx, save: %lx", pInfo->size, hash, pInfo->hash);
    return hash == pInfo->hash;
}

CL_Result_t EraseFlash(uint32_t addr, uint32_t pages)
{
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
