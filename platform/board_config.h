#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* Use this macro to handle MISRA Rule 11.4/11.6 in a single place */
#define REG32(addr) (*(volatile uint32_t *)(addr))

/* --- RCC Registers --- */
#define RCC_BASE      0x40023800U
#define RCC_CR        REG32(RCC_BASE + 0x00U)
#define RCC_PLLCFGR   REG32(RCC_BASE + 0x04U)
#define RCC_CFGR      REG32(RCC_BASE + 0x08U)

/* --- Flash Registers --- */
#define FLASH_BASE    0x40023C00U
#define FLASH_ACR     REG32(FLASH_BASE + 0x00U)

/* --- Clock Constants --- */
#define CPU_FREQ      96000000U
#define HSI_FREQ      16000000U

#endif /* BOARD_CONFIG_H */

