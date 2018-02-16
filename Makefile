CROSS = i386-elf-
CFLAGS = -fPIC -fno-asynchronous-unwind-tables -ffreestanding -nostdlib -mno-red-zone -fno-stack-protector -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -maccumulate-outgoing-args -Wall -Wextra -I./include -fno-exceptions -g -DDebug

TARGET_BOOT = yui-os-boot
TARGET_KERNEL = yui-os-kernel
TARGET = yui-os

$(TARGET).iso: $(TARGET).bin
	cp $^ iso/boot
	grub-mkrescue -o '$@' iso

$(TARGET).bin: $(TARGET_BOOT).bin $(TARGET_KERNEL).bin
	cp $(TARGET_BOOT).bin $@
	truncate --size=4096 --no-create $@
	cat $(TARGET_KERNEL).bin >> $@


$(TARGET_BOOT).bin: $(TARGET_BOOT).elf
	objcopy -O binary $^ $@

$(TARGET_KERNEL).bin: $(TARGET_KERNEL).elf
	objcopy -O binary $^ $@

$(TARGET_KERNEL).elf: kernel/main.o
	ld --script elf64.lds $^ -o $@

$(TARGET_BOOT).elf: arch/x86/boot/boot.o arch/x86/boot/main.o arch/x86/boot/io.o arch/x86/boot/mini-printf.o
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
	rm -f arch/x86/boot/*.o kernel/*.o *.elf $(TARGET).iso iso/boot/*.bin *.bin
