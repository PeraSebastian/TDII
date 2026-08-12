#include <stdint.h>
// register address
#define RCC_BASE 0x40021000
#define AFIO_BASE 0x40010000
#define EXTI_BASE 0x40010400

#define GPIOA_BASE 0x40010800 //base de los resgistros de los puertos
#define GPIOB_BASE 0x40010C00
#define GPIOC_BASE 0x40011000

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18) //registro base clock

#define GPIOA_CRL *(volatile uint32_t *)(GPIOA_BASE + 0x00) //registro base pines bajos 0-7
#define GPIOA_IDR *(volatile uint32_t *)(GPIOA_BASE + 0x08) // registro base para leer estado de un pin
#define GPIOA_ODR *(volatile uint32_t *)(GPIOA_BASE + 0x0C) //regist base para escribir un pin o configurar pulldown/up interno
#define GPIOA0 (1UL << 0)//mascara 1 desplazado 0 lugares para usar para el bit 0 en el odr y decirle que pin es del PA

#define GPIOC_CRH *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)
#define GPIOC13 (1UL << 13)//mascara 1 desplazado 13 lugares para usar para el bit 13 en el odr y decirle que pin es del PB

#define GPIOB_CRH *(volatile uint32_t *)(GPIOB_BASE + 0x04)
#define GPIOB_ODR *(volatile uint32_t *)(GPIOB_BASE + 0x0C)
#define GPIOB11 (1UL << 11)
#define GPIOB12 (1UL << 12)
#define GPIOB13 (1UL << 13)
#define GPIOB14 (1UL << 14)
#define GPIOB15 (1UL << 15)

//exti
#define AFIO_EXTICR1 *(volatile uint32_t *)(AFIO_BASE + 0x08) //son 16 bits el registro del 0 al 3 es el exti0, del 4 al 7 el exti1 del 8 al 11 exti2 y del 12al15 exti3, luego si cada exti tiene 0000 viene del PA si es 0001->PB ,0010->PC y 0011->PD

#define EXTI_IMR *(volatile uint32_t *)(EXTI_BASE + 0x00) //registro mascara para habilitar o bloqquear interrupciones en exti
#define EXTI_RTSR *(volatile uint32_t *)(EXTI_BASE + 0x08) //registro para mascara habilitar con flanco ascendente
#define EXTI_FTSR *(volatile uint32_t *)(EXTI_BASE + 0x0C)//registro mascara habilitar con flanco descendente
#define EXTI_PR *(volatile uint32_t *)(EXTI_BASE + 0x14) //registro bandera para ver si hay una interrupcion pendiente
//nvic
#define NVIC_ISER0 *(volatile uint32_t *)(0xE000E100) //el exti genero la peticion pero hay que habilitarla en el cpu con el nvic, el nvic_iser0 tiene 32 bits, por lo que el iser0 genera la interrupcion para los irq0 al irq31, y despues se usa el irq6 para conectar con el exti0 

// bit fields
#define RCC_IOPAEN (1 << 2)//mascara para habilitar el clock en los puertos
#define RCC_IOPBEN (1 << 3)
#define RCC_IOPCEN (1 << 4)
#define RCC_AFIOEN (1 << 0)
#define RCC_ADC1EN (1 << 9) //mascara para habilitar clock adc1
//adc
#define ADC1_BASE 0x40012400
#define ADC_SR *(volatile uint32_t *)(ADC1_BASE + 0x00) // direccion del registro de estado
#define ADC_CR1 *(volatile uint32_t *)(ADC1_BASE + 0x04) //direccion registro de control1
#define ADC_CR2 *(volatile uint32_t *)(ADC1_BASE + 0x08)// direccion registro de control2 aca estan ADON, SWSTART, TSVREFE
#define ADC_SMPR1 *(volatile uint32_t *)(ADC1_BASE + 0x0C) //registro para configurar el tiempo de muestreo del canal 10 al 17(sensor temp canal 16, vref interna canal17), cada canal tiene 3 bits ej:000-> muestrea en 1.5ciclos, 001->7.5 ciclos, 010->13.5 ciclos
#define ADC_SMPR2 *(volatile uint32_t *)(ADC1_BASE + 0x10)//registro para configurar el tiempo de muestreo del canal 10 al 9(PA0 canal 0, PA1 canal 1 , ....)
#define ADC_SQR1 *(volatile uint32_t *)(ADC1_BASE + 0x2C)
#define ADC_SQR2 *(volatile uint32_t *)(ADC1_BASE + 0x30)
#define ADC_SQR3 *(volatile uint32_t *)(ADC1_BASE + 0x34)// aca elegimos el canal para hacer la conversion
#define ADC_DR *(volatile uint32_t *)(ADC1_BASE + 0x4C) // aca aparece el resultado de la conversion

