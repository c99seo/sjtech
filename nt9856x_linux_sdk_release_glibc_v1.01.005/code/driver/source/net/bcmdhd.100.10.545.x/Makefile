
.PHONY: modules modules_install clean

###############################################################################
# Linux Makefile                                                              #
###############################################################################
ifeq ($(NVT_PRJCFG_CFG),Linux)
# bcmdhd
# 1. WL_IFACE_COMB_NUM_CHANNELS must be added if Android version is 4.4 with Kernel version 3.0~3.4,
#    otherwise please remove it.

MODULE_NAME := bcmdhd
CONFIG_BCMDHD = m
CONFIG_BCMDHD_SDIO =m
#CONFIG_BCMDHD_PCIE := y
#CONFIG_BCMDHD_OOB := y
CONFIG_BCMDHD_PROPTXSTATUS = y
CONFIG_BCMDHD_DISABLE_WOWLAN = y
#CONFIG_DHD_USE_STATIC_BUF := y

DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                 \
	-DBCMDONGLEHOST -DBCMDMA32 -DBCMFILEIMAGE                             \
	-DDHDTHREAD -DDHD_DEBUG -DSHOW_EVENTS -DBCMDBG -DGET_OTP_MAC_ENABLE   \
	-DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT -DSUPPORT_PM2_ONLY             \
	-DKEEP_ALIVE -DPKT_FILTER_SUPPORT -DDHDTCPACK_SUPPRESS                \
	-DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT                           \
	-DMULTIPLE_SUPPLICANT -DTSQ_MULTIPLIER -DMFP -DDHD_8021X_DUMP         \
	-DPOWERUP_MAX_RETRY=0 -DIFACE_HANG_FORCE_DEV_CLOSE -DWAIT_DEQUEUE     \
	-DWL_EXT_IAPSTA -DWL_ESCAN -DCCODE_LIST                               \
	-Idrivers/net/wireless/bcmdhd -Idrivers/net/wireless/bcmdhd/include

#YongChang
DHDCFLAGS += -I$(src)/include -I$(src)/

DHDOFILES = aiutils.o siutils.o sbutils.o bcmutils.o bcmwifi_channels.o   \
	dhd_linux.o dhd_linux_platdev.o dhd_linux_sched.o dhd_pno.o           \
	dhd_common.o dhd_ip.o dhd_linux_wq.o dhd_custom_gpio.o                \
	bcmevent.o hndpmu.o linux_osl.o wldev_common.o wl_android.o           \
	dhd_debug_linux.o dhd_debug.o dhd_mschdbg.o dhd_dbg_ring.o            \
	hnd_pktq.o hnd_pktpool.o bcmxtlv.o linux_pkt.o bcmstdlib_s.o frag.o   \
	dhd_linux_exportfs.o dhd_linux_pktdump.o                              \
	dhd_config.o dhd_ccode.o wl_event.o wl_android_ext.o wl_escan.o

ifneq ($(CONFIG_WIRELESS_EXT),)
	DHDOFILES += wl_iw.o
	DHDCFLAGS += -DSOFTAP -DWL_WIRELESS_EXT -DUSE_IW
endif
ifneq ($(CONFIG_CFG80211),)
	DHDOFILES += wl_cfg80211.o wl_cfgscan.o wl_cfgp2p.o
	DHDOFILES += wl_linux_mon.o wl_cfg_btcoex.o wl_cfgvendor.o
	DHDOFILES += dhd_cfg80211.o
	DHDCFLAGS += -DWL_CFG80211 -DWLP2P -DWL_CFG80211_STA_EVENT
	DHDCFLAGS += -DWL_IFACE_COMB_NUM_CHANNELS
	DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=10
	DHDCFLAGS += -DWL_SUPPORT_AUTO_CHANNEL
	DHDCFLAGS += -DWL_SUPPORT_BACKPORTED_KPATCHES
	DHDCFLAGS += -DESCAN_RESULT_PATCH -DESCAN_BUF_OVERFLOW_MGMT
	DHDCFLAGS += -DVSDB -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	DHDCFLAGS += -DWLTDLS -DMIRACAST_AMPDU_SIZE=8
	DHDCFLAGS += -DWL_VIRTUAL_APSTA
	DHDCFLAGS += -DPNO_SUPPORT -DEXPLICIT_DISCIF_CLEANUP
	DHDCFLAGS += -DDHD_USE_SCAN_WAKELOCK
#	DHDCFLAGS += -DWL_STATIC_IF
#	DHDCFLAGS += -DWL_CLIENT_SAE
#	DHDCFLAGS += -DWL_SAE
endif

