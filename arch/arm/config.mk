# This selects which instruction set is used.
head-y := arch/arm/cpu/$(CPU)/start.o

arch-$(CONFIG_CPU_ARM720T)  =-march=armv4
arch-$(CONFIG_CPU_ARM920T)  =-march=armv4
arch-$(CONFIG_CPU_ARM926EJS)=-march=armv5te
arch-$(CONFIG_CPU_ARM946ES) =-march=armv4
arch-$(CONFIG_CPU_SA1100)   =-march=armv4
arch-$(CONFIG_CPU_PXA)      =
arch-$(CONFIG_CPU_ARM1136)  =-march=armv5
arch-$(CONFIG_CPU_ARM1176)  =-march=armv5t
arch-$(CONFIG_CPU_V7)       =$(call cc-option, -march=armv7-a, \
					$(call cc-option, -march))
arch-$(CONFIG_CPU_V7M)      =-march=armv7-m -mthumb
arch-$(CONFIG_ARM64)        =-march=armv8-a

# Evaluate arch cc-option calls now
arch-y := $(arch-y)

# This selects how we optimise for the processor.
tune-$(CONFIG_CPU_ARM720T)  =-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM920T)  =
tune-$(CONFIG_CPU_ARM926EJS)=
tune-$(CONFIG_CPU_ARM946ES) =
tune-$(CONFIG_CPU_SA1100)   =-mtune=strongarm1100
tune-$(CONFIG_CPU_PXA)      =-mcpu=xscale
tune-$(CONFIG_CPU_ARM1136)  =
tune-$(CONFIG_CPU_ARM1176)  =
tune-$(CONFIG_CPU_V7)       =
tune-$(CONFIG_ARM64)        =

# Evaluate tune cc-option calls now
tune-y := $(tune-y)

PLATFORM_CPPFLAGS += $(arch-y) $(tune-y)
LDFLAGS_FINAL += -Wl,--gc-sections
PLATFORM_CPPFLAGS += -fmessage-length=0 -fno-exceptions \
		     -ffunction-sections -fdata-sections -fno-common -ffixed-r9 \
			 -fno-delete-null-pointer-checks -fomit-frame-pointer

CC_SYMBOLS += -D__ARM__
ifeq (CONFIG_CPU_HAS_FPU,y)
CC_SYMBOLS += -D__FPU_PRESENT
PLATFORM_CPPFLAGS += -mfloat-abi=hard
endif

INCLUDE_PATHS += -I$(srctree)/arch/$(ARCH)/include
