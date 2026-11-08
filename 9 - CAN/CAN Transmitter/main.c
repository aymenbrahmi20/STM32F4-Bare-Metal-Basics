#include <stdint.h>

// 1 = loopback silencieux (test sans transceiver), 0 = vrai bus CAN
#define CAN_LOOPBACK  1

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOD_BASE        (AHB1PERIPH_BASE + 0x0C00UL)
#define USART2_BASE       (APB1PERIPH_BASE + 0x4400UL)
#define CAN1_BASE         (APB1PERIPH_BASE + 0x6400UL)   // 0x40006400

// RCC : GPIOA/GPIOD sur AHB1, USART2 et CAN1 sur APB1
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA (TX de l'UART) et GPIOD (broches CAN)
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00UL))
#define GPIOD_OSPEEDR (*(volatile uint32_t *)(GPIOD_BASE + 0x08UL))
#define GPIOD_PUPDR   (*(volatile uint32_t *)(GPIOD_BASE + 0x0CUL))
#define GPIOD_AFRL    (*(volatile uint32_t *)(GPIOD_BASE + 0x20UL))

// CAN1 : registres de contrôle
#define CAN1_MCR      (*(volatile uint32_t *)(CAN1_BASE + 0x000UL))   // configuration générale
#define CAN1_MSR      (*(volatile uint32_t *)(CAN1_BASE + 0x004UL))   // état (init, veille)
#define CAN1_TSR      (*(volatile uint32_t *)(CAN1_BASE + 0x008UL))   // état des 3 boîtes d'émission
#define CAN1_RF0R     (*(volatile uint32_t *)(CAN1_BASE + 0x00CUL))   // état de la FIFO 0 de réception
#define CAN1_ESR      (*(volatile uint32_t *)(CAN1_BASE + 0x018UL))   // compteurs et état d'erreur
#define CAN1_BTR      (*(volatile uint32_t *)(CAN1_BASE + 0x01CUL))   // timing du bit

// CAN1 : boîte d'émission n = base + 0x180 + n x 0x10
#define CAN_TXMB(n, off)  (*(volatile uint32_t *)(CAN1_BASE + 0x180UL + (n) * 0x10UL + (off)))
#define TX_TIR   0x00UL    // identifiant + demande d'envoi
#define TX_TDTR  0x04UL    // longueur des données
#define TX_TDLR  0x08UL    // données, octets 0 à 3
#define TX_TDHR  0x0CUL    // données, octets 4 à 7

// CAN1 : réception, FIFO 0
#define CAN1_RI0R     (*(volatile uint32_t *)(CAN1_BASE + 0x1B0UL))
#define CAN1_RDT0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1B4UL))
#define CAN1_RDL0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1B8UL))
#define CAN1_RDH0R    (*(volatile uint32_t *)(CAN1_BASE + 0x1BCUL))

// CAN1 : filtres
#define CAN1_FMR      (*(volatile uint32_t *)(CAN1_BASE + 0x200UL))
#define CAN1_FM1R     (*(volatile uint32_t *)(CAN1_BASE + 0x204UL))
#define CAN1_FS1R     (*(volatile uint32_t *)(CAN1_BASE + 0x20CUL))
#define CAN1_FFA1R    (*(volatile uint32_t *)(CAN1_BASE + 0x214UL))
#define CAN1_FA1R     (*(volatile uint32_t *)(CAN1_BASE + 0x21CUL))
#define CAN1_F0R1     (*(volatile uint32_t *)(CAN1_BASE + 0x240UL))
#define CAN1_F0R2     (*(volatile uint32_t *)(CAN1_BASE + 0x244UL))

// Bits de MCR / MSR
#define CAN_MCR_INRQ   (1UL << 0)    // demande d'entrée en mode initialisation
#define CAN_MCR_SLEEP  (1UL << 1)    // demande de veille
#define CAN_MCR_TXFP   (1UL << 2)    // priorité d'émission par ordre de demande (sinon par ID)
#define CAN_MCR_ABOM   (1UL << 6)    // sortie automatique du bus-off
#define CAN_MSR_INAK   (1UL << 0)    // 1 = on est bien en mode initialisation
#define CAN_MSR_SLAK   (1UL << 1)    // 1 = on est bien en veille

