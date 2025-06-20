# Simple kernel Makefile

AS=as
CC=gcc
LD=ld
CFLAGS=-m32 -ffreestanding -nostdlib -fno-exceptions -fno-stack-protector
LDFLAGS=-melf_i386 -T linker.ld

all: kernel.bin

boot.o: boot.s
	$(AS) --32 $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

interrupts.o: interrupts.s
	$(AS) --32 $< -o $@

kernel.bin: boot.o kernel.o interrupts.o linker.ld
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o interrupts.o

clean:
	rm -f *.o kernel.bin

.PHONY: all clean iso

iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	printf 'set timeout=0\nset default=0\n\nmenuentry "SimpleKernel" {\nmultiboot /boot/kernel.bin\n    boot\n}\n' > iso/boot/grub/grub.cfg
