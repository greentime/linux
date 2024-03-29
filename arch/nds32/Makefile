LDFLAGS_vmlinux	:= --no-undefined -X
OBJCOPYFLAGS	:= -O binary -R .note -R .note.gnu.build-id -R .comment -S

KBUILD_DEFCONFIG := ae3xx_defconfig

comma = ,

KBUILD_CFLAGS	+=-mno-sched-prolog-epilog -mcmodel=large

KBUILD_CFLAGS	+=$(arch-y) $(tune-y)
KBUILD_AFLAGS	+=$(arch-y) $(tune-y)

#Default value
head-y		 := arch/nds32/kernel/head.o
textaddr-y	 := 0xc000c000

TEXTADDR := $(textaddr-y)

export	TEXTADDR


# If we have a machine-specific directory, then include it in the build.
core-y				+= arch/nds32/kernel/ arch/nds32/mm/
libs-y				+= arch/nds32/lib/
LIBGCC_PATH     		:= \
  $(shell $(CC) $(KBUILD_CFLAGS) $(KCFLAGS) -print-libgcc-file-name)
libs-y				+= $(LIBGCC_PATH)

ifneq '$(CONFIG_NDS32_BUILTIN_DTB)' '""'
BUILTIN_DTB := y
else
BUILTIN_DTB := n
endif

boot := arch/nds32/boot
core-$(BUILTIN_DTB) += $(boot)/dts/

.PHONY: FORCE

Image: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $(boot)/$@


PHONY += vdso_install
vdso_install:
	$(Q)$(MAKE) $(build)=arch/nds32/kernel/vdso $@

prepare: vdso_prepare
vdso_prepare: prepare0
	$(Q)$(MAKE) $(build)=arch/nds32/kernel/vdso include/generated/vdso-offsets.h

CLEAN_FILES += include/asm-nds32/constants.h*

# We use MRPROPER_FILES and CLEAN_FILES now
archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

define archhelp
  echo  '  Image         - kernel image (arch/$(ARCH)/boot/Image)'
endef
