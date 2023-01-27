declare -A toolchains=(
	# arm-ca9-linux-gnueabihf-6.5
	[arm-ca9-linux-gnueabihf-6.5, host]="arm-ca9-linux-gnueabihf"
	[arm-ca9-linux-gnueabihf-6.5, sysroot]="usr/arm-ca9-linux-gnueabihf/sysroot"
	[arm-ca9-linux-gnueabihf-6.5, compile]="usr/bin/arm-ca9-linux-gnueabihf-"
	# arm-ca9-linux-uclibcgnueabihf-6.5
	[arm-ca9-linux-uclibcgnueabihf-6.5, host]="arm-linux"
	[arm-ca9-linux-uclibcgnueabihf-6.5, sysroot]="usr/arm-ca9-linux-uclibcgnueabihf/sysroot"
	[arm-ca9-linux-uclibcgnueabihf-6.5, compile]="usr/bin/arm-linux-"
	# arm-ca9-linux-gnueabihf-8.4
	[arm-ca9-linux-gnueabihf-8.4, host]="arm-ca9-linux-gnueabihf"
	[arm-ca9-linux-gnueabihf-8.4, sysroot]="usr/arm-ca9-linux-gnueabihf/sysroot"
	[arm-ca9-linux-gnueabihf-8.4, compile]="usr/bin/arm-ca9-linux-gnueabihf-"
	# arm-ca9-linux-uclibcgnueabihf-8.4: Not support perf user space app debug
	[arm-ca9-linux-uclibcgnueabihf-8.4, host]="arm-linux"
	[arm-ca9-linux-uclibcgnueabihf-8.4, sysroot]="usr/arm-ca9-linux-uclibcgnueabihf/sysroot"
	[arm-ca9-linux-uclibcgnueabihf-8.4, compile]="usr/bin/arm-linux-"
	# arm-ca9-linux-uclibcgnueabihf-8.4.01: Support perf user space app debug
	[arm-ca9-linux-uclibcgnueabihf-8.4.01, host]="arm-linux"
	[arm-ca9-linux-uclibcgnueabihf-8.4.01, sysroot]="usr/arm-ca9-linux-uclibcgnueabihf/sysroot"
	[arm-ca9-linux-uclibcgnueabihf-8.4.01, compile]="usr/bin/arm-linux-"
	# arm-ca53-linux-gnueabihf-7.4
	[arm-ca53-linux-gnueabihf-7.4, host]="arm-ca53-linux-gnueabihf"
	[arm-ca53-linux-gnueabihf-7.4, sysroot]="usr/arm-ca53-linux-gnueabihf/sysroot"
	[arm-ca53-linux-gnueabihf-7.4, compile]="usr/bin/arm-ca53-linux-gnueabihf-"
	# aarch64-ca53-linux-gnueabihf-8.4
	[aarch64-ca53-linux-gnueabihf-8.4, host]="aarch64-ca53-linux-gnu"
	[aarch64-ca53-linux-gnueabihf-8.4, sysroot]="usr/aarch64-ca53-linux-gnu/sysroot"
	[aarch64-ca53-linux-gnueabihf-8.4, compile]="usr/bin/aarch64-ca53-linux-gnu-"
	# aarch64-ca53-linux-gnueabihf-8.4.01
	[aarch64-ca53-linux-gnueabihf-8.4.01, host]="aarch64-ca53-linux-gnu"
	[aarch64-ca53-linux-gnueabihf-8.4.01, sysroot]="usr/aarch64-ca53-linux-gnu/sysroot"
	[aarch64-ca53-linux-gnueabihf-8.4.01, compile]="usr/bin/aarch64-ca53-linux-gnu-"
	# gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi
	[gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi, host]="arm-eabi"
	[gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi, sysroot]=""
	[gcc-linaro-6.4.1-2018.05-x86_64_arm-eabi, compile]="bin/arm-eabi-"
	# gcc-6.5-newlib-2.4-2019.11-arm-ca9-eabihf
	[gcc-6.5-newlib-2.4-2019.11-arm-ca9-eabihf, host]="arm-eabihf"
	[gcc-6.5-newlib-2.4-2019.11-arm-ca9-eabihf, sysroot]=""
	[gcc-6.5-newlib-2.4-2019.11-arm-ca9-eabihf, compile]="bin/arm-eabihf-"
)

