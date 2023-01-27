# DO NOT USE SAPCE AT LEFT SIDE AND RIGHT SIDE OF EQUAL.
# e.g DO NOT USE 'NVT_FPGA = OFF', but rather 'NVT_FPGA=OFF'
export NVT_FPGA=OFF
export NVT_EMULATION=OFF
export NVT_RUN_CORE2=OFF
export SDK_CODENAME=na51089

# this setting only for RTOS.
# cortex-a53
# cortex-a9
export RTOS_CPU_TYPE=cortex-a9

# this setting only for Linux
# cortex-a53
# cortex-a9
# cortex-a53x64
export LINUX_CPU_TYPE=cortex-a9

# this setting only for New IPP porting.
export OLD_IPP=OFF
