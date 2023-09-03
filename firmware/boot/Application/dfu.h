#pragma once

#include "cl_common.h"

void JumpToApp(void);

void Dfu_Init(void);
void Dfu_Process(void);

//*********************************
bool NeedDfu(void);
CL_Result_t UnmarkDfu(void);
CL_Result_t EraseAppSection(void);
CL_Result_t SaveAppInfo(uint32_t addr, uint32_t size);
CL_Result_t EraseFlash(uint32_t addr, uint32_t pages);
CL_Result_t WriteFlash(uint32_t addr, const uint8_t *buff, uint32_t length);
bool IsAppValid(void);

