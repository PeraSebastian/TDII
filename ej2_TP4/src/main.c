#include "stm32f1xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


/* ============================================================
 * VARIABLES GLOBALES
 * ============================================================ */

/*
 * Guarda temporalmente el resultado de la conversión ADC.
 */
volatile uint16_t adcValue = 0;


/*
 * Semáforo binario:
 *
 * ADC ISR
 *    |
 *    +----> xSemaphoreGiveFromISR()
 *
 * Productora
 *    |
 *    +----> xSemaphoreTake()
 */
SemaphoreHandle_t adcSemaphore;


/*
 * Cola para comunicación Productor -> Consumidor.
 *
 * La cola tendrá espacio para 5 valores ADC.
 */
QueueHandle_t adcQueue;


/* ============================================================
 * GPIO
 * ============================================================ */

static void GPIO_Init(void)
{
    /*
     * Habilitar reloj GPIOA
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

ADC1->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
    /*
     * PA0:
     *
     * ADC1_IN0
     *
     * MODE0 = 00
     * CNF0  = 00
     *
     * Esto configura PA0 como entrada analógica.
     */
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);


    /*
     * Habilitar GPIOC para LED PC13
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;


    /*
     * PC13 como salida push-pull 10 MHz
     */
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
    GPIOC->CRH |= GPIO_CRH_MODE13_0;


    /*
     * LED inicialmente apagado.
     */
    GPIOC->BSRR = GPIO_BSRR_BS13;
}


/* ============================================================
 * USART1
 *
 * PA9  -> TX
 * PA10 -> RX
 *
 * 115200 8N1
 * ============================================================ */

static void UART1_Init(void)
{
    /*
     * Habilitar GPIOA
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;


    /*
     * Habilitar USART1
     */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;


    /*
     * PA9 -> Alternate Function Push-Pull
     *
     * MODE9 = 11 -> 50 MHz
     * CNF9  = 10 -> AF Push-Pull
     */
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);

    GPIOA->CRH |= GPIO_CRH_MODE9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;


    /*
     * PA10 -> Input Floating
     */
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);

    GPIOA->CRH |= GPIO_CRH_CNF10_0;


    /*
     * APB2 = 72 MHz
     *
     * 72 MHz / 115200 ~= 625
     *
     * BRR = 0x0271
     */
    USART1->BRR = 0x0271;


    /*
     * 8 bits
     * Sin paridad
     * 1 stop
     */
    USART1->CR1 &= ~USART_CR1_M;
    USART1->CR1 &= ~USART_CR1_PCE;

    USART1->CR2 &= ~USART_CR2_STOP;


    /*
     * Habilitar USART
     */
    USART1->CR1 |= USART_CR1_UE;


    /*
     * Habilitar transmisor
     */
    USART1->CR1 |= USART_CR1_TE;
}


/* ============================================================
 * UART - ENVIAR CARACTER
 * ============================================================ */

static void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE))
    {
    }

    USART1->DR = (uint16_t)c;
}


/* ============================================================
 * UART - ENVIAR STRING
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
 * UART - ENVIAR NUMERO ENTERO
 * ============================================================ */

static void UART1_SendUInt(uint16_t value)
{
    char buffer[6];

    int i = 0;

    /*
     * Caso especial: 0
     */
    if (value == 0)
    {
        UART1_SendChar('0');
        return;
    }


    /*
     * Convertir número a ASCII
     * comenzando desde el final.
     */
    while (value > 0)
    {
        buffer[i] = '0' + (value % 10);

        value /= 10;

        i++;
    }


    /*
     * Transmitir al revés.
     */
    while (i > 0)
    {
        i--;

        UART1_SendChar(buffer[i]);
    }
}


/* ============================================================
 * ADC1
 *
 * ADC1_IN0 -> PA0
 *
 * Conversión por software.
 * Interrupción cuando termina conversión.
 * ============================================================ */

static void ADC1_Init(void)
{
    /*
     * Habilitar reloj ADC1
     */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;


    /*
     * ADC clock:
     *
     * APB2 = 72 MHz
     * ADC = 72 / 6 = 12 MHz
     */
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;


    /*
     * Reset configuración ADC
     */
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;


    /*
     * Conversión externa:
     *
     * EXTSEL = 111
     *
     * Esto selecciona SWSTART como disparador.
     */

    ADC1->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;

    /*
     * Una sola conversión.
     */
    ADC1->SQR1 = 0;

    ADC1->SQR2 = 0;

    ADC1->SQR3 = 0;


    /*
     * Canal 0 -> PA0
     *
     * Tiempo de muestreo:
     * 55.5 ciclos.
     */
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP0;

    ADC1->SMPR2 |= ADC_SMPR2_SMP0_1 |
                   ADC_SMPR2_SMP0_0;


    /*
     * Interrupción cuando termina la conversión.
     */
    ADC1->CR1 |= ADC_CR1_EOCIE;


    /*
     * Habilitar ADC.
     */
    ADC1->CR2 |= ADC_CR2_ADON;


    /*
     * Calibración.
     */

    /* Reset calibration */
    ADC1->CR2 |= ADC_CR2_RSTCAL;

    while (ADC1->CR2 & ADC_CR2_RSTCAL)
    {
    }


    /* Start calibration */
    ADC1->CR2 |= ADC_CR2_CAL;

    while (ADC1->CR2 & ADC_CR2_CAL)
    {
    }


    /*
     * IMPORTANTE:
     *
     * La ISR utiliza funciones FromISR de FreeRTOS.
     * Por eso configuramos una prioridad compatible
     * con FreeRTOS.
     */
    NVIC_SetPriority(ADC1_2_IRQn, 5);

    NVIC_EnableIRQ(ADC1_2_IRQn);
}

