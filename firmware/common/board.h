#pragma once

#include "main.h"

#define GPIO_APB (LL_APB2_GRP1_PERIPH_GPIOC)

#define DFU_BTN_PORT (GPIOC)
#define DFU_BTN_PIN (LL_GPIO_PIN_15)

#define STA_LED_PROT (GPIOC)
#define STA_LED_PIN (LL_GPIO_PIN_13)


//****************HC165********************
#define HC165_CLK_PORT  (GPIOB)
#define HC165_CLK_PIN  (LL_GPIO_PIN_12)

#define HC165_LOAD_PORT  (GPIOB)
#define HC165_LOAD_PIN  (LL_GPIO_PIN_13)

#define HC165_DAT_PORT (GPIOB)
#define HC165_DAT_PIN (LL_GPIO_PIN_14)

