#pragma once

#include "cl_common.h"

typedef enum
{
    SpgCmd_Dfu = 0x01,
} SpgCmd_t;

typedef enum
{
    SgpSubCmd_DfuReq = 0x70,
    SgpSubCmd_DfuData = 0x71,
    SgpSubCmd_DfuVerify = 0x72,
    SgpSubCmd_DfuBootVer = 0x73,
    SgpSubCmd_AppVer = 0x74,

    SgpSubCmd_DfuReady = 0x70 | 0x80,
    SgpSubCmd_DfuDataRsp = 0x71 | 0x80,
    SgpSubCmd_DfuVerifyRsp = 0x72 | 0x80,
    SgpSubCmd_DfuBootVerRsp = 0x73 | 0x80,
    SgpSubCmd_AppVerRsp = 0x74 | 0x80,

    SgpSubCmd_DfuError = 0x7f | 0x80,
} SgpSubCmd_t;
