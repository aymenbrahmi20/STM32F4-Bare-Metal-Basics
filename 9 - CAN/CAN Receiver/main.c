#include <stdint.h>

// 1 = loopback silencieux avec émission de trames de test, 0 = vrai bus CAN
#define CAN_LOOPBACK  1

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define USART2_BASE       (APB1PERIPH_BASE + 0x4400UL)
#define CAN1_BASE         (APB1PERIPH_BASE + 0x6400UL)

// RCC
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA (TX de l'UART) et GPIOD (broches CAN)
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OSPEEDR (*(volatile uint32_t *)(GPIOD_BASE + 0x08UL))
#define GPIOD_PUPDR   (*(volatile uint32_t *)(GPIOD_BASE + 0x0CUL))
#define GPIOD_AFRL    (*(volatile uint32_t *)(GPIOD_BASE + 0x20UL))

// CAN1 : contrôle
#define CAN1_MCR      (*(volatile uint32_t *)(CAN1_BASE + 0x000UL))
#define CAN1_MSR      (*(volatile uint32_t *)(CAN1_BASE + 0x004UL))
#define CAN1_TSR      (*(volatile uint32_t *)(CAN1_BASE + 0x008UL))
#define CAN1_RF0R     (*(volatile uint32_t *)(CAN1_BASE + 0x00CUL))   // état de la FIFO 0
#define CAN1_IER      (*(volatile uint32_t *)(CAN1_BASE + 0x014UL))   // activation des interruptions
#define CAN1_ESR      (*(volatile uint32_t *)(CAN1_BASE + 0x018UL))
#define CAN1_BTR      (*(volatile uint32_t *)(CAN1_BASE + 0x01CUL))

// CAN1 : boîte d'émission n (utilisée seulement par les trames de test du loopback)
#define CAN_TXMB(n, off)  (*(volatile uint32_t *)(CAN1_BASE + 0x180UL + (n) * 0x10UL + (off)))
#define TX_TIR   0x00UL
#define TX_TDTR  0x04UL
#define TX_TDLR  0x08UL
#define TX_TDHR  0x0CUL

// CAN1 : réception, FIFO 0
#define CAN1_RI0R     (*(volatile uint32_t *)(CAN1_BASE + 0x1B0UL))   // identifiant + IDE + RTR
#define CAN1_RDT0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1B4UL))   // DLC + numéro de filtre (FMI)
#define CAN1_RDL0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1B8UL))
#define CAN1_RDH0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1BCUL))

// CAN1 : filtres
#define CAN1_FMR      (*(volatile uint32_t *)(CAN1_BASE + 0x200UL))
#define CAN1_FM1R     (*(volatile uint32_t *)(CAN1_BASE + 0x204UL))   // 0 = masque, 1 = liste
#define CAN1_FS1R     (*(volatile uint32_t *)(CAN1_BASE + 0x20CUL))   // 0 = 2x16 bits, 1 = 32 bits
#define CAN1_FFA1R    (*(volatile uint32_t *)(CAN1_BASE + 0x214UL))   // 0 = FIFO 0, 1 = FIFO 1
#define CAN1_FA1R     (*(volatile uint32_t *)(CAN1_BASE + 0x21CUL))   // activation des banques
#define CAN1_F0R1     (*(volatile uint32_t *)(CAN1_BASE + 0x240UL))   // banque 0, registre 1
#define CAN1_F0R2     (*(volatile uint32_t *)(CAN1_BASE + 0x244UL))
#define CAN1_F1R1     (*(volatile uint32_t *)(CAN1_BASE + 0x248UL))   // banque 1, registre 1
#define CAN1_F1R2     (*(volatile uint32_t *)(CAN1_BASE + 0x24CUL))

// Bits utiles
#define CAN_MCR_INRQ    (1UL << 0)
#define CAN_MCR_SLEEP   (1UL << 1)
#define CAN_MCR_TXFP    (1UL << 2)
#define CAN_MCR_ABOM    (1UL << 6)
#define CAN_MSR_INAK    (1UL << 0)
#define CAN_MSR_SLAK    (1UL << 1)

#define CAN_IER_FMPIE0  (1UL << 1)    // interruption "un message est arrivé dans la FIFO 0"
#define CAN_IER_FOVIE0  (1UL << 3)    // interruption "la FIFO 0 a débordé"

#define CAN_RF0R_FOVR0  (1UL << 4)    // débordement de la FIFO 0 (écrire 1 pour effacer)
#define CAN_RF0R_RFOM0  (1UL << 5)    // libère le message en tête de FIFO

#define CAN_RIR_IDE     (1UL << 2)    // 1 = identifiant étendu 29 bits
#define CAN_RIR_RTR     (1UL << 1)    // 1 = trame de requête (remote), sans données

#define CAN_TSR_RQCP    (1UL << 0)
#define CAN_TSR_TME_ANY (7UL << 26)
#define CAN_TIR_TXRQ    (1UL << 0)

// Timing : 500 kbit/s avec une horloge APB1 de 16 MHz (16 time quanta par bit)
#define CAN_PRESCALER  2UL
#define CAN_TS1        13UL
#define CAN_TS2        2UL
#define CAN_SJW        1UL
#define CAN_BTR_LBKM   (1UL << 30)
#define CAN_BTR_SILM   (1UL << 31)

// NVIC : CAN1_RX0 est l'IRQ numéro 20 (vecteur CAN1_RX0_IRQHandler)
#define NVIC_ISER0    (*(volatile uint32_t *)0xE000E100UL)
#define CAN1_RX0_IRQn 20U

// USART2 (émission seule)
#define USART2_SR     (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR     (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR    (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1    (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART_SR_TXE  (1UL << 7)
#define USART_CR1_TE  (1UL << 3)
#define USART_CR1_UE  (1UL << 13)

#define TX_PIN        2U    // PA2

#define CAN_OK         0
#define CAN_ERR_INIT  -3
#define CAN_ERR_BUSY  -1
#define CAN_TIMEOUT   200000UL

static void delay(volatile uint32_t n)
{
    while (n--) { __asm volatile ("nop"); }
}

/* ------------------------------------------------------------------
 *  UART (pour l'affichage)
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

static void uart_send_hex(uint32_t v, uint32_t digits)
{
    const char hex[] = "0123456789ABCDEF";
    while (digits--) uart_send_byte((uint8_t)hex[(v >> (digits * 4U)) & 0xFU]);
}

/* ------------------------------------------------------------------
 *  Buffer circulaire entre l'interruption (producteur) et le main (consommateur)
 * ---------------------------------------------------------------- */
typedef struct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  dlc;
    uint8_t  extended;   // 1 = identifiant 29 bits
    uint8_t  rtr;        // 1 = trame de requête
    uint8_t  fmi;        // numéro du filtre qui a accepté la trame
} can_msg_t;

