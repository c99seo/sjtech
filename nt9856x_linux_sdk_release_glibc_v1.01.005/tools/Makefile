MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
TOOLS = ${PWD}
SHELL = /bin/bash
INSTALL_DIR = __install

ZLIB := zlib-1.2.8
LZO := lzo-2.08
E2FSPROGS := e2fsprogs-1.44.6
MTD_UTILS := mtd-utils-2.1.1
MEMTESTER := memtester-4.3.0
BONNIE := bonnie++-1.03e
ETHTOOL := ethtool-3.18
GDB := gdb-8.3.1
TERMCAP := termcap-1.3.1
NCURSES := ncurses-6.1
HTOP := htop-1.0.3
PROCPS := procps-3.2.8
SDCARD_TEST := sdcard_test.sh
I2C_TOOL := i2c-tools-3.1.2
STRESS := stress-1.0.4
STRESSNG := stress-ng-0.07.09
IOZONE := iozone-3.469
NETSTRESS := netstress_1.2.0
STRACE := strace-5.14
ALSA_LIB := alsa-lib-1.1.9
ALSA_UTILS := alsa-utils-1.1.9
OPROFILE := oprofile-0.9.9
GATOR := gator
VDBENCH := VDBench
PHYTOOL := phytool
LIBPCAP := libpcap-1.9.1
TCPDUMP := tcpdump-4.9.3
DOSFSTOOLS := dosfstools-4.1
FIO := fio-3.20
SYSSTAT := sysstat-12.5.2
USER_MEM_USAGE := user_space_mem_usage.sh

unexport CC
unexport CPP
unexport CXX
unexport LD
unexport AR
unexport NM
unexport STRIP
unexport OBJCOPY
unexport OBJDUMP

CC="${CROSS_COMPILE}gcc"
CXX="${CROSS_COMPILE}g++"
CPP="${CROSS_COMPILE}gcc"

