/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_ctlreq.h"
#include "cl_serialize.h"
#include "cl_log.h"
#include "pad_func.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init(USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit(USBD_HandleTypeDef *pdev,
                                uint8_t cfgidx);

static uint8_t  USBD_HID_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req);

static uint8_t  *USBD_HID_GetFSCfgDesc(uint16_t *length);

static uint8_t  *USBD_HID_GetHSCfgDesc(uint16_t *length);

static uint8_t  *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length);

static uint8_t  *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);

static uint8_t  USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_HID_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_HID =
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,
  NULL,
  USBD_HID_GetFSCfgDesc,
  NULL,
  USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgFSDesc[]  __ALIGN_END =
{
    0x09, 0x02, 0x8B, 0x00, 0x04, 0x01, 0x00, 0xA0, 0xFA, //todo remote wakeup
    0x09, 0x04, 0x00, 0x00, 0x02, 0xFF, 0x5D, 0x01, 0x00,
    0x11, 0x21, 0x10, 0x01, 0x01, 0x25, 0x81, 0x14, 0x03, 0x03, 0x03, 0x04, 0x13, 0x02, 0x08, 0x03, 0x03,
    0x07, 0x05, 0x81, 0x03, 0x20, 0x00, 0x04,
    0x07, 0x05, 0x02, 0x03, 0x20, 0x00, 0x08,
    0x09, 0x04, 0x01, 0x00, 0x02, 0xFF, 0x5D, 0x03, 0x00,
    0x1B, 0x21, 0x00, 0x01, 0x01, 0x01, 0x83, 0x40, 0x01, 0x04, 0x20, 0x16, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0x05, 0x83, 0x03, 0x20, 0x00, 0x02,
    0x07, 0x05, 0x04, 0x03, 0x20, 0x00, 0x04,
    0x09, 0x04, 0x02, 0x00, 0x01, 0xFF, 0x5D, 0x02, 0x00,
    0x09, 0x21, 0x00, 0x01, 0x01, 0x22, 0x86, 0x07, 0x00,
    0x07, 0x05, 0x86, 0x03, 0x20, 0x00, 0x10,
    0x09, 0x04, 0x03, 0x00, 0x00, 0xFF, 0xFD, 0x13, 0x04,
    0x06, 0x41, 0x00, 0x01, 0x01, 0x03};


/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

__ALIGN_BEGIN static uint8_t ep2RecvBuff[32] __ALIGN_END;

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* Open EP IN */
  USBD_LL_OpenEP(pdev, 0x81, USBD_EP_TYPE_INTR, 0x20);
  pdev->ep_in[0x81 & 0xFU].is_used = 1U;
  pdev->ep_in[0x81 & 0xFU].maxpacket = 0x20;

  USBD_LL_OpenEP(pdev, 0x02, USBD_EP_TYPE_INTR, 0x20);
  pdev->ep_out[0x02 & 0xFU].is_used = 1U;
  pdev->ep_out[0x02 & 0xFU].maxpacket = 0x20;

  USBD_LL_OpenEP(pdev, 0x83, USBD_EP_TYPE_INTR, 0x20);
  pdev->ep_in[0x83 & 0xFU].is_used = 1U;
  pdev->ep_in[0x83 & 0xFU].maxpacket = 0x20;

  USBD_LL_OpenEP(pdev, 0x04, USBD_EP_TYPE_INTR, 0x20);
  pdev->ep_out[0x04 & 0xFU].is_used = 1U;
  pdev->ep_out[0x04 & 0xFU].maxpacket = 0x20;

  USBD_LL_OpenEP(pdev, 0x86, USBD_EP_TYPE_INTR, 0x20);
  pdev->ep_in[0x86 & 0xFU].is_used = 1U;
  pdev->ep_in[0x86 & 0xFU].maxpacket = 0x20;

  pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

  if (pdev->pClassData == NULL)
  {
    return USBD_FAIL;
  }

  USBD_LL_PrepareReceive(pdev, 0x02, ep2RecvBuff, sizeof(ep2RecvBuff));
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit(USBD_HandleTypeDef *pdev,
                                uint8_t cfgidx)
{
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev, 0x81);
  pdev->ep_in[0x81 & 0xFU].is_used = 0;

  USBD_LL_CloseEP(pdev, 0x02);
  pdev->ep_out[0x02 & 0xFU].is_used = 0;

  USBD_LL_CloseEP(pdev, 0x83);
  pdev->ep_in[0x83 & 0xFU].is_used = 0;

  USBD_LL_CloseEP(pdev, 0x04);
  pdev->ep_out[0x04 & 0xFU].is_used = 0;

  USBD_LL_CloseEP(pdev, 0x86);
  pdev->ep_in[0x86 & 0xFU].is_used = 0;

  /* FRee allocated memory */
  if (pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *) pdev->pClassData;
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_VENDOR:
      if (req->bmRequest == 0xc1 && req->bRequest == 0x01)
      {
        if (req->wValue >> 8 == 1)
        {
          static uint8_t rsp_c1_01_01[] = {0x00, 0x14, 0xFF, 0xF7, 0xFF, 0xFF, 0xC0, 0xFF, 0xC0, 0xFF, 0xC0, 0xFF, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
          len = req->wLength;
          if (len > sizeof(rsp_c1_01_01))
              len = sizeof(rsp_c1_01_01);
          USBD_CtlSendData(pdev, rsp_c1_01_01, len);
        }
        else if (req->wValue >> 8 == 0)
        {
          static uint8_t rsp_c1_01_00[] = {0x00, 0x08, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00};
          len = req->wLength;
          if (len > sizeof(rsp_c1_01_00))
              len = sizeof(rsp_c1_01_00);
          USBD_CtlSendData(pdev, rsp_c1_01_00, len);
        }
        else
        {
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
        }
      }
      else // if(req->bmRequest == 0xc0)
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case HID_REQ_GET_PROTOCOL:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
          break;

        case HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case HID_REQ_GET_IDLE:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
            USBD_CtlSendStatus(pdev);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return ret;
}

