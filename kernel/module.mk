MODULE_SRCS := \
	kernel/main.c \
	kernel/console.c \
	kernel/libc/printf.c \
	kernel/libc/memset.c \
	kernel/libc/strlen.c \
	kernel/memory.c \
	kernel/interrupts.asm \
	kernel/idt.c

TARGET_KERNEL := yui-os-kernel.elf
MODULE_TARGET := $(TARGET_KERNEL)

MODULE_LDFLAGS := --script kernel/kernel.lds

MODULE_ASMFLAGS := -f elf64

include make/compile.mk