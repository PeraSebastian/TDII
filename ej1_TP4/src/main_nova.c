#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "task.h"

void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        GPIOC->BSRR = GPIO_BSRR_BR13; // LED ON
        vTaskDelay(pdMS_TO_TICKS(100));

        GPIOC->BSRR = GPIO_BSRR_BS13; // LED OFF
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vUARTTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        while(*c){
        	while(!(USART1->SR & USART_SR_TXE));
        		USART1->DR = *c;
                	c++;
        }
        USART1->DR = i;
    }
}

void vCountTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        i++;
        
    }
}

int main(void)
{
    SystemCoreClockUpdate();

    // Habilitar reloj GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // Configurar PC13 como salida push-pull a 10 MHz
    GPIOC->CRH &= ~GPIO_CRH_CNF13;
    GPIOC->CRH |= GPIO_CRH_MODE13_0;

    char *c= "Sistema Operativo Ejecutando idle: ";
    int i=0;
    
    //habilitar PA9(Tx) y PA10(Rx) clok, UART clok.
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    //Configurar PA9 como Tx y PA10 como Rx
    GPIOA->CRH &= ~(0xF<<4);
    GPIOA->CRH |= ~(0xB<<4); //alternate function cnf=10, mode=11

    //Configurar PA10 com Rx input floating cnf=01, mode=00
    GPIOA->CRH &= ~(0xF<<8);
    GPIOA->CRH |= ~(0x4<<8);

    //Configuramos trama 8N1
    USART1->CR1 &=  ~USART_CR1_M;
    USART1->CR1 &=  ~USART_CR1_PCE;
    USART1->CR1 &=  ~USART_CR2_STOP;

    //BRR
    USART1->BRR = Ox1D4C

    //habilitar USART y Tx
    USART1->CR1 &= USART_CR1_UE;
    USART1->CR1 &= USART_CR1_TE;

    // Crear tarea Blink
    // Stack: 256 palabras (1024 bytes), prioridad 1
    BaseType_t res =
        xTaskCreate(vBlinkTask, "Blink", 256, NULL, 1, NULL);
        xTaskCreate(vUARTTask, "Telemetria", 1000, NULL, 2, NULL);
        xTaskCreate(vCountTask, "Cuenta", 1000, NULL, 2, NULL);        

    if (res == pdPASS)
    {
        // Arrancar scheduler
        vTaskStartScheduler();
    }

    // Nunca deberia llegar aqui
    for (;;);
}
