CROSS = i686-elf-
CFLAGS = -ffreestanding -nostdlib -mno-red-zone -fno-stack-protector -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -maccumulate-outgoing-args -Wall -Wextra -I./include -fno-exceptions -g -DDebug

yui-os-x86.elf: arch/x86/boot/boot.o arch/x86/boot/main.o arch/x86/boot/io.o arch/x86/boot/mini-printf.o
	$(CROSS)ld --script elf32.lds $^ -o $@

arch/x86/boot/main.o: arch/x86/boot/main.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/io.o: arch/x86/boot/io.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/mini-printf.o: arch/x86/boot/mini-printf.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/boot.o: arch/x86/boot/boot.asm
	nasm -f elf32 $^ -o $@

.PHONY: clean

clean:
	rm -f arch/x86/boot/*.o yui-os.elf