#include <stdint.h>

// register address
#define RCC_BASE 0x40021000
#define GPIOC_BASE 0x40011000
#define GPIOB_BASE 0x40010C00

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOB_CRH   *(volatile uint32_t *)(GPIOB_BASE + 0x04)
#define GPIOB_ODR   *(volatile uint32_t *)(GPIOB_BASE + 0x0C)
#define GPIOC_CRH   *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR   *(volatile uint32_t *)(GPIOC_BASE + 0x0C)

// bit fields
#define RCC_IOPBEN (1 << 3)
#define GPIOB13 (1 << 13)
#define RCC_IOPCEN (1 << 4)
#define GPIOC13 (1UL << 13)

void main(void)
{
    RCC_APB2ENR |= (RCC_IOPBEN | RCC_IOPCEN);

    GPIOB_CRH &= 0xFF0FFFFF;
    GPIOB_CRH |= 0x00200000;
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    while (1)
    {
	GPIOB_ODR |= GPIOB13;
	for (int i = 0; i < 500000; i++);
       	GPIOB_ODR &= ~GPIOB13;
        for (int i = 0; i < 500000; i++);
    }

    while (1)
    {
	GPIOC_ODR |= GPIOC13;
	for (int i = 0; i < 500000; i++);
       	GPIOC_ODR &= ~GPIOC13;
        for (int i = 0; i < 500000; i++);
    }
}
