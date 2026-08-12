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
#define GPIOA1 (1UL << 1)//mascara para pin 1 

#define GPIOB_CRH *(volatile uint32_t *)(GPIOB_BASE + 0x04)
#define GPIOB_ODR *(volatile uint32_t *)(GPIOB_BASE + 0x0C)
#define GPIOB13 (1UL << 13)//mascara 1 desplazado 13 lugares para usar para el bit 13 en el odr y decirle que pin es del PB
#define GPIOB12 (1UL << 12)

#define GPIOC_CRH *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)
#define GPIOC13 (1UL << 13)

//exti
#define AFIO_EXTICR1 *(volatile uint32_t *)(AFIO_BASE + 0x08) //son 16 bits el registro del 0 al 3 es el exti0, del 4 al 7 el exti1 del 8 al 11 exti2 y del 12al15 exti3, luego si cada exti tiene 0000 viene del PA si es 0001->PB ,0010->PC y 0011->PD

#define EXTI_IMR *(volatile uint32_t *)(EXTI_BASE + 0x00) //registro mascara para habilitar o bloqquear interrupciones en exti
#define EXTI_RTSR *(volatile uint32_t *)(EXTI_BASE + 0x08) //registro para mascara habilitar con flanco ascendente
#define EXTI_FTSR *(volatile uint32_t *)(EXTI_BASE + 0x0C)//registro mascara habilitar con flanco descendente
#define EXTI_PR *(volatile uint32_t *)(EXTI_BASE + 0x14) //registro bandera para ver si hay una interrupcion pendiente
//nvic
#define NVIC_ISER0 *(volatile uint32_t *)(0xE000E100) //el exti genero la peticion pero hay que habilitarla en el cpu con el nvic, el nvic_iser0 tiene 32 bits, por lo que el iser0 genera la interrupcion para los irq0 al irq31, y despues se usa el irq6 para conectar con el exti0 
#define NVIC_IPR1 *(volatile uint32_t *)(0xE000E404)//vector para definir prioridades del irq4 al irq7 

// bit fields
#define RCC_IOPAEN (1 << 2)//mascara para habilitar el clock en los puertos
#define RCC_IOPBEN (1 << 3)
#define RCC_IOPCEN (1 << 4)
#define RCC_AFIOEN (1 << 0)

void main(void)
{
RCC_APB2ENR |= RCC_IOPAEN;//habilito clocks en los puertos para habilitar cada periferico
RCC_APB2ENR |= RCC_IOPBEN;
RCC_APB2ENR |= RCC_IOPCEN;
RCC_APB2ENR |= RCC_AFIOEN;


GPIOA_CRL &= 0xFFFFFFF0; //mascara pone los primeros 4 bit en 0 pin A0 entrada pulldown intenro
GPIOA_CRL |= 0x00000008; //mascara pone 1000 en el ultimo byte para configurar como entrada pullup/down (1000 se usa para configurar como entrada pull up/down)
GPIOA_ODR &= ~GPIOA0;//pone un 0 en odr para configurar como pulldown (1 para pull up)


GPIOA_CRL &= 0xFFFFFF0F; //mascara pone los primeros 4 bit en 0 pin A1 entrada pulldown intenro
GPIOA_CRL |= 0x00000080; //mascara pone 1000 en el anteultimo byte para configurar como entrada pullup/down (1000 se usa para configurar como entrada pull up/down)
GPIOA_ODR &= ~GPIOA1;//pone un 0 en odr para configurar como pulldown (1 para pull up)

GPIOB_CRH &= 0xFF0FFFFF;//pb13 esta en los bits 20-23
GPIOB_CRH |= 0x00200000;//pone 0010 en el byte 13 de CR (osea byte 7 de CRH) salida pushpull 2mhz

GPIOB_CRH &= 0xFFF0FFFF;//pb12 esta en los bits 20-23
GPIOB_CRH |= 0x00020000;//pone 0010 en el byte 12 de CR (osea byte 6 de CRH)salida pushpull 2mhz


GPIOC_CRH &= 0xFF0FFFFF;//pc13 esta en los bits 20-23
GPIOC_CRH |= 0x00200000;//pone 0010 en el byte 13 de CR (osea byte 7 de CRH) 0010 se usa para configurar como salida 2Mhz pushpull

//ahora vamos a mapear el pa en el exticr1 
AFIO_EXTICR1 &=0xFFFFF0F0;//ponemos 0000 en en exti0(ultimo byte) para decir que viene del PA, si fuera PB->0001 y 0000 en el exti1

//configuramos el exti para flanco acendente
EXTI_IMR |=(1<<0); //este registro tiene como 19 bits cada bits es para cada pin, entonces como queremos el pin0 ponemos un 1 en el bit0, y con imr habilitamos la interrupcion de ese pin
EXTI_RTSR |=(1<<0); //lo mismo pero decimos si se detecta con flanco ascendente
EXTI_FTSR &= ~(1 << 0);//lo mismo pero decimos si se detecta con flanco descendente(como no uqeremos le ponemos un 0 ignora 3,3V->0V)
NVIC_ISER0 |= (1 << 6);// habilitamos la interrupcion irq6 correspondiente al exti0

EXTI_IMR |=(1<<1); //este registro tiene como 19 bits cada bits es para cada pin, entonces como queremos el pin1 ponemos un 1 en el bit1, y con imr habilitamos la interrupcion de ese pin
EXTI_RTSR |=(1<<1); //lo mismo pero decimos si se detecta con flanco ascendente
EXTI_FTSR &= ~(1 << 1);//lo mismo pero decimos si se detecta con flanco descendente(como no uqeremos le ponemos un 0 ignora 3,3V->0V)
NVIC_ISER0 |= (1 << 7);// habilitamos la interrupcion irq7 correspondiente al exti1

//ahora configuramos las prioridades
// EXTI0 (IRQ6) prioridad alta
NVIC_IPR1 &= ~(0xFF << 16);//ponemos en 0 la prioridad del irq6(maxima prioridad) , el ipr1 es para el irq4 irq5 irq6 y irq7, el 3er byte corresponde a irq6

// EXTI1 (IRQ7) prioridad baja
NVIC_IPR1 &= ~(0xFF << 24); //el ultimo byte es de irq7 ponemos en 0011 prioridad 3(menos prioridad)
NVIC_IPR1 |=  (0xC0 << 24);


while (1) // programa led pc13 parpadea
{
GPIOC_ODR |= GPIOC13;
for (int i = 0; i < 500000; i++); // arbitrary delay
 GPIOC_ODR &= ~GPIOC13;
for (int i = 0; i < 500000; i++); // arbitrary delay
}
}


void EXTI0_IRQHandler(void)
{
if (EXTI_PR & (1 << 0))// si hay una interrupcion pendiente en el exti0(flag exti_pr)
{
    EXTI_PR = (1<<0);//limpiamos la bandera poniendo un 1 en exti_pr por alguna razon se limpia con 1, y como es el pin 0 movemos 0 lugares.
    GPIOB_ODR |= GPIOB13;
    for (int i=0; i<4000000; i++);
    GPIOB_ODR &=~GPIOB13;
}
}

void EXTI1_IRQHandler(void)
{
if (EXTI_PR & (1 << 1))// si hay una interrupcion pendiente en el exti1 (flag exti_pr)
{
    EXTI_PR = (1<<1);//limpiamos la bandera poniendo un 1 en exti_pr por alguna razon se limpia con 1, y como es el pin 1 movemos 1 lugar.
    GPIOB_ODR |= GPIOB12;
    for (int i=0; i<5000000; i++);
    GPIOB_ODR &=~GPIOB12;
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

