MODULE_NAME = fdt_test
# DIRs
RTOS_OUTPUT_DIR = $(HDAL_SAMPLE_DIR)/output
# INCs
LIBRARY_INC_PATH = $(LIBRARY_DIR)/include
LIBFDT_INC_PATH = $(LIBRARY_INC_PATH)/libfdt
LIBRARY_LIB_PATH = $(LIBRARY_DIR)/output
# INC FLAGs
EXTRA_INCLUDE += -I$(LIBRARY_INC_PATH) -I$(LIBFDT_INC_PATH)
.PHONY: all clean
###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
#--------- ENVIRONMENT SETTING --------------------
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes -Werror
COMPILE_OPTS	=  -I. -O2 -fPIC -ffunction-sections -fdata-sections -D__LINUX
C_CFLAGS	= $(PLATFORM_CFLAGS) $(COMPILE_OPTS) $(WARNING) $(EXTRA_INCLUDE)
LD_FLAGS	= -L$(LIBRARY_LIB_PATH) -lfdt
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = $(MODULE_NAME)
SRC = fdt_test.c
DTB = application.dtb
DTS = application.dts

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
clean:
	@echo "nothing to be done for '$(OUTPUT_NAME)'"
else
all: $(LIB_NAME) $(DTB)

#Because kernel .dts depend on .dtsi inclusion, they have to be preprocessed first with
#the C preprocessor (cpp). The dtc tool can convert between .dts and .dtb:
$(DTB): $(DTS)
	cpp -nostdinc -I include -undef -x assembler-with-cpp $< > $<.tmp \
	&& dtc -O dtb -b 0 -o $@ $<.tmp
	rm $<.tmp

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	@$(NM) -n $@ > $@.sym
	@$(STRIP) $@
	@$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so* $(DTB)
endif

install:
	@cp -avf $(LIB_NAME) $(ROOTFS_DIR)/rootfs/usr/bin
	@cp -avf $(DTB) ${ROOTFS_DIR}/rootfs/etc

###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)
#--------- ENVIRONMENT SETTING --------------------
C_CFLAGS = $(PLATFORM_CFLAGS) $(EXTRA_INCLUDE)
#--------- END OF ENVIRONMENT SETTING -------------
LIB_NAME = lib$(MODULE_NAME).a
SRC = fdt_test.c
DTB = application.dtb
DTS = application.dts

OBJ = $(SRC:.c=.o)

all: $(LIB_NAME) $(DTB)

#Because kernel .dts depend on .dtsi inclusion, they have to be preprocessed first with
#the C preprocessor (cpp). The dtc tool can convert between .dts and .dtb:
$(DTB): $(DTS)
	cpp -nostdinc -I include -undef -x assembler-with-cpp $< > $<.tmp \
	&& dtc -O dtb -b 0 -o $@ $<.tmp
	rm $<.tmp

$(LIB_NAME): $(OBJ)
	@echo Creating $@...
	@$(AR) rcsD $@ $(OBJ)
	@$(BUILD_DIR)/nvt-tools/nvt-ld-op --arc-sha1 $@

%.o: %.c
	@echo Compiling $<
	@$(CC) $(C_CFLAGS) -c $< -o $@

clean:
	@rm -f $(LIB_NAME) $(OBJ) $(LIB_NAME).sym *.o *.a *.so* $(DTB)

install:
	@mkdir -p $(RTOS_OUTPUT_DIR)
	@cp -avf $(LIB_NAME) $(RTOS_OUTPUT_DIR)
	@cp -avf $(DTB) $(RTOS_OUTPUT_DIR)

endif