// Bits de TSR (par boîte n : décalage de 8 x n)
#define CAN_TSR_RQCP   (1UL << 0)    // requête terminée
#define CAN_TSR_TXOK   (1UL << 1)    // envoi réussi (trame acquittée)
#define CAN_TSR_ABRQ   (1UL << 7)    // demande d'annulation
#define CAN_TSR_TME_ANY (7UL << 26)  // au moins une boîte vide

// Bits de TIxR / RIxR
#define CAN_TIR_TXRQ   (1UL << 0)    // demande d'envoi
#define CAN_RIR_IDE    (1UL << 2)    // 1 = identifiant étendu 29 bits
#define CAN_RF0R_RFOM0 (1UL << 5)    // libère le message en tête de FIFO

// Timing : 500 kbit/s avec une horloge APB1 de 16 MHz
#define CAN_PRESCALER  2UL     // 16 MHz / 2 = 8 MHz, un time quantum = 125 ns
#define CAN_TS1        13UL    // segment avant l'échantillonnage
#define CAN_TS2        2UL     // segment après l'échantillonnage
#define CAN_SJW        1UL     // saut de resynchronisation maximal
#define CAN_BTR_LBKM   (1UL << 30)
#define CAN_BTR_SILM   (1UL << 31)

// USART2 (émission seule)
#define USART2_SR     (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR     (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR    (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1    (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART_SR_TXE  (1UL << 7)
#define USART_CR1_TE  (1UL << 3)
#define USART_CR1_UE  (1UL << 13)

#define TX_PIN        2U    // PA2

// Codes de retour
#define CAN_OK           0
#define CAN_ERR_BUSY    -1   // les trois boîtes d'émission sont occupées
#define CAN_ERR_TIMEOUT -2   // personne n'a acquitté la trame
#define CAN_ERR_INIT    -3   // le contrôleur n'a pas changé de mode
#define CAN_ERR_EMPTY   -4   // rien à lire

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

// Valeur en hexadécimal sur un nombre de chiffres fixé : (0x321, 3) -> "321"
static void uart_send_hex(uint32_t v, uint32_t digits)
{
    const char hex[] = "0123456789ABCDEF";

    while (digits--)
    {
        uart_send_byte((uint8_t)hex[(v >> (digits * 4U)) & 0xFU]);
    }
}

/* ------------------------------------------------------------------
 *  Driver CAN
 * ---------------------------------------------------------------- */

// Attend qu'un bit de MSR prenne la valeur voulue (passage en init ou sortie)
static int can_wait_msr(uint32_t mask, uint32_t expected)
{
    uint32_t t = CAN_TIMEOUT;

    while ((CAN1_MSR & mask) != expected)
    {
        if (--t == 0) return CAN_ERR_INIT;
    }
    return CAN_OK;
}

