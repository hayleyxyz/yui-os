CROSS = i386-elf-
CFLAGS = -fno-asynchronous-unwind-tables -ffreestanding -nostdlib -mno-red-zone -fno-stack-protector -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -maccumulate-outgoing-args -Wall -Wextra -I./include -fno-exceptions -g -DDebug

TARGET_x86 = yui-os-boot-x86
TARGET_x86_64 = yui-os-kernel-x86-64
TARGET = yui-os

$(TARGET).iso: $(TARGET_x86).elf $(TARGET_x86_64).elf
	cp $^ iso/boot
	grub-mkrescue -o '$@' iso

$(TARGET_x86).elf: $(TARGET_x86).debug.elf
	objcopy --strip-debug --remove-section=.comment $^ $@

$(TARGET_x86_64).elf: $(TARGET_x86_64).debug.elf
	objcopy --strip-debug --remove-section=.comment $^ $@

$(TARGET_x86_64).debug.elf: kernel/main.o
	ld --script elf64.lds $^ -o $@

$(TARGET_x86).debug.elf: $(TARGET_x86_64).elf arch/x86/boot/boot.o arch/x86/boot/main.o arch/x86/boot/io.o arch/x86/boot/mini-printf.o
	$(CROSS)ld --script elf32.lds $(filter %.o,$^) -o $@

arch/x86/boot/main.o: arch/x86/boot/main.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/io.o: arch/x86/boot/io.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/mini-printf.o: arch/x86/boot/mini-printf.c	
	$(CROSS)gcc -c $(CFLAGS) $^ -o $@

arch/x86/boot/boot.o: arch/x86/boot/boot.asm
	nasm -f elf32 $^ -o $@

kernel/main.o: kernel/main.c
	gcc -c $(CFLAGS) -fPIC $^ -o $@

.PHONY: clean

clean:
	rm -f arch/x86/boot/*.o kernel/*.o *.elf $(TARGET).iso iso/boot/*.elf
