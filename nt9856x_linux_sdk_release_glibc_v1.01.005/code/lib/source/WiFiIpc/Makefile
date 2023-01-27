MODULE_NAME = libWiFiIpc
# DIRs
OUTPUT_DIR = $(LIBRARY_DIR)/output
# INCs
LIB_INC_PATH = $(LIBRARY_DIR)/include/
# INC FLAGs
EXTRA_INCLUDE += -I$(LIB_INC_PATH) -I$(NVT_VOS_DIR)/include
.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING	= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Wno-sign-compare
COMPILE_OPTS	= -I. -O2 -fPIC -ffunction-sections -fdata-sections
C_CFLAGS	= $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS) $(EXTRA_INCLUDE)
LD_FLAGS	= -fPIC -shared -lpthread
#--------- END OF ENVIRONMENT SETTING -------------
SRC = \
    WiFiIpcAPI.c \

OBJ = $(SRC:.c=.o)

FWDIR = $(ROOTFS_DIR)/rootfs/lib/firmware/ap6255

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(OBJ)
	@echo Creating library $(MODULE_NAME) ...
#	@$(CC) -o $(MODULE_NAME).so $(OBJ) $(LD_FLAGS)
	@$(AR) rcs $(MODULE_NAME).a $(OBJ)

%.o:%.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) $< -c -o $@

clean:
	@rm -f $(OBJ) *.o *.a *.so*
	@rm -rf $(OUTPUT_DIR)
endif

install:
	@mkdir -p $(OUTPUT_DIR)
	@cp -avf *.a $(OUTPUT_DIR)
	@mkdir -p $(ROOTFS_DIR)/rootfs/usr/share/wifiscripts
	@cp -avfr scripts/8189ftv/* $(ROOTFS_DIR)/rootfs/usr/share/wifiscripts/
	#@cp -avfr scripts/ap6255/*.sh $(ROOTFS_DIR)/rootfs/usr/share/wifiscripts/
	#@mkdir -p $(FWDIR)
	#@cp -af scripts/ap6255/fw_bcm43455c0_ag_apsta.bin $(FWDIR)
	#@cp -af scripts/ap6255/nvram_ap6255.txt $(FWDIR)

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
#--------- ENVIRONMENT SETTING --------------------
C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE)
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME).a
SRC = \

OBJ = $(SRC:.c=.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so*
	@rm -rf $(OUTPUT_DIR)

install:
	@mkdir -p $(RTOS_OUTPUT_DIR)
	@cp -avf *.a $(RTOS_OUTPUT_DIR)

endif