#define QUEUE_SIZE  8U                    // puissance de 2, pour pouvoir masquer au lieu de diviser
#define QUEUE_MASK  (QUEUE_SIZE - 1U)

static volatile can_msg_t rx_queue[QUEUE_SIZE];
static volatile uint8_t   q_head = 0;     // écrit uniquement par l'ISR
static volatile uint8_t   q_tail = 0;     // écrit uniquement par le main
static volatile uint32_t  dropped = 0;    // trames perdues car notre buffer était plein
static volatile uint32_t  hw_overruns = 0;// débordements de la FIFO matérielle

// Vecteur d'interruption : le nom doit correspondre à celui du startup file
void CAN1_RX0_IRQHandler(void)
{
    // On vide toute la FIFO d'un coup : FMP0 (bits 1:0) = nombre de messages en attente
    while (CAN1_RF0R & 3UL)
    {
        uint8_t next = (uint8_t)((q_head + 1U) & QUEUE_MASK);

        if (next == q_tail)
        {
            // Notre buffer est plein : on perd cette trame mais on la libère quand même,
            // sinon on bouclerait ici indéfiniment
            dropped++;
            CAN1_RF0R = CAN_RF0R_RFOM0;
            continue;
        }

        volatile can_msg_t *m = &rx_queue[q_head];

        uint32_t ri  = CAN1_RI0R;
        uint32_t rdt = CAN1_RDT0R;
        uint32_t lo  = CAN1_RDL0R;
        uint32_t hi  = CAN1_RDH0R;

        m->extended = (ri & CAN_RIR_IDE) ? 1U : 0U;
        m->rtr      = (ri & CAN_RIR_RTR) ? 1U : 0U;
        m->id       = m->extended ? (ri >> 3) : (ri >> 21);   // 29 bits ou 11 bits
        m->dlc      = (uint8_t)(rdt & 0xFUL);
        m->fmi      = (uint8_t)((rdt >> 8) & 0xFFUL);

        for (uint32_t i = 0; i < 8U; i++)
        {
            m->data[i] = (i < 4U) ? (uint8_t)(lo >> (8U * i))
                                  : (uint8_t)(hi >> (8U * (i - 4U)));
        }

        CAN1_RF0R = CAN_RF0R_RFOM0;   // message lu : on libère sa place dans la FIFO
        q_head = next;                // et seulement maintenant on le publie au main
    }

    // La FIFO matérielle a débordé : c'est qu'on a été trop lent à la vider
    if (CAN1_RF0R & CAN_RF0R_FOVR0)
    {
        CAN1_RF0R = CAN_RF0R_FOVR0;   // écrire 1 efface le flag
        hw_overruns++;
    }
}

