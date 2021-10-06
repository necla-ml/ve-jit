COMPILE_TYPE:=$(word 1,$(shell $(CC) --version 2>&1))
$(if $(COMPILE_TYPE),,$(error could not determine C compiler type for CC=$(CC))) # VE(ncc) vs x86(gcc or ...)
ifneq ($(COMPILE_TYPE),ncc)
include Makefile.x86
else
include Makefile.ve
endif
#