static int can1_init(void)
{
    // Horloges : GPIOD et CAN1 (bit 25 de APB1ENR)
    RCC_AHB1ENR |= (1UL << 3);
    RCC_APB1ENR |= (1UL << 25);
    (void)RCC_APB1ENR;

    // PD0 (RX) et PD1 (TX) en fonction alternative, AF9 = CAN1
    for (uint32_t pin = 0U; pin <= 1U; pin++)
    {
        GPIOD_MODER   &= ~(3UL   << (pin * 2));
        GPIOD_MODER   |=  (2UL   << (pin * 2));
        GPIOD_OSPEEDR |=  (2UL   << (pin * 2));
        GPIOD_AFRL    &= ~(0xFUL << (pin * 4));
        GPIOD_AFRL    |=  (9UL   << (pin * 4));
    }
    // Pull-up sur RX : si le transceiver n'est pas branché, la ligne reste au niveau
    // "récessif" (1) au lieu de flotter et de déclencher de fausses erreurs
    GPIOD_PUPDR &= ~(3UL << 0);
    GPIOD_PUPDR |=  (1UL << 0);

    // Au reset le contrôleur est en veille : on le réveille
    CAN1_MCR &= ~CAN_MCR_SLEEP;
    if (can_wait_msr(CAN_MSR_SLAK, 0)) return CAN_ERR_INIT;

    // Puis on entre en mode initialisation, seul endroit où on peut régler BTR
    CAN1_MCR |= CAN_MCR_INRQ;
    if (can_wait_msr(CAN_MSR_INAK, CAN_MSR_INAK)) return CAN_ERR_INIT;

    // Timing du bit. Les champs sont stockés "valeur - 1".
    CAN1_BTR = ((CAN_SJW - 1UL) << 24)
             | ((CAN_TS2 - 1UL) << 20)
             | ((CAN_TS1 - 1UL) << 16)
             |  (CAN_PRESCALER - 1UL);
#if CAN_LOOPBACK
    CAN1_BTR |= CAN_BTR_LBKM | CAN_BTR_SILM;   // le contrôleur s'écoute lui-même, la broche TX reste calme
#endif

    // ABOM : si le nœud tombe en bus-off, il revient tout seul.
    // TXFP : les trames partent dans l'ordre où on les demande.
    CAN1_MCR |= CAN_MCR_ABOM | CAN_MCR_TXFP;

    // Filtre 0 : on accepte tout (masque à zéro = aucun bit n'est comparé), vers la FIFO 0.
    // Inutile pour émettre, mais nécessaire pour relire nos trames en loopback.
    CAN1_FMR  |=  1UL;          // FINIT : on passe en mode réglage des filtres
    CAN1_FA1R &= ~1UL;          // filtre 0 désactivé pendant qu'on le règle
    CAN1_FS1R |=  1UL;          // une seule banque de 32 bits
    CAN1_FM1R &= ~1UL;          // mode "identifiant + masque"
    CAN1_FFA1R &= ~1UL;         // les messages acceptés vont dans la FIFO 0
    CAN1_F0R1 = 0;              // identifiant de référence
    CAN1_F0R2 = 0;              // masque : 0 = tout passe
    CAN1_FA1R |=  1UL;
    CAN1_FMR  &= ~1UL;

    // On sort du mode init : le contrôleur se synchronise sur le bus
    // (il attend 11 bits récessifs consécutifs) puis devient actif
    CAN1_MCR &= ~CAN_MCR_INRQ;
    return can_wait_msr(CAN_MSR_INAK, 0);
}

// Envoie une trame à identifiant standard (11 bits), 0 à 8 octets
static int can_send(uint16_t std_id, const uint8_t *data, uint8_t dlc)
{
    uint32_t tsr = CAN1_TSR;

    if (!(tsr & CAN_TSR_TME_ANY)) return CAN_ERR_BUSY;   // aucune boîte libre

    // Le champ CODE (bits 25:24) donne directement le numéro d'une boîte vide
    uint32_t mb = (tsr >> 24) & 3UL;

    if (dlc > 8U) dlc = 8U;

    // Les octets sont rangés petit-boutiste : data[0] dans l'octet de poids faible
    uint32_t lo = 0, hi = 0;
    for (uint32_t i = 0; i < dlc; i++)
    {
        if (i < 4U) lo |= (uint32_t)data[i] << (8U * i);
        else        hi |= (uint32_t)data[i] << (8U * (i - 4U));
    }

    CAN_TXMB(mb, TX_TDTR) = dlc;
    CAN_TXMB(mb, TX_TDLR) = lo;
    CAN_TXMB(mb, TX_TDHR) = hi;

    // L'identifiant standard occupe les bits 31:21. IDE et RTR à 0 : trame de données standard.
    CAN_TXMB(mb, TX_TIR) = ((uint32_t)std_id << 21);
    CAN_TXMB(mb, TX_TIR) |= CAN_TIR_TXRQ;                 // et seulement maintenant : go !

    // Attente de la fin : RQCP passe à 1 que l'envoi ait réussi ou non
    uint32_t rqcp = CAN_TSR_RQCP << (8U * mb);
    uint32_t txok = CAN_TSR_TXOK << (8U * mb);
    uint32_t t    = CAN_TIMEOUT;

    while (!(CAN1_TSR & rqcp))
    {
        if (--t == 0)
        {
            // Personne n'a acquitté : la trame serait renvoyée indéfiniment. On annule.
            CAN1_TSR = CAN_TSR_ABRQ << (8U * mb);
            t = CAN_TIMEOUT;
            while (!(CAN1_TSR & rqcp) && --t) { }
            CAN1_TSR = rqcp;
            return CAN_ERR_TIMEOUT;
        }
    }

    int ok = (CAN1_TSR & txok) ? CAN_OK : CAN_ERR_TIMEOUT;
    CAN1_TSR = rqcp;   // écrire 1 sur RQCP l'efface, et efface TXOK avec
    return ok;
}

