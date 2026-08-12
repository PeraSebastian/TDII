#include <stdint.h>

// Direcciones base
#define RCC_BASE    0x40021000
#define GPIOB_BASE  0x40010C00
#define GPIOC_BASE  0x40011000

// Registros de Reloj y GPIO
#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOB_CRH   *(volatile uint32_t *)(GPIOB_BASE + 0x04)
#define GPIOC_CRH   *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOB_ODR   *(volatile uint32_t *)(GPIOB_BASE + 0x0C)
#define GPIOC_ODR   *(volatile uint32_t *)(GPIOC_BASE + 0x0C)

// Máscaras de bits
#define RCC_IOPBEN  (1 << 3) // Habilitar reloj Puerto B
#define RCC_IOPCEN  (1 << 4) // Habilitar reloj Puerto C
#define GPIOB13      (1 << 13)  // Pin 13
#define GPIOC13     (1UL << 13) // Pin 13

// --- Registros SysTick ---
#define SysTick_BASE 0xE000E010
#define SysTick_CTRL *(volatile uint32_t *)(SysTick_BASE + 0x00)
#define SysTick_LOAD *(volatile uint32_t *)(SysTick_BASE + 0x04)
#define SysTick_VAL  *(volatile uint32_t *)(SysTick_BASE + 0x08)

volatile uint32_t tick;

void SysTick_Handler(void) { tick++; }

void systick_init_ms(void) {
    tick = 0;
    SysTick_CTRL &= ~(1 << 2); // Reloj a HCLK/8
    SysTick_LOAD = 999;
    SysTick_VAL = 0;
    SysTick_CTRL |= 0x03; // Enable e interrupción
}

void main(void) {
    // 1. Habilitar relojes para Puerto B y C
    RCC_APB2ENR |= (RCC_IOPBEN | RCC_IOPCEN);

    // 2. Configurar PC13 como salida (LED de la placa)
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    // 3. Configurar PB13 como salida (LED externo)
    GPIOB_CRH &= 0xFF0FFFFF; // Limpiar bits del pin 13
    GPIOB_CRH |= 0x00200000; // Modo salida 2MHz push-pull

    systick_init_ms();

    while (1) {
        // --- Lógica del LED de la placa (PC13) ---
        // Cambia de estado cada 1000ms
        if ((tick % 1000) < 500) {
            GPIOC_ODR &= ~GPIOC13; // Encender
        } else {
            GPIOC_ODR |= GPIOC13;  // Apagar
        }

        // --- Lógica del LED externo (PB13) ---
        // Cambia de estado cada 200ms
        if ((tick % 200) < 100) {
            GPIOB_ODR |= GPIOB13;   // Encender
        } else {
            GPIOB_ODR &= ~GPIOB13;  // Apagar
        }
    }
}