#----------------------------------------------------------------------
# Machine Dependent Options
#----------------------------------------------------------------------
# -march=armv8-a    : Generate code that will run on ARMV8
# -mtune=cortex-a53 : Optimize for cortex-a53
# -mfpu=neon-fp-armv8 : Do not use floating-point coprocessor instructions
# -mfloat-abi=hard  : To indicate that NEON variables must be passed in general purpose registers
# -march=armv7-a    : Generate code that will run on ARMV7
# -mtune=cortex-a9  : Optimize for cortex-a9
# -mfpu=neon        : Do not use floating-point coprocessor instructions, simd + vfpv3
# -mfloat-abi=hard  : To indicate that NEON variables must be passed in general purpose registers
#----------------------------------------------------------------------
# C Language Options
#----------------------------------------------------------------------
# -fno-builtin  : Don't recognize built-in functions that do not begin with `__builtin_' as prefix
# -ffreestanding: Assert that compilation takes place in a freestanding environment
#----------------------------------------------------------------------
# Code Generation Options
#----------------------------------------------------------------------
# -fno-common   : The compiler should place uninitialized global variables in the data section of the object file, rather than generating them as common blocks
# -fshort-wchar : Override the underlying type for `wchar_t' to be `short unsigned int' instead of the default for the target.
#----------------------------------------------------------------------

declare -A compiler_flags=(
	# Linux, cortex-a9
	[Linux, cortex-a9, c]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
	[Linux, cortex-a9, cxx]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
	[Linux, cortex-a9, s]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon"
	# Linux, cortex-a53
	[Linux, cortex-a53, c]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
	[Linux, cortex-a53, cxx]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
	[Linux, cortex-a53, s]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8"
	# Linux, cortex-a53x64
	[Linux, cortex-a53x64, c]="-march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1 "
	[Linux, cortex-a53x64, cxx]="-march=armv8-a -mtune=cortex-a53 -ftree-vectorize -fno-builtin -fno-common -Wformat=1"
	[Linux, cortex-a53x64, s]="-march=armv8-a -mtune=cortex-a53"
	# rtos, cortex-a9
	[rtos, cortex-a9, c]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -nostartfiles -Wall -Winline -Wno-missing-braces  -Wpointer-arith -Wsign-compare -Wstrict-prototypes -Wundef -Werror -fno-exceptions -fno-common -fno-optimize-sibling-calls  -fno-strict-aliasing -fshort-wchar -O2 -fPIC -ffunction-sections -fdata-sections -fno-omit-frame-pointer"
	[rtos, cortex-a9, cxx]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -nostartfiles -Wall -Winline -Wno-missing-braces  -Wpointer-arith -Wsign-compare -Wundef -Werror -fno-common -fno-optimize-sibling-calls  -fno-strict-aliasing -fshort-wchar -O2 -fPIC -ffunction-sections -fdata-sections -fno-omit-frame-pointer"
	[rtos, cortex-a9, s]="-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -nostartfiles -Werror"
	# [rtos, cortex-a53
	[rtos, cortex-a53, c]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -ftree-vectorize -nostartfiles -Wall -Winline -Wno-missing-braces  -Wpointer-arith -Wsign-compare -Wstrict-prototypes -Wundef -Werror -fno-exceptions -fno-common -fno-optimize-sibling-calls  -fno-strict-aliasing -fshort-wchar -O2 -fPIC -ffunction-sections -fdata-sections -fno-omit-frame-pointer"
	[rtos, cortex-a53, cxx]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard -ftree-vectorize -nostartfiles -Wall -Winline -Wno-missing-braces  -Wpointer-arith -Wsign-compare -Wundef -Werror -fno-exceptions -fno-common -fno-optimize-sibling-calls  -fno-strict-aliasing -fshort-wchar -O2 -fPIC -ffunction-sections -fdata-sections -fno-omit-frame-pointer"
	[rtos, cortex-a53, s]="-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard  -ftree-vectorize -nostartfiles -Werror"
)

#####################################################################################################################
#
#	Add for OS type: Linux or RTOS etc....
#
#####################################################################################################################
function add_prjcfgconfig()
{
	unset LUNCH_PROJ_CONFIG_MENU_CHOICES
	add_lunch_proj_config_combo Linux
	add_lunch_proj_config_combo rtos
}

#####################################################################################################################
#
#	Add for toolchain type: glibc, uclibc, linaro, newlib....etc
#
#####################################################################################################################
function add_lunch_toolchain_menu()
{
	unset LUNCH_TOOLCHAIN_MENU_CHOICES
	if [ $NVT_PRJCFG_CFG == "Linux" ]; then
		add_lunch_toolchain_combo arm-ca9-linux-gnueabihf-8.4
		#add_lunch_toolchain_combo arm-ca9-linux-uclibcgnueabihf-8.4
		add_lunch_toolchain_combo arm-ca9-linux-uclibcgnueabihf-8.4.01
	elif [ $NVT_PRJCFG_CFG == "rtos" ]; then
		add_lunch_toolchain_combo gcc-6.5-newlib-2.4-2019.11-arm-ca9-eabihf
	else
		echo -e "\e[1;45mERROR: We can't find your OS type!!!!!\r\n";
	fi
}

