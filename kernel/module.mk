MODULE_SRCS := \
	kernel/main.c \
	kernel/console.c \
	kernel/printf.c

TARGET_KERNEL := yui-os-kernel.elf
MODULE_TARGET := $(TARGET_KERNEL)

MODULE_LDFLAGS := --script kernel/kernel.lds

include make/compile.mk