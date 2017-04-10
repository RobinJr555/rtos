hal-$(CONFIG_CPU_V7M_CM4) += hal/stm32f4

BOARD_HAL := $(hal-y)

INCLUDE_PATHS += -I$(srctree)/board/$(VENDOR)/$(BOARD_HAL)
INCLUDE_PATHS += -I$(srctree)/$(BOARDDIR)/include

CC_SYMBOLS += -D__CMSIS_RTOS