#####################################################################################################################
#
#	Kernel upgrade
#
#####################################################################################################################
function kernel_upgrade()
{
	export KERNELDIR=${LINUX_BUILD_BSP}/linux-4.19
}

#####################################################################################################################
#
#	Uboot upgrade
#
#####################################################################################################################
function uboot_upgrade()
{
	export UBOOT_DIR=${LINUX_BUILD_BSP}/u-boot-2019.04
}

#####################################################################################################################
#
#	To config the codebase system environmenr parameters
#
#####################################################################################################################
function set_stuff_for_environment()
{
	chk_for_the_same_codebase
	ret=$?
	if [ -z $NVT_PRJCFG_CFG ] || [ $ret -ne 1 ] ; then
		NVT_PRJCFG_CFG=$DEFAULT_NVT_ACTIVE_PROJ
	fi
	export LINUX_BUILD_TOP=$PWD
	export LINUX_BUILD_BSP=${LINUX_BUILD_TOP}/BSP
	export LINUX_BUILD_CODE=${LINUX_BUILD_TOP}/code
	export ROOTFS_DIR=${LINUX_BUILD_BSP}/root-fs
	export UBOOT_DIR=${LINUX_BUILD_BSP}/u-boot
	export TOYBOX_DIR=${LINUX_BUILD_BSP}/toybox
	export OPTEE_DIR=${LINUX_BUILD_BSP}/optee
	if [ $NVT_PRJCFG_CFG == "Linux" ]; then
		export KERNELDIR=${LINUX_BUILD_BSP}/linux-kernel
	elif [ $NVT_PRJCFG_CFG == "rtos" ]; then
		export KERNELDIR=${LINUX_BUILD_BSP}/rtos/amazon-freertos
	fi
	export BUSYBOX_DIR=${LINUX_BUILD_BSP}/busybox
	export TOYBOX_DIR=${LINUX_BUILD_BSP}/toybox
	export APP_DIR=${LINUX_BUILD_CODE}/application
	export LIBRARY_DIR=${LINUX_BUILD_CODE}/lib
	export INCLUDE_DIR=${LINUX_BUILD_CODE}/lib/include
	export NVT_HDAL_DIR=${LINUX_BUILD_CODE}/hdal
	export NVT_VOS_DIR=${LINUX_BUILD_CODE}/vos
	export NVT_RTOS_MAIN_DIR=${LINUX_BUILD_CODE}/rtos-main
	export SAMPLE_DIR=${LINUX_BUILD_CODE}/sample

	if [ $NVT_PRJCFG_CFG == "rtos" ]; then
		export NVT_DRIVER_DIR=${LINUX_BUILD_CODE}/driver/${SDK_CODENAME}
	else
		export NVT_DRIVER_DIR=${LINUX_BUILD_CODE}/driver
	fi
	export TOOLS_DIR=${LINUX_BUILD_TOP}/tools
	export BUILD_DIR=${LINUX_BUILD_TOP}/build
	export SHELL=/bin/bash
	export MAKE=${BUILD_DIR}/nvt-tools/make-4.1
	export OUTPUT_DIR=${LINUX_BUILD_TOP}/output
	export LOGS_DIR=${LINUX_BUILD_TOP}/logs
	export CONFIG_DIR=${LINUX_BUILD_TOP}/configs
	export NVT_DSP_DIR=${LINUX_BUILD_TOP}/dsp
	export NVT_LINUX_VER="4.19.91"
	if [ -z $NVT_CROSS ]; then
		export NVT_CROSS=${DEFAULT_NVT_CROSS}
	fi
	export NVT_MULTI_CORES_FLAG=-j`grep -c ^processor /proc/cpuinfo`
	setpaths
	setsymbolic
}

