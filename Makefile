# ============================================================
#  LoraOS v1 MVP - Master Makefile
#  Target: x86 (32-bit), bootabil cu Rufus via GRUB
# ============================================================

# --- Toolchain ---
CC      := i686-linux-gnu-gcc
AS      := nasm
LD      := i686-linux-gnu-ld
GRUB    := grub-mkrescue

# --- Flags ---
CFLAGS  := -m32 -ffreestanding -O2 -Wall -Wextra \
           -fno-stack-protector  \
           -I./kernel/include
ASFLAGS := -f elf32
LDFLAGS := -T linker.ld -nostdlib

# --- Directoare ---
KERNEL_DIR  := kernel
DRIVERS_DIR := drivers
FS_DIR      := fs
UI_DIR      := ui
BUILD_DIR   := build
ISO_DIR     := iso

# --- Surse ---
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

# --- Obiecte compilate ---
OBJECTS := \
    $(BUILD_DIR)/boot.o \
    $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(C_SOURCES)))

# ============================================================
#  Reguli principale
# ============================================================

.PHONY: all clean iso run

all: $(BUILD_DIR)/loraos.kernel

# --- Link final: kernel ELF ---
$(BUILD_DIR)/loraos.kernel: $(OBJECTS) linker.ld
	@echo "[LD]  Linkez kernel-ul..."
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "[OK]  loraos.kernel generat!"

# --- Compilare assembly (boot entry) ---
$(BUILD_DIR)/boot.o: $(KERNEL_DIR)/boot/boot.asm
	@mkdir -p $(BUILD_DIR)
	@echo "[AS]  $<"
	$(AS) $(ASFLAGS) $< -o $@

# --- Compilare C generica ---
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "[CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "[CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FS_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "[CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(UI_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "[CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

# --- Genereaza ISO bootabil (pentru Rufus) ---
iso: all
	@echo "[ISO] Construiesc structura ISO..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(BUILD_DIR)/loraos.kernel $(ISO_DIR)/boot/
	@cp grub.cfg $(ISO_DIR)/boot/grub/
	$(GRUB) -o loraos.iso $(ISO_DIR)
	@echo "[OK]  loraos.iso gata! Scrie-l cu Rufus pe USB."

# --- Ruleaza in QEMU (pentru debug rapid) ---
run: iso
	@echo "[QEMU] Pornesc emulatorul..."
	qemu-system-i386 -cdrom loraos.iso -m 256M -vga std

# --- Curatenie ---
clean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) loraos.iso
	@echo "[CLEAN] Gata."
