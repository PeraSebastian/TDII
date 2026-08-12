.syntax unified
.cpu cortex-m3
.thumb

.global _reset
.extern main
.extern _esstack
.extern EXTI0_IRQHandler
.extern EXTI1_IRQHandler
.extern HardFault_Handler
// =====================================================
// HANDLERS
// =====================================================

.section .text.manejadores


// =====================================================
// WEAK HANDLERS
// =====================================================

.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

// =====================================================
// VECTOR TABLE
// =====================================================

.section .isr_vector,"a",%progbits

.word _esstack
.word _reset + 1
.word NMI_Handler + 1
.word HardFault_Handler +1 
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word SysTick_Handler +1

// IRQ externas

.word 0                  // IRQ0 WWDG
.word 0                  // IRQ1 PVD
.word 0                  // IRQ2 TAMPER
.word 0                  // IRQ3 RTC
.word 0                  // IRQ4 FLASH
.word 0                  // IRQ5 RCC

.word EXTI0_IRQHandler +1   // IRQ6 EXTI0
.word EXTI1_IRQHandler +1   // IRQ7 EXTI1


// =====================================================
// RESET
// =====================================================

.section .text.reset

.thumb_func
_reset:
    bl main
b .

.thumb_func
.weak Default_Handler
Default_Handler:
b .
