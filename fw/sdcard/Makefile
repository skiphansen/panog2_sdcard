include ../../project.mk

STACK_SIZE   = 8192
TARGET       = sdcard

# source to include
SRC_DIR      = .
SRC_DIR     += $(CORES_DIR)/core_soc/src_c
SRC_DIR     += $(PANO_FW_DIR)/lib/libstd
EXCLUDE_SRC += $(PANO_FW_DIR)/lib/libstd/printf.c
EXTRA_SRC   += $(PANO_FW_DIR)/common/spi_drv.c
EXTRA_SRC   += $(PANO_FW_DIR)/common/cmd_parser.c
EXTRA_SRC   += $(PANO_FW_DIR)/common/log.c
EXTRA_CFLAGS += -DALTERNATE_PRINTF
# use alternate printf
EXTRA_CFLAGS += -DALTERNATE_PRINTF -DPRINTF_ALIAS_STANDARD_FUNCTION_NAMES=1

#EXTRA_CFLAGS += -DBIT_BANG_SPI

INCLUDE_PATH += ../include
INCLUDE_PATH += $(PANO_FW_DIR)/common

# build optons
OPT        = 2
ENABLE_LST = yes

include $(TOPDIR)/pano/make/c_prog.mk

