# ============================================================
# Makefile - STM32F103 + CMSIS + FreeRTOS
# Trabajo Practico Nro 3 - Tecnicas Digitales II
# ============================================================

# ------------------------------------------------------------
# Toolchain
# ------------------------------------------------------------

PREFIX = arm-none-eabi-

CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE    = $(PREFIX)size


# ------------------------------------------------------------
# Proyecto
# ------------------------------------------------------------

TARGET = app


# ------------------------------------------------------------
# CPU
# ------------------------------------------------------------

CPU = cortex-m3


# ------------------------------------------------------------
# Directorios
# ------------------------------------------------------------

SRC_DIR      = src
CMSIS_DIR    = CMSIS
FREERTOS_DIR = FreeRTOS
LD_DIR       = ld


# ------------------------------------------------------------
# Directorios de include
# ------------------------------------------------------------

INCLUDES = \
	-I$(SRC_DIR) \
	-I$(CMSIS_DIR) \
	-I$(FREERTOS_DIR)/include \
	-I$(FREERTOS_DIR)/portable/GCC/ARM_CM3


# ------------------------------------------------------------
# Defines
# ------------------------------------------------------------

DEFINES = \
	-DSTM32F103xB


# ------------------------------------------------------------
# Flags de compilacion
# ------------------------------------------------------------

CFLAGS = \
	-mcpu=$(CPU) \
	-mthumb \
	-mfloat-abi=soft \
	-g \
	-O0 \
	-Wall \
	-Wextra \
	-ffunction-sections \
	-fdata-sections \
	-MMD \
	-MP \
	$(DEFINES) \
	$(INCLUDES)


# ------------------------------------------------------------
# Flags de linker
# ------------------------------------------------------------

LDFLAGS = \
	-mcpu=$(CPU) \
	-mthumb \
	-mfloat-abi=soft \
	-T$(LD_DIR)/STM32F103XB_FLASH.ld \
	-Wl,--gc-sections \
	-Wl,-Map=$(TARGET).map


# ------------------------------------------------------------
# Archivos fuente de la aplicacion
# ------------------------------------------------------------

APP_SRCS = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/syscalls.c


# ------------------------------------------------------------
# Archivos CMSIS
# ------------------------------------------------------------

CMSIS_SRCS = \
	$(CMSIS_DIR)/system_stm32f1xx.c


# ------------------------------------------------------------
# Archivos FreeRTOS - Kernel
# ------------------------------------------------------------

FREERTOS_KERNEL_SRCS = \
	$(FREERTOS_DIR)/kernel/tasks.c \
	$(FREERTOS_DIR)/kernel/queue.c \
	$(FREERTOS_DIR)/kernel/list.c \
	$(FREERTOS_DIR)/kernel/timers.c \
	$(FREERTOS_DIR)/kernel/event_groups.c \
	$(FREERTOS_DIR)/kernel/stream_buffer.c \
	$(FREERTOS_DIR)/kernel/croutine.c


# ------------------------------------------------------------
# FreeRTOS - Port Cortex-M3
# ------------------------------------------------------------

FREERTOS_PORT_SRCS = \
	$(FREERTOS_DIR)/portable/GCC/ARM_CM3/port.c


# ------------------------------------------------------------
# FreeRTOS - Gestion de memoria
# ------------------------------------------------------------

FREERTOS_MEM_SRCS = \
	$(FREERTOS_DIR)/portable/MemMang/heap_4.c


# ------------------------------------------------------------
# Startup
# ------------------------------------------------------------

STARTUP_SRC = \
	$(CMSIS_DIR)/startup_stm32f103xb.s


# ------------------------------------------------------------
# Todos los fuentes
# ------------------------------------------------------------

SRCS = \
	$(APP_SRCS) \
	$(CMSIS_SRCS) \
	$(FREERTOS_KERNEL_SRCS) \
	$(FREERTOS_PORT_SRCS) \
	$(FREERTOS_MEM_SRCS) \
	$(STARTUP_SRC)


# ------------------------------------------------------------
# Archivos objeto
# ------------------------------------------------------------

OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)


# ------------------------------------------------------------
# Dependencias
# ------------------------------------------------------------

DEPS = $(OBJS:.o=.d)


# ============================================================
# Reglas
# ============================================================

.PHONY: all clean flash size


# ------------------------------------------------------------
# Compilacion completa
# ------------------------------------------------------------

all: $(TARGET).elf $(TARGET).bin $(TARGET).hex
	@echo
	@echo "=========================================="
	@echo " Compilacion terminada correctamente"
	@echo "=========================================="
	@echo
	$(SIZE) $(TARGET).elf


# ------------------------------------------------------------
# Link
# ------------------------------------------------------------

$(TARGET).elf: $(OBJS)
	@echo "LINK $@"
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


# ------------------------------------------------------------
# Compilar archivos C
# ------------------------------------------------------------

%.o: %.c
	@echo "CC   $<"
	$(CC) $(CFLAGS) -c $< -o $@


# ------------------------------------------------------------
# Compilar archivos Assembly
# ------------------------------------------------------------

%.o: %.s
	@echo "AS   $<"
	$(CC) $(CFLAGS) -c $< -o $@


# ------------------------------------------------------------
# ELF -> BIN
# ------------------------------------------------------------

$(TARGET).bin: $(TARGET).elf
	@echo "BIN  $@"
	$(OBJCOPY) -O binary $< $@


# ------------------------------------------------------------
# ELF -> HEX
# ------------------------------------------------------------

$(TARGET).hex: $(TARGET).elf
	@echo "HEX  $@"
	$(OBJCOPY) -O ihex $< $@


# ------------------------------------------------------------
# Mostrar tamaño
# ------------------------------------------------------------

size: $(TARGET).elf
	$(SIZE) $(TARGET).elf


# ------------------------------------------------------------
# Grabar mediante OpenOCD + ST-Link
# ------------------------------------------------------------

flash: $(TARGET).elf
	openocd \
		-f interface/stlink.cfg \
		-c "set CPUTAPID 0x2ba01477" \
		-f target/stm32f1x.cfg \
		-c "program $(TARGET).elf verify reset exit"


# ------------------------------------------------------------
# Limpiar archivos generados
# ------------------------------------------------------------

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(TARGET).elf
	rm -f $(TARGET).bin
	rm -f $(TARGET).hex
	rm -f $(TARGET).map


# ------------------------------------------------------------
# Dependencias automaticas
# ------------------------------------------------------------

-include $(DEPS)
