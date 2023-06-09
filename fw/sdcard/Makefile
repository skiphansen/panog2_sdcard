include ../../project.mk

TARGET       = sdcard

# source to include
SRC_DIR      = .
SRC_DIR     += $(CORES_DIR)/core_soc/src_c
SRC_DIR     += $(PANO_FW_DIR)/lib/libstd
EXCLUDE_SRC += $(PANO_FW_DIR)/lib/libstd/printf.c
EXTRA_SRC   += $(PANO_FW_DIR)/common/spi_drv.c
EXTRA_SRC   += $(PANO_FW_DIR)/common/cmd_parser.c
EXTRA_CFLAGS += -DALTERNATE_PRINTF

INCLUDE_PATH += ../include
INCLUDE_PATH += $(PANO_FW_DIR)/common

# build optons
OPT        = 2
ENABLE_LST = yes

include $(TOPDIR)/pano/make/c_prog.mk

