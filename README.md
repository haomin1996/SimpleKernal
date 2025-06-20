# SimpleKernel

This repository contains a minimal 32‑bit kernel that prints a message to the
screen. The kernel is multiboot compliant and can be loaded using GRUB. The
instructions below describe how to build the kernel and run it in
[VirtualBox](https://www.virtualbox.org/).

## Building

You need a few build tools including **gcc**, **nasm**, **ld**, and **grub**.
Because the kernel is 32-bit, the `gcc -m32` option must be available. On
Debian/Ubuntu systems install the following packages:

```bash
sudo apt-get install build-essential nasm grub-pc-bin xorriso gcc-multilib
```
Alternatively you can use a 32-bit cross-compiler package such as
`gcc-i686-linux-gnu` so that `gcc -m32` works.

To build the kernel binary run:

```bash
make
```

This produces a `kernel.bin` file.

## Creating a bootable ISO

Create the GRUB configuration and ISO image with the following commands:

```bash
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/kernel.bin
cat > iso/boot/grub/grub.cfg <<CFG
set timeout=0
set default=0

menuentry "SimpleKernel" {
    multiboot /boot/kernel.bin
    boot
}
CFG

grub-mkrescue -o SimpleKernel.iso iso
```

The resulting `SimpleKernel.iso` can be used to boot the kernel.

## Running in VirtualBox

1. Open VirtualBox and create a new virtual machine. Select **Other/Unknown**
   as the OS type and choose **32-bit**.
2. Attach `SimpleKernel.iso` as the optical drive for the VM.
3. Start the VM. GRUB should load and the screen will display the message from
   the kernel.

The kernel now features a small preemptive round robin scheduler. Two command
prompts run in parallel: one echoes back each line and the other prints the word
count. Press **Tab** to switch the focused program. Input is delivered only to
the focused prompt while the other continues running in the background.