#BCMDHD_SDIO
ifneq ($(CONFIG_BCMDHD_SDIO),)
DHDCFLAGS += -DBCMSDIO -DMMC_SDIO_ABORT -DBCMLXSDMMC -DUSE_SDIOFIFO_IOVAR \
	-DSDTEST -DBDC -DDHD_USE_IDLECOUNT -DCUSTOM_SDIO_F2_BLKSIZE=256       \
	-DBCMSDIOH_TXGLOM -DBCMSDIOH_TXGLOM_EXT -DRXFRAME_THREAD              \
	-DDHDENABLE_TAILPAD -DSUPPORT_P2P_GO_PS                               \
	-DBCMSDIO_RXLIM_POST -DCONSOLE_DPC
ifeq ($(CONFIG_BCMDHD_OOB),y)
	DHDCFLAGS += -DOOB_INTR_ONLY -DCUSTOMER_OOB -DHW_OOB
ifeq ($(CONFIG_BCMDHD_DISABLE_WOWLAN),y)
	DHDCFLAGS += -DDISABLE_WOWLAN
endif
else
	DHDCFLAGS += -DSDIO_ISR_THREAD
endif
DHDOFILES += bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o  \
	dhd_sdio.o dhd_cdc.o dhd_wlfc.o
endif

#BCMDHD_PCIE
ifneq ($(CONFIG_BCMDHD_PCIE),)
DHDCFLAGS += -DPCIE_FULL_DONGLE -DBCMPCIE -DCUSTOM_DPC_PRIO_SETTING=-1    \
	-DDONGLE_ENABLE_ISOLATION
DHDCFLAGS += -DDHD_LB -DDHD_LB_RXP -DDHD_LB_STATS -DDHD_LB_TXP
DHDCFLAGS += -DDHD_PKTID_AUDIT_ENABLED
ifeq ($(CONFIG_BCMDHD_OOB),y)
	DHDCFLAGS += -DCUSTOMER_OOB -DBCMPCIE_OOB_HOST_WAKE
endif
ifneq ($(CONFIG_PCI_MSI),)
	DHDCFLAGS += -DDHD_MSI_SUPPORT
endif
DHDOFILES += dhd_pcie.o dhd_pcie_linux.o pcie_core.o dhd_flowring.o       \
	dhd_msgbuf.o dhd_linux_lb.o
endif

#BCMDHD_USB
ifneq ($(CONFIG_BCMDHD_USB),)
DHDCFLAGS += -DUSBOS_TX_THREAD -DBCMDBUS -DBCMTRXV2 -DDBUS_USB_LOOPBACK   \
	-DBDC
DHDCFLAGS += -DBCM_REQUEST_FW -DEXTERNAL_FW_PATH
#DHDCFLAGS :=$(filter-out -DENABLE_INSMOD_NO_FW_LOAD,$(DHDCFLAGS))
ifneq ($(CONFIG_BCMDHD_CUSB),)
	DHDCFLAGS += -DBCMUSBDEV_COMPOSITE
	DHDCFLAGS :=$(filter-out -DENABLE_INSMOD_NO_FW_LOAD,$(DHDCFLAGS))
endif
DHDOFILES += dbus.o dbus_usb.o dbus_usb_linux.o dhd_cdc.o dhd_wlfc.o
endif

#PROPTXSTATUS
ifeq ($(CONFIG_BCMDHD_PROPTXSTATUS),y)
ifneq ($(CONFIG_BCMDHD_USB),)
	DHDCFLAGS += -DPROP_TXSTATUS
endif
ifneq ($(CONFIG_BCMDHD_SDIO),)
	DHDCFLAGS += -DPROP_TXSTATUS -DPROPTX_MAXCOUNT
endif
ifneq ($(CONFIG_CFG80211),)
	DHDCFLAGS += -DPROP_TXSTATUS_VSDB
endif
endif

ifeq ($(CONFIG_64BIT),y)
    DHDCFLAGS := $(filter-out -DBCMDMA32,$(DHDCFLAGS))
    DHDCFLAGS += -DBCMDMA64OSL
endif

#VTS_SUPPORT
ifeq ($(CONFIG_VTS_SUPPORT),y)
ifneq ($(CONFIG_CFG80211),)
DHDCFLAGS += -DGSCAN_SUPPORT -DRTT_SUPPORT -DLINKSTAT_SUPPORT             \
	-DCUSTOM_COUNTRY_CODE -DDHD_GET_VALID_CHANNELS                        \
	-DDEBUGABILITY -DDBG_PKT_MON -DDHD_LOG_DUMP -DDHD_FW_COREDUMP         \
	-DAPF -DNDO_CONFIG_SUPPORT -DRSSI_MONITOR_SUPPORT -DDHD_WAKE_STATUS
DHDOFILES += dhd_rtt.o bcm_app_utils.o
endif
endif

# For Debug
ifneq ($(CONFIG_BCMDHD_DEBUG),)
DHDCFLAGS += -DDEBUGFS_CFG80211
DHDCFLAGS += -DSHOW_LOGTRACE -DDHD_LOG_DUMP -DDHD_FW_COREDUMP             \
	-DBCMASSERT_LOG -DSI_ERROR_ENFORCE
