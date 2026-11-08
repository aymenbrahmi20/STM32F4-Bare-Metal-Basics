```c
#include <stdint.h>

/* 1. PERIPHERAL BASE ADDRESSES */

#define PERIPH_BASE       0x40000000UL
#define APB2PERIPH_BASE   (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define SYSCFG_BASE       (APB2PERIPH_BASE + 0x3800UL)
#define EXTI_BASE         (APB2PERIPH_BASE + 0x3C00UL)
#define NVIC_ISER_BASE    0xE000E100UL

#define __IO volatile


/* 2. RCC REGISTER MAP */

typedef struct
{
    __IO uint32_t CR;             // 0x00 - Clock control register
    __IO uint32_t PLLCFGR;        // 0x04 - PLL configuration register
    __IO uint32_t CFGR;           // 0x08 - Clock configuration register
    __IO uint32_t CIR;            // 0x0C - Clock interrupt register
    __IO uint32_t AHB1RSTR;       // 0x10 - AHB1 peripheral reset
    __IO uint32_t AHB2RSTR;       // 0x14 - AHB2 peripheral reset
    __IO uint32_t AHB3RSTR;       // 0x18 - AHB3 peripheral reset
    uint32_t RESERVED0;           // 0x1C - Reserved
    __IO uint32_t APB1RSTR;       // 0x20 - APB1 peripheral reset
    __IO uint32_t APB2RSTR;       // 0x24 - APB2 peripheral reset
    uint32_t RESERVED1[2];        // 0x28-0x2C - Reserved
    __IO uint32_t AHB1ENR;        // 0x30 - AHB1 peripheral clock enable
    __IO uint32_t AHB2ENR;        // 0x34 - AHB2 peripheral clock enable
    __IO uint32_t AHB3ENR;        // 0x38 - AHB3 peripheral clock enable
    uint32_t RESERVED2;           // 0x3C - Reserved
    __IO uint32_t APB1ENR;        // 0x40 - APB1 peripheral clock enable
    __IO uint32_t APB2ENR;        // 0x44 - APB2 peripheral clock enable
} RCC_TypeDef;


/* 3. GPIO REGISTER MAP */

typedef struct
{
    __IO uint32_t MODER;          // 0x00 - GPIO port mode register
    __IO uint32_t OTYPER;         // 0x04 - GPIO output type register
    __IO uint32_t OSPEEDR;        // 0x08 - GPIO output speed register
    __IO uint32_t PUPDR;          // 0x0C - GPIO pull-up/pull-down register
    __IO uint32_t IDR;            // 0x10 - GPIO input data register
    __IO uint32_t ODR;            // 0x14 - GPIO output data register
    __IO uint32_t BSRR;            // 0x18 - GPIO bit set/reset register
    __IO uint32_t LCKR;           // 0x1C - GPIO configuration lock register
    __IO uint32_t AFR[2];         // 0x20-0x24 - Alternate function registers
} GPIO_TypeDef;


/* 4. SYSCFG REGISTER MAP */

typedef struct
{
    __IO uint32_t MEMRMP;         // 0x00 - Memory remap register
    __IO uint32_t PMC;            // 0x04 - Peripheral mode configuration
    __IO uint32_t EXTICR[4];      // 0x08-0x14 - EXTI source selection registers
    uint32_t RESERVED0[2];        // 0x18-0x1C - Reserved
    __IO uint32_t CMPCR;          // 0x20 - Compensation cell control register
} SYSCFG_TypeDef;


/* 5. EXTI REGISTER MAP */

typedef struct
{
    __IO uint32_t IMR;            // 0x00 - Interrupt mask register
    __IO uint32_t EMR;            // 0x04 - Event mask register
    __IO uint32_t RTSR;           // 0x08 - Rising trigger selection register
    __IO uint32_t FTSR;           // 0x0C - Falling trigger selection register
    __IO uint32_t SWIER;          // 0x10 - Software interrupt event register
    __IO uint32_t PR;             // 0x14 - Pending register
} EXTI_TypeDef;


/* 6. PERIPHERAL ACCESS POINTERS */

#define RCC    ((RCC_TypeDef *)RCC_BASE)
#define GPIOA  ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOD  ((GPIO_TypeDef *)GPIOD_BASE)
#define SYSCFG ((SYSCFG_TypeDef *)SYSCFG_BASE)
#define EXTI   ((EXTI_TypeDef *)EXTI_BASE)

/* Direct access to the NVIC interrupt set-enable registers */
#define NVIC_ISER ((volatile uint32_t *)NVIC_ISER_BASE)


/* 7. PIN AND INTERRUPT DEFINITIONS */

#define BTN_PIN       0U
#define LED_PIN       12U
#define EXTI0_IRQn    6U

/* Shared flag used to pass button events from the ISR to main */
static volatile uint32_t button_pressed = 0U;


/* 8. SIMPLE SOFTWARE DELAY */

static void delay(volatile uint32_t count)
{
    while (count > 0U)
    {
        count--;
        __asm volatile ("nop");
    }
}


/* 9. EXTI0 INTERRUPT HANDLER
 * The startup file must provide the EXTI0 interrupt vector.
 */

void EXTI0_IRQHandler(void)
{
    /* Check whether EXTI line 0 has a pending interrupt */
    if ((EXTI->PR & (1UL << BTN_PIN)) != 0U)
    {
        /* Clear the pending flag by writing 1 to the bit */
        EXTI->PR = (1UL << BTN_PIN);

        /* Notify the main loop that a button event occurred */
        button_pressed = 1U;
    }
}


/* 10. GPIO AND EXTI INITIALIZATION */

static void GPIO_EXTI_Init(void)
{
    /* Enable the GPIOA, GPIOD and SYSCFG peripheral clocks */
    RCC->AHB1ENR |= (1UL << 0U) | (1UL << 3U);
    RCC->APB2ENR |= (1UL << 14U);

    /* Read back the clock registers to allow the writes to take effect */
    (void)RCC->AHB1ENR;
    (void)RCC->APB2ENR;


    /* Configure PA0 as a digital input */
    GPIOA->MODER &= ~(3UL << (BTN_PIN * 2U));

    /* Disable the internal pull-up and pull-down resistors.
     * This assumes that the button circuit provides the required
     * external pull-down resistor.
     */
    GPIOA->PUPDR &= ~(3UL << (BTN_PIN * 2U));


    /* Configure PD12 as a general-purpose output */
    GPIOD->MODER &= ~(3UL << (LED_PIN * 2U));
    GPIOD->MODER |=  (1UL << (LED_PIN * 2U));

    /* Select push-pull output mode */
    GPIOD->OTYPER &= ~(1UL << LED_PIN);

    /* Select low output speed */
    GPIOD->OSPEEDR &= ~(3UL << (LED_PIN * 2U));

    /* Disable the internal pull-up and pull-down resistors */
    GPIOD->PUPDR &= ~(3UL << (LED_PIN * 2U));

    /* Turn the LED off by resetting PD12 through BSRR */
    GPIOD->BSRR = (1UL << (LED_PIN + 16U));


    /* Route EXTI line 0 to GPIOA pin 0 */
    SYSCFG->EXTICR[0] &= ~(0xFUL << 0U);


    /* Enable rising-edge detection and disable falling-edge detection */
    EXTI->RTSR |=  (1UL << BTN_PIN);
    EXTI->FTSR &= ~(1UL << BTN_PIN);


    /* Clear any previously pending EXTI0 interrupt */
    EXTI->PR = (1UL << BTN_PIN);

    /* Unmask EXTI line 0 to allow interrupt requests */
    EXTI->IMR |= (1UL << BTN_PIN);


    /* Enable EXTI0 in the NVIC.
     * EXTI0 uses IRQ number 6, which belongs to ISER register 0.
     */
    NVIC_ISER[EXTI0_IRQn / 32U] =
        (1UL << (EXTI0_IRQn % 32U));
}


/* 11. MAIN APPLICATION */

int main(void)
{
    /* Initialize the GPIO pins and external interrupt */
    GPIO_EXTI_Init();

    while (1)
    {
        /* Process a button event reported by the interrupt handler */
        if (button_pressed != 0U)
        {
            /* Consume the event flag */
            button_pressed = 0U;

            /* Toggle the LED output state */
            GPIOD->ODR ^= (1UL << LED_PIN);

            /* Apply a basic delay to reduce repeated button events */
            delay(200000U);

            /* Clear any EXTI0 request that accumulated during the delay */
            EXTI->PR = (1UL << BTN_PIN);
        }
    }
}
