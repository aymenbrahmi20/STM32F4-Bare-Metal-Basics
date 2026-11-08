#include <stdint.h>

// Adresses de base
#define PERIPH_BASE       0x40000000UL
#define APB1PERIPH_BASE   (PERIPH_BASE + 0x00000000UL)
#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE          (AHB1PERIPH_BASE + 0x3800UL)
#define GPIOA_BASE        (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE        (AHB1PERIPH_BASE + 0x0400UL)
#define USART2_BASE       (APB1PERIPH_BASE + 0x4400UL)
#define I2C1_BASE         (APB1PERIPH_BASE + 0x5400UL)   // 0x40005400

// RCC : GPIOA/GPIOB sur AHB1, USART2 et I2C1 sur APB1
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

// GPIOA (TX de l'UART) et GPIOB (I2C)
#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER  (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_OSPEEDR (*(volatile uint32_t *)(GPIOB_BASE + 0x08UL))
#define GPIOB_PUPDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x0CUL))
#define GPIOB_AFRL    (*(volatile uint32_t *)(GPIOB_BASE + 0x20UL))

// I2C1
#define I2C1_CR1      (*(volatile uint32_t *)(I2C1_BASE + 0x00UL))
#define I2C1_CR2      (*(volatile uint32_t *)(I2C1_BASE + 0x04UL))
#define I2C1_DR       (*(volatile uint32_t *)(I2C1_BASE + 0x10UL))
#define I2C1_SR1      (*(volatile uint32_t *)(I2C1_BASE + 0x14UL))
#define I2C1_SR2      (*(volatile uint32_t *)(I2C1_BASE + 0x18UL))
#define I2C1_CCR      (*(volatile uint32_t *)(I2C1_BASE + 0x1CUL))
#define I2C1_TRISE    (*(volatile uint32_t *)(I2C1_BASE + 0x20UL))

#define I2C_CR1_PE      (1UL << 0)     // active le périphérique
#define I2C_CR1_START   (1UL << 8)     // génère une condition START
#define I2C_CR1_STOP    (1UL << 9)     // génère une condition STOP
#define I2C_CR1_ACK     (1UL << 10)    // répond ACK après chaque octet reçu
#define I2C_CR1_POS     (1UL << 11)    // position de l'ACK (utilisé pour la lecture de 2 octets)
#define I2C_CR1_SWRST   (1UL << 15)    // reset logiciel

#define I2C_SR1_SB      (1UL << 0)     // START envoyé
#define I2C_SR1_ADDR    (1UL << 1)     // adresse acquittée par l'esclave
#define I2C_SR1_BTF     (1UL << 2)     // octet terminé (transfert fini)
#define I2C_SR1_RXNE    (1UL << 6)     // un octet est arrivé
#define I2C_SR1_TXE     (1UL << 7)     // on peut écrire le suivant
#define I2C_SR1_AF      (1UL << 10)    // pas d'ACK reçu (acknowledge failure)
#define I2C_SR1_BERR    (1UL << 8)     // erreur de bus
#define I2C_SR2_BUSY    (1UL << 1)     // le bus est occupé

// USART2 (émission seule, pour afficher le résultat)
#define USART2_SR     (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR     (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR    (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1    (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))
#define USART_SR_TXE  (1UL << 7)
#define USART_CR1_TE  (1UL << 3)
#define USART_CR1_UE  (1UL << 13)

#define SCL_PIN   6U    // PB6
#define SDA_PIN   7U    // PB7
#define TX_PIN    2U    // PA2

#define MPU_ADDR        0x68U   // adresse 7 bits du MPU6050 (AD0 à GND)
#define MPU_REG_WHOAMI  0x75U

// Codes de retour du driver
#define I2C_OK        0
#define I2C_ERR_NACK  1    // personne n'a répondu à cette adresse
#define I2C_ERR_TIMEOUT 2  // le bus est resté bloqué

#define I2C_TIMEOUT   100000UL   // nombre de tours avant d'abandonner une attente

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

static void uart_send_hex8(uint8_t v)
{
    const char hex[] = "0123456789ABCDEF";
    uart_send_string("0x");
    uart_send_byte((uint8_t)hex[v >> 4]);
    uart_send_byte((uint8_t)hex[v & 0xF]);
}

/* ------------------------------------------------------------------
 *  Driver I2C
 * ---------------------------------------------------------------- */