// Lit un message de la FIFO 0 s'il y en a un
static int can_receive(uint32_t *id, uint8_t *data, uint8_t *dlc)
{
    if ((CAN1_RF0R & 3UL) == 0) return CAN_ERR_EMPTY;    // FMP0 = nombre de messages en attente

    uint32_t ri = CAN1_RI0R;
    *id  = (ri & CAN_RIR_IDE) ? (ri >> 3) : (ri >> 21);  // étendu sur 29 bits, standard sur 11
    *dlc = (uint8_t)(CAN1_RDT0R & 0xFUL);

    uint32_t lo = CAN1_RDL0R;
    uint32_t hi = CAN1_RDH0R;
    for (uint32_t i = 0; i < 8U; i++)
    {
        data[i] = (i < 4U) ? (uint8_t)(lo >> (8U * i))
                           : (uint8_t)(hi >> (8U * (i - 4U)));
    }

    CAN1_RF0R = CAN_RF0R_RFOM0;   // on libère le message pour laisser la place au suivant
    return CAN_OK;
}

// Affiche une trame : "ID=0x321 DLC=8 DATA=01 00 00 00 43 41 4E 21"
static void print_frame(uint32_t id, const uint8_t *data, uint8_t dlc)
{
    uart_send_string("ID=0x");
    uart_send_hex(id, 3);
    uart_send_string(" DLC=");
    uart_send_uint(dlc);
    uart_send_string(" DATA=");
    for (uint32_t i = 0; i < dlc; i++)
    {
        uart_send_hex(data[i], 2);
        uart_send_byte(' ');
    }
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

    uart_send_string("=== CAN Transmitter (500 kbit/s) ===\r\n");
#if CAN_LOOPBACK
    uart_send_string("Mode loopback silencieux (pas de transceiver necessaire)\r\n");
#endif

    if (can1_init() != CAN_OK)
    {
        uart_send_string("Erreur : init CAN impossible\r\n");
        while (1) { }
    }

    uint32_t counter = 0;

    while (1)
    {
        // Charge utile : compteur sur 4 octets, puis le texte "CAN!"
        uint8_t tx[8] = {
            (uint8_t)(counter),       (uint8_t)(counter >> 8),
            (uint8_t)(counter >> 16), (uint8_t)(counter >> 24),
            'C', 'A', 'N', '!'
        };

        int r = can_send(0x321U, tx, 8U);

        uart_send_string("TX ");
        if (r == CAN_OK)
        {
            print_frame(0x321U, tx, 8U);
            uart_send_string("-> OK\r\n");

#if CAN_LOOPBACK
            // En loopback, notre propre trame revient dans la FIFO 0 : on vérifie
            uint32_t rx_id;
            uint8_t  rx[8];
            uint8_t  rx_dlc;

            if (can_receive(&rx_id, rx, &rx_dlc) == CAN_OK)
            {
                uart_send_string("RX ");
                print_frame(rx_id, rx, rx_dlc);
                uart_send_string("\r\n");
            }
#endif
        }
        else
        {
            uart_send_string("-> ECHEC (pas d'acquittement ?)  TEC=");
            uart_send_uint((CAN1_ESR >> 16) & 0xFFUL);   // compteur d'erreurs d'émission
            uart_send_string("\r\n");
        }

        counter++;
        delay(2000000);   // une trame par seconde environ
    }
}