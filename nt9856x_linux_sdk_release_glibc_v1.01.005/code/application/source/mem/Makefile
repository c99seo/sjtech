include $(NVT_PRJCFG_MODEL_CFG)
#--------- ENVIRONMENT SETTING --------------------
INCLUDES	= -I$(NVT_HDAL_DIR)/include -I$(LIBRARY_DIR)/include
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS	= $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS	=
CFLAGS		= $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS)
C_FLAGS		= $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS	= -L$(LIBRARY_DIR)/output/lib -Wl,-rpath-link=$(LIBRARY_DIR)/output/lib -lrt
#--------- END OF ENVIRONMENT SETTING -------------

#--------- Compiling -------------------
BIN = mem
SRC = mem.c
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
	@cp -avf ${BIN} $(ROOTFS_DIR)/rootfs/usr/bin