// Attend qu'un flag de SR1 passe à 1. Renvoie 0 si OK, -1 si ça n'arrive jamais
// ou si l'esclave a refusé (NACK). Sans cette garde-fou, un bus bloqué ou un
// capteur débranché figerait le programme pour toujours.
static int i2c_wait_flag(uint32_t flag)
{
    uint32_t t = I2C_TIMEOUT;

    while (!(I2C1_SR1 & flag))
    {
        if (I2C1_SR1 & I2C_SR1_AF) return -1;   // NACK : inutile d'attendre plus longtemps
        if (--t == 0)              return -1;   // timeout
    }
    return 0;
}

// Clôt proprement une transaction ratée : on efface l'erreur et on libère le bus
static void i2c_abort(void)
{
    I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR);   // on efface les flags d'erreur
    I2C1_CR1 |= I2C_CR1_STOP;                   // STOP pour rendre la main au bus
}

// START + adresse. 'read' = 0 pour écrire, 1 pour lire.
static int i2c_start(uint8_t addr7, uint8_t read)
{
    I2C1_CR1 |= I2C_CR1_START;
    if (i2c_wait_flag(I2C_SR1_SB)) { i2c_abort(); return I2C_ERR_TIMEOUT; }

    // Le dernier bit de l'octet d'adresse dit si on lit (1) ou écrit (0)
    I2C1_DR = (uint8_t)((addr7 << 1) | read);

    if (i2c_wait_flag(I2C_SR1_ADDR)) { i2c_abort(); return I2C_ERR_NACK; }

    // Pour effacer ADDR, il faut lire SR1 puis SR2 (c'est la séquence imposée par le matériel)
    (void)I2C1_SR1;
    (void)I2C1_SR2;
    return I2C_OK;
}

