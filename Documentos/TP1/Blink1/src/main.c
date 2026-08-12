# include < stdint .h >
// register address
# define RCC_BASE 0 x40021000
# define GPIOC_BASE 0 x40011000
# define RCC_APB2ENR *( volatile uint32_t *) ( RCC_BASE + 0 x18 )
# define GPIOC_CRH *( volatile uint32_t *) ( GPIOC_BASE + 0 x04 )
# define GPIOC_ODR *( volatile uint32_t *) ( GPIOC_BASE + 0 x0C )
// bit fields
# define RCC_IOPCEN (1 << 4)
# define GPIOC13 (1 UL << 13)
void main ( void )
{
RCC_APB2ENR |= RCC_IOPCEN ;
GPIOC_CRH &= 0 xFF0FFFFF ;
GPIOC_CRH |= 0 x00200000 ;
while (1)
{
GPIOC_ODR |= GPIOC13 ;
for ( int i = 0; i < 500000; i ++) ; // arbitrary delay
GPIOC_ODR &= ~ GPIOC13 ;
for ( int i = 0; i < 50000; i ++) ; // arbitrary delay
}
}