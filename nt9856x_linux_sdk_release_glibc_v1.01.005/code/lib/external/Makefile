include $(NVT_PRJCFG_MODEL_CFG)

EXTLIB = ${LIBRARY_DIR}/external
SHELL = /bin/bash
INSTALL_DIR =${EXTLIB}/__install
#MAKEFILE_LIST = freetype libnl openssl
# openssl for tee
MULTI_CORES ?= $(shell grep -c ^processor /proc/cpuinfo)
MAKEFILE_LIST = openssl libnl


XML2 := libxml2-2.9.3
AXTLS := axtls
FUSE := fuse-2.9.4
FREETYPE := freetype-2.3.5
UNWIND := libunwind-1.3.0
SQLITE3 :=sqlite3-3.10.0
FFMPEG := ffmpeg-3.0
LIBNL := libnl-3.2.27
OPENSSL := openssl-1.0.2d
CURL := curl-7.45.0

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
AR="${CROSS_COMPILE}ar"
LD="${CROSS_COMPILE}ld"
RANLIB="${CROSS_COMPILE}ranlib"

check_exist = if [ ! -e $(1)/.nvt_finish ]; then rm -rf $(1); tar -jxvf $(1).tar.bz2; fi

.PHONY: $(MAKEFILE_LIST)
all: $(MAKEFILE_LIST)

list:
	@echo $(MAKEFILE_LIST)

init:
	@if [ ! -e "${INSTALL_DIR}" ]; then \
		mkdir -p ${INSTALL_DIR}; \
	fi
	@if [ -z "${CROSS_COMPILE}" ]; then \
		echo "Please execute \"source build/envsetup.sh\" firstly.  Stop."; exit 1; \
	fi

libxml2: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${XML2}/.nvt_finish ]; then \
		$(call check_exist, ${XML2}); \
		cd ${XML2}; ./configure --prefix=${INSTALL_DIR} CC=${CC} CXX=${CXX} AR=${AR} --host=${NVT_HOST} --without-python --target=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
	@cd ${XML2}; make install; cd -;
	@rm -rf ${INSTALL_DIR}/lib/cmake ${INSTALL_DIR}/lib/xml2Conf.sh ${INSTALL_DIR}/lib/pkgconfig
	@touch ${XML2}/.nvt_finish

axtls: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${AXTLS}/.nvt_finish ]; then \
		$(call check_exist, ${AXTLS}); \
		cd ${AXTLS}; make -j$(MULTI_CORES) CC=${CC} AR=${AR}; \
	fi
	@cd ${AXTLS}; make PREFIX=${INSTALL_DIR} install;
	@touch ${AXTLS}/.nvt_finish

