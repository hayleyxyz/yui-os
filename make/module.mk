# Initial variables for module.mk files

MODULE_CC := $(CC)
MODULE_LD := $(LD)
MODULE_ASM := $(ASM)

MODULE_CFLAGS := $(CFLAGS)
MODULE_LDFLAGS := $(LDFLAGS)
MODULE_ASMFLAGS := $(ASMFLAGS)

# These *will* be changed between modules
MODULE_SRCS :=
MODULE_TARGET :=
MODULE_LDSCRIPT :=
