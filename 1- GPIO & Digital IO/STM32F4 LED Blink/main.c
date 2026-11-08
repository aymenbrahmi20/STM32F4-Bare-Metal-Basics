#include <stdint.h>

/* --- Types d'accès aux registres --- */
#define __IO volatile

/* GPIO : layout STM32F4 (offsets 0x00 -> 0x24) */
typedef struct
{
    __IO uint32_t MODER;    /* 0x00 : mode register              */
    __IO uint32_t OTYPER;   /* 0x04 : output type register       */
    __IO uint32_t OSPEEDR;  /* 0x08 : output speed register      */
    __IO uint32_t PUPDR;    /* 0x0C : pull-up/pull-down register */
    __IO uint32_t IDR;      /* 0x10 : input data register        */
    __IO uint32_t ODR;      /* 0x14 : output data register       */
    __IO uint32_t BSRR;     /* 0x18 : bit set/reset register     */
    __IO uint32_t LCKR;     /* 0x1C : configuration lock         */
    __IO uint32_t AFR[2];   /* 0x20 : alternate function low/high*/
} GPIO_TypeDef;

/* RCC : layout STM32F4 (jusqu'à AHB1ENR à 0x30) */
typedef struct
{
    __IO uint32_t CR;           /* 0x00 */
    __IO uint32_t PLLCFGR;      /* 0x04 */
    __IO uint32_t CFGR;         /* 0x08 */
    __IO uint32_t CIR;          /* 0x0C */
    __IO uint32_t AHB1RSTR;     /* 0x10 */
    __IO uint32_t AHB2RSTR;     /* 0x14 */
    __IO uint32_t AHB3RSTR;     /* 0x18 */
    uint32_t      RESERVED0;    /* 0x1C */
    __IO uint32_t APB1RSTR;     /* 0x20 */
    __IO uint32_t APB2RSTR;     /* 0x24 */
    uint32_t      RESERVED1[2]; /* 0x28 - 0x2C */
    __IO uint32_t AHB1ENR;      /* 0x30 */
} RCC_TypeDef;

/* --- Adresses de base --- */
#define PERIPH_BASE     0x40000000UL
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE        (AHB1PERIPH_BASE + 0x3800UL)   /* 0x40023800 */
#define GPIOD_BASE      (AHB1PERIPH_BASE + 0x0C00UL)   /* 0x40020C00 */

/* --- Pointeurs vers les périphériques --- */
#define RCC             ((RCC_TypeDef  *)RCC_BASE)
#define GPIOD           ((GPIO_TypeDef *)GPIOD_BASE)

#define RCC_AHB1ENR_GPIODEN  (1UL << 3)
#define LED_PIN              12U

static void delay(volatile uint32_t n)
{
    while (n--) { __asm volatile ("nop"); }
}

int main(void)
{
    /* 1. Activer l'horloge du port D */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;             /* lecture de synchronisation */

    /* 2. PD12 en sortie (MODER = 01) */
    GPIOD->MODER &= ~(3UL << (LED_PIN * 2));
    GPIOD->MODER |=  (1UL << (LED_PIN * 2));

    /* 3. Push-pull (OTYPER = 0) */
    GPIOD->OTYPER &= ~(1UL << LED_PIN);

    /* 4. Vitesse basse (OSPEEDR = 00) */
    GPIOD->OSPEEDR &= ~(3UL << (LED_PIN * 2));

    /* 5. Sans pull-up / pull-down (PUPDR = 00) */
    GPIOD->PUPDR &= ~(3UL << (LED_PIN * 2));

    while (1)
    {
        GPIOD->BSRR = (1UL << LED_PIN);          /* LED ON  : set   (bits 0-15)  */
        delay(1000000);
        GPIOD->BSRR = (1UL << (LED_PIN + 16U));  /* LED OFF : reset (bits 16-31) */
        delay(1000000);
    }
}