.PHONY: all iso run clean

CC = gcc
AS = nasm
LD = ld
QEMU = qemu-system-i386

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector \
         -fno-builtin -Wall -Wextra -Isrc
LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES = src/kernel.c src/commands.c src/keyboard.c src/video.c
C_OBJECTS = $(C_SOURCES:.c=.o)

all: kernel.bin

boot.o: boot.asm
	$(AS) -f elf32 $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: boot.o $(C_OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ boot.o $(C_OBJECTS)

iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	grub-mkrescue -o kernel.iso iso

run: iso
	$(QEMU) -cdrom kernel.iso

clean:
	rm -f boot.o $(C_OBJECTS) kernel.bin kernel.iso
	rm -f iso/boot/kernel.bin