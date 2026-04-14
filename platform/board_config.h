#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

/* FLASH */
typedef struct {
    volatile uint32_t ACR;
} FLASH_TypeDef;

/* RCC */
typedef struct {
    volatile uint32_t CR;           /* Offset: 0x00 - Clock Control Reg. */
    volatile uint32_t PLLCFGR;      /* Offset: 0x04 - PLL Config Reg. */
    volatile uint32_t CFGR;         /* Offset: 0x08 - Clock Config Reg. */
    volatile uint32_t CIR;          /* Offset: 0x0C - Clock Interrupt Reg. */
    volatile uint32_t AHB1RSTR;     /* Offset: 0x10 - AHB1 Peripherial Reset Reg. */
    volatile uint32_t AHB2RSTR;     /* Offset: 0x14 - AHB2 Peripherial Reset Reg. */
    const uint32_t RESERVED0[2];          /* Offsets 0x18, 0x1C */
    volatile uint32_t APB1RSTR;     /* Offset: 0x20 - APB1 Peripherial Reset Reg. */
    volatile uint32_t APB2RSTR;     /* Offset: 0x24 */
    const uint32_t RESERVED1[2];          /* Offset: 0x28, 0x2C */
    volatile uint32_t AHB1ENR;      /* Offset: 0x30 */
    volatile uint32_t AHB2ENR;
    const uint32_t RESERVED2[2];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    const uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    const uint32_t RESERVED4[2];
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    const uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    const uint32_t RESERVED6[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2S_CFGR;
    const uint32_t RESERVED7;
    volatile uint32_t DCKCFGR;
} RCC_TypeDef;

/* GPIO */
typedef struct {
    volatile uint32_t MODER;        /* Offset: 0x00 - */
    volatile uint32_t OTYPER;       /* Offset: 0x04 - */
    volatile uint32_t OSPEEDR;      /* Offset: 0x08 - */
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;         /* Offset: 0x18 - */
    volatile uint32_t LCKR;         /* Offset: 0x1C - */
    volatile uint32_t AFRL;         /* Offset: 0x20 - */
    volatile uint32_t AFRH;         /* Offset: 0x24 - */
} GPIOD_TypeDef;

/* --- RCC Registers --- */
#define RCC_BASE            0x40023800UL
#define RCC                 ((RCC_TypeDef *)(RCC_BASE))

/* HSI clock enable */
#define RCC_CR_HSION_Pos    (0U)
#define RCC_CR_HSION_Msk    (1UL << RCC_CR_HSION_Pos)    /* 0x0000 0001 */ 
#define RCC_CR_HSION        RCC_CR_HSION_Msk

/* HSI clock ready flag */
#define RCC_CR_HSIRDY_Pos   (1U)
#define RCC_CR_HSIRDY_Msk   (1UL << RCC_CR_HSIRDY_Pos)   /* 0x0000 0002 */
#define RCC_CR_HSIRDY       RCC_CR_HSIRDY_Msk

/* PLL clock enable */
#define RCC_CR_PLLON_Pos    (24U)
#define RCC_CR_PLLON_Msk    (1UL << RCC_CR_PLLON_Pos)
#define RCC_CR_PLLON        RCC_CR_PLLON_Msk

/* PLL clock ready flag */
#define RCC_CR_PLLRDY_Pos   (25U)
#define RCC_CR_PLLRDY_Msk   (1UL << RCC_CR_PLLRDY_Pos)
#define RCC_CR_PLLRDY      RCC_CR_PLLRDY_Msk

/* IO Port D enable */
#define RCC_AHB1ENR_GPIODEN_Pos (3U)
#define RCC_AHB1ENR_GPIODEN_Msk (1UL << RCC_AHB1ENR_GPIODEN_Pos)
#define RCC_AHB1ENR_GPIODEN      RCC_AHB1ENR_GPIODEN_Msk

/* RCC CFGR Definitions */
#define RCC_CFGR_SW_Pos         (0U)
#define RCC_CFGR_SW_PLL         (2UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SWS_Pos        (2U)
#define RCC_CFGR_SWS_PLL        (2UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_HPRE_DIV1      (0UL << 4U)
#define RCC_CFGR_PPRE1_DIV2     (4UL << 10U)        /**Note: not fully implemented in clock */
#define RCC_CFGR_PPRE2_DIV1     (0UL << 13U)


/* --- Flash Registers --- */
#define FLASH_BASE          0x40023C00UL
#define FLASH               ((FLASH_TypeDef *)(FLASH_BASE))

/* FLASH ACR Definitions */
#define FLASH_ACR_LATENCY_Pos   (0U)
#define FLASH_ACR_LATENCY_Msk   (0xFU << FLASH_ACR_LATENCY_Pos)
#define FLASH_ACR_PRFTEN        (1UL << 8U)
#define FLASH_ACR_ICEN          (1UL << 9U)
#define FLASH_ACR_DCEN          (1UL << 10U)


/* --- GPIO Registers --- */
#define AHB1_BASE           0x40020000UL
#define GPIOD_BASE          (AHB1_BASE + 0x0C00UL)
#define GPIOD               ((GPIOD_TypeDef *)(GPIOD_BASE))


/* --- Clock Constants --- */
#define CPU_FREQ    96000000UL
#define HSI_FREQ    16000000UL

#endif /* BOARD_CONFIG_H */

