#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <asm/signal.h>
#include "sen_det_main.h"
#include "sen_det_proc.h"
#include "sen_det_drv.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================

//=============================================================================
// Global variable
//=============================================================================
SEN_DET_MODULE_INFO *psen_det_module_info;
SEN_DET_ENTRY sen_det_candicate[SEN_DET_NUM] = {0};
CTL_SEN_INIT_CFG_OBJ sen_det_cfg_obj;
int sensor1_i2c = 0;
int sensor2_i2c = -1;
module_param_named(sensor1_i2c, sensor1_i2c, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sensor1_i2c, "sensor_1 i2c");
module_param_named(sensor2_i2c, sensor2_i2c, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sensor2_i2c, "sensor_2 i2c");
//=============================================================================
// function declaration
//=============================================================================
int __init nvt_sen_det_module_init(void);
void __exit nvt_sen_det_module_exit(void);

//=============================================================================
// function define
//=============================================================================

static int init_sen_tbl(void)
{
	int idx;
	idx = 0;
	
#if SEN_SUPPORT_OS12D
	memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[0].sen_name));
	memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_os12d40",strlen("nvt_sen_os12d40")+1);
	sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
	sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_OS12D;
	idx ++;
#endif

#if SEN_SUPPORT_IMX415
	memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_imx415",strlen("nvt_sen_imx415")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
	sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_IMX415;
	idx ++;
#endif

#if SEN_SUPPORT_OS05A	
	memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_os05a10",strlen("nvt_sen_os05a10")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_OS05A;
	idx ++;
#endif

#if SEN_SUPPORT_GC4663
    memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_gc4663",strlen("nvt_sen_gc4663")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_GC4663;
	idx ++;
#endif

#if SEN_SUPPORT_OS08A
    memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_os08a10",strlen("nvt_sen_os08a10")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_OS08A;
	idx ++;
#endif

#if SEN_SUPPORT_GC4653
    memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_gc4653",strlen("nvt_sen_gc4653")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_GC4653;
	idx ++;
#endif

#if SEN_SUPPORT_SC401AI
    memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_sc401ai",strlen("nvt_sen_sc401ai")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_NONE;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_SC401AI;
    idx ++;
#endif
	
#if SEN_SUPPORT_OS04C
	memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_os04c10",strlen("nvt_sen_os04c10")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_STREAM_ON;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_OS04C;
    idx ++;
#endif

#if SEN_SUPPORT_SC3336
    memset(sen_det_candicate[idx].sen_name,0,sizeof(sen_det_candicate[1].sen_name));
    memcpy(sen_det_candicate[idx].sen_name,"nvt_sen_sc3336",strlen("nvt_sen_sc3336")+1);
    sen_det_candicate[idx].option = SEN_DET_OPTION_NONE;
    sen_det_candicate[idx].sen_tbl_idx = SEN_DET_TBL_SC3336;
    idx ++;
#endif
	sen_det_candicate[idx].option = SEN_DET_OPTION_END;


	return 0;
}

static int init_sen_config(void)
{
	//sen_det_cfg_obj.pin_cfg.pinmux.sensor_pinmux = 0x220;
    //sen_det_cfg_obj.pin_cfg.pinmux.serial_if_pinmux = 0xf04;
    //sen_det_cfg_obj.pin_cfg.pinmux.cmd_if_pinmux = 0x1;//0x10; //I2C
    sen_det_cfg_obj.pin_cfg.clk_lane_sel = CTL_SEN_CLANE_SEL_CSI0_USE_C2;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[0] = 0;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[1] = 1;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[2] = 0;//0xffffffff;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[3] = 1;//0xffffffff;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[4] = 0xffffffff;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[5] = 0xffffffff;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[6] = 0xffffffff;
    sen_det_cfg_obj.pin_cfg.sen_2_serial_pin_map[7] = 0xffffffff;

    sen_det_cfg_obj.if_cfg.type = CTL_SEN_IF_TYPE_MIPI;
	return 0;
}

int __init nvt_sen_det_module_init(void)
{
	int ret;

	nvt_dbg(WRN, "\n");

	psen_det_module_info = kzalloc(sizeof(SEN_DET_MODULE_INFO), GFP_KERNEL);
	if (!psen_det_module_info) {
		nvt_dbg(ERR, "failed to allocate memory\n");
		return -ENOMEM;
	}

	printk("i2c=[%d,%d]\r\n",sensor1_i2c,sensor2_i2c);
	if(sensor1_i2c == -1)
		sensor1_i2c = 0;
	ret = nvt_sen_det_proc_init(psen_det_module_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_INIT;
	}

	ret = nvt_sen_det_drv_init(&psen_det_module_info->drv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in sen det drv init.\n");
		goto FAIL_PROC;
	}

	init_sen_tbl();
	psen_det_module_info->drv_info.entry = sen_det_candicate;
	init_sen_config();
	psen_det_module_info->drv_info.sen_cfg_obj = &sen_det_cfg_obj;
	nvt_sen_det_drv_open(&psen_det_module_info->drv_info);
	nvt_sen_det_drv_start();

	return ret;

FAIL_PROC:
	nvt_sen_det_proc_remove(psen_det_module_info);

FAIL_INIT:
	kfree(psen_det_module_info);
	psen_det_module_info = NULL;
	return 0;
}

void __exit nvt_sen_det_module_exit(void)
{
	nvt_dbg(WRN, "\n");

	nvt_sen_det_proc_remove(psen_det_module_info);

	kfree(psen_det_module_info);
	psen_det_module_info = NULL;

}

module_init(nvt_sen_det_module_init);
module_exit(nvt_sen_det_module_exit);

MODULE_VERSION("1.03.000.0000");
MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("sen_det driver");
MODULE_LICENSE("GPL");
