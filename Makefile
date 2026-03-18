# Project name and build directory
TARGET = firmware
BUILD_DIR = build

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# MCU Flags (STM32F411 - Cortex M4 with FPU)
# -mfloat-abi=hard -mfpu=fpv4-sp-d16: Enables the hardware floating point unit
MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Compilation Flags
# -nostartfiles: Tells GCC not to use standard startup files (using our setup.c)
# -ffreestanding: Informs the compiler that the standard library may not be present
CFLAGS  = $(MCU_FLAGS) -ffreestanding -nostartfiles -Wall -Wextra -O0 -g3 -I. -Icore -Iplatform

# Linker Flags
# -T: Specifies the path to our custom linker script
LDFLAGS = -T core/memory.ld -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

# Source files
SRCS = core/setup.c \
	   core/system_clock.c \
	   app/main.c 

# Object files (placed inside build/ directory)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

# Rules
.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

# Link the object files into the ELF executable
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) $@

# Compile each .c into a .o inside build/
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create a raw binary from the ELF for flashing
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Remove build directory
clean: 
	rm -rf $(BUILD_DIR)

# Quick flash using st-flash utility
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000


