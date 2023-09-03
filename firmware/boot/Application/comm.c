#include "comm.h"
#include "usb_device.h"
#include "cl_event_system.h"
#include "multi_buffer.h"
#include "cl_log.h"
#include "cl_queue.h"
#include "usbd_cdc_if.h"

//**********************receive********************************
MULTIBUFFER_STATIC_DEF(recvMultiBuff, 64, 10, static);
uint8_t *Comm_GetRecvBuff(void)
{
    uint8_t *pBuff = NULL;
    int res = MultiBufferGetBack(&recvMultiBuff, &pBuff);
    if (res != 0)
    { // exception
    }
    return pBuff;
}

bool Comm_RecvDone(uint32_t len)
{
    uint8_t *pBuff = NULL;
    MultiBufferGetBack(&recvMultiBuff, &pBuff);
    return MultiBufferPush(&recvMultiBuff, len) == 0;
}
//--------------------------------------------------------

//**********************send********************************
CL_QUEUE_DEF_INIT(sendQueue, 256, uint8_t, static);
CL_Result_t SgpAcmSendFunc(const uint8_t *buff, uint16_t count)
{
    if (CL_QueueFreeSpace(&sendQueue) < count)
        return CL_ResFailed;

    for (uint16_t i = 0; i < count; i++)
        CL_QueueAdd(&sendQueue, (void *)&buff[i]);
    return CL_ResSuccess;
}
//--------------------------------------------------------

void Comm_Init(void)
{
    SgpProtocol_AddChannel(SpgChannelHandle_Acm, SgpAcmSendFunc);
}

void Comm_Process(void)
{
    //********receive*******
    if (MultiBufferGetCount(&recvMultiBuff) > 0)
    {
        uint8_t *pRecvBuff;
        uint32_t bufLen;
        MultiBufferPeek(&recvMultiBuff, 0, &pRecvBuff, &bufLen);

        SgpProtocol_RecvData(SpgChannelHandle_Acm, pRecvBuff, bufLen);

        MultiBufferPop(&recvMultiBuff);
    }

    //********send*******
    if (CDC_GetTransmitStatus() == USBD_OK && CL_QueueLength(&sendQueue) > 0)
    {
        uint16_t len = CL_QueueLength(&sendQueue);
        if (len > APP_TX_DATA_SIZE)
            len = APP_TX_DATA_SIZE;

        for (uint16_t i = 0; i < len; i++)
        {
            CL_QueuePoll(&sendQueue, &UserTxBufferFS[i]);
        }

        CDC_Transmit_FS(UserTxBufferFS, len);
    }
}