ifneq ($(CONFIG_BCMDHD_PCIE),)
	DHDCFLAGS += -DEWP_EDL
	DHDCFLAGS += -DDNGL_EVENT_SUPPORT
	DHDCFLAGS += -DDHD_SSSR_DUMP
endif
endif

# MESH support for kernel 3.10 later
ifeq ($(CONFIG_WL_MESH),y)
	DHDCFLAGS += -DWLMESH
ifneq ($(CONFIG_CFG80211),)
	DHDCFLAGS += -DWLMESH_CFG80211
endif
ifneq ($(CONFIG_BCMDHD_PCIE),)
	DHDCFLAGS += -DBCM_HOST_BUF -DDMA_HOST_BUFFER_LEN=0x80000
endif
	DHDCFLAGS += -DDHD_UPDATE_INTF_MAC
	DHDCFLAGS :=$(filter-out -DDHD_FW_COREDUMP,$(DHDCFLAGS))
	DHDCFLAGS :=$(filter-out -DWL_STATIC_IF,$(DHDCFLAGS))
endif

ifeq ($(CONFIG_WL_EASYMESH),y)
	DHDCFLAGS :=$(filter-out -DDHD_FW_COREDUMP,$(DHDCFLAGS))
	DHDCFLAGS :=$(filter-out -DDHD_LOG_DUMP,$(DHDCFLAGS))
	DHDCFLAGS += -DWLEASYMESH -DWL_STATIC_IF -DWLDWDS -DFOURADDR_AUTO_BRG
endif

#CSI_SUPPORT
ifneq ($(CONFIG_CSI_SUPPORT),)
	DHDCFLAGS += -DCSI_SUPPORT
	DHDOFILES += dhd_csi.o
endif

obj-$(CONFIG_BCMDHD) += $(MODULE_NAME).o
$(MODULE_NAME)-objs += $(DHDOFILES)

ifeq ($(CONFIG_MACH_PLATFORM),y)
	DHDOFILES += dhd_gpio.o
ifeq ($(CONFIG_BCMDHD_DTS),y)
	DHDCFLAGS += -DCONFIG_DTS
else
	DHDCFLAGS += -DCUSTOMER_HW -DDHD_OF_SUPPORT
endif
#	DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
endif

ifeq ($(CONFIG_BCMDHD_AG),y)
	DHDCFLAGS += -DBAND_AG
endif

ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
	obj-m += dhd_static_buf.o
	DHDCFLAGS += -DSTATIC_WL_PRIV_STRUCT -DENHANCED_STATIC_BUF
	DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
	DHDCFLAGS += -DDHD_USE_STATIC_MEMDUMP
endif

EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCMDHD),m)
EXTRA_LDFLAGS += --strip-debug
endif

ifeq ($(KERNELRELEASE),)
PWD := $(shell pwd)
KERVER ?= $(NVT_LINUX_VER)
KDIR ?= $(KERNELDIR)
MDIR ?= $(KERNELDIR)/_install_modules/lib/modules/$(KERVER)/extra
MODPATH := $(shell echo $(PWD) | awk -F'linux-driver/' '{print $$NF}')
MODNAME := $(shell echo $(obj-m:.o=.ko))

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

modules_install:
	@if [ -z $(NVT_MOD_INSTALL) ]; then \
		rm -f $(MDIR)/$(MODPATH)/$(MODNAME); \
		install -m644 -b -D $(MODNAME) ${MDIR}/$(MODPATH)/$(MODNAME); \
		cd $(KDIR)/_install_modules/lib/modules/$(KERVER)/; depmod -b $(KDIR)/_install_modules/ -a $(KERVER); \
	else \
		mkdir -p $(NVT_MOD_INSTALL)/lib/modules/$(KERVER); \
		install -m644 -b -D $(MODNAME) $(NVT_MOD_INSTALL)/lib/modules/$(KERVER)/hdal/$(MODPATH)/$(MODNAME); \
	fi

clean:
	@SRC="`find . -name  "*.c" ! -name "*.mod.c"`"; \
	if [ "$$SRC" ]; then \
		rm -rf .tmp_versions Module.symvers modules.order `find . -type f -name "*.mod.c" -o -name ".*.cmd" -o -name "*.o" -o -name "*.ko" -o -name "modules.order" -o -name *~`; \
		echo ">>> Clean"; \
	else \
		echo ">>> Skip"; \
	fi \

.PHONY: modules modules_install clean
endif
###############################################################################
# rtos Makefile                                                               #
###############################################################################
else ifeq ($(NVT_PRJCFG_CFG),rtos)

modules:
	@echo "nothing to be done for bcmdhd.100.10.545.x"

clean:
	@echo "nothing to be done for bcmdhd.100.10.545.x"

modules_install:
	@echo "nothing to be done for bcmdhd.100.10.545.x"

endif
