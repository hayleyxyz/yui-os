OS_TARGET := 'yui-os.bin'

$(OS_TARGET)::

include make/base.mk
include boot/module.mk
include kernel/module.mk

$(OS_TARGET):: $(TARGETS)
	objcopy -O binary $(TARGET_KERNEL) $(OS_TARGET)

clean:
	rm -rf $(ALL_OBJS) $(TARGETS) $(OS_TARGET) $(TARGET_KERNEL:.elf=)* $(ALL_OBJS:.o=.d)