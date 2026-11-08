#include <stdint.h>

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define USART2_BASE       (APB1PERIPH_BASE + 0x4400UL)

// RCC
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIO
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_BSRR    (*(volatile uint32_t *)(GPIOD_BASE + 0x18UL))

// USART2
#define USART2_SR     (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR     (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR    (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1    (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))

#define USART_SR_ORE   (1UL << 3)    // overrun : un octet est arrivé avant qu'on lise le précédent
#define USART_SR_RXNE  (1UL << 5)    // 1 = un octet est arrivé, il attend dans DR
#define USART_SR_TXE   (1UL << 7)    // 1 = on peut écrire le prochain octet à émettre
#define USART_CR1_RE   (1UL << 2)    // active le récepteur
#define USART_CR1_TE   (1UL << 3)    // active l'émetteur
#define USART_CR1_UE   (1UL << 13)   // active l'USART

#define TX_PIN   2U    // PA2
#define RX_PIN   3U    // PA3
#define LED_PIN  12U   // PD12 (LED verte)

static void uart_putc(char c)
{
    while (!(USART2_SR & USART_SR_TXE)) { }   // on attend que l'émetteur soit libre
    USART2_DR = (uint32_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

// Bloque jusqu'à l'arrivée d'un caractère, puis le renvoie
static char uart_getc(void)
{
    uint32_t sr;

    do {
        sr = USART2_SR;
        if (sr & USART_SR_ORE)
        {
            (void)USART2_DR;   // en cas d'overrun, lire SR puis DR remet le flag à zéro
        }
    } while (!(sr & USART_SR_RXNE));

    return (char)(USART2_DR & 0xFFUL);   // lire DR vide aussi le flag RXNE tout seul
}

int main(void)
{
    // Horloges : GPIOA, GPIOD et USART2
    RCC_AHB1ENR |= (1UL << 0) | (1UL << 3);
    RCC_APB1ENR |= (1UL << 17);
    (void)RCC_APB1ENR;

    // PA2 et PA3 en fonction alternative (MODER = 10)
    GPIOA_MODER &= ~((3UL << (TX_PIN * 2)) | (3UL << (RX_PIN * 2)));
    GPIOA_MODER |=  ((2UL << (TX_PIN * 2)) | (2UL << (RX_PIN * 2)));

    // AF7 (= USART2) pour les deux broches, 4 bits chacune dans AFRL
    GPIOA_AFRL &= ~((0xFUL << (TX_PIN * 4)) | (0xFUL << (RX_PIN * 4)));
    GPIOA_AFRL |=  ((7UL   << (TX_PIN * 4)) | (7UL   << (RX_PIN * 4)));

    // PD12 en sortie pour la LED
    GPIOD_MODER &= ~(3UL << (LED_PIN * 2));
    GPIOD_MODER |=  (1UL << (LED_PIN * 2));

    // 115200 bauds à 16 MHz : 138 + 14/16 = 0x8AE
    USART2_BRR = (138UL << 4) | 14UL;

    // Émetteur + récepteur + USART, on active tout en dernier
    USART2_CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    uart_puts("Pret. Tape 1 ou 0 pour la LED.\r\n");

    while (1)
    {
        char c = uart_getc();

        uart_putc(c);   // écho : on voit à l'écran ce qu'on tape

        if (c == '1')
            GPIOD_BSRR = (1UL << LED_PIN);           // LED allumée
        else if (c == '0')
            GPIOD_BSRR = (1UL << (LED_PIN + 16U));   // LED éteinte
    }
}