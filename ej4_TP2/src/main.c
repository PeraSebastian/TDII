#include "stm32f1xx.h"
#include <stdint.h>

/*
 * =========================================================
 * TP2 - Ejercicio 4
 * Comunicación UART entre BluePill y PC
 *
 * USART1:
 *      PA9  -> TX
 *      PA10 -> RX
 *
 * Configuración:
 *      9600 baudios
 *      8 bits
 *      sin paridad
 *      1 bit de stop
 *
 * Comandos recibidos desde PC:
 *
 *      '1' -> LED ON
 *      '2' -> LED OFF
 *      '3' -> ADC1 (PA1)
 *      '4' -> Sensor de temperatura interno
 *
 * La recepción se realiza mediante interrupción USART1.
 * =========================================================
 */


/* =========================================================
 * PROTOTIPOS
 * ========================================================= */

static void gpio_init(void);
static void adc_init(void);
static void usart1_init(void);

static void usart1_send_char(char c);
static void usart1_send_string(const char *str);


/* =========================================================
 * VARIABLES GLOBALES
 * ========================================================= */

/*
 * modo_adc:
 *
 * 0 -> sensor de temperatura interno, canal 16
 * 1 -> ADC1 externo, PA1, canal 1
 */
volatile uint8_t modo_adc = 0;

/*
 * Variable donde la interrupción USART guarda
 * el último comando recibido.
 */
volatile uint8_t comando = 0;

/*
 * Indica al programa principal que llegó
 * un nuevo comando.
 */
volatile uint8_t comando_recibido = 0;


/* =========================================================
 * MAIN
 * ========================================================= */