/* ============================================================
 * INICIAR CONVERSION ADC
 * ============================================================ */

static void ADC1_StartConversion(void)
{
    /*
     * Iniciar conversión mediante software.
     */
    ADC1->CR2 |= ADC_CR2_SWSTART;
}


/* ============================================================
 * INTERRUPCION ADC
 *
 * Se ejecuta cuando:
 *
 * ADC termina conversión
 *        |
 *        v
 * ADC1_2_IRQHandler()
 *        |
 *        +--> guardar ADC
 *        |
 *        +--> liberar semáforo
 * ============================================================ */

void ADC1_2_IRQHandler(void)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;


    /*
     * Verificar End Of Conversion.
     */
    if (ADC1->SR & ADC_SR_EOC)
    {
        /*
         * Leer DR.
         *
         * La lectura limpia EOC.
         */
        adcValue = ADC1->DR;


        /*
         * Avisar a la tarea productora
         * que terminó la conversión.
         */
        xSemaphoreGiveFromISR(
            adcSemaphore,
            &higherPriorityTaskWoken
        );


        /*
         * Si el semáforo despertó una tarea
         * de mayor prioridad, pedir cambio
         * de contexto al salir de la ISR.
         */
        portYIELD_FROM_ISR(
            higherPriorityTaskWoken
        );
    }
}


/* ============================================================
 * TAREA PRODUCTORA
 *
 * Lee ADC cada 100 ms.
 *
 * Productor:
 *
 * 1. Inicia ADC
 * 2. Espera semáforo
 * 3. Obtiene resultado
 * 4. Envía resultado a Queue
 * 5. Espera 100 ms
 * ============================================================ */

static void vProducerTask(void *pvParameters)
{
    (void)pvParameters;

    uint16_t value;

    while (1)
    {
        UART1_SendString("Iniciando ADC\r\n");

        /*
         * Iniciar conversión
         */
        ADC1_StartConversion();

        /*
         * Esperar a que termine
         */
        if (xSemaphoreTake(
                adcSemaphore,
                pdMS_TO_TICKS(100)
            ) == pdTRUE)
        {
            UART1_SendString("ADC terminado\r\n");

            value = adcValue;

            /*
             * Enviar resultado a Queue
             */
            xQueueSend(
                adcQueue,
                &value,
                portMAX_DELAY
            );
        }
        else
        {
            UART1_SendString("ERROR: ADC timeout\r\n");
        }

        /*
         * Periodo de 100 ms
         */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/* ============================================================
 * TAREA CONSUMIDORA
 *
 * Recibe valores desde Queue
 * y los envía por UART.
 * ============================================================ */

static void vConsumerTask(void *pvParameters)
{
    (void)pvParameters;


    uint16_t value;


    while (1)
    {
        /*
         * Esperar un valor de la Queue.
         */
        if (xQueueReceive(
                adcQueue,
                &value,
                portMAX_DELAY
            ) == pdTRUE)
        {
            /*
             * Mostrar resultado.
             */
            UART1_SendString("ADC1 = ");

            UART1_SendUInt(value);

            UART1_SendString("\r\n");
        }
    }
}


/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    /*
     * Actualizar frecuencia del sistema.
     */
    SystemCoreClockUpdate();


    /*
     * Inicializar hardware.
     */
    GPIO_Init();

    UART1_Init();

    ADC1_Init();


    /*
     * Crear semáforo binario.
     *
     * IMPORTANTE:
     *
     * Se crea vacío.
     *
     * La ISR será quien lo libere cuando
     * termine una conversión.
     */
    adcSemaphore = xSemaphoreCreateBinary();


    /*
     * Crear Queue.
     *
     * 5 posiciones.
     *
     * Cada posición almacena un uint16_t.
     */
    adcQueue = xQueueCreate(5, sizeof(uint16_t));
    
    if (adcSemaphore == NULL)
{
    UART1_SendString("ERROR: SEMAFORO\r\n");

    while (1)
    {
    }
}

if (adcQueue == NULL)
{
    UART1_SendString("ERROR: QUEUE\r\n");

    while (1)
    {
    }
}

    /*
     * Crear Productora.
     *
     * Prioridad 2.
     */
    xTaskCreate(
        vProducerTask,
        "Productor",
        256,
        NULL,
        2,
        NULL
    );


    /*
     * Crear Consumidora.
     *
     * Prioridad 1.
     */
    xTaskCreate(
        vConsumerTask,
        "Consumidor",
        256,
        NULL,
        1,
        NULL
    );


    /*
     * Iniciar Scheduler.
     */
    vTaskStartScheduler();


    /*
     * Nunca debería llegar acá.
     */
    while (1)
    {
    }
}

