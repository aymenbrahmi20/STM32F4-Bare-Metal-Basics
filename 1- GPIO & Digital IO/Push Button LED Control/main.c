#include <stdint.h>

// Adresses de base des périphériques
#define PERIPH_BASE      0x40000000UL
#define AHB1PERIPH_BASE  (PERIPH_BASE + 0x00020000UL)
#define RCC_BASE         (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE       (AHB1PERIPH_BASE + 0x0000UL)   // le bouton est sur PA0
#define GPIOD_BASE       (AHB1PERIPH_BASE + 0x0C00UL)   // les LEDs sont sur PD12..15

// Registres (volatile : le compilateur ne doit pas les optimiser)
#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE   + 0x30UL))
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_PUPDR  (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR    (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))
#define GPIOD_MODER  (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_BSRR   (*(volatile uint32_t *)(GPIOD_BASE + 0x18UL))

#define BTN_PIN  0U
#define LED_PIN  12U

int main(void)
{
    // Sans horloge, un port GPIO ne répond pas : on allume A (bit 0) et D (bit 3)
    RCC_AHB1ENR |= (1UL << 0) | (1UL << 3);
    (void)RCC_AHB1ENR;  // petite lecture pour laisser l'horloge démarrer

    // PA0 en entrée, sans pull : la carte a déjà un pull-down sur le bouton
    GPIOA_MODER &= ~(3UL << (BTN_PIN * 2));
    GPIOA_PUPDR &= ~(3UL << (BTN_PIN * 2));

    // PD12 en sortie : on nettoie les 2 bits, puis on met 01
    GPIOD_MODER &= ~(3UL << (LED_PIN * 2));
    GPIOD_MODER |=  (1UL << (LED_PIN * 2));

    while (1)
    {
        if (GPIOA_IDR & (1UL << BTN_PIN))
            GPIOD_BSRR = (1UL << LED_PIN);          // bouton appuyé : on allume
        else
            GPIOD_BSRR = (1UL << (LED_PIN + 16U));  // sinon on éteint (les bits 16+ font le reset)
    }
}