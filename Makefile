ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
OUT_BINARY = bin
OUT_BUILD = build

ASMFLAGS = -f elf32

CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -O2 -Wall -Wextra \
         -I./include \
         -isystem /home/khadr/cross/i686-elf/include

LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJS = boot/boot.o \
              kernel/core/kernel.o \
              kernel/core/monitor.o \
              kernel/hal/gdt.o \
              kernel/hal/gdt_flush.o \
              kernel/hal/idt.o \
              kernel/hal/idt_load.o \
              kernel/hal/isr.o \
              kernel/hal/isr_stubs.o \
              kernel/hal/irq.o \
              kernel/hal/irq_stubs.o \
              kernel/hal/pic.o \
              kernel/mm/pmm.o \
              kernel/mm/heap.o \
              kernel/proc/process.o \
              kernel/proc/scheduler.o \
              kernel/proc/switch.o \
              kernel/drivers/timer/pit.o \
              kernel/drivers/keyboard/keyboard.o \
			  kernel/mm/paging.o \
              kernel/shell/shell.o \
              lib/libc/string.o

all: zenix.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

zenix.bin: $(KERNEL_OBJS)
	mkdir -p $(OUT_BINARY)
	$(LD) $(LDFLAGS) -o $(OUT_BINARY)/$@ $^

zenix.iso: zenix.bin
	mkdir -p $(OUT_BUILD)
	mkdir -p isodir/boot/grub
	cp $(OUT_BINARY)/zenix.bin isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(OUT_BUILD)/zenix.iso isodir

run: zenix.bin
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin -d cpu_reset

debug: zenix.bin
	qemu-system-i386 -kernel $(OUT_BINARY)/zenix.bin -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(OUT_BINARY)/zenix.bin"

clean:
	rm -f boot/*.o kernel/core/*.o kernel/hal/*.o kernel/mm/*.o \
	      kernel/proc/*.o kernel/drivers/timer/*.o kernel/drivers/keyboard/*.o \
	      kernel/shell/*.o lib/libc/*.o $(OUT_BINARY)/zenix.bin $(OUT_BUILD)/zenix.iso
	rm -rf isodir

.PHONY: all run debug clean