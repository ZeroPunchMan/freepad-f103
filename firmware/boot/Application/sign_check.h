#pragma once

#include "cl_common.h"

void SignCheck_Init(void);
CL_Result_t SingCheck(const uint8_t *data, uint32_t dataSize, const uint8_t *sign, uint32_t signSize);