// Côté main : récupère un message s'il y en a un
static int queue_pop(can_msg_t *out)
{
    if (q_tail == q_head) return 0;   // vide

    const volatile can_msg_t *m = &rx_queue[q_tail];

    out->id       = m->id;
    out->dlc      = m->dlc;
    out->extended = m->extended;
    out->rtr      = m->rtr;
    out->fmi      = m->fmi;
    for (uint32_t i = 0; i < 8U; i++) out->data[i] = m->data[i];

    q_tail = (uint8_t)((q_tail + 1U) & QUEUE_MASK);   // on libère la case après la copie
    return 1;
}

/* ------------------------------------------------------------------
 *  Driver CAN
 * ---------------------------------------------------------------- */
static int can_wait_msr(uint32_t mask, uint32_t expected)
{
    uint32_t t = CAN_TIMEOUT;

    while ((CAN1_MSR & mask) != expected)
    {
        if (--t == 0) return CAN_ERR_INIT;
    }
    return CAN_OK;
}

// Les deux filtres. Pour un identifiant standard dans un filtre 32 bits :
// bits 31:21 = ID, bit 2 = IDE, bit 1 = RTR.
#define FILTER_ID(id)  ((uint32_t)(id) << 21)

static void can_filters_init(void)
{
    CAN1_FMR |= 1UL;                 // FINIT : mode réglage des filtres
    CAN1_FA1R = 0;                   // toutes les banques coupées pendant le réglage

    CAN1_FS1R  |= 0x3UL;             // banques 0 et 1 en 32 bits
    CAN1_FM1R   = (CAN1_FM1R & ~0x3UL) | 0x1UL;   // banque 0 en mode liste, banque 1 en mode masque
    CAN1_FFA1R &= ~0x3UL;            // les deux envoient vers la FIFO 0

    // Banque 0 : mode liste, deux ID exacts. Les bits IDE et RTR valent 0 :
    // on n'accepte que des trames de données à identifiant standard.
    CAN1_F0R1 = FILTER_ID(0x321);
    CAN1_F0R2 = FILTER_ID(0x322);

    // Banque 1 : mode masque. Un bit à 1 dans le masque = ce bit de l'ID doit correspondre.
    // 0x7F0 vérifie les 7 bits de poids fort : il laisse libres les 4 bits de poids faible,
    // donc on accepte 0x100 à 0x10F. Le bit IDE du masque (bit 2) force "identifiant standard".
    CAN1_F1R1 = FILTER_ID(0x100);
    CAN1_F1R2 = FILTER_ID(0x7F0) | (1UL << 2);

    CAN1_FA1R = 0x3UL;               // on active les banques 0 et 1
    CAN1_FMR &= ~1UL;                // et on quitte le mode réglage
}