#####################################################################################################################
#
#	To config the toolchain path and compile cflags
#
#####################################################################################################################
function setpath_toolchain_config()
{
	if [ -z $NVT_PRJCFG_CFG ]; then
		echo -e "\e[1;45mERROR: We can't find your OS type in setpath toolchain config!!!!!\r\n"
		return;
	fi

	if [ -d /opt/ivot/$NVT_CROSS ]; then
		export CROSS_TOOLCHAIN_DIR=/opt/ivot
	else
		export CROSS_TOOLCHAIN_DIR=/opt
	fi
	export CROSS_TOOLCHAIN_PATH=${CROSS_TOOLCHAIN_DIR}/${NVT_CROSS}
	export CROSS_TOOLCHAIN_BIN_PATH=${CROSS_TOOLCHAIN_PATH}/usr/bin
	export NVT_HOST=${toolchains[${NVT_CROSS}, host]}
	export SYSROOT_PATH=${CROSS_TOOLCHAIN_PATH}/${toolchains[${NVT_CROSS}, sysroot]}
	export CROSS_COMPILE=${CROSS_TOOLCHAIN_PATH}/${toolchains[${NVT_CROSS}, compile]}
	export LINUX_BUILD_PATH=${CROSS_TOOLCHAIN_PATH}/bin
	export PATH=${CROSS_TOOLCHAIN_BIN_PATH}:${PATH}

	if [[ $LINUX_CPU_TYPE == *"x64" ]]; then
		export ARCH=arm64
	else
		export ARCH=arm
	fi

	if [ $NVT_PRJCFG_CFG == "Linux" ]; then
		export PLATFORM_CFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${LINUX_CPU_TYPE}, c]} -D_BSP_${SDK_CODENAME^^}_"
		export PLATFORM_CXXFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${LINUX_CPU_TYPE}, cxx]} -D_BSP_${SDK_CODENAME^^}_"
		export PLATFORM_AFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${LINUX_CPU_TYPE}, s]} -D_BSP_${SDK_CODENAME^^}_"
		export UBOOT_CROSS_COMPILE=${CROSS_COMPILE}
	elif [ $NVT_PRJCFG_CFG == "rtos" ]; then
		export PLATFORM_CFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${RTOS_CPU_TYPE}, c]} -D_BSP_${SDK_CODENAME^^}_ -D__FREERTOS -I${KERNELDIR}/lib/include -I${KERNELDIR}/lib/include/private -I${KERNELDIR}/arch/arm/mach-nvt-${SDK_CODENAME}/include/mach"
		export PLATFORM_CXXFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${RTOS_CPU_TYPE}, cxx]} -D_BSP_${SDK_CODENAME^^}_ -D__FREERTOS -I${KERNELDIR}/lib/include -I${KERNELDIR}/lib/include/private -I${KERNELDIR}/arch/arm/mach-nvt-${SDK_CODENAME}/include/mach"
		export PLATFORM_AFLAGS="${compiler_flags[${NVT_PRJCFG_CFG}, ${RTOS_CPU_TYPE}, s]} -D_BSP_${SDK_CODENAME^^}_ -D__FREERTOS -I${KERNELDIR}/lib/include -I${KERNELDIR}/lib/include/private -I${KERNELDIR}/arch/arm/mach-nvt-${SDK_CODENAME}/include/mach"
		export UBOOT_CROSS_COMPILE_PATH=${CROSS_TOOLCHAIN_DIR}/${DEFAULT_NVT_UBOOT_CROSS_FOR_RTOS}
		export UBOOT_CROSS_COMPILE=${UBOOT_CROSS_COMPILE_PATH}/${toolchains[${DEFAULT_NVT_UBOOT_CROSS_FOR_RTOS}, compile]}
		if [ -z "`echo $LD_LIBRARY_PATH | grep ${UBOOT_CROSS_COMPILE_PATH}/usr/`" ]; then
			if [ ! -z "${LD_LIBRARY_PATH}" ]; then
				export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${UBOOT_CROSS_COMPILE_PATH}/usr/local/lib
			else
				export LD_LIBRARY_PATH=${UBOOT_CROSS_COMPILE_PATH}/usr/local/lib
			fi
		fi
	fi

	if [ "$NVT_FPGA" == "ON" ]; then
		export PLATFORM_CFLAGS="${PLATFORM_CFLAGS} -D_NVT_FPGA_"
		export PLATFORM_AFLAGS="${PLATFORM_AFLAGS} -D_NVT_FPGA_"
	fi
	if [ "$NVT_EMULATION" == "ON" ]; then
		export PLATFORM_CFLAGS="${PLATFORM_CFLAGS} -D_NVT_EMULATION_"
		export PLATFORM_AFLAGS="${PLATFORM_AFLAGS} -D_NVT_EMULATION_"
	fi
	if [ "$NVT_RUN_CORE2" == "ON" ]; then
		export PLATFORM_CFLAGS="${PLATFORM_CFLAGS} -D_NVT_RUN_CORE2_"
		export PLATFORM_AFLAGS="${PLATFORM_AFLAGS} -D_NVT_RUN_CORE2_"
	fi
    if [ "$OLD_IPP" == "ON" ]; then
		export PLATFORM_CFLAGS="${PLATFORM_CFLAGS} -D_OLD_IPP_"
		export PLATFORM_AFLAGS="${PLATFORM_AFLAGS} -D_OLD_IPP_"
	fi	
}

#####################################################################################################################
#
#	To config the export includes NVT_GCOV and NVT_KGCOV for Code Coverage tool
#
#####################################################################################################################
function set_gcov()
{
	export NVT_GCOV="-fprofile-arcs -ftest-coverage -lgcov --coverage"
	export NVT_KGCOV="GCOV_PROFILE=y"
}
