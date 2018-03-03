CC := gcc

CFLAGS := -fno-asynchronous-unwind-tables -ffreestanding \
	-nostdlib -mno-red-zone -fno-stack-protector -mno-sse -mno-mmx \
	-mno-sse2 -mno-3dnow -mno-avx -maccumulate-outgoing-args -Wall \
	-Wextra -I./include -I./include/libc -fno-exceptions -g -DDebug -fPIC

LD := ld
LDFLAGS :=

ASM := nasm
ASMFLAGS := 

TARGETS :=
ALL_OBJS :=

include make/module.mk