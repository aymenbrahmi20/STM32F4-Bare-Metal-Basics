#include <stdint.h>

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define TIM4_BASE         (APB1PERIPH_BASE + 0x0800UL)   // 0x40000800

// RCC : GPIOD sur AHB1, TIM4 sur APB1
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOD
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_AFRH    (*(volatile uint32_t *)(GPIOD_BASE + 0x24UL))  // fonctions alternatives des broches 8 à 15

// TIM4
#define TIM4_CR1      (*(volatile uint32_t *)(TIM4_BASE + 0x00UL))   // contrôle (démarrage, preload...)
#define TIM4_EGR      (*(volatile uint32_t *)(TIM4_BASE + 0x14UL))   // génération d'événements (update)
#define TIM4_CCMR1    (*(volatile uint32_t *)(TIM4_BASE + 0x18UL))   // mode des canaux 1 et 2
#define TIM4_CCER     (*(volatile uint32_t *)(TIM4_BASE + 0x20UL))   // activation des sorties
#define TIM4_PSC      (*(volatile uint32_t *)(TIM4_BASE + 0x28UL))   // prédiviseur
#define TIM4_ARR      (*(volatile uint32_t *)(TIM4_BASE + 0x2CUL))   // valeur de rechargement = période
#define TIM4_CCR1     (*(volatile uint32_t *)(TIM4_BASE + 0x34UL))   // seuil de comparaison = rapport cyclique

#define LED_PIN       12U      // PD12 = TIM4_CH1
#define PWM_PERIOD    1000U    // 1000 ticks à 1 MHz = 1 kHz

// Délai à la louche, juste pour cadencer l'effet
static void delay(volatile uint32_t n)
{
    while (n--) { __asm volatile ("nop"); }
}

// Règle la luminosité de 0 (éteint) à 999 (presque plein)
static void led_set_level(uint32_t level)
{
    // L'œil perçoit la lumière de façon non linéaire : sans correction, la LED
    // paraît déjà "presque à fond" à mi-course. Élever au carré donne une
    // montée beaucoup plus naturelle.
    TIM4_CCR1 = (level * level) / (PWM_PERIOD - 1U);
}

int main(void)
{
    // Horloges : GPIOD et TIM4
    RCC_AHB1ENR |= (1UL << 3);
    RCC_APB1ENR |= (1UL << 2);
    (void)RCC_APB1ENR;

    // PD12 en fonction alternative (MODER = 10) : la broche est pilotée par le timer
    GPIOD_MODER &= ~(3UL << (LED_PIN * 2));
    GPIOD_MODER |=  (2UL << (LED_PIN * 2));

    // AF2 = TIM4. Pour la broche 12, on est dans AFRH, au rang (12-8) = 4
    GPIOD_AFRH &= ~(0xFUL << ((LED_PIN - 8U) * 4));
    GPIOD_AFRH |=  (2UL   << ((LED_PIN - 8U) * 4));

    // Base de temps : 16 MHz / 16 = 1 MHz, on compte de 0 à 999
    // (le registre PSC divise par PSC+1, d'où le -1)
    TIM4_PSC = 16U - 1U;
    TIM4_ARR = PWM_PERIOD - 1U;
    TIM4_CCR1 = 0;   // on démarre éteint

    // Canal 1 en PWM mode 1 : sortie à 1 tant que CNT < CCR1, puis à 0
    // OC1M = 110 (bits 6:4), OC1PE = 1 (bit 3) pour que CCR1 ne change qu'en fin de période
    TIM4_CCMR1 &= ~(0xFFUL);
    TIM4_CCMR1 |=  (6UL << 4) | (1UL << 3);

    // On active la sortie du canal 1 (CC1E, bit 0)
    TIM4_CCER |= (1UL << 0);

    // ARPE : même principe de preload pour ARR. Puis on force un update
    // pour charger PSC et ARR tout de suite, sinon ils ne seraient pris
    // en compte qu'au premier débordement
    TIM4_CR1 |= (1UL << 7);
    TIM4_EGR |= (1UL << 0);

    // C'est parti : on démarre le compteur (CEN, bit 0)
    TIM4_CR1 |= (1UL << 0);

    while (1)
    {
        // Montée progressive
        for (uint32_t level = 0; level < PWM_PERIOD; level += 5U)
        {
            led_set_level(level);
            delay(20000);
        }

        // Puis descente
        for (uint32_t level = PWM_PERIOD; level > 0U; level -= 5U)
        {
            led_set_level(level - 1U);
            delay(20000);
        }
    }
}