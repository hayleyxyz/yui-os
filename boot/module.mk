MODULE_SRCS := \
	boot/boot.asm \
	boot/console.c \
	boot/main.c

TARGET_BOOT := yui-os-boot.elf
MODULE_TARGET := $(TARGET_BOOT)
MODULE_CC := i686-elf-$(CC)
MODULE_LD := i686-elf-$(LD)

MODULE_LDFLAGS := --script boot/boot.lds

MODULE_ASMFLAGS := -f elf32

include make/compile.mk