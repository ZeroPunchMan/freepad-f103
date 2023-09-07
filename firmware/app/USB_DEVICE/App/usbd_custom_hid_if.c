/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_custom_hid_if.c
 * @version        : v2.0_Cube
 * @brief          : USB Device Custom HID interface file.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  /* USER CODE BEGIN 0 */
        0x05, 0x01,                   // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,                   // Usage (Game Pad)
        0xA1, 0x01,                   // Collection (Application)
        0x85, 0x01,                   //   Report ID (1)
        0x09, 0x01,                   //   Usage (Pointer)
        0xA1, 0x00,                   //   Collection (Physical)
        0x09, 0x30,                   //--function:leftX     Usage (X)
        0x09, 0x31,                   //--function:leftY     Usage (Y)
        0x15, 0x00,                   //     Logical Minimum (0)
        0x27, 0xFF, 0xFF, 0x00, 0x00, //     Logical Maximum (65535)
        0x95, 0x02,                   //     Report Count (2)
        0x75, 0x10,                   //     Report Size (16)
        0x81, 0x02,                   //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,                         //   End Collection
        0x09, 0x01,                   //   Usage (Pointer)
        0xA1, 0x00,                   //   Collection (Physical)
        0x09, 0x32,                   //--function:rightX     Usage (Z)
        0x09, 0x35,                   //--function:rightY     Usage (Rz)
        0x15, 0x00,                   //     Logical Minimum (0)
        0x27, 0xFF, 0xFF, 0x00, 0x00, //     Logical Maximum (65535)
        0x95, 0x02,                   //     Report Count (2)
        0x75, 0x10,                   //     Report Size (16)
        0x81, 0x02,                   //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,                         //   End Collection
        0x05, 0x02,                   //   Usage Page (Sim Ctrls)
        0x09, 0xC5,                   //--function:LT   Usage (Brake)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x26, 0xFF, 0x03,             //   Logical Maximum (1023)
        0x95, 0x01,                   //   Report Count (1)
        0x75, 0x0A,                   //   Report Size (10)
        0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x00,                   //   Logical Maximum (0)
        0x75, 0x06,                   //   Report Size (6)
        0x95, 0x01,                   //   Report Count (1)
        0x81, 0x03,                   //--reserved   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x02,                   //   Usage Page (Sim Ctrls)
        0x09, 0xC4,                   //--function:RT   Usage (Accelerator)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x26, 0xFF, 0x03,             //   Logical Maximum (1023)
        0x95, 0x01,                   //   Report Count (1)
        0x75, 0x0A,                   //   Report Size (10)
        0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x00,                   //   Logical Maximum (0)
        0x75, 0x06,                   //   Report Size (6)
        0x95, 0x01,                   //   Report Count (1)
        0x81, 0x03,                   //--reserved   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,                   //   Usage Page (Generic Desktop Ctrls)
        0x09, 0x39,                   //   Usage (Hat switch)
        0x15, 0x01,                   //   Logical Minimum (1)
        0x25, 0x08,                   //   Logical Maximum (8)
        0x35, 0x00,                   //   Physical Minimum (0)
        0x46, 0x3B, 0x01,             //   Physical Maximum (315)
        0x66, 0x14, 0x00,             //   Unit (System: English Rotation, Length: Centimeter)
        0x75, 0x04,                   //   Report Size (4)
        0x95, 0x01,                   //   Report Count (1)
        0x81, 0x42,                   //--fuction:D-Pad   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
        0x75, 0x04,                   //   Report Size (4)
        0x95, 0x01,                   //   Report Count (1)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x00,                   //   Logical Maximum (0)
        0x35, 0x00,                   //   Physical Minimum (0)
        0x45, 0x00,                   //   Physical Maximum (0)
        0x65, 0x00,                   //   Unit (None)
        0x81, 0x03,                   //--reserved 4-bits   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x09,                   //   Usage Page (Button)
        0x19, 0x01,                   //   Usage Minimum (0x01)
        0x29, 0x0F,                   //   Usage Maximum (0x0F)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x01,                   //   Logical Maximum (1)
        0x75, 0x01,                   //   Report Size (1)
        0x95, 0x0F,                   //   Report Count (15)
        0x81, 0x02,                   //--fucntion: 15 Buttons   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x00,                   //   Logical Maximum (0)
        0x75, 0x01,                   //   Report Size (1)
        0x95, 0x01,                   //   Report Count (1)
        0x81, 0x03,                   //--reserved 1 bit   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x0C,                   //   Usage Page (Consumer)
        0x0A, 0xB2, 0x00,             //   Usage (Record)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x01,                   //   Logical Maximum (1)
        0x95, 0x01,                   //   Report Count (1)
        0x75, 0x01,                   //   Report Size (1)
        0x81, 0x02,                   //--fucntion: unknown   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x15, 0x00,                   //   Logical Minimum (0)
        0x25, 0x00,                   //   Logical Maximum (0)
        0x75, 0x07,                   //   Report Size (7)
        0x95, 0x01,                   //   Report Count (1)
        0x81, 0x03,                   //--reserved 7 bits   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x0F,                   //   Usage Page (PID Page)
        0x09, 0x21,                   //   Usage (0x21)  --- Set Effect Report
        0x85, 0x03,                   //   Report ID (3)
        0xA1, 0x02,                   //   Collection (Logical)
        0x09, 0x97,                   //     Usage (0x97) --- DC Enable Actuators
        0x15, 0x00,                   //     Logical Minimum (0)
        0x25, 0x01,                   //     Logical Maximum (1)
        0x75, 0x04,                   //     Report Size (4)
        0x95, 0x01,                   //     Report Count (1)
        0x91, 0x02,                   //--function: 4bits(0~1)     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x15, 0x00,                   //     Logical Minimum (0)
        0x25, 0x00,                   //     Logical Maximum (0)
        0x75, 0x04,                   //     Report Size (4)
        0x95, 0x01,                   //     Report Count (1)
        0x91, 0x03,                   //--reseved 4 bits     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x09, 0x70,                   //     Usage (0x70) --- Normalized magnitude of the Effect
        0x15, 0x00,                   //     Logical Minimum (0)
        0x25, 0x64,                   //     Logical Maximum (100)
        0x75, 0x08,                   //     Report Size (8)
        0x95, 0x04,                   //     Report Count (4)
        0x91, 0x02,                   //--function: 4bytes(0~100)     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x09, 0x50,                   //     Usage (0x50) --- Duration
        0x66, 0x01, 0x10,             //     Unit (System: SI Linear, Time: Seconds)
        0x55, 0x0E,                   //     Unit Exponent (-2)
        0x15, 0x00,                   //     Logical Minimum (0)
        0x26, 0xFF, 0x00,             //     Logical Maximum (255)
        0x75, 0x08,                   //     Report Size (8)
        0x95, 0x01,                   //     Report Count (1)
        0x91, 0x02,                   //--function: 1 byte(0~255)     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x09, 0xA7,                   //     Usage (0xA7) --- Start Delay
        0x15, 0x00,                   //     Logical Minimum (0)
        0x26, 0xFF, 0x00,             //     Logical Maximum (255)
        0x75, 0x08,                   //     Report Size (8)
        0x95, 0x01,                   //     Report Count (1)
        0x91, 0x02,                   //--function: 1 byte(0~255)     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x65, 0x00,                   //     Unit (None)
        0x55, 0x00,                   //     Unit Exponent (0)
        0x09, 0x7C,                   //     Usage (0x7C) --- Loop Count
        0x15, 0x00,                   //     Logical Minimum (0)
        0x26, 0xFF, 0x00,             //     Logical Maximum (255)
        0x75, 0x08,                   //     Report Size (8)
        0x95, 0x01,                   //     Report Count (1)
        0x91, 0x02,                   //--function: 1 byte(0~255)     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,                         //   End Collection
  /* USER CODE END 0 */
  0xC0    /*     END_COLLECTION	             */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
  /* USER CODE BEGIN 6 */
  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
 * @brief  Send the report to the Host
 * @param  report: The report to be sent
 * @param  len: The report length
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
typedef struct
{
	uint16_t leftX, leftY, rightX, rightY, leftTrigger, rightTrigger;
	uint8_t dPad;
	uint8_t button[2];
	uint8_t reserved;
} XosHidReport_t;

