# include <stdint.h> 
 
# define _IO volatile uint32_t


typedef struct {
	
	_IO MODER ;
	_IO OTYPER ;
	_IO OSPEEDER ;
	_IO PUPDR ;
	_IO IDR ;
	_IO ODR ;
	_IO BSRR ;
	_IO LCKR ;
	_IO ARF[2] ;
} GPIO_Typedef;

typedef struct
{
    _IO CR;           /* 0x00 */
    _IO PLLCFGR;      /* 0x04 */
    _IO CFGR;         /* 0x08 */
    _IO CIR;          /* 0x0C */
    _IO AHB1RSTR;     /* 0x10 */
    _IO AHB2RSTR;     /* 0x14 */
    _IO AHB3RSTR;     /* 0x18 */
    uint32_t      RESERVED0;    /* 0x1C */
    _IO APB1RSTR;     /* 0x20 */
    _IO APB2RSTR;     /* 0x24 */
    uint32_t      RESERVED1[2]; /* 0x28 - 0x2C */
    _IO AHB1ENR;      /* 0x30 */
} RCC_TypeDef;

# define PERIPH_BASE  0x40000000UL
# define AHB1PERIPH_BASE  (PERIPH_BASE + 0x00020000UL)