static int can1_init(void)
{
    // Horloges : GPIOD et CAN1
    RCC_AHB1ENR |= (1UL << 3);
    RCC_APB1ENR |= (1UL << 25);
    (void)RCC_APB1ENR;

    // PD0 (RX) et PD1 (TX), AF9 = CAN1
    for (uint32_t pin = 0U; pin <= 1U; pin++)
    {
        GPIOD_MODER   &= ~(3UL   << (pin * 2));
        GPIOD_MODER   |=  (2UL   << (pin * 2));
        GPIOD_OSPEEDR |=  (2UL   << (pin * 2));
        GPIOD_AFRL    &= ~(0xFUL << (pin * 4));
        GPIOD_AFRL    |=  (9UL   << (pin * 4));
    }
    // Pull-up sur RX : sans transceiver branché, la ligne reste à l'état récessif
    GPIOD_PUPDR &= ~(3UL << 0);
    GPIOD_PUPDR |=  (1UL << 0);

    CAN1_MCR &= ~CAN_MCR_SLEEP;                          // on sort de la veille
    if (can_wait_msr(CAN_MSR_SLAK, 0)) return CAN_ERR_INIT;

    CAN1_MCR |= CAN_MCR_INRQ;                            // mode initialisation
    if (can_wait_msr(CAN_MSR_INAK, CAN_MSR_INAK)) return CAN_ERR_INIT;

    CAN1_BTR = ((CAN_SJW - 1UL) << 24)
             | ((CAN_TS2 - 1UL) << 20)
             | ((CAN_TS1 - 1UL) << 16)
             |  (CAN_PRESCALER - 1UL);
#if CAN_LOOPBACK
    CAN1_BTR |= CAN_BTR_LBKM | CAN_BTR_SILM;             // le contrôleur s'écoute lui-même
#endif

    CAN1_MCR |= CAN_MCR_ABOM | CAN_MCR_TXFP;
    can_filters_init();

    CAN1_IER = CAN_IER_FMPIE0 | CAN_IER_FOVIE0;          // interruptions de réception
    NVIC_ISER0 = (1UL << CAN1_RX0_IRQn);                 // et on les autorise dans le NVIC

    CAN1_MCR &= ~CAN_MCR_INRQ;                           // on sort du mode init
    return can_wait_msr(CAN_MSR_INAK, 0);
}

#if CAN_LOOPBACK
// Émission d'une trame de test (même principe que l'exemple CAN Transmitter)
static int can_send(uint16_t std_id, const uint8_t *data, uint8_t dlc)
{
    uint32_t tsr = CAN1_TSR;

    if (!(tsr & CAN_TSR_TME_ANY)) return CAN_ERR_BUSY;

    uint32_t mb = (tsr >> 24) & 3UL;     // numéro d'une boîte vide
    if (dlc > 8U) dlc = 8U;

    uint32_t lo = 0, hi = 0;
    for (uint32_t i = 0; i < dlc; i++)
    {
        if (i < 4U) lo |= (uint32_t)data[i] << (8U * i);
        else        hi |= (uint32_t)data[i] << (8U * (i - 4U));
    }

    CAN_TXMB(mb, TX_TDTR) = dlc;
    CAN_TXMB(mb, TX_TDLR) = lo;
    CAN_TXMB(mb, TX_TDHR) = hi;
    CAN_TXMB(mb, TX_TIR)  = ((uint32_t)std_id << 21);
    CAN_TXMB(mb, TX_TIR) |= CAN_TIR_TXRQ;

    uint32_t rqcp = CAN_TSR_RQCP << (8U * mb);
    uint32_t t    = CAN_TIMEOUT;
    while (!(CAN1_TSR & rqcp))
    {
        if (--t == 0) return CAN_ERR_BUSY;
    }
    CAN1_TSR = rqcp;
    return CAN_OK;
}
#endif

