#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ------------ Seccion 1: Configuracion del Hardware ------------ */

#define configCPU_CLOCK_HZ                  (72000000UL)
#define configTICK_RATE_HZ                  ((TickType_t)1000)

/* ------------ Seccion 2: Configuracion del Kernel ------------ */

#define configUSE_PREEMPTION                1
#define configSUPPORT_DYNAMIC_ALLOCATION    1
#define configSUPPORT_STATIC_ALLOCATION     0

#define configTOTAL_HEAP_SIZE               ((size_t)(5 * 1024))

#define configMINIMAL_STACK_SIZE            128
#define configMAX_PRIORITIES                5
#define configMAX_TASK_NAME_LEN             16

#define configUSE_16_BIT_TICKS              0

/* ------------ Seccion 3: Hooks y Opciones de Depuracion ------------ */

#define configUSE_IDLE_HOOK                 0
#define configUSE_TICK_HOOK                 0
#define configCHECK_FOR_STACK_OVERFLOW      0

/* ------------ Seccion 4: Inclusion de APIs ------------ */

#define INCLUDE_vTaskDelay                  1

/* ------------ Seccion 5: Prioridades de Interrupcion ------------ */

/* En Cortex-M, prioridad 0 es la prioridad mas alta. */

#define configKERNEL_INTERRUPT_PRIORITY      255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 191

/*
 * Las interrupciones con prioridad numerica inferior a esta
 * no pueden llamar a la API de FreeRTOS.
 */

/* ------------ Seccion 6: Mapeo de Handlers a CMSIS ------------ */

#define vPortSVCHandler                    SVC_Handler
#define xPortPendSVHandler                 PendSV_Handler
#define xPortSysTickHandler                SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
