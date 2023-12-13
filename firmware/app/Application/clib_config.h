#pragma once

#include "cl_common.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    //-------------event type------------------
    typedef enum
    {
        CL_Event_Button = 0, // session = ButtonIdx_t, eventArg = ButtonEvent_t*
        CL_EventMax,
    } CL_Event_t;

//---------------log-------------------------
#include "stdio.h"
#define CL_PRINTF printf

#define CL_LOG_LEVEL_INFO
#define CL_LOG_LEVEL_WARN
#define CL_LOG_LEVEL_ERROR

#define CL_LOG_ASSERT_ENABLED (1)
#define CL_LOG_CALI_ENABLED (1)
#define CL_LOG_PAD_ENABLED (0)
#define CL_LOG_APP_ENABLED (0)
#define CL_LOG_USB_ENABLED (0)
#define CL_LOG_BUTTON_ENABLED (0)
#define CL_LOG_SGP_ENABLED (0)

#ifdef __cplusplus
}
#endif
