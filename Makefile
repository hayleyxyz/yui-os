OS_TARGET := 'yui-os.bin'

$(OS_TARGET)::

include make/base.mk
include boot/module.mk
include kernel/module.mk

$(OS_TARGET):: $(TARGETS)
	objcopy -O binary $(TARGET_BOOT) $(OS_TARGET)
	truncate --size=4096 --no-create $(OS_TARGET)

	objcopy -O binary $(TARGET_KERNEL) $(TARGET_KERNEL:.elf=.bin)
	cat $(TARGET_KERNEL:.elf=.bin) >> $@


clean:
	rm -rf $(ALL_OBJS) $(TARGETS) $(OS_TARGET) $(TARGET_KERNEL:.elf=)*