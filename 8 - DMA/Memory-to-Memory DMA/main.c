#include <stdint.h>

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define DMA2_BASE         (AHB1PERIPH_BASE + 0x6400UL)   // 0x40026400
#define USART2_BASE       (APB1PERIPH_BASE + 0x4400UL)

// RCC
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA (TX de l'UART)
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

// DMA2 : registres communs à tous les streams
#define DMA2_LISR     (*(volatile uint32_t *)(DMA2_BASE + 0x00UL))   // flags des streams 0 à 3
#define DMA2_LIFCR    (*(volatile uint32_t *)(DMA2_BASE + 0x08UL))   // pour effacer ces flags

// DMA2 stream 0 : ses registres commencent à 0x10 (puis +0x18 par stream)
#define DMA2_S0CR     (*(volatile uint32_t *)(DMA2_BASE + 0x10UL))   // configuration
#define DMA2_S0NDTR   (*(volatile uint32_t *)(DMA2_BASE + 0x14UL))   // nombre de données à transférer
#define DMA2_S0PAR    (*(volatile uint32_t *)(DMA2_BASE + 0x18UL))   // adresse "périphérique" = SOURCE ici
#define DMA2_S0M0AR   (*(volatile uint32_t *)(DMA2_BASE + 0x1CUL))   // adresse "mémoire" = DESTINATION ici
#define DMA2_S0FCR    (*(volatile uint32_t *)(DMA2_BASE + 0x24UL))   // configuration du FIFO

#define DMA_CR_EN        (1UL << 0)     // active le stream
#define DMA_CR_PINC      (1UL << 9)     // l'adresse "périphérique" (source) avance à chaque transfert
#define DMA_CR_MINC      (1UL << 10)    // l'adresse "mémoire" (destination) avance aussi
#define DMA_CR_PSIZE_32  (2UL << 11)    // données de 32 bits côté source
#define DMA_CR_MSIZE_32  (2UL << 13)    // données de 32 bits côté destination
#define DMA_CR_DIR_M2M   (2UL << 6)     // DIR = 10 : mémoire vers mémoire
#define DMA_FCR_DMDIS    (1UL << 2)     // mode direct désactivé = FIFO utilisé (obligatoire en mem-to-mem)
#define DMA_FCR_FTH_FULL (3UL << 0)     // seuil du FIFO : plein

// Flags du stream 0 (identiques dans LISR et LIFCR)
#define DMA_FEIF0     (1UL << 0)    // erreur de FIFO
#define DMA_DMEIF0    (1UL << 2)    // erreur du mode direct
#define DMA_TEIF0     (1UL << 3)    // erreur de transfert
#define DMA_HTIF0     (1UL << 4)    // mi-parcours atteint
#define DMA_TCIF0     (1UL << 5)    // transfert terminé
#define DMA_ALL_FLAGS0 (DMA_FEIF0 | DMA_DMEIF0 | DMA_TEIF0 | DMA_HTIF0 | DMA_TCIF0)

// SysTick : compteur 24 bits qui décompte à la vitesse du CPU, pratique pour chronométrer
#define SYST_CSR      (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR      (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR      (*(volatile uint32_t *)0xE000E018UL)

// USART2 (émission seule)
#define USART2_SR     (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR     (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR    (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1    (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART_SR_TXE  (1UL << 7)
#define USART_CR1_TE  (1UL << 3)
#define USART_CR1_UE  (1UL << 13)

#define TX_PIN    2U
#define NB_WORDS  256U    // 256 mots de 32 bits = 1 Ko

// Les trois tableaux sont dans la SRAM normale. "volatile" empêche le compilateur
// de supprimer nos copies, puisqu'il ne voit pas que le DMA les modifie.
static uint32_t          src[NB_WORDS];
static volatile uint32_t dst_dma[NB_WORDS];
static volatile uint32_t dst_cpu[NB_WORDS];

/* ------------------------------------------------------------------
 *  UART
 * ---------------------------------------------------------------- */
static void uart_send_byte(uint8_t b)
{
    while (!(USART2_SR & USART_SR_TXE)) { }
    USART2_DR = b;
}

static void uart_send_string(const char *s)
{
    while (*s) uart_send_byte((uint8_t)*s++);
}

static void uart_send_uint(uint32_t n)
{
    char tmp[10];
    int  i = 0;

    if (n == 0) { uart_send_byte('0'); return; }
    while (n > 0) { tmp[i++] = (char)('0' + (n % 10)); n /= 10; }
    while (i--)   { uart_send_byte((uint8_t)tmp[i]); }
}

/* ------------------------------------------------------------------
 *  Chronomètre (SysTick en simple compteur, sans interruption)
 * ---------------------------------------------------------------- */
static void timer_init(void)
{
    SYST_RVR = 0x00FFFFFFUL;   // on compte depuis le maximum, soit environ 1 seconde à 16 MHz
    SYST_CVR = 0;
    SYST_CSR = 5UL;            // actif, horloge du CPU, pas d'interruption
}

static uint32_t timer_now(void)
{
    return SYST_CVR;
}

// SysTick décompte : le temps écoulé, c'est "avant - après"
static uint32_t timer_elapsed(uint32_t start, uint32_t end)
{
    return (start - end) & 0x00FFFFFFUL;
}

/* ------------------------------------------------------------------
 *  DMA
 * ---------------------------------------------------------------- */

// Lance la copie et attend la fin. Renvoie 0 si tout va bien, -1 en cas d'erreur.
static int dma_copy(const uint32_t *from, volatile uint32_t *to, uint32_t nb_words)
{
    // On ne reconfigure un stream que s'il est arrêté. Si EN est déjà à 0 c'est instantané,
    // sinon le DMA termine d'abord son transfert en cours.
    DMA2_S0CR &= ~DMA_CR_EN;
    while (DMA2_S0CR & DMA_CR_EN) { }

    // Les anciens flags empêcheraient de détecter correctement la fin : on les efface
    DMA2_LIFCR = DMA_ALL_FLAGS0;

    DMA2_S0PAR  = (uint32_t)from;    // source
    DMA2_S0M0AR = (uint32_t)to;      // destination
    DMA2_S0NDTR = nb_words;          // nombre d'éléments (pas d'octets !)

    // FIFO obligatoire pour la mémoire vers mémoire : on coupe le mode direct
    DMA2_S0FCR = DMA_FCR_DMDIS | DMA_FCR_FTH_FULL;

    // Mémoire -> mémoire, mots de 32 bits, les deux adresses avancent.
    // Pas de mode circulaire : il est interdit dans ce mode.
    DMA2_S0CR = DMA_CR_DIR_M2M
              | DMA_CR_PINC | DMA_CR_MINC
              | DMA_CR_PSIZE_32 | DMA_CR_MSIZE_32;

    DMA2_S0CR |= DMA_CR_EN;          // top départ, le CPU est libre dès maintenant

    // Ici le CPU pourrait faire autre chose. On préfère attendre pour la démo.
    while (!(DMA2_LISR & (DMA_TCIF0 | DMA_TEIF0))) { }

    if (DMA2_LISR & DMA_TEIF0)
    {
        DMA2_LIFCR = DMA_ALL_FLAGS0;
        return -1;
    }

    DMA2_LIFCR = DMA_ALL_FLAGS0;     // on nettoie pour la fois suivante
    return 0;
}

// Compare deux tableaux, renvoie le nombre de mots différents
static uint32_t count_diff(const uint32_t *a, const volatile uint32_t *b, uint32_t n)
{
    uint32_t diff = 0;
    for (uint32_t i = 0; i < n; i++)
    {
        if (a[i] != b[i]) diff++;
    }
    return diff;
}

int main(void)
{
    // Horloges : GPIOA, DMA2 (bit 22 de AHB1ENR), USART2
    RCC_AHB1ENR |= (1UL << 0) | (1UL << 22);
    RCC_APB1ENR |= (1UL << 17);
    (void)RCC_APB1ENR;

    // UART : PA2 en AF7, 115200 bauds
    GPIOA_MODER &= ~(3UL << (TX_PIN * 2));
    GPIOA_MODER |=  (2UL << (TX_PIN * 2));
    GPIOA_AFRL  &= ~(0xFUL << (TX_PIN * 4));
    GPIOA_AFRL  |=  (7UL   << (TX_PIN * 4));
    USART2_BRR = (138UL << 4) | 14UL;
    USART2_CR1 = USART_CR1_TE | USART_CR1_UE;

    timer_init();

    uart_send_string("=== Memory-to-Memory DMA ===\r\n");

    // On remplit la source avec des valeurs reconnaissables
    for (uint32_t i = 0; i < NB_WORDS; i++)
    {
        src[i] = 0xA5000000UL + i;
    }

    // --- Copie par le DMA ---
    uint32_t t0 = timer_now();
    int r = dma_copy(src, dst_dma, NB_WORDS);
    uint32_t t1 = timer_now();

    // --- Copie par le CPU, pour comparer ---
    uint32_t t2 = timer_now();
    for (uint32_t i = 0; i < NB_WORDS; i++)
    {
        dst_cpu[i] = src[i];
    }
    uint32_t t3 = timer_now();

    // --- Résultats ---
    if (r != 0)
    {
        uart_send_string("Erreur DMA (flag TEIF)\r\n");
    }
    else
    {
        uint32_t diff = count_diff(src, dst_dma, NB_WORDS);

        uart_send_string(diff == 0 ? "DMA : copie OK\r\n" : "DMA : copie FAUSSE, mots differents = ");
        if (diff != 0) { uart_send_uint(diff); uart_send_string("\r\n"); }
    }

    uart_send_string("Cycles DMA (config + copie) = ");
    uart_send_uint(timer_elapsed(t0, t1));
    uart_send_string("\r\nCycles CPU (boucle for)    = ");
    uart_send_uint(timer_elapsed(t2, t3));
    uart_send_string("\r\n");

    while (1) { }
}