# Makefile for Zenix with Intel HD 4600 KMS Driver
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

OUT_BINARY = bin

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin \
         -fno-stack-protector -nostartfiles -nodefaultlibs \
         -Wall -Wextra -Werror -c -I./include -g

LDFLAGS = -m elf_i386 -T linker.ld

# Kernel objects
KERNEL_OBJS = boot/boot.o \
              kernel/core/kernel.o kernel/core/monitor.o kernel/core/panic.o \
              kernel/hal/gdt.o kernel/hal/gdt_flush.o \
              kernel/hal/idt.o kernel/hal/idt_load.o \
              kernel/hal/isr.o kernel/hal/isr_stubs.o \
              kernel/hal/irq.o kernel/hal/irq_stubs.o kernel/hal/pic.o \
              kernel/mm/pmm.o kernel/mm/heap.o kernel/mm/paging.o kernel/mm/paging_asm.o \
              kernel/fs/vfs.o kernel/fs/vfs_complete.o kernel/fs/initrd.o \
              kernel/proc/process.o kernel/proc/scheduler.o kernel/proc/switch.o \
              kernel/drivers/timer/pit.o kernel/drivers/keyboard/keyboard.o \
              kernel/shell/shell.o \
              kernel/syscall/syscall.o kernel/syscall/syscall_stub.o kernel/syscall/handlers.o \
              kernel/usermode/usermode.o \
              kernel/drivers/vga/vga.o \
              kernel/drivers/mouse/mouse.o \
              kernel/drivers/video/gop_fb.o \
              kernel/gui/gui.o kernel/gui/color.o \
              kernel/apps/calculator/calculator_app.o \
              kernel/apps/app_manager.o \
              kernel/drivers/gpu/gpu_detect.o \
              kernel/drivers/gpu/intel/i915_hd4600.o \
              lib/libc/string.o

.PHONY: all clean run run-debug iso test-hw

all: $(OUT_BINARY)/zenix.bin

$(OUT_BINARY)/zenix.bin: $(KERNEL_OBJS)
	@mkdir -p $(OUT_BINARY)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "==================================="
	@echo "Kernel built: $@"
	@echo "Size: $$(du -h $@ | cut -f1)"
	@echo "==================================="

# Assembly files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# C files
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Special targets for new directories
kernel/drivers/gpu/intel/%.o: kernel/drivers/gpu/intel/%.c
	@mkdir -p kernel/drivers/gpu/intel
	$(CC) $(CFLAGS) $< -o $@

kernel/apps/calculator/%.o: kernel/apps/calculator/%.c
	@mkdir -p kernel/apps/calculator
	$(CC) $(CFLAGS) $< -o $@

clean:
	@echo "Cleaning build files..."
	@rm -f boot/*.o 
	@rm -f kernel/*/*.o kernel/*/*/*.o kernel/*/*/*/*.o 
	@rm -f lib/*/*.o
	@rm -f $(OUT_BINARY)/zenix.bin
	@rm -f zenix.iso
	@rm -rf isofiles
	@echo "Clean complete"

# Run in QEMU (standard VGA)
run: $(OUT_BINARY)/zenix.bin
	@echo "Starting QEMU with VESA framebuffer..."
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin -m 256M

# Run with UEFI (GOP framebuffer support)
run-uefi: iso
	@echo "Starting QEMU with UEFI + GOP..."
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom zenix.iso -m 256M -vga std

# Run with debugging
run-debug: $(OUT_BINARY)/zenix.bin
	@echo "Starting QEMU with GDB stub..."
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin -m 256M -s -S &
	@echo "Connect GDB with: target remote localhost:1234"

# Create bootable ISO
iso: $(OUT_BINARY)/zenix.bin
	@echo "Creating bootable ISO..."
	@mkdir -p isofiles/boot/grub
	@cp $(OUT_BINARY)/zenix.bin isofiles/boot/
	@echo 'set timeout=3' > isofiles/boot/grub/grub.cfg
	@echo 'set default=0' >> isofiles/boot/grub/grub.cfg
	@echo '' >> isofiles/boot/grub/grub.cfg
	@echo 'menuentry "Zenix OS - HD 4600 KMS" {' >> isofiles/boot/grub/grub.cfg
	@echo '    multiboot /boot/zenix.bin' >> isofiles/boot/grub/grub.cfg
	@echo '    boot' >> isofiles/boot/grub/grub.cfg
	@echo '}' >> isofiles/boot/grub/grub.cfg
	@grub-mkrescue -o zenix.iso isofiles/ 2>/dev/null || \
		(echo "Error: grub-mkrescue not found. Install grub-pc-bin or grub2-tools" && exit 1)
	@echo "==================================="
	@echo "ISO created: zenix.iso"
	@echo "Test: qemu-system-i386 -cdrom zenix.iso -m 256M"
	@echo "==================================="

# Test on real hardware (requires USB)
test-hw: iso
	@echo "==================================="
	@echo "To test on real hardware:"
	@echo "1. Insert USB drive"
	@echo "2. Find device: lsblk"
	@echo "3. Write ISO: sudo dd if=zenix.iso of=/dev/sdX bs=4M status=progress"
	@echo "   (Replace sdX with your USB device!)"
	@echo "4. Sync: sync"
	@echo "5. Boot from USB"
	@echo "==================================="

# Quick test in QEMU with graphics
test: $(OUT_BINARY)/zenix.bin
	@echo "Quick graphics test..."
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin -m 256M -display gtk

help:
	@echo "Zenix Build System"
	@echo "=================="
	@echo "Targets:"
	@echo "  make          - Build kernel"
	@echo "  make run      - Run in QEMU"
	@echo "  make run-debug- Run with GDB"
	@echo "  make iso      - Create bootable ISO"
	@echo "  make test     - Quick graphics test"
	@echo "  make test-hw  - Instructions for hardware testing"
	@echo "  make clean    - Clean build files"