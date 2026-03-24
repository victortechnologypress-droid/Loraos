# ============================================================
#  LoraOS v1 MVP - Master Makefile
#  Target: x86 (32-bit), optimizat pentru Acer Aspire One
# ============================================================

CC      := i686-linux-gnu-gcc
AS      := nasm
LD      := i686-linux-gnu-ld
GRUB    := grub-mkrescue

CFLAGS  := -m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -I./kernel/include
ASFLAGS := -f elf32
LDFLAGS := -T linker.ld -nostdlib

KERNEL_DIR  := kernel
DRIVERS_DIR := drivers
FS_DIR      := fs
UI_DIR      := ui
BUILD_DIR   := build
ISO_DIR     := iso

ASM_SOURCES := $(KERNEL_DIR)/boot/boot.asm
C_SOURCES   := \
    $(KERNEL_DIR)/kernel.c          \
    $(KERNEL_DIR)/klib.c            \
    $(KERNEL_DIR)/pmm.c             \
    $(KERNEL_DIR)/scheduler.c       \
    $(DRIVERS_DIR)/vbe.c            \
    $(DRIVERS_DIR)/keyboard.c       \
    $(DRIVERS_DIR)/ps2mouse.c       \
    $(FS_DIR)/fat32.c               \
    $(UI_DIR)/desktop.c             \
    $(UI_DIR)/taskbar.c             \
    $(UI_DIR)/window.c

OBJECTS := $(patsubst %.asm, $(BUILD_DIR)/%.o, $(notdir $(ASM_SOURCES))) \
           $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(C_SOURCES)))

.PHONY: all clean iso

all: $(BUILD_DIR)/loraos.kernel

$(BUILD_DIR)/loraos.kernel: $(OBJECTS) linker.ld
	@echo "[LD] Linkez kernel-ul (boot.o este forțat primul)..."
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/boot.o $(filter-out $(BUILD_DIR)/boot.o, $(OBJECTS))

iso: $(BUILD_DIR)/loraos.kernel grub.cfg
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(BUILD_DIR)/loraos.kernel $(ISO_DIR)/boot/loraos.bin
	@cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB) -o loraos.iso $(ISO_DIR)

$(BUILD_DIR)/boot.o: $(KERNEL_DIR)/boot/boot.asm
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FS_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(UI_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) loraos.iso
