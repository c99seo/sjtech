# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
include $(NVT_PRJCFG_MODEL_CFG)
ifeq ($(KERNELRELEASE),)
	export KBUILD_EXTRA_SYMBOLS = $(shell find $(NVT_HDAL_DIR)/drivers -name Module.symvers)
	# SUBDIRS := $(dir $(wildcard */*/Makefile */*/makefile */Makefile))
	SUBDIRS := $(dir $(shell find . -name Makefile))
	SUBDIRS := $(shell echo $(SUBDIRS) | sed -e 's/\.\///g';)
	# The current directory is passed to sub-makes as argument
	PWD := $(shell pwd)

modules:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) modules $(NVT_MULTI_CORES_FLAG);

modules_install:
	@if [ -z $(NVT_MOD_INSTALL) ]; then \
		$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=./_install_modules/ INSTALL_MOD_DIR=extra modules_install; \
	else \
		$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(NVT_MOD_INSTALL) INSTALL_MOD_DIR=extra modules_install; \
	fi

clean:
	@for n in $(SUBDIRS); do \
		SRC="`find $$n -name  *.c ! -name *.mod.c`"; \
		if [ "$$SRC" ]; then \
			rm -rf .tmp_versions Module.symvers modules.order `find $$n -type f -name *.mod.c -o -name .*.cmd -o -name *.o -o -name *.o.d -o -name *.ko -o -name modules.order -o -name *~ -o -name .Makefile.swp -o -name Module.symvers`; \
		else \
			echo ">>> Skip"; \
		fi \
	done

.PHONY: modules modules_install clean

else
# called from kernel build system: just declare what our modules are
obj-m += \
	sample/fake/ \
	net/rtl8189fs/ \
	crypto/cryptodev-linux/ \
	msdcnvt/msdcnvt/ \
	msdcnvt/msdcnvt_adj/ \
	msdcnvt/msdcnvt_custom_si/ \
	touch/touch_gt911/ \
	nvt_sen_det/ \
	
#obj-m += net/bcmdhd.100.10.545.x/ \

obj-$(CONFIG_HAVE_HW_BREAKPOINT) += debug/nvt_data_breakpoint/

endif
