include $(NVT_PRJCFG_MODEL_CFG)
#--------- ENVIRONMENT SETTING --------------------
INCLUDES	= -I$(LIBRARY_DIR)/include/ -I$(NVT_HDAL_DIR)/include
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS	= $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS	= 
CFLAGS		= $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS)
C_FLAGS		= $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS	= -fPIC -shared -L$(LIBRARY_DIR)
#--------- END OF ENVIRONMENT SETTING -------------
DEP_LIBRARIES := 

LIB_NAME=libnvtinfo
INC = nvtinfo.h
INC_PROTECT =
SRC = nvtinfo.c
HEADER = $(shell find . -name "*.h")

copy = if [ ! -z $(1) ]; then cp -avf $(1) $(2); fi

OBJ = $(SRC:.c=.o)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"
install_header:
	@echo ">>> Skip"
else
all : $(OBJ)
	$(CC) -o $(LIB_NAME).so $(OBJ) $(LD_FLAGS)
	$(AR) rcs $(LIB_NAME).a *.o

%.o:%.c $(HEADER)
	$(CC) $(C_FLAGS) $< -c

clean:
	rm -vf $(OBJ) *.o *.a *.so*

install_header:
	@$(call copy, $(INC), $(LIBRARY_DIR)/include)
	@$(call copy, $(INC_PROTECT), $(LIBRARY_DIR)/include/protected)
endif

install: install_header
	@mkdir -p $(LIBRARY_DIR)/output
	@cp -avf *.so* *.a $(LIBRARY_DIR)/output
