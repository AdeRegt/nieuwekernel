rm -f boot.o
rm -f kernel.o
rm -f myos.bin
nasm -felf32 src/boot/boot.asm -o build/boot.o
gcc -c src/kernel/kernel.c -o build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/executables/elf.c -o build/elf.o  -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/multitasking.c -o build/multitasking.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/string.c -o build/string.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/interrupts.c -o build/interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/serialports.c -o build/serialports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/i386/ports.c -o build/ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/dev/ps2/mouse.c -o build/ps2mice.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/dev/ps2/keyboard.c -o build/ps2keyboard.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/video.c -o build/video.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/dev/ata/atapi.c -o build/atapi.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/dev/ata/ata.c -o build/ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
gcc -c src/kernel/dev/ata/atadetection.c -o build/atadetection.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I include
as src/kernel/i386/switch.a -o build/switch.o
ld -n -T src/linker.ld -o myos.bin  build/boot.o build/kernel.o build/atadetection.o build/atapi.o build/ata.o build/ps2keyboard.o build/ps2mice.o build/video.o build/serialports.o build/interrupts.o build/switch.o build/elf.o build/string.o build/ports.o build/multitasking.o

if grub-file --is-x86-multiboot myos.bin; then
	echo multiboot confirmed
	mkdir -p isodir/boot/grub
	mkdir -p isodir/src
	cp myos.bin isodir/boot/myos.bin
	cp src/grub.cfg isodir/boot/grub/grub.cfg
	cp src/kernel.c isodir/src/kernel.c
	cp src/boot.asm isodir/src/boot.asm
	cp src/linker.ld isodir/src/linker.ld
	cp src/build.sh isodir/src/build.sh
	grub-mkrescue -o myos.iso isodir
#	qemu-system-i386 -cdrom myos.iso
#	cp -r *.* /media/sf_nieuwekernel
	echo "OKE"
else
	echo the file is not multiboot
fi