#define ADC_EOC (1<<1) //mascara bit 1 de SR (end of conversion)
#define ADC_ADON (1<<0) //mascara bit 0 de cr2 (habilita el ADC)
#define ADC_SWSTART (1<<22) //mascara bit 22 de cr2 (arranca la conversion)
#define ADC_TSVREFE (1<<23) //mascara bit 23 de cr2 (habilita el sensor de temperatura interno y tension de referencia interna)
volatile uint8_t modo_adc = 0;
void main(void)
{
uint16_t valor_adc=0;
RCC_APB2ENR |= RCC_IOPAEN;//habilito clocks en los puertos para habilitar cada periferico
RCC_APB2ENR |= RCC_IOPBEN;
RCC_APB2ENR |= RCC_IOPCEN;
RCC_APB2ENR |= RCC_AFIOEN;
RCC_APB2ENR |= RCC_ADC1EN;

ADC_CR2 |=ADC_ADON; //encendemos el adc
for(int i=0; i<10000; i++);
ADC_CR2 |=ADC_ADON;

ADC_CR2 |=ADC_TSVREFE;//habilitamos el sensor de temp
ADC_SQR3 = 16;//elegimos canal 16
ADC_SMPR1 |= (7<<18); //configuramos en 239.5 ciclos
ADC_SMPR2 |= (7 << 3);

GPIOA_CRL &= 0xFFFFFFF0; //mascara pone los primeros 4 bit en 0 pin A0 entrada pulldown intenro
GPIOA_CRL |= 0x00000008; //mascara pone 1000 en el ultimo byte para configurar como entrada pullup/down (1000 se usa para configurar como entrada pull up/down)
GPIOA_ODR &= ~GPIOA0;//pone un 0 en odr para configurar como pulldown (1 para pull up)

GPIOA_CRL &= 0xFFFFFF0F; //mascara pA1
GPIOA_CRL |= 0x00000000; //mascara pone 0000 para configurar como entrada ADC

GPIOB_CRH &= 0x00000FFF;//0 en los pines PB15 a PB11
GPIOB_CRH |= 0x22222000;//0010 salida push pull 2mhz en PB11 a PB15

GPIOC_CRH &= 0xFF0FFFFF;
GPIOC_CRH |= 0x00200000;//0010 salida push pull 2mhz en PC13


//ahora vamos a mapear el pa en el exticr1 
AFIO_EXTICR1 &=0xFFFFFFF0;//ponemos 0000 en en exti0(ultimo byte) para decir que viene del PA, si fuera PB->0001

//configuramos el exti para flanco acendente
EXTI_IMR |=(1<<0); //este registro tiene como 19 bits cada bits es para cada pin, entonces como queremos el pin0 ponemos un 1 en el bit0, y con imr habilitamos la interrupcion de ese pin
EXTI_RTSR |=(1<<0); //lo mismo pero decimos si se detecta con flanco ascendente
EXTI_FTSR &= ~(1 << 0);//lo mismo pero decimos si se detecta con flanco descendente(como no uqeremos le ponemos un 0 ignora 3,3V->0V)
NVIC_ISER0 |= (1 << 6);// habilitamos la interrupcion irq6 correspondiente al exti0
// Habilitar disparo por software (EXTSEL = 111) y habilitar trigger (EXTTRIG = 1)
ADC_CR2 |= (7 << 17) | (1 << 20);
while (1) // programa led pc13 parpadea
{

GPIOC_ODR  ^= GPIOC13; //led tp1
for(int i=0; i<50000; i++);
if(modo_adc==0)
{
ADC_SQR3 = 16; //sensor temp
}
else
{
ADC_SQR3 = 1; //PA1
}

ADC_CR2 |=ADC_SWSTART; //iniciamos la conversion

while(!(ADC_SR & ADC_EOC));//mientras este conviertiendo se queda en el bucle while

valor_adc =ADC_DR; //leemos el resultado

GPIOB_ODR &=~(GPIOB11 | GPIOB12 | GPIOB13 | GPIOB14 | GPIOB15);//apagamos todos los leds

if(valor_adc > 800)
  GPIOB_ODR |=GPIOB11; //prende led 1
  
if(valor_adc > 1600)
  GPIOB_ODR |=GPIOB12;//led 2
  
if(valor_adc > 2400)
  GPIOB_ODR |=GPIOB13;  //led 3

if(valor_adc > 3200)
  GPIOB_ODR |=GPIOB14;//led 4

if(valor_adc > 4000)
  GPIOB_ODR |=GPIOB15;//led 5
}
}

void EXTI0_IRQHandler(void)
{
    if(EXTI_PR & (1<<0))
    {
        EXTI_PR = (1<<0);

        for(volatile int i=0;i<100000;i++);

        modo_adc ^= 1;
    }
}

void HardFault_Handler(void)//si ocurre un error entra aca parpadea el led del pb13
{
    while(1)
    {
        // prender LED
        GPIOB_ODR |= GPIOB13;

        for(int i = 0; i < 500000; i++);

        // apagar LED
        GPIOB_ODR &= ~GPIOB13;

        for(int i = 0; i < 500000; i++);
    }
}

