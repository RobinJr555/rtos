DIR_ROOT = $(shell pwd)
VAR_HOST = $(shell uname | tr '[:upper:]' '[:lower:]' | cut -b1-6)

######################################################################
MAKEFLAGS += -rR --no-print-directory --include-dir=$(DIR_ROOT)

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

export quiet Q KBUILD_VERBOSE

######################################################################
srctree := $(DIR_ROOT)
objtree := $(DIR_ROOT)
export srctree objtree

ifneq (.config,$(wildcard .config))
$(warning ".config does not exists, run 'make menuconfig' first.")
else
include $(srctree)/.config
endif

ARCH	:= $(CONFIG_SYS_ARCH:"%"=%)
CPU		:= $(CONFIG_SYS_CPU:"%"=%)
SOC		:= $(CONFIG_SYS_SOC:"%"=%)
CPUDIR		= arch/$(ARCH)/cpu$(if $(CPU),/$(CPU),)

BOARD	:= $(CONFIG_SYS_BOARD:"%"=%)
VENDOR	:= $(CONFIG_SYS_VENDOR:"%"=%)
BOARDDIR	= board/$(VENDOR)/$(BOARD)

KERNEL		:= $(CONFIG_RTOS_KERNEL:"%"=%)
KERNELDIR	= kernel/$(KERNEL)

APPDIR		= app/$(CONFIG_APP_DIR:"%"=%)
APP			:= $(notdir $(APPDIR))

export ARCH CPU BOARD VENDOR SOC CPUDIR BOARDDIR KERNEL KERNELDIR

######################################################################
CROSS_COMPILE := $(CONFIG_CROSS_COMPILE:"%"=%)
ifeq ($(CROSS_COMPILE),)
$(warning "Please set cross compiler[General setup -> Cross compiler prefix]")
endif
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP

######################################################################
PLATFORM_CPPFLAGS :=
PLATFORM_LDFLAGS  :=
CC_SYMBOLS        :=

sinclude arch/$(ARCH)/config.mk
sinclude board/$(VENDOR)/config.mk
sinclude $(KERNELDIR)/config.mk

CC_FLAGS := -Wall -Wextra -Wno-unused-parameter \
		-Wno-missing-field-initializers \
		-fno-builtin

ifdef CONFIG_CC_OPTIMIZE_FOR_SIZE
CC_FLAGS += -Os
else
CC_FLAGS += -O2
endif

ifdef CONFIG_CC_DEBUG
CC_FLAGS += -g
endif

ifdef CONFIG_CODE_DEBUG
LDSCRIPT := $(srctree)/$(BOARDDIR)/debug_rtos.ld
else
LDSCRIPT := $(srctree)/$(BOARDDIR)/rtos.ld
endif

export CC_FLAGS LD_FLAGS
export PLATFORM_CPPFLAGS PLATFORM_LDFLAGS CC_SYMBOLS

# Add GCC lib
ifeq ($(CONFIG_USE_PRIVATE_LIBGCC),y)
PLATFORM_LIBGCC = arch/$(ARCH)/lib/lib.a
else
PLATFORM_LIBGCC := -lgcc
endif
PLATFORM_LIBS += $(PLATFORM_LIBGCC)
PLATFORM_LIBS += -lm -lc -lnosys -lstdc++ -lsupc++
export PLATFORM_LIBS

LDFLAGS_rtos += $(LDFLAGS_FINAL)

######################################################################

INCLUDE_PATHS += -I$(srctree)/include

export INCLUDE_PATHS
######################################################################
include scripts/Kbuild.include

libs-y += arch/$(ARCH)/
libs-y += $(BOARDDIR)/
libs-y += library/common/
libs-y += $(KERNELDIR)/
libs-y += $(APPDIR)/

rtos-dirs := $(patsubst %/,%,$(filter %/, $(libs-y)))

libs-y := $(patsubst %/, %/built-in.o, $(libs-y))

rtos-init := $(head-y)
rtos-main := $(libs-y)


ALL-y += rtos.elf rtos.bin rtos.text

######################################################################
.PHONY: all clean

all: $(ALL-y)

######################################################################
BIN_config := mconf
BIN_menuconfig := mconf
BIN_silentconfig := conf
OPT_config :=
OPT_menuconfig :=
OPT_silentconfig := --silentoldconfig

