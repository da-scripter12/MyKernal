# MyKernal

A tiny x86 kernel written in C and Assembly.

## Features

- Boots with GRUB
- VGA text output
- Keyboard input
- Basic shell
- `help`
- `clear`
- `echo`
- `about`
- `reboot`

## Building

```bash
nasm -f elf32 src/boot.asm -o src/boot.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -c src/kernel.c -o src/kernel.o
ld -m elf_i386 -T linker.ld src/boot.o src/kernel.o -o kernel.bin
cp kernel.bin iso/boot/kernel.bin
grub-mkrescue -o kernel.iso iso
qemu-system-i386 -cdrom kernel.iso