// Écrit un registre : START, adresse (W), registre, valeur, STOP
static int i2c_write_reg(uint8_t addr7, uint8_t reg, uint8_t value)
{
    int r = i2c_start(addr7, 0);
    if (r) return r;

    if (i2c_wait_flag(I2C_SR1_TXE)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
    I2C1_DR = reg;                                  // quel registre on vise

    if (i2c_wait_flag(I2C_SR1_TXE)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
    I2C1_DR = value;                                // et ce qu'on y met

    if (i2c_wait_flag(I2C_SR1_BTF)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
    I2C1_CR1 |= I2C_CR1_STOP;                       // fin de transaction
    return I2C_OK;
}

// Lit 'len' octets à partir du registre 'reg' (len >= 1).
// Séquence : START + adresse (W) + registre, puis RESTART + adresse (R) + lecture.
static int i2c_read_regs(uint8_t addr7, uint8_t reg, uint8_t *buf, uint32_t len)
{
    int r = i2c_start(addr7, 0);
    if (r) return r;

    if (i2c_wait_flag(I2C_SR1_TXE)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
    I2C1_DR = reg;                                  // on indique à partir d'où lire
    if (i2c_wait_flag(I2C_SR1_BTF)) { i2c_abort(); return I2C_ERR_TIMEOUT; }

    // Repeated START (sans STOP entre les deux) : on passe en lecture
    r = i2c_start(addr7, 1);
    if (r) return r;

    // Le matériel demande un traitement particulier pour les derniers octets :
    // il faut dire "NACK" avant de recevoir le dernier, pour que l'esclave s'arrête.
    // C'est ce qui rend le cas 1 octet un peu différent du cas général.
    if (len == 1)
    {
        I2C1_CR1 &= ~I2C_CR1_ACK;                   // pas d'ACK : on ne veut qu'un octet
        I2C1_CR1 |= I2C_CR1_STOP;                   // STOP programmé juste après la réception

        if (i2c_wait_flag(I2C_SR1_RXNE)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
        buf[0] = (uint8_t)I2C1_DR;
    }
    else
    {
        I2C1_CR1 |= I2C_CR1_ACK;                    // on acquitte chaque octet reçu

        for (uint32_t i = 0; i < len; i++)
        {
            if (i == len - 1U)
            {
                // Dernier octet : on retire l'ACK et on demande le STOP à l'avance
                I2C1_CR1 &= ~I2C_CR1_ACK;
                I2C1_CR1 |= I2C_CR1_STOP;
            }

            if (i2c_wait_flag(I2C_SR1_RXNE)) { i2c_abort(); return I2C_ERR_TIMEOUT; }
            buf[i] = (uint8_t)I2C1_DR;
        }
    }

    I2C1_CR1 |= I2C_CR1_ACK;                        // on remet l'ACK pour la prochaine transaction
    return I2C_OK;
}

// Cas courant : lire un seul registre
static int i2c_read_reg(uint8_t addr7, uint8_t reg, uint8_t *value)
{
    return i2c_read_regs(addr7, reg, value, 1);
}

static void i2c1_init(void)
{
    // Horloges : GPIOB et I2C1 (bit 21 de APB1ENR)
    RCC_AHB1ENR |= (1UL << 1);
    RCC_APB1ENR |= (1UL << 21);
    (void)RCC_APB1ENR;

    // PB6 et PB7 en fonction alternative, AF4 = I2C1
    for (uint32_t pin = SCL_PIN; pin <= SDA_PIN; pin++)
    {
        GPIOB_MODER   &= ~(3UL   << (pin * 2));
        GPIOB_MODER   |=  (2UL   << (pin * 2));
        GPIOB_OTYPER  |=  (1UL   << pin);            // open-drain : obligatoire en I2C
        GPIOB_OSPEEDR |=  (2UL   << (pin * 2));
        GPIOB_PUPDR   &= ~(3UL   << (pin * 2));
        GPIOB_PUPDR   |=  (1UL   << (pin * 2));      // pull-up interne en renfort (faible : garde les résistances externes)
        GPIOB_AFRL    &= ~(0xFUL << (pin * 4));
        GPIOB_AFRL    |=  (4UL   << (pin * 4));
    }

    // Reset logiciel : certains esclaves peuvent laisser le bus dans un état bizarre
    // après un reset du MCU, et ça remet le périphérique dans un état propre
    I2C1_CR1 |=  I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;

    // Fréquence de l'horloge APB1, en MHz (ici 16)
    I2C1_CR2 = 16U;

    // CCR : en mode standard, la période SCL = 2 x CCR x T_PCLK1
    // Pour 100 kHz : CCR = 16 MHz / (2 x 100 kHz) = 80
    I2C1_CCR = 80U;

    // TRISE : temps de montée maximal = 1000 ns en mode standard
    // -> 1000 ns / 62,5 ns + 1 = 17
    I2C1_TRISE = 17U;

    // On active le périphérique en dernier, avec l'ACK
    I2C1_CR1 = I2C_CR1_PE | I2C_CR1_ACK;
}

int main(void)
{
    // UART : PA2 en AF7
    RCC_AHB1ENR |= (1UL << 0);
    RCC_APB1ENR |= (1UL << 17);
    GPIOA_MODER &= ~(3UL << (TX_PIN * 2));
    GPIOA_MODER |=  (2UL << (TX_PIN * 2));
    GPIOA_AFRL  &= ~(0xFUL << (TX_PIN * 4));
    GPIOA_AFRL  |=  (7UL   << (TX_PIN * 4));
    USART2_BRR = (138UL << 4) | 14UL;
    USART2_CR1 = USART_CR1_TE | USART_CR1_UE;

    i2c1_init();

    uart_send_string("=== I2C Master : WHO_AM_I du MPU6050 ===\r\n");

    // Le MPU6050 démarre en veille (bit SLEEP du registre PWR_MGMT_1 = 0x6B) :
    // on l'écrit à 0 pour le réveiller, ça sert aussi de test d'écriture
    if (i2c_write_reg(MPU_ADDR, 0x6B, 0x00) != I2C_OK)
    {
        uart_send_string("Pas de reponse du capteur (verifie cablage, pull-ups, adresse)\r\n");
    }

    while (1)
    {
        uint8_t id = 0;
        int r = i2c_read_reg(MPU_ADDR, MPU_REG_WHOAMI, &id);

        if (r == I2C_OK)
        {
            uart_send_string("WHO_AM_I = ");
            uart_send_hex8(id);
            uart_send_string(id == 0x68 ? "  -> MPU6050 OK\r\n" : "  -> valeur inattendue\r\n");
        }
        else if (r == I2C_ERR_NACK)
        {
            uart_send_string("NACK : aucun peripherique a cette adresse\r\n");
        }
        else
        {
            uart_send_string("Timeout : bus bloque ?\r\n");
        }

        delay(2000000);   // une lecture par seconde environ
    }
}