void XosHidReportSerialize(uint8_t *buff, const XosHidReport_t *report)
{
	buff[0] = report->leftX & 0xff;
	buff[1] = report->leftX >> 8;
	buff[2] = report->leftY & 0xff;
	buff[3] = report->leftY >> 8;

	buff[4] = report->rightX & 0xff;
	buff[5] = report->rightX >> 8;
	buff[6] = report->rightY & 0xff;
	buff[7] = report->rightY >> 8;

	buff[8] = report->leftTrigger & 0xff;
	buff[9] = report->leftTrigger >> 8;
	buff[10] = report->rightTrigger & 0xff;
	buff[11] = report->rightTrigger >> 8;

	buff[12] = report->dPad;
	buff[13] = report->button[0];
	buff[14] = report->button[1];
	buff[15] = report->reserved;
}


static XosHidReport_t xosReport = 
{
	.leftX = UINT16_MAX, //max 65535
	.leftY = UINT16_MAX,

	.rightX = UINT16_MAX / 2,
	.rightY = UINT16_MAX / 2,

	.leftTrigger = 0x3ff, //max 0x3ff
	.rightTrigger = 0x3ff,

	.dPad = 0, //1~8
	.button[0] = 0,
	.button[1] = 0,
	.reserved = 0,
};

void SendHidTestReport(void)
{
  static uint8_t inputReportData[17] = {0};
  inputReportData[0] = 1;

  XosHidReportSerialize(inputReportData + 1, &xosReport);
  // if (xosReport.button[0])
  //   xosReport.button[0] = 0;
  // else
  //   xosReport.button[0] = 0x08;

  if (xosReport.leftTrigger)
    xosReport.leftTrigger = 0;
  else
    xosReport.leftTrigger = 0x3ff;

  if (xosReport.rightTrigger)
    xosReport.rightTrigger = 0;
  else
    xosReport.rightTrigger = 0x3ff;

  USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, inputReportData, sizeof(inputReportData));
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

