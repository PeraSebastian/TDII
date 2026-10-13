#include "stm32f1xx.h"

#include "FreeRTOS.h"
#include "task.h"

/* ============================================================
 * VARIABLES GLOBALES
 * ============================================================ */

/* Contador de ejecuciones del Idle Hook */
volatile unsigned long idleCount = 0;


/* ============================================================
 * CONFIGURACION GPIO - LED PC13
 * ============================================================ */

static void GPIO_Init(void)
{
    /* Habilitar reloj de GPIOC */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /*
     * PC13:
     * MODE13 = 01 -> salida 10 MHz
     * CNF13  = 00 -> salida push-pull
     */
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;

    /*
     * En la BluePill el LED integrado es activo en bajo.
     * Lo dejamos inicialmente apagado.
     */
    GPIOC->BSRR = GPIO_BSRR_BS13;
}


/* ============================================================
 * CONFIGURACION USART1
 *
 * PA9  -> TX
 * PA10 -> RX
 *
 * 115200 baudios
 * 8 bits
 * Sin paridad
 * 1 bit de stop
 * ============================================================ */

static void UART1_Init(void)
{
    /* Habilitar reloj GPIOA */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* Habilitar reloj USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /*
     * PA9 -> Alternate Function Push-Pull
     *
     * MODE9 = 11 -> salida 50 MHz
     * CNF9  = 10 -> Alternate Function Push-Pull
     */
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |= GPIO_CRH_MODE9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;

    /*
     * PA10 -> Input Floating
     *
     * MODE10 = 00
     * CNF10  = 01
     */
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    /*
     * USART1:
     *
     * Si APB2 = 72 MHz:
     *
     * BRR = 72000000 / 115200
     *     ~= 625
     *     = 0x0271
     */
    USART1->BRR = 0x0271;

    /* 8 bits, sin paridad */
    USART1->CR1 &= ~USART_CR1_M;
    USART1->CR1 &= ~USART_CR1_PCE;

    /* 1 bit de stop */
    USART1->CR2 &= ~USART_CR2_STOP;

    /*
     * Habilitar USART
     * Habilitar transmisor
     */
    USART1->CR1 |= USART_CR1_UE;
    USART1->CR1 |= USART_CR1_TE;
}


/* ============================================================
 * FUNCION PARA TRANSMITIR UN CARACTER
 * ============================================================ */

static void UART1_SendChar(char c)
{
    /* Esperar hasta que el registro de transmisión esté vacío */
    while (!(USART1->SR & USART_SR_TXE))
    {
    }

    USART1->DR = (uint16_t)c;
}


/* ============================================================
 * FUNCION PARA TRANSMITIR UNA CADENA
 * ============================================================ */

static void UART1_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART1_SendChar(*str);
        str++;
    }
}


/* ============================================================
 * TAREA A - BLINKY
 *
 * Prioridad alta
 * Frecuencia: 5 Hz
 *
 * 5 Hz -> periodo = 1/5 = 200 ms
 *
 * Como hacemos:
 *
 * ON  -> 100 ms
 * OFF -> 100 ms
 *
 * obtenemos un ciclo completo cada 200 ms.
 * ============================================================ */

static void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        /* LED ON */
        GPIOC->BSRR = GPIO_BSRR_BR13;

        /* Esperar 100 ms */
        vTaskDelay(pdMS_TO_TICKS(100));

        /* LED OFF */
        GPIOC->BSRR = GPIO_BSRR_BS13;

        /* Esperar 100 ms */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/* ============================================================
 * TAREA B - TELEMETRIA
 *
 * Envía información cada 2 segundos.
 * ============================================================ */

static void vUARTTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        UART1_SendString(
            "Sistema Operativo Ejecutando idle:"
        );

        /*
         * Enviar contador de Idle como número decimal.
         *
         * Para mantener el código sencillo se envían
         * los 4 dígitos menos significativos.
         */
        unsigned long valor = idleCount;

        UART1_SendChar(
            '0' + ((valor / 1000) % 10)
        );

        UART1_SendChar(
            '0' + ((valor / 100) % 10)
        );

        UART1_SendChar(
            '0' + ((valor / 10) % 10)
        );

        UART1_SendChar(
            '0' + (valor % 10)
        );

        UART1_SendString("\r\n");

        /*
         * Liberar CPU durante 2 segundos.
         */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


/* ============================================================
 * IDLE HOOK
 *
 * FreeRTOS llama esta función cada vez que ejecuta
 * la Idle Task.
 *
 * IMPORTANTE:
 * En FreeRTOSConfig.h debe estar:
 *
 * #define configUSE_IDLE_HOOK 1
 * ============================================================ */

void vApplicationIdleHook(void)
{
    idleCount++;
}


/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    /* Actualizar SystemCoreClock */
    SystemCoreClockUpdate();

    /* Inicializar hardware */
    GPIO_Init();
    UART1_Init();

    /*
     * Crear Tarea A - Blinky
     *
     * Prioridad 3 -> ALTA
     */
    xTaskCreate(
        vBlinkTask,
        "Blink",
        256,
        NULL,
        3,
        NULL
    );

    /*
     * Crear Tarea B - Telemetría
     *
     * Prioridad 2 -> menor que Blinky
     */
    xTaskCreate(
        vUARTTask,
        "Telemetria",
        256,
        NULL,
        2,
        NULL
    );

    /* Iniciar scheduler */
    vTaskStartScheduler();

    /*
     * Nunca debería llegar acá.
     */
    while (1)
    {
    }
}
