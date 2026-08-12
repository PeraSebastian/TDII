SRC_DIR = src
BUILD_DIR = build

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
BIN = arm-none-eabi-objcopy

CFLAGS = -mthumb -mcpu=cortex-m3 -O0 -g

all: app.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

crt.o: $(BUILD_DIR) $(SRC_DIR)/crt.s
	$(AS) -o $(BUILD_DIR)/crt.o $(SRC_DIR)/crt.s

main.o: $(BUILD_DIR) $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/main.o $(SRC_DIR)/main.c

app.elf: crt.o main.o $(SRC_DIR)/linker.ld
	$(LD) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/app.elf \
	$(BUILD_DIR)/crt.o $(BUILD_DIR)/main.o

app.bin: app.elf
	$(BIN) -O binary $(BUILD_DIR)/app.elf $(BUILD_DIR)/app.bin

clean:
	rm -rf $(BUILD_DIR)

flash:
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	-c "program $(BUILD_DIR)/app.bin 0x08000000 verify reset exit"
