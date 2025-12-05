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
              kernel/drivers/timer/pit.o

all: kinix.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

kinix.bin: $(KERNEL_OBJS)
	mkdir -p $(OUT_BINARY)
	$(LD) $(LDFLAGS) -o $(OUT_BINARY)/$@ $^

kinix.iso: kinix.bin
	mkdir -p $(OUT_BUILD)
	mkdir -p isodir/boot/grub
	cp $(OUT_BINARY)/kinix.bin isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(OUT_BUILD)/kinix.iso isodir

run: kinix.bin
	qemu-system-i386 -kernel $(OUT_BINARY)/kinix.bin

debug: kinix.bin
	qemu-system-i386 -kernel kinix.bin -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kinix.bin"

clean:
	rm -f boot/*.o kernel/core/*.o kernel/hal/*.o kernel/drivers/timer/*.o kinix.bin kinix.iso
	rm -rf isodir

.PHONY: all run debug clean