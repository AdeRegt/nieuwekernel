rm -f boot.o
rm -f kernel.o
rm -f myos.bin
nasm -felf32 boot.asm -o boot.o
gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
#ld -n -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
ld -n -T linker.ld -o myos.bin  boot.o kernel.o

if grub-file --is-x86-multiboot myos.bin; then
echo multiboot confirmed
mkdir -p isodir/boot/grub
mkdir -p isodir/src
cp myos.bin isodir/boot/myos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
cp kernel.c isodir/src/kernel.c
cp boot.asm isodir/src/boot.asm
cp linker.ld isodir/src/linker.ld
cp build.sh isodir/src/build.sh
grub-mkrescue -o myos.iso isodir
cp myos.iso /media/sf_osdev/myiso.iso
cp -R * /media/sf_osdev/nieuwekernel
qemu-system-i386 -cdrom myos.iso
else
  echo the file is not multiboot
fi
