#--------- ENVIRONMENT SETTING --------------------
# DIRs
VOS_DRIVER_DIR = $(NVT_VOS_DIR)/drivers
HDAL_SAMPLE_DIR = $(NVT_HDAL_DIR)/samples
# INCs
VOS_INC_PATH = $(VOS_DRIVER_DIR)/include
HDAL_INC_PATH = $(NVT_HDAL_DIR)/include
HDAL_LIB_PATH = $(NVT_HDAL_DIR)/output
VOS_LIB_PATH  = $(NVT_VOS_DIR)/output
VENDOR_LIB_PATH = $(NVT_HDAL_DIR)/vendor/output
AI_LIB        = $(NVT_HDAL_DIR)/vendor/ai2/source

uclibc=$(shell echo $(CROSS_COMPILE)|grep uclib)
ifeq ($(uclibc),)
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/glibc
else
    PREBUILD_LIB=$(NVT_HDAL_DIR)/vendor/ai/source/prebuilt/lib/uclibc
endif

EXTRA_INCLUDE += -I$(HDAL_INC_PATH) -I$(NVT_HDAL_DIR)/source -I$(NVT_HDAL_DIR)/vendor/isp/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/drivers/k_driver/source/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/source_pub/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/source_pub/vendor_ai_cpu
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/source_pub/vendor_ai_dsp
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai/drivers/k_driver/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/drivers/k_flow/include
EXTRA_INCLUDE += -I$(NVT_HDAL_DIR)/vendor/ai2/drivers/k_flow/source/kflow_ai_net
INCLUDES = -I$(LIBRARY_DIR)/include -I$(HDAL_INC_PATH) -I$(VOS_INC_PATH) -I$(NVT_HDAL_DIR)/vendor/isp/include -I$(NVT_HDAL_DIR)/vendor/media/include
WARNING = -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS = $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS =
CFLAGS = $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS) $(EXTRA_INCLUDE) -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_
C_FLAGS = $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS = -L$(LIBRARY_DIR)/output -L$(NVT_HDAL_DIR)/output -lstdc++ -lpthread -lm -lrt -lhdal -lvendor_isp -lprebuilt_isp -lvendor_media -L$(NVT_HDAL_DIR)/vendor/output
LD_FLAGS += -L$(VOS_LIB_PATH) -lvos
LD_FLAGS += -L$(AI_LIB) -lvendor_ai2
LD_FLAGS += -L$(AI_LIB)_pub -lvendor_ai2_pub
LD_FLAGS += -L$(PREBUILD_LIB) -lprebuilt_ai
#--------- Compiling -------------------
BIN = plug_pq_nnsc
SRC = plug_pq_nnsc.c
HEADER = $(shell find . -name "*.h")

OBJ = $(SRC:.c=.o)

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"

else
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	$(NM) -n $@ > $@.sym
	$(STRIP) $@
	$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c $(HEADER)
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -vf $(BIN) $(OBJ) $(BIN).sym *.o *.a *.so*
endif

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(APP_DIR)/output
	@cp -avf $(BIN) $(APP_DIR)/output
	@cp -avf ${BIN} ${ROOTFS_DIR}/rootfs/bin/