int main(void)
{
    uint16_t valor_adc = 0;


    /* -----------------------------------------------------
     * Inicialización
     * ----------------------------------------------------- */

    gpio_init();
    adc_init();
    usart1_init();


    /* -----------------------------------------------------
     * Mensaje inicial
     * ----------------------------------------------------- */

    usart1_send_string("\r\n");
    usart1_send_string("================================\r\n");
    usart1_send_string("       BLUEPILL - UART\r\n");
    usart1_send_string("================================\r\n");
    usart1_send_string("UART OK - 9600 8N1\r\n");
    usart1_send_string("Esperando comandos...\r\n");


    /* -----------------------------------------------------
     * Bucle principal
     * ----------------------------------------------------- */

    while (1)
    {

        /* =================================================
         * PROCESAMIENTO DE COMANDOS RECIBIDOS
         * ================================================= */

        if (comando_recibido)
        {
            /*
             * Desactivar la bandera antes de procesar
             * el comando.
             */
            comando_recibido = 0;


            switch (comando)
            {

                /* -----------------------------------------
                 * COMANDO 1 -> LED ON
                 * ----------------------------------------- */

                case '1':

                    /*
                     * LED integrado de la BluePill en PC13.
                     *
                     * El LED es activo en LOW:
                     *   0 -> encendido
                     *   1 -> apagado
                     */
                    GPIOC->ODR &= ~GPIO_ODR_ODR13;

                    usart1_send_string("LED ON\r\n");

                    break;


                /* -----------------------------------------
                 * COMANDO 2 -> LED OFF
                 * ----------------------------------------- */

                case '2':

                    GPIOC->ODR |= GPIO_ODR_ODR13;

                    usart1_send_string("LED OFF\r\n");

                    break;


                /* -----------------------------------------
                 * COMANDO 3 -> ADC1 PA1
                 * ----------------------------------------- */

                case '3':

                    /*
                     * Canal 1 del ADC1:
                     * PA1
                     */
                    modo_adc = 1;

                    usart1_send_string("ADC1 PA1\r\n");

                    break;


                /* -----------------------------------------
                 * COMANDO 4 -> SENSOR TEMPERATURA
                 * ----------------------------------------- */

                case '4':

                    /*
                     * Canal 16:
                     * sensor de temperatura interno
                     */
                    modo_adc = 0;

                    usart1_send_string("SENSOR TEMPERATURA\r\n");

                    break;


                /* -----------------------------------------
                 * COMANDO DESCONOCIDO
                 * ----------------------------------------- */

                default:

                    usart1_send_string("COMANDO INVALIDO\r\n");

                    break;
            }
        }


        /* =================================================
         * SELECCIÓN DEL CANAL ADC
         * ================================================= */

        if (modo_adc == 0)
        {
            /*
             * Canal 16:
             * Sensor de temperatura interno
             */
            ADC1->SQR3 = 16;
        }
        else
        {
            /*
             * Canal 1:
             * PA1
             */
            ADC1->SQR3 = 1;
        }


        /* =================================================
         * INICIAR CONVERSIÓN
         * ================================================= */

        ADC1->CR2 |= ADC_CR2_SWSTART;


        /* =================================================
         * ESPERAR FIN DE CONVERSIÓN
         * ================================================= */

        while (!(ADC1->SR & ADC_SR_EOC));


        /* =================================================
         * LEER RESULTADO
         * ================================================= */

        valor_adc = ADC1->DR;


        /* =================================================
         * APAGAR TODOS LOS LEDs DE LA BARRA
         * ================================================= */

        GPIOB->ODR &= ~(GPIO_ODR_ODR11 |
                        GPIO_ODR_ODR12 |
                        GPIO_ODR_ODR13 |
                        GPIO_ODR_ODR14 |
                        GPIO_ODR_ODR15);


        /* =================================================
         * BARRA DE LEDs SEGÚN VALOR ADC
         * ================================================= */

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


/* =========================================================
 * CONFIGURACIÓN DE GPIO
 * ========================================================= */

static void gpio_init(void)
{
    /*
     * Habilitar clocks:
     *
     * GPIOA -> USART1
     * GPIOB -> barra de LEDs
     * GPIOC -> LED integrado
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN;


    /* =====================================================
     * PA1 -> ENTRADA ANALÓGICA
     * ===================================================== */

    GPIOA->CRL &= ~(GPIO_CRL_MODE1 |
                    GPIO_CRL_CNF1);


    /* =====================================================
     * PB11-PB15 -> SALIDAS PUSH-PULL 2 MHz
     * ===================================================== */

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


    /* =====================================================
     * PC13 -> SALIDA PUSH-PULL 2 MHz
     * ===================================================== */

    GPIOC->CRH &= ~(GPIO_CRH_MODE13 |
                    GPIO_CRH_CNF13);

    GPIOC->CRH |= GPIO_CRH_MODE13_1;


    /*
     * LED inicialmente apagado.
     *
     * PC13 = HIGH -> LED OFF
     */
    GPIOC->ODR |= GPIO_ODR_ODR13;
}


/* =========================================================
 * CONFIGURACIÓN ADC1
 * ========================================================= */

static void adc_init(void)
{
    /*
     * Habilitar clock ADC1
     */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;


    /*
     * Configurar prescaler ADC.
     *
     * Si APB2 = 72 MHz:
     *
     * ADC clock = 72 MHz / 6 = 12 MHz
     *
     * dentro del límite del ADC del STM32F1.
     */
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;


    /*
     * Encender ADC
     */
    ADC1->CR2 |= ADC_CR2_ADON;


    /*
     * Pequeño retardo
     */
    for (volatile int i = 0; i < 10000; i++);


    /*
     * Habilitar sensor de temperatura interno
     */
    ADC1->CR2 |= ADC_CR2_TSVREFE;


    /*
     * Canal inicial:
     *
     * 16 -> sensor de temperatura
     */
    ADC1->SQR3 = 16;


    /*
     * Tiempo de muestreo:
     *
     * Canal 16 -> SMPR1
     * 111 = 239.5 ciclos
     */
    ADC1->SMPR1 |= (7U << 18);


    /*
     * Canal 1 -> SMPR2
     * 111 = 239.5 ciclos
     */
    ADC1->SMPR2 |= (7U << 3);


    /*
     * Trigger por software.
     *
     * EXTSEL = 111
     * EXTTRIG = 1
     */
    ADC1->CR2 |= ADC_CR2_EXTSEL |
                 ADC_CR2_EXTTRIG;
}


/* =========================================================
 * CONFIGURACIÓN USART1
 * ========================================================= */

static void usart1_init(void)
{
    /*
     * Habilitar clocks:
     *
     * GPIOA
     * USART1
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_USART1EN;


    /* =====================================================
     * PA9 -> USART1_TX
     *
     * Alternate Function Push-Pull
     * 50 MHz
     * ===================================================== */

    GPIOA->CRH &= ~(GPIO_CRH_MODE9 |
                    GPIO_CRH_CNF9);

    GPIOA->CRH |= GPIO_CRH_MODE9 |
                  GPIO_CRH_CNF9_1;


    /* =====================================================
     * PA10 -> USART1_RX
     *
     * Entrada flotante
     * ===================================================== */

    GPIOA->CRH &= ~(GPIO_CRH_MODE10 |
                    GPIO_CRH_CNF10);

    GPIOA->CRH |= GPIO_CRH_CNF10_0;


    /* =====================================================
     * CONFIGURACIÓN 9600 8N1
     * ===================================================== */

    /*
     * M = 0
     * 8 bits de datos
     */
    USART1->CR1 &= ~USART_CR1_M;


    /*
     * PCE = 0
     * Sin paridad
     */
    USART1->CR1 &= ~USART_CR1_PCE;


    /*
     * STOP = 00
     * 1 bit de stop
     */
    USART1->CR2 &= ~USART_CR2_STOP;


    /*
     * Baudrate:
     *
     * USART1 clock = 72 MHz
     * Baudrate = 9600
     */
    USART1->BRR = 0x1D4C;


    /* =====================================================
     * HABILITAR USART
     * ===================================================== */

    /*
     * UE -> USART enable
     * TE -> Transmitter enable
     * RE -> Receiver enable
     */
    USART1->CR1 |= USART_CR1_UE |
                   USART_CR1_TE |
                   USART_CR1_RE;


    /* =====================================================
     * INTERRUPCIÓN DE RECEPCIÓN
     * ===================================================== */

    /*
     * RXNEIE = 1
     *
     * Se genera una interrupción cuando
     * llega un dato y RXNE pasa a 1.
     */
    USART1->CR1 |= USART_CR1_RXNEIE;


    /*
     * Habilitar USART1 en el NVIC
     */
    NVIC_EnableIRQ(USART1_IRQn);
}


/* =========================================================
 * TRANSMITIR UN CARÁCTER
 * ========================================================= */

static void usart1_send_char(char c)
{
    /*
     * Esperar hasta que el registro
     * de datos de transmisión esté vacío.
     */
    while (!(USART1->SR & USART_SR_TXE));


    /*
     * Escribir carácter en DR.
     */
    USART1->DR = c;
}


/* =========================================================
 * TRANSMITIR UNA CADENA
 * ========================================================= */

static void usart1_send_string(const char *str)
{
    while (*str)
    {
        usart1_send_char(*str);
        str++;
    }
}


/* =========================================================
 * INTERRUPCIÓN USART1
 * ========================================================= */

void USART1_IRQHandler(void)
{
    /*
     * Verificar si se recibió un dato.
     */
    if (USART1->SR & USART_SR_RXNE)
    {
        /*
         * Leer DR limpia RXNE.
         */
        comando = (uint8_t)USART1->DR;


        /*
         * Avisar al programa principal
         * que llegó un nuevo comando.
         */
        comando_recibido = 1;
    }
}


/* =========================================================
 * HARDFAULT HANDLER
 * ========================================================= */

void HardFault_Handler(void)
{
    while (1)
    {
        /*
         * Encender PB13
         */
        GPIOB->ODR |= GPIO_ODR_ODR13;

        for (volatile int i = 0; i < 500000; i++);


        /*
         * Apagar PB13
         */
        GPIOB->ODR &= ~GPIO_ODR_ODR13;

        for (volatile int i = 0; i < 500000; i++);
    }
}