void PadHidReportSerialize(uint8_t *buff, const PadReport_t *report)
{
  buff[2] = report->button[0];
  buff[3] = report->button[1];
  buff[4] = report->leftTrigger;
  buff[5] = report->rightTrigger;
  CL_Int16ToBytes(report->leftX, buff + 6, CL_LittleEndian);
  CL_Int16ToBytes(report->leftY, buff + 8, CL_LittleEndian);
  CL_Int16ToBytes(report->rightX, buff + 10, CL_LittleEndian);
  CL_Int16ToBytes(report->rightY, buff + 12, CL_LittleEndian);
}

CL_Result_t USBD_SendPadReport(USBD_HandleTypeDef *pdev, const PadReport_t* report)
{
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == HID_IDLE)
    {
      static uint8_t inputReportData[20] = {0x00, 0x14, 0x00, 0x10, 0x00, 
                                  0x00, 0x00, 0x00, 0x00, 0x00, 
                                  0x00, 0x00, 0x00, 0x00, 0x00, 
                                  0x00, 0x00, 0x00, 0x00, 0x00};
      PadHidReportSerialize(inputReportData, report);

      hhid->state = HID_BUSY;
      USBD_LL_Transmit(pdev, 0x81, inputReportData, sizeof(inputReportData));
      return CL_ResSuccess;
    }
    else
    {
      return CL_ResBusy;
    }
  }
  return CL_ResFailed;
}


/**
  * @brief  USBD_HID_GetPollingInterval
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0U;

  /* HIGH-speed endpoints */
  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Sets the data transfer polling interval for high speed transfers.
     Values between 1..16 are allowed. Values correspond to interval
     of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1U << (HID_HS_BINTERVAL - 1U))) / 8U);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }

  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgFSDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgFSDesc);
  return USBD_HID_CfgFSDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn(USBD_HandleTypeDef *pdev,
                                uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}

static uint8_t USBD_HID_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    //if([0] == 0x00 && [1] == 0x08)
    //  [3] = left motor, [4] = right motor
    if(epnum == 2)
    {
      uint32_t len = USBD_LL_GetRxDataSize(pdev, 0x02);
      if(ep2RecvBuff[0] == 0 && ep2RecvBuff[1] == 8)
      {
        SetPadVibration(PadVbrtIdx_LeftBottom, ep2RecvBuff[3]);
        SetPadVibration(PadVbrtIdx_RightBottom, ep2RecvBuff[4]);
      }
      // CL_LOG_INFO(USB,"len: %u---", len);
      // for(uint32_t i = 0; i < len; i++)
      // {
      //   CL_LOG_INFO(USB,"%d ", ep2RecvBuff[i]);
      // }
      // CL_LOG_INFO(USB,"\r\n");
      //0 8 0 255 117 0 0 0    ..... 255=left; 117=right
      USBD_LL_PrepareReceive(pdev, 0x02, ep2RecvBuff, sizeof(ep2RecvBuff));
    }
    return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}

/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
