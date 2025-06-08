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

kernel.bin: boot.o kernel.o linker.ld
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o


clean:
	rm -f *.o kernel.bin

.PHONY: all clean
