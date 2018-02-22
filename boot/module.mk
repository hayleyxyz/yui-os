MODULE_SRCS := \
	boot/boot.asm \
	boot/io.c \
	boot/main.c \
	boot/mini-printf.c

TARGET_BOOT := yui-os-boot.elf
MODULE_TARGET := $(TARGET_BOOT)
MODULE_CC := i386-elf-$(CC)
MODULE_LD := i386-elf-$(LD)

MODULE_LDFLAGS := --script boot/boot.lds

MODULE_ASMFLAGS := -f elf32

include make/compile.mk