// Affiche une trame reçue : "RX ID=0x321 DLC=8 [filtre 0] DATA=01 00 ..."
static void print_msg(const can_msg_t *m)
{
    uart_send_string("RX ID=0x");
    uart_send_hex(m->id, m->extended ? 8U : 3U);
    uart_send_string(" DLC=");
    uart_send_uint(m->dlc);
    uart_send_string(" [filtre ");
    uart_send_uint(m->fmi);
    uart_send_string("]");

    if (m->rtr)
    {
        uart_send_string(" (requete, pas de donnees)");
    }
    else
    {
        uart_send_string(" DATA=");
        for (uint32_t i = 0; i < m->dlc; i++)
        {
            uart_send_hex(m->data[i], 2);
            uart_send_byte(' ');
        }
    }
    uart_send_string("\r\n");
}

int main(void)
{
    // UART : PA2 en AF7, 115200 bauds
    RCC_AHB1ENR |= (1UL << 0);
    RCC_APB1ENR |= (1UL << 17);
    GPIOA_MODER &= ~(3UL << (TX_PIN * 2));
    GPIOA_MODER |=  (2UL << (TX_PIN * 2));
    GPIOA_AFRL  &= ~(0xFUL << (TX_PIN * 4));
    GPIOA_AFRL  |=  (7UL   << (TX_PIN * 4));
    USART2_BRR = (138UL << 4) | 14UL;
    USART2_CR1 = USART_CR1_TE | USART_CR1_UE;

    uart_send_string("=== CAN Receiver (500 kbit/s) ===\r\n");
    uart_send_string("Filtres : 0x321, 0x322 et plage 0x100-0x10F\r\n");

    if (can1_init() != CAN_OK)
    {
        uart_send_string("Erreur : init CAN impossible\r\n");
        while (1) { }
    }

#if CAN_LOOPBACK
    uart_send_string("Mode loopback : le programme s'envoie des trames de test\r\n");

    // Quelques ID : certains passent les filtres, d'autres non
    static const uint16_t test_ids[] = { 0x321, 0x322, 0x323, 0x100, 0x105, 0x110, 0x555 };
    uint32_t n = 0;

    while (1)
    {
        uint16_t id = test_ids[n % (sizeof(test_ids) / sizeof(test_ids[0]))];
        uint8_t  payload[8] = { (uint8_t)n, (uint8_t)(n >> 8), 'T', 'E', 'S', 'T', 0, 0 };

        uart_send_string("TX ID=0x");
        uart_send_hex(id, 3);
        uart_send_string("\r\n");

        if (can_send(id, payload, 6U) != CAN_OK)
            uart_send_string("   echec d'emission\r\n");

        delay(200000);   // le temps que la trame traverse le contrôleur et que l'ISR la range

        can_msg_t msg;
        uint32_t got = 0;
        while (queue_pop(&msg)) { print_msg(&msg); got++; }

        if (got == 0)
            uart_send_string("   (rejetee par les filtres, rien recu)\r\n");

        if (dropped || hw_overruns)
        {
            uart_send_string("   pertes : buffer=");
            uart_send_uint(dropped);
            uart_send_string(" FIFO=");
            uart_send_uint(hw_overruns);
            uart_send_string("\r\n");
        }

        n++;
        delay(2000000);  // une trame de test par seconde environ
    }
#else
    uart_send_string("En attente de trames sur le bus...\r\n");

    while (1)
    {
        can_msg_t msg;

        // Le main n'a plus qu'à afficher ce que l'interruption a rangé
        while (queue_pop(&msg))
        {
            print_msg(&msg);
        }
    }
#endif
}