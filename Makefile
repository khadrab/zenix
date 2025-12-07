AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

OUT_BINARY = bin

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin \
         -fno-stack-protector -nostartfiles -nodefaultlibs \
         -Wall -Wextra -Werror -c -I./include -g
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJS = boot/boot.o \
              kernel/core/kernel.o kernel/core/monitor.o \
              kernel/hal/gdt.o kernel/hal/gdt_flush.o \
              kernel/hal/idt.o kernel/hal/idt_load.o \
              kernel/hal/isr.o kernel/hal/isr_stubs.o \
              kernel/hal/irq.o kernel/hal/irq_stubs.o kernel/hal/pic.o \
              kernel/mm/pmm.o kernel/mm/heap.o \
              kernel/fs/vfs.o kernel/fs/initrd.o \
              kernel/proc/process.o kernel/proc/scheduler.o kernel/proc/switch.o \
              kernel/drivers/timer/pit.o kernel/drivers/keyboard/keyboard.o \
              kernel/shell/shell.o \
              kernel/syscall/syscall.o kernel/syscall/syscall_stub.o kernel/syscall/handlers.o \
              kernel/usermode/usermode.o \
              lib/libc/string.o

.PHONY: all clean run

all: $(OUT_BINARY)/zenix.bin

$(OUT_BINARY)/zenix.bin: $(KERNEL_OBJS)
	@mkdir -p $(OUT_BINARY)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $@"

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

run: $(OUT_BINARY)/zenix.bin
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin

clean:
	@rm -f boot/*.o kernel/*/*.o kernel/*/*/*.o lib/*/*.o
	@rm -f $(OUT_BINARY)/*
	@echo "Clean"
