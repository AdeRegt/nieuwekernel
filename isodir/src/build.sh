rm -f boot.o
rm -f kernel.o
rm -f myos.bin
nasm -felf32 src/boot.asm -o build/boot.o
gcc -c src/kernel.c -o build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/elf.c -o build/elf.o  -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
as src/switch.a -o build/switch.o
ld -n -T src/linker.ld -o myos.bin  build/boot.o build/kernel.o build/switch.o build/elf.o

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
#	qemu-system-i386 -cdrom myos.iso
#	cp -r *.* /media/sf_nieuwekernel
	echo "OKE"
else
	echo the file is not multiboot
fi