check_exist = if [ ! -e $(1) ]; then tar -jxvf $(1).tar.bz2; fi
check_exist_cp = if [ -e $(INSTALL_DIR)/$(1) ]; then cp -arfv $(INSTALL_DIR)/$(1)/* ${ROOTFS_DIR}/rootfs/$(1); fi

.PHONY: mtd-utils bonnie ethtool iozone gdb htop procps memtester
all: mtd-utils bonnie ethtool iozone gdb htop procps memtester

init:
	@if [ ! -e "${TOOLS}/${INSTALL_DIR}" ]; then \
		mkdir ${TOOLS}/${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

zlib: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ZLIB})
	@cd ${ZLIB}; prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} CFLAGS="-O4" ./configure; make; make install;

lzo: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LZO})
	@cd ${LZO}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} --host=${NVT_HOST}; make; make install;

e2fsprogs: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${E2FSPROGS})
	@cd ${E2FSPROGS}; patch -p1 < misc-e4defrag.c-use-posix_fallocate64-if-fallocate64-is-unavailable; ./configure --prefix=${TOOLS}/${INSTALL_DIR} CC=${CC} --host=${NVT_HOST}; make -j$(MULTI_CORES); make install;

mtd-utils: zlib lzo e2fsprogs
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${MTD_UTILS})
	@cd ${MTD_UTILS}; ./autogen.sh; ./configure LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CPPFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --without-crypto --without-xattr --without-zstd; make -j$(MULTI_CORES); make install-exec;

memtester: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${MEMTESTER})
	@cd ${MEMTESTER}; echo "${CC} -O2 -DPOSIX -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -DTEST_NARROW_WRITES -c" > conf-cc; echo "${CC} -s" > conf-ld; make;
	@cd ${MEMTESTER}; mkdir ${TOOLS}/${INSTALL_DIR}/bin/; cp -arvf memtester ${TOOLS}/${INSTALL_DIR}/bin/;

bonnie: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${BONNIE})
	@cd ${BONNIE}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make; mkdir -p ${TOOLS}/${INSTALL_DIR}/sbin/; cp bonnie++ zcav ${TOOLS}/${INSTALL_DIR}/sbin/;

ethtool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ETHTOOL})
	@cd ${ETHTOOL}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make; make install;

gdb: termcap ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${GDB})
	@cd ${GDB}; ./configure LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CXXFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --without-x --disable-gdbtk --disable-tui --without-included-regex --without-included-gettext; make -j$(MULTI_CORES); make install

termcap: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${TERMCAP})
	@cd ${TERMCAP}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make -j$(MULTI_CORES); make install;

ncurses: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${NCURSES})
	@cd ${NCURSES}; ./configure LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" CXXFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make -j$(MULTI_CORES); make install.libs install.data install.includes install.ncurses;

htop: ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${HTOP})
	@cd ${HTOP}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR} --disable-unicode LDFLAGS="-static -L${TOOLS}/${INSTALL_DIR}/lib" CPPFLAGS="-I${TOOLS}/${INSTALL_DIR}/include" CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include"; make; make install;

procps: ncurses
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PROCPS})
	@cd ${PROCPS}; make lib64=lib SHARED=0 CFLAGS="-I${TOOLS}/${INSTALL_DIR}/include/ncurses -I${TOOLS}/${INSTALL_DIR}/include" \
			DESTDIR=${TOOLS}/${INSTALL_DIR} CC=${CC} LDFLAGS="-L${TOOLS}/${INSTALL_DIR}/lib" install="install -D" install

mmctest: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp ${SDCARD_TEST} ${TOOLS}/${INSTALL_DIR}/usr/bin/

i2c_tool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${I2C_TOOL})
	@cd ${I2C_TOOL}; CC=${CC} make; prefix=${TOOLS}/${INSTALL_DIR}/usr make install;

stress: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRESS})
	@cd ${STRESS}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make; make install;

stress-ng: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRESSNG})
	@cd ${STRESSNG}; make CC=${CC} HAVE_LIB_AIO=0 HAVE_SYS_CAP_H=0 HAVE_FLOAT_DECIMAL=0 HAVE_SECCOMP_H=0 HAVE_LIB_Z=0 DESTDIR=${TOOLS}/${INSTALL_DIR} install;

strace: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${STRACE})
	@cd ${STRACE}; ./configure --host=${NVT_HOST} --target=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make install

iozone: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${IOZONE})
	@cd ${IOZONE}; ./configure --host=${NVT_HOST} --prefix=${TOOLS}/${INSTALL_DIR}; make linux CC=${CC}; mkdir -p ${TOOLS}/${INSTALL_DIR}/sbin/; cp iozone fileop pit_server ${TOOLS}/${INSTALL_DIR}/sbin/;

netstress: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${NETSTRESS})
	@cd ${NETSTRESS}; mkdir -p ${TOOLS}/${INSTALL_DIR}/bin/; ${CC} -o ${TOOLS}/${INSTALL_DIR}/bin/$@ netstress.c

alsa_lib: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ALSA_LIB})
	@cd ${ALSA_LIB};./configure CC=${CC} --host=${NVT_HOST} --disable-python;make;make install DESTDIR=${TOOLS}/${INSTALL_DIR};

alsa_utils: init alsa_lib
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${ALSA_UTILS})
	cd ${ALSA_UTILS};./configure --with-udev-rules-dir=""  --with-systemdsystemunitdir="$(pkg-config systemd --variable=systemdsystemunitdir)" CC=${CC} --host=${NVT_HOST} --with-alsa-inc-prefix="${TOOLS}/${INSTALL_DIR}/usr/include" --with-alsa-prefix="${TOOLS}/${INSTALL_DIR}/usr/lib" --disable-alsamixer;make;make install DESTDIR=${TOOLS}/${INSTALL_DIR};

alsa_nvt_libs:
	@if [ -e $(LIBRARY_DIR)/output/libaec.so ]; \
	then \
		cp $(LIBRARY_DIR)/output/libaec.so ${ROOTFS_DIR}/rootfs/usr/lib; \
	fi

	@if [ -e $(LIBRARY_DIR)/output/libanr.so ]; \
	then \
		cp $(LIBRARY_DIR)/output/libanr.so ${ROOTFS_DIR}/rootfs/usr/lib; \
	fi

	@if [ -e $(LIBRARY_DIR)/output/alsa-lib/libasound_module_pcm_aec.so ] && [ -e $(LIBRARY_DIR)/output/alsa-lib/libasound_module_pcm_anr.so ]; \
	then \
		cp -arfv $(LIBRARY_DIR)/output/alsa-lib ${ROOTFS_DIR}/rootfs/usr/lib; \
	fi

gator: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${GATOR})
	cd ${GATOR}/driver; make -C ${KERNELDIR} M=${PWD}/${GATOR}/driver ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} modules; cp gator.ko ${TOOLS}/${INSTALL_DIR}
	cd ${GATOR}/daemon; make CROSS_COMPILE=${CROSS_COMPILE} ;cp gatord ${TOOLS}/${INSTALL_DIR}
	cp ${CROSS_TOOLCHAIN_PATH}/target/lib/libgcc_s.so.1 ${ROOTFS_DIR}/rootfs/lib

oprofile: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${OPROFILE})
	@cd ${OPROFILE}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} --with-kernel=${OUTPUT_DIR}; make -j${MULTI_CORES}; make install

VDBench: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${VDBENCH})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${VDBENCH}; make -j${MULTI_CORES}; rm Bins/*.o; cp Bins/* ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp -R Scripts ${TOOLS}/${INSTALL_DIR}/usr/bin/; cp VDBench.sh ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${VDBENCH}/ThirdBenches/coremark_v1.0; make -j${MULTI_CORES}
	@make install

phytool: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${PHYTOOL})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${PHYTOOL}; make CC=${CC} -j${MULTI_CORES}; rm *.o; cp phytool ${TOOLS}/${INSTALL_DIR}/usr/bin/

libpcap: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${LIBPCAP})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${LIBPCAP}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install; rm *.o

tcpdump: libpcap
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${TCPDUMP})
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin/
	@cd ${TCPDUMP}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST}; make -j${MULTI_CORES}; make install; rm *.o

dosfstools: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${DOSFSTOOLS})
	@cd ${DOSFSTOOLS}; ./configure --prefix=${TOOLS}/${INSTALL_DIR} --host=${NVT_HOST} --without-udev; make -j${MULTI_CORES}; make install

fio: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${FIO})
	@cd ${FIO}; ./configure --build-static  --prefix=${TOOLS}/${INSTALL_DIR}/fio; make; make install

sysstat: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	$(call check_exist, ${SYSSTAT})
	@cd ${SYSSTAT}; ./configure --host=${NVT_HOST} --disable-documentation; make -j${MULTI_CORES}; make DESTDIR=${TOOLS}/${INSTALL_DIR} install;

user_mem_usage: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@mkdir -p ${TOOLS}/${INSTALL_DIR}/usr/bin; cp -arfv ${USER_MEM_USAGE} ${TOOLS}/${INSTALL_DIR}/usr/bin;

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@$(call check_exist_cp,bin)
	@$(call check_exist_cp,sbin)
	@$(call check_exist_cp,usr/bin)
	@$(call check_exist_cp,usr/sbin)
	@$(call check_exist_cp,usr/lib)
	@$(call check_exist_cp,lib)
	@$(call check_exist_cp,usr/share)

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${TOOLS}/${INSTALL_DIR}
	@rm -rf ${ZLIB} ${LZO} ${E2FSPROGS} ${MTD_UTILS} ${BONNIE} ${ETHTOOL} ${TERMCAP} ${GDB} ${NCURSES} ${HTOP} ${IPERF} ${PROCPS} ${I2C_TOOL} ${STRESS} ${STRESSNG} ${IOZONE} ${NETSTRESS} ${STRACE} ${MEMTESTER} ${ALSA_LIB} ${ALSA_UTILS} ${GATOR} ${VDBENCH}
	@rm -rf ${PHYTOOL} ${LIBPCAP} ${TCPDUMP} ${DOSFSTOOLS} ${FIO} $(SYSSTAT)