fuse: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FUSE}/.nvt_finish ]; then \
		$(call check_exist, ${FUSE}); \
		cd ${FUSE}; ./configure --host=${NVT_HOST} CC=${CC} LD=${LD} AR=${AR}; \
		make -j$(MULTI_CORES); \
	fi
	cd ${FUSE}; make -j$(MULTI_CORES) install DESTDIR=${EXTLIB}/${FUSE}/install; cp -arvf ${EXTLIB}/${FUSE}/install/usr/local/* ${INSTALL_DIR};
	@touch ${FUSE}/.nvt_finish

freetype: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FREETYPE}/.nvt_finish ]; then \
		$(call check_exist, ${FREETYPE}); \
		cd ${FREETYPE}; mkdir result; CC=${NVT_HOST}-gcc; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST}; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${FREETYPE}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${FREETYPE}/.nvt_finish

libunwind: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${UNWIND}/.nvt_finish ]; then \
		$(call check_exist, ${UNWIND}); \
		cd ${UNWIND}; mkdir result; CC=${NVT_HOST}-gcc; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST}; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${UNWIND}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${UNWIND}/.nvt_finish

sqlite3: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${SQLITE3}/.nvt_finish ]; then \
		$(call check_exist, ${SQLITE3}); \
		cd ${SQLITE3}; make -j$(MULTI_CORES) CC=${CC} AR=${AR} STRIP="${CROSS_COMPILE}strip"; \
	fi
	@cd ${SQLITE3}; make INSTALL_DIR=${INSTALL_DIR} install;
	@touch ${SQLITE3}/.nvt_finish

ffmpeg: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${FFMPEG}/.nvt_finish ]; then \
		$(call check_exist, ${FFMPEG}); \
		cd ${FFMPEG}; ./configure --prefix=${INSTALL_DIR} --cross-prefix=${NVT_HOST}- --target-os=linux --arch=${ARCH} --target-exec="qemu-mips -cpu 24Kec" --disable-everything --enable-muxer=mpegts --enable-muxer=mp4 --disable-armv5te --disable-armv6t2 --disable-armv6 --disable-asm --disable-encoders --disable-decoders --enable-protocol=file --enable-small --disable-static --enable-shared --enable-cross-compile --disable-zlib --disable-mipsfpu --disable-ffmpeg --disable-ffserver --disable-ffprobe --disable-swscale --disable-swresample --disable-avdevice --disable-avfilter --disable-debug; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${FFMPEG}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${FFMPEG}/.nvt_finish

libnl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${LIBNL}/.nvt_finish ]; then \
		$(call check_exist, ${LIBNL}); \
		cd ${LIBNL}; ./configure --prefix=${INSTALL_DIR} --host=${NVT_HOST} CC=${CC} LD=${LD} AR=${AR}; \
		make -j$(MULTI_CORES); \
	fi
	@cd ${LIBNL}; make install;
	@cp ${INSTALL_DIR}/lib/libnl-3.so ${INSTALL_DIR}/lib/libnl.so ; cp ${INSTALL_DIR}/lib/libnl-genl-3.so ${INSTALL_DIR}/lib/libnl-genl.so
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@touch ${LIBNL}/.nvt_finish

openssl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
	@if [ ! -f ${OPENSSL}/.nvt_finish ]; then \
		$(call check_exist, ${OPENSSL}); \
		cd ${OPENSSL}; CROSS_COMPILE="" CC=${CC} LD=${LD} AR=${AR} RANLIB=${RANLIB} ./Configure shared linux-generic32 -DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGESTS -DOPENSSL_SSL_CLIENT_ENGINE_AUTO --prefix=${INSTALL_DIR} -I$(NVT_DRIVER_DIR)/include; make -j$(MULTI_CORES); \
	fi
	@cd ${OPENSSL}; make install;
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig ${INSTALL_DIR}/lib/engines
	@touch ${OPENSSL}/.nvt_finish

curl: init
	@echo ">>>>>>>>>>>>>>>>>>> $@ compiling >>>>>>>>>>>>>>>>>>>"
ifeq ($(NVT_CURL_SSL),NVT_CURL_SSL_OPENSSL)
	@if [ -z `cat ${CURL}/.nvt_finish | grep NVT_CURL_SSL_OPENSSL` ]; then \
		$(call check_exist, ${CURL}); \
		cd ${CURL}; CPPFLAGS="-I${INSTALL_DIR}/include -I${INCLUDE_DIR}" LDFLAGS="-L${LIBRARY_DIR} -L${INSTALL_DIR}/lib -Wl,-rpath-link=$(INSTALL_DIR)/lib" LIBS="-lssl" ./configure --with-ssl --prefix=${INSTALL_DIR} --with-ca-path="/etc/ssl/certs" --host=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
else
	@if [ -z `cat ${CURL}/.nvt_finish | grep NVT_CURL_SSL_AXTLS` ]; then \
		$(call check_exist, ${CURL}); \
		cd ${CURL}; CPPFLAGS="-I${INSTALL_DIR}/include -I${INCLUDE_DIR}" LDFLAGS="-L${LIBRARY_DIR} -L${INSTALL_DIR}/lib -Wl,-rpath-link=$(INSTALL_DIR)/lib" LIBS="-laxtls" ./configure --without-ssl --with-axtls --enable-ipv6 --prefix=${INSTALL_DIR} --host=${NVT_HOST}; make -j$(MULTI_CORES); \
	fi
endif
	@cd ${CURL}; make install;
	@cd ${INSTALL_DIR}/bin; rm curl-config
	@rm -rf ${INSTALL_DIR}/lib/pkgconfig
	@echo $(NVT_CURL_SSL) > ${CURL}/.nvt_finish

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@if [ -x ${INSTALL_DIR}/include ]; then \
		cp -arvf ${INSTALL_DIR}/include ${LIBRARY_DIR}; \
	fi
	@if [ -x ${INSTALL_DIR}/lib ]; then \
		mkdir -p ${LIBRARY_DIR}/output; \
		cp -arvf ${INSTALL_DIR}/lib/*.* ${LIBRARY_DIR}/output; \
	fi
	@if [ -x ${INSTALL_DIR}/usr/lib ]; then \
		cp -arvf ${INSTALL_DIR}/usr/lib/*.* ${LIBRARY_DIR}/output/usr/lib; \
	fi

clean:
	@echo ">>>>>>>>>>>>>>>>>>> Remove >>>>>>>>>>>>>>>>>>>"
	@rm -rf ${INSTALL_DIR}
	@rm -rf ${XML2} ${AXTLS} ${FUSE} ${FREETYPE} ${FFMPEG} ${LIBNL} ${SQLITE3} ${OPENSSL} ${CURL} ${UNWIND} ${LIBP11}
