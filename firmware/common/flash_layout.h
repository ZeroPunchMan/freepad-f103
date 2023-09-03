#pragma once

#define BOOT_MAX_SIZE (30 * 1024ul)
#define APP_MAX_SIZE (48 * 1024ul)

#define BOOT_START_ADDR (0x08000000UL)
#define APP_START_ADDR (BOOT_START_ADDR + BOOT_MAX_SIZE)

#define DFU_APP_INFO_ADDR (APP_START_ADDR + APP_MAX_SIZE)