TARGETS += $(MODULE_TARGET)

MODULE_CSRCS := $(filter %.c,$(MODULE_SRCS))
MODULE_COBJS := $(patsubst %.c,%.c.o,$(MODULE_CSRCS))

MODULE_ASMSRCS := $(filter %.asm,$(MODULE_SRCS))
MODULE_ASMOBJS := $(patsubst %.asm,%.asm.o,$(MODULE_ASMSRCS))

MODULE_OBJS := $(MODULE_COBJS) $(MODULE_ASMOBJS)
ALL_OBJS += $(MODULE_OBJS)

# https://www.gnu.org/software/make/manual/html_node/Target_002dspecific.html
$(MODULE_OBJS): MODULE_LD := $(MODULE_LD)
$(MODULE_OBJS): MODULE_LDFLAGS := $(MODULE_LDFLAGS)
$(MODULE_OBJS): MODULE_CC := $(MODULE_CC)
$(MODULE_OBJS): MODULE_CFLAGS := $(MODULE_CFLAGS)
$(MODULE_OBJS): MODULE_ASM := $(MODULE_ASM)
$(MODULE_OBJS): MODULE_ASMFLAGS := $(MODULE_ASMFLAGS)
$(MODULE_TARGET): MODULE_LD := $(MODULE_LD)
$(MODULE_TARGET): MODULE_LDFLAGS := $(MODULE_LDFLAGS)

$(MODULE_TARGET): $(MODULE_OBJS)
	$(MODULE_LD) $(MODULE_LDFLAGS) $^ -o $@ 

$(MODULE_COBJS): %.c.o: %.c
	$(MODULE_CC) -c $< -MMD -MP -MF $(@:%o=%d) -o $@ $(MODULE_CFLAGS)

$(MODULE_ASMOBJS): %.asm.o: %.asm
	$(MODULE_ASM) $< -o $@ $(MODULE_ASMFLAGS)

%.asm: # https://stackoverflow.com/questions/36226843/circular-dependency-dropped-with-asm-files-when-building-with-make

MODULE_COBJS :=
MODULE_CSRCS :=
MODULE_ASMSRCS :=
MODULE_ASMOBJS :=

include make/module.mk