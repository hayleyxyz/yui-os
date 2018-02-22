MODULE_SRCS := \
	kernel/main.c

TARGET_KERNEL := yui-os-kernel.elf
MODULE_TARGET := $(TARGET_KERNEL)

MODULE_LDFLAGS := --script kernel/kernel.lds

include make/compile.mk