#include "stm32f1xx.h"   // CMSIS: registros y bits del STM32F103
#include <stdint.h>

static void gpio_init(void);
static void adc_init(void);
static void exti_init(void);

volatile uint8_t modo_adc = 0;


int main(void)
{
    uint16_t valor_adc = 0;

    gpio_init();
    adc_init();
    exti_init();


    while (1)
    {
        //=================================================
        // Parpadeo del LED de PC13
        //=================================================

        GPIOC->ODR ^= GPIO_ODR_ODR13;

        for (volatile int i = 0; i < 100000; i++);


        //=================================================
        // Selección del canal ADC
        // modo_adc = 0 -> sensor de temperatura interno
        // modo_adc = 1 -> PA1 (ADC channel 1)
        //=================================================

        if (modo_adc == 0)
        {
            // Sensor de temperatura interno
            ADC1->SQR3 = 16;
        }
        else
        {
            // PA1 -> canal ADC1
            ADC1->SQR3 = 1;
        }


        //=================================================
        // Iniciar conversión
        //=================================================

        ADC1->CR2 |= ADC_CR2_SWSTART;


        // Esperar fin de conversión
        while (!(ADC1->SR & ADC_SR_EOC));


        // Leer resultado
        valor_adc = ADC1->DR;


        //=================================================
        // Apagar todos los LEDs de PB11-PB15
        //=================================================

        GPIOB->ODR &= ~(GPIO_ODR_ODR11 |
                        GPIO_ODR_ODR12 |
                        GPIO_ODR_ODR13 |
                        GPIO_ODR_ODR14 |
                        GPIO_ODR_ODR15);


        //=================================================
        // Barra de LEDs según valor ADC
        //=================================================

        if (valor_adc > 800)
            GPIOB->ODR |= GPIO_ODR_ODR11;

        if (valor_adc > 1600)
            GPIOB->ODR |= GPIO_ODR_ODR12;

        if (valor_adc > 2400)
            GPIOB->ODR |= GPIO_ODR_ODR13;

        if (valor_adc > 3200)
            GPIOB->ODR |= GPIO_ODR_ODR14;

        if (valor_adc > 4000)
            GPIOB->ODR |= GPIO_ODR_ODR15;
    }
}


/*=========================================================
 * CONFIGURACIÓN DE GPIO
 *=========================================================*/
static void gpio_init(void)
{
    //=====================================================
    // Habilitar clocks
    //=====================================================

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN |
                    RCC_APB2ENR_AFIOEN;


    //=====================================================
    // PA0 -> entrada pull-down
    //=====================================================

    GPIOA->CRL &= ~(GPIO_CRL_MODE0 |
                    GPIO_CRL_CNF0);

    GPIOA->CRL |= GPIO_CRL_CNF0_1;

    // Pull-down
    GPIOA->ODR &= ~GPIO_ODR_ODR0;


    //=====================================================
    // PA1 -> entrada analógica
    //=====================================================

    GPIOA->CRL &= ~(GPIO_CRL_MODE1 |
                    GPIO_CRL_CNF1);


    //=====================================================
    // PB11-PB15 -> salidas push-pull 2 MHz
    //=====================================================

    GPIOB->CRH &= ~(GPIO_CRH_MODE11 |
                    GPIO_CRH_CNF11 |
                    GPIO_CRH_MODE12 |
                    GPIO_CRH_CNF12 |
                    GPIO_CRH_MODE13 |
                    GPIO_CRH_CNF13 |
                    GPIO_CRH_MODE14 |
                    GPIO_CRH_CNF14 |
                    GPIO_CRH_MODE15 |
                    GPIO_CRH_CNF15);

    GPIOB->CRH |= GPIO_CRH_MODE11_1 |
                  GPIO_CRH_MODE12_1 |
                  GPIO_CRH_MODE13_1 |
                  GPIO_CRH_MODE14_1 |
                  GPIO_CRH_MODE15_1;


    //=====================================================
    // PC13 -> salida push-pull 2 MHz
    //=====================================================

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 |
                    GPIO_CRH_CNF13);

    GPIOC->CRH |= GPIO_CRH_MODE13_1;
}


/*=========================================================
 * CONFIGURACIÓN DEL ADC1
 *=========================================================*/
static void adc_init(void)
{
    //=====================================================
    // Habilitar clock de ADC1
    //=====================================================

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;


    //=====================================================
    // Encender ADC
    //=====================================================

    ADC1->CR2 |= ADC_CR2_ADON;

    // Pequeño retardo
    for (volatile int i = 0; i < 10000; i++);

    // El código original vuelve a escribir ADON
    ADC1->CR2 |= ADC_CR2_ADON;


    //=====================================================
    // Habilitar sensor de temperatura interno
    //=====================================================

    ADC1->CR2 |= ADC_CR2_TSVREFE;


    //=====================================================
    // Canal 16 -> sensor de temperatura
    //=====================================================

    ADC1->SQR3 = 16;


    //=====================================================
    // Tiempo de muestreo
    //=====================================================

    // Canal 16 está en SMPR1.
    // 111 = 239.5 ciclos
    ADC1->SMPR1 |= (7U << 18);


    // Canal 1 está en SMPR2.
    // 111 = 239.5 ciclos
    ADC1->SMPR2 |= (7U << 3);


    //=====================================================
    // Disparo por software
    //
    // EXTSEL = 111 -> software
    // EXTTRIG = 1  -> habilita trigger externo
    //=====================================================

    ADC1->CR2 |= ADC_CR2_EXTSEL |
                 ADC_CR2_EXTTRIG;


    //=====================================================
    // Calibración del ADC
    //
    // No estaba incluida en el código original,
    // por lo tanto se mantiene fuera para conservar
    // exactamente su comportamiento.
    //=====================================================
}


/*=========================================================
 * CONFIGURACIÓN DE EXTI0 + NVIC
 *=========================================================*/
static void exti_init(void)
{
    //=====================================================
    // EXTI0 conectado a PA0
    //=====================================================

    // 0000 = GPIOA
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;


    //=====================================================
    // Interrupción EXTI0
    //=====================================================

    // Habilitar línea 0
    EXTI->IMR |= EXTI_IMR_MR0;


    // Flanco ascendente
    EXTI->RTSR |= EXTI_RTSR_TR0;


    // No utilizar flanco descendente
    EXTI->FTSR &= ~EXTI_FTSR_TR0;


    //=====================================================
    // Habilitar EXTI0 en NVIC
    //=====================================================

    NVIC_EnableIRQ(EXTI0_IRQn);
}


/*=========================================================
 * HANDLER DE EXTI0
 *=========================================================*/
void EXTI0_IRQHandler(void)
{
    // Verificar si EXTI0 está pendiente
    if (EXTI->PR & EXTI_PR_PR0)
    {
        // Limpiar bandera
        EXTI->PR = EXTI_PR_PR0;


        // Pequeño retardo
        for (volatile int i = 0; i < 100000; i++);


        // Cambiar modo del ADC
        //
        // 0 -> sensor de temperatura
        // 1 -> PA1
        modo_adc ^= 1;
    }
}


/*=========================================================
 * HARDFAULT HANDLER
 *=========================================================*/
void HardFault_Handler(void)
{
    while (1)
    {
        // Encender PB13
        GPIOB->ODR |= GPIO_ODR_ODR13;

        for (volatile int i = 0; i < 500000; i++);


        // Apagar PB13
        GPIOB->ODR &= ~GPIO_ODR_ODR13;

        for (volatile int i = 0; i < 500000; i++);
    }
}

