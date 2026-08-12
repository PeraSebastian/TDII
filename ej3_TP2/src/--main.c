/*
 * Programa: Migración a CMSIS - Ejemplo BluePill STM32F103
 *
 * Funcionamiento general:
 * ------------------------
 * Este programa implementa una versión del TP utilizando CMSIS
 * (Cortex Microcontroller Software Interface Standard), reemplazando
 * el acceso manual a registros mediante punteros por las estructuras
 * definidas en stm32f1xx.h.
 *
 * El sistema realiza las siguientes funciones:
 *
 * 1) Configura los GPIO:
 *    - PC13 como salida para controlar el LED onboard de la BluePill.
 *    - PB13 como salida para un LED externo.
 *    - PB12 como entrada digital para leer el estado de un pulsador.
 *
 * 2) Mantiene el LED onboard (PC13) titilando continuamente,
 *    indicando que el programa está en ejecución.
 *
 * 3) Realiza la lectura del pulsador mediante polling:
 *    - Se consulta continuamente el estado del pin PB12
 *      dentro del bucle principal.
 *    - Según el estado leído, se modifica el estado del LED
 *      conectado al pin PB13.
 *
 * 4) Ambas tareas se ejecutan simultáneamente dentro del
 *    lazo principal: el titilado del LED onboard y la lectura
 *    continua del pulsador.
 *
 * Objetivo:
 * ----------
 * Comprender el uso de CMSIS para acceder a los registros del
 * microcontrolador STM32F103, manteniendo el comportamiento
 * del programa BareMetal original pero con una implementación
 * más legible y mantenible.
 */
#include "stm32f1xx.h"   // CMSIS: registros y bits del STM32F103
#include <stdint.h>

// register address
/*#define RCC_BASE 0x40021000
#define GPIOC_BASE 0x40011000
#define GPIOB_BASE 0x40010C00

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)

#define GPIOC_CRH *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)

#define GPIOB_CRH *(volatile uint32_t *)(GPIOB_BASE + 0x04)
#define GPIOB_ODR *(volatile uint32_t *)(GPIOB_BASE + 0x0C)
#define GPIOB_IDR *(volatile uint32_t *)(GPIOB_BASE + 0x08)

// campos de bit
#define RCC_IOPCEN (1 << 4)
#define RCC_IOPBEN (1 << 3)
#define GPIOC13 (1UL << 13)
#define GPIOB13 (1UL << 13)
#define GPIOB12 (1UL << 12)
*/
static void gpio_init(void);

int main(void)
{
    gpio_init();

    while (1)
    {
        GPIOC->ODR |= GPIO_ODR_ODR13;     // Antes: GPIOC_ODR |= GPIOC13
	
	for (int i = 0; i < 500000; i++)
        {	if (GPIOB->IDR & GPIO_IDR_IDR12)  // Antes: GPIOB_IDR & GPIOB12
            		GPIOB->ODR |= GPIO_ODR_ODR13; // Antes: GPIOB_ODR |= GPIOB13
        	else
            		GPIOB->ODR &= ~GPIO_ODR_ODR13;
	}
        GPIOC->ODR &= ~GPIO_ODR_ODR13;    // Antes: GPIOC_ODR &= ~GPIOC13
	
	for (int i = 0; i < 500000; i++)
        {	if (GPIOB->IDR & GPIO_IDR_IDR12)
            		GPIOB->ODR |= GPIO_ODR_ODR13;
        	else
            		GPIOB->ODR &= ~GPIO_ODR_ODR13;
    	}
	}
}

static void gpio_init(void)
{
    // Habilitar clock de GPIOB y GPIOC
    // Antes: RCC_APB2ENR |= RCC_IOPBEN | RCC_IOPCEN
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN;

    // PC13 salida push-pull 2 MHz
    // Antes: GPIOC_CRH &= 0xFF0FFFFF; GPIOC_CRH |= 0x00200000
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |=  GPIO_CRH_MODE13_1;

    // PB13 salida push-pull 2 MHz
    GPIOB->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOB->CRH |=  GPIO_CRH_MODE13_1;

    // PB12 entrada pull-up/pull-down
    // Antes: GPIOB_CRH &= 0xFFF0FFFF; GPIOB_CRH |= 0x00080000
    GPIOB->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12);
    GPIOB->CRH |=  GPIO_CRH_CNF12_1;

    // Pull-down interno en PB12
    // Antes: GPIOB_ODR &= ~GPIOB12
    GPIOB->ODR &= ~GPIO_ODR_ODR12;
}

