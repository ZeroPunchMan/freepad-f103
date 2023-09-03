#pragma once

#include "cl_common.h"
#include "sgp_protocol.h"
#include "sgp_cmd.h"

void Comm_Init(void);
void Comm_Process(void);

uint8_t *Comm_GetRecvBuff(void);
bool Comm_RecvDone(uint32_t len);

static inline CL_Result_t Comm_SendMsg(SpgCmd_t cmd, SgpSubCmd_t subCmd, const uint8_t *data, uint8_t length)
{
    return SgpProtocol_SendMsg(SpgChannelHandle_Acm, (uint8_t)cmd, (uint8_t)subCmd, data, length);
}