.PHONY += config menuconfig silentconfig
config menuconfig silentconfig:
	$(Q)if [ ! -e scripts/$(VAR_HOST)/$(BIN_$@) ]; then   \
		echo;                                             \
		echo "ERROR: host $(VAR_HOST) not supported yet"; \
		exit 1;                                           \
	fi
	$(Q)scripts/$(VAR_HOST)/$(BIN_$@) $(OPT_$@) Kconfig

.PHONY += prepare
prepare:
	$(Q)chmod u+x scripts/gen_main_menu.sh
	$(Q)chmod u+x scripts/gen_sub_menu.py
	$(Q)scripts/gen_main_menu.sh > Kconfig
	$(Q)scripts/gen_sub_menu.py >& /dev/nell

######################################################################
# Rule to link rtos.elf
# FIXME: cann't find _start
#quiet_cmd_rtos__ = LD      $@
#      cmd_rtos__ = $(LD) $(LD_FLAGS) $(LDFLAGS_rtos) -o $@  \
#      -T $(LDSCRIPT) $(rtos-init)            \
#      --start-group $(rtos-main) --end-group \
#      $(PLATFORM_LIBS) -Map $(@:.elf=.map)
quiet_cmd_rtos = CC      $@
      cmd_rtos = $(CC) $(LD_FLAGS) $(LDFLAGS_rtos) -o $@  \
      -T $(LDSCRIPT) $(rtos-init) $(rtos-main)              \
      $(PLATFORM_CPPFLAGS) $(PLATFORM_LIBS) -Wl,-Map,$(@:.elf=.map)

quiet_cmd_objcopy = OBJCOPY $@
      cmd_objcopy = $(OBJCOPY) -O binary $< $@

quiet_cmd_objdump = OBJDUMP $@
      cmd_objdump = $(OBJDUMP) -SD $< > $@

rtos.elf: $(rtos-init) $(rtos-main) FORCE
	$(call if_changed,rtos)

rtos.bin: rtos.elf FORCE
	$(call if_changed,objcopy)

rtos.text: rtos.elf FORCE
	$(call if_changed,objdump)

$(sort $(rtos-init) $(rtos-main)): $(rtos-dirs) ;


PHONY += $(rtos-dirs)
$(rtos-dirs): FORCE
	$(Q)$(MAKE) $(build)=$@


######################################################################
CLEAN_DIRS  +=
CLEAN_FILES += rtos.*

clean: rm-dirs  := $(CLEAN_DIRS)
clean: rm-files := $(CLEAN_FILES)

clean-dirs := $(foreach f,$(rtos-dirs),$(if $(wildcard $(srctree)/$f/Makefile),$f))

clean-dirs := $(addprefix _clean_, $(clean-dirs))

PHONY += $(clean-dirs) clean distclean
$(clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _clean_%,%,$@)

export RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o         \
				-name CVS -o -name .pc -o -name .hg -o -name .git \) \
				-prune -o

clean distclean: $(clean-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find $(srctree) $(RCS_FIND_IGNORE) \
		\( -name '.*.d' -o -name '.*.tmp' -o -name '*.tmp_*.o.*' \
		-o -name '.*.cmd' \) -type f -print | xargs rm -f

quiet_cmd_rmdirs = $(if $(wildcard $(rm-dirs)),CLEAN   $(wildcard $(rm-dirs )))
      cmd_rmdirs = rm -rf $(rm-dirs)

quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),CLEAN   $(wildcard $(rm-fi les)))
      cmd_rmfiles = rm -f $(rm-files)

targets := $(wildcard $(sort $(targets)))
######################################################################
PHONY += help
help:
	@echo  'Cleaning targets:'
	@echo  '  clean         - Remove most generated objects but keep the config'
	@echo  '  distclean     - Remove all generated objects + configs + various path symbolic'
	@echo  ''
	@echo  'Configuration targets:'
	@echo  '  config        - mbed package configurations'
	@echo  '  menuconfig    - Alias of "make config"'
	@echo  ''
	@echo  'Other generic targets:'
	@echo  '  all           - Build all targets'
	@echo  '  help          - Show this message'
	@echo  ''
	@echo  '  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build'
	@echo  ''


######################################################################
PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
