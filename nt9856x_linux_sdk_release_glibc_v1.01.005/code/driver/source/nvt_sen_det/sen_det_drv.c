#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
//#include "hd_type.h"

//#include "kflow_videocapture/isf_vdocap.h"
//#include "kflow_videocapture/ctl_sie.h"
//#include "kflow_videocapture/ctl_sen.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "kflow_videocapture/isf_vdocap.h"
#include "kflow_videocapture/ctl_sen.h"
#include "kflow_videocapture/ctl_sie.h"
#include "sen_det_drv.h"


/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
extern ER nvt_ctl_sen_init_cfg(CTL_SEN_ID id);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef struct _NVT_SEN_DET_CONFIG {
	char sen_name[32];
	UINT32 sen_mode;
	unsigned int frc;
	CTL_SEN_INIT_CFG_OBJ *sen_cfg_obj;
	VDOCAP_SEN_INIT_OPTION *sen_init_option;
}NVT_SEN_DET_CONFIG;

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
int i_event_flag = 0;
char det_result_name_1[32];
char det_result_name_2[32];
SEN_DET_ENTRY *sen_list = NULL;
CTL_SEN_INIT_CFG_OBJ *sen_det_cfg;

extern int sensor1_i2c;
extern int sensor2_i2c;
UINT32 ctl_sen_init_buf[512] = {0};
/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
static INT32 ctl_sen_init_cfg(UINT32 id,PCTL_SEN_OBJ p_sen_ctl_obj, NVT_SEN_DET_CONFIG  *p_sen_cfg)
{
    CTL_SEN_INIT_CFG_OBJ cfg_obj = {0};
  //  VDOCAP_SEN_INIT_CFG sen_init = {0};
	CTL_SEN_PINMUX pinmux[3] = {0};
    INT32 ret = HD_OK;

    //------------- HDAL flow -----------------------------
   // memcpy(sen_init.driver_name, p_sen_cfg->sen_dev.driver_name, VDOCAP_SEN_NAME_LEN);
//    if (FALSE == set_if_cfg(&sen_init.sen_init_cfg.if_cfg, &p_sen_cfg->sen_dev))
  //      return HD_ERR_PARAM;
    //if (FALSE == set_cmd_if_cfg(&sen_init.sen_init_cfg.cmd_if_cfg, &p_sen_cfg->sen_dev))
      //  return HD_ERR_PARAM;
   // if (FALSE == set_pin_cfg(&sen_init.sen_init_cfg.pin_cfg, &p_sen_cfg->sen_dev))
     //   return HD_ERR_PARAM;
    //memcpy(&sen_init.sen_init_option, &p_sen_cfg->sen_dev.option, sizeof(VDOCAP_SEN_INIT_OPTION));

    //-------------- isf_vdocap flow ----------------------------------------------------------
    //_dump_sen_init(&sen_init);
    #if 1 // hard code, Ben need to modify
        if (id == 0) {
            pinmux[0].func = 4; //PIN_FUNC_SENSOR;
			pinmux[0].cfg = 0x220;
        } else {
            pinmux[0].func = 5; //PIN_FUNC_SENSOR2
        	pinmux[0].cfg = 0x1020;
        }
		pinmux[0].pnext = &pinmux[1];

		pinmux[1].func = 6; // PIN_FUNC_MIPI_LVDS;	
		if (id == 0)
		{
        	pinmux[1].cfg = 0xF01;
		}
		else {
            pinmux[1].cfg = 0xC02;
		}
        pinmux[1].pnext = &pinmux[2];

        pinmux[2].func = 7; //PIN_FUNC_I2C;
		if (id == 0)
		{
			if(sensor1_i2c == 0)
				pinmux[2].cfg = 0x1;
			else
        		pinmux[2].cfg = 0x10;
		}
		else
		{
            if(sensor2_i2c == 0)
                pinmux[2].cfg = 0x1;
            else
                pinmux[2].cfg = 0x10;
		}

        pinmux[2].pnext = NULL;

        cfg_obj.pin_cfg.pinmux.func = pinmux[0].func;
        cfg_obj.pin_cfg.pinmux.cfg = pinmux[0].cfg;
        cfg_obj.pin_cfg.pinmux.pnext = pinmux[0].pnext;
#else
        cfg_obj.pin_cfg.pinmux.sensor_pinmux = p_sen_cfg->sen_cfg_obj->pin_cfg.pinmux.sensor_pinmux;//sen_init.sen_init_cfg.pin_cfg.pinmux.sensor_pinmux;
        cfg_obj.pin_cfg.pinmux.serial_if_pinmux = p_sen_cfg->sen_cfg_obj->pin_cfg.pinmux.serial_if_pinmux;//sen_init.sen_init_cfg.pin_cfg.pinmux.serial_if_pinmux;
        cfg_obj.pin_cfg.pinmux.cmd_if_pinmux = p_sen_cfg->sen_cfg_obj->pin_cfg.pinmux.cmd_if_pinmux;//sen_init.sen_init_cfg.pin_cfg.pinmux.cmd_if_pinmux;
#endif
	//printk("sen_pinmux = %x, serial_if_pinmux = %x, cmd_if_pinmux = %x\r\n",cfg_obj.pin_cfg.pinmux.sensor_pinmux,cfg_obj.pin_cfg.pinmux.serial_if_pinmux,cfg_obj.pin_cfg.pinmux.cmd_if_pinmux);
    cfg_obj.pin_cfg.clk_lane_sel = p_sen_cfg->sen_cfg_obj->pin_cfg.clk_lane_sel;//CTL_SEN_CLANE_SEL_CSI0_USE_C2;
	if(id == 0 )
	{
    	cfg_obj.pin_cfg.sen_2_serial_pin_map[0] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[0];
		cfg_obj.pin_cfg.sen_2_serial_pin_map[1] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[1];
		cfg_obj.pin_cfg.sen_2_serial_pin_map[2] = 0xFFFFFFFF;
		cfg_obj.pin_cfg.sen_2_serial_pin_map[3] = 0xFFFFFFFF;
	}
	else
	{
		cfg_obj.pin_cfg.sen_2_serial_pin_map[0] = 0xFFFFFFFF;
        cfg_obj.pin_cfg.sen_2_serial_pin_map[1] = 0xFFFFFFFF;
        cfg_obj.pin_cfg.sen_2_serial_pin_map[2] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[2];
        cfg_obj.pin_cfg.sen_2_serial_pin_map[3] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[3];
	}
	cfg_obj.pin_cfg.sen_2_serial_pin_map[4] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[4];
	cfg_obj.pin_cfg.sen_2_serial_pin_map[5] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[5];
	cfg_obj.pin_cfg.sen_2_serial_pin_map[6] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[6];
	cfg_obj.pin_cfg.sen_2_serial_pin_map[7] = p_sen_cfg->sen_cfg_obj->pin_cfg.sen_2_serial_pin_map[7];
	//printk("sen_clk_lan_sel = %x, sen_2_serial_pin_map = %x %x %x %x\r\n",cfg_obj.pin_cfg.clk_lane_sel,cfg_obj.pin_cfg.sen_2_serial_pin_map[0],cfg_obj.pin_cfg.sen_2_serial_pin_map[1],cfg_obj.pin_cfg.sen_2_serial_pin_map[2],cfg_obj.pin_cfg.sen_2_serial_pin_map[3]);

    cfg_obj.if_cfg.type = p_sen_cfg->sen_cfg_obj->if_cfg.type;//VDOCAP_SEN_IF_TYPE_MIPI;
	//printk("if_cfg type = %x\r\n",cfg_obj.if_cfg.type);
#if 0
    cfg_obj.if_cfg.tge.tge_en = sen_init.sen_init_cfg.if_cfg.tge.tge_en;
    cfg_obj.if_cfg.tge.swap = sen_init.sen_init_cfg.if_cfg.tge.swap;
    cfg_obj.if_cfg.tge.sie_vd_src = sen_init.sen_init_cfg.if_cfg.tge.sie_vd_src;
    cfg_obj.cmd_if_cfg.vx1.en = sen_init.sen_init_cfg.cmd_if_cfg.vx1.en;
    cfg_obj.cmd_if_cfg.vx1.if_sel = sen_init.sen_init_cfg.cmd_if_cfg.vx1.if_sel;
    cfg_obj.cmd_if_cfg.vx1.ctl_sel = sen_init.sen_init_cfg.cmd_if_cfg.vx1.ctl_sel;
    cfg_obj.cmd_if_cfg.vx1.tx_type = sen_init.sen_init_cfg.cmd_if_cfg.vx1.tx_type;
#endif
#if 1 // hard code, Ben need to modify
    cfg_obj.drvdev = 0;
    if (id == 0) {
        if (cfg_obj.if_cfg.type == CTL_SEN_IF_TYPE_LVDS) {
            cfg_obj.drvdev |= CTL_SEN_DRVDEV_LVDS_0;
        } else if (cfg_obj.if_cfg.type == CTL_SEN_IF_TYPE_MIPI) {
            cfg_obj.drvdev |= CTL_SEN_DRVDEV_CSI_0;
        }
    } else if (id == 1) {
        if (cfg_obj.if_cfg.type == CTL_SEN_IF_TYPE_LVDS) {
            cfg_obj.drvdev |= CTL_SEN_DRVDEV_LVDS_1;
        } else if (cfg_obj.if_cfg.type == CTL_SEN_IF_TYPE_MIPI) {
            cfg_obj.drvdev |= CTL_SEN_DRVDEV_CSI_1;
        }
    }
#endif
    ret = p_sen_ctl_obj->init_cfg((CHAR *)p_sen_cfg->sen_name, &cfg_obj);
    if(ret) {
        DBG_ERR("sen init_cfg failed(%d)!\r\n", ret);
        ret = ISF_ERR_INVALID_VALUE;
    }
    return ret;
}

char *nvt_set_det_drv_get_result(int id)
{
	switch(id)
	{
		case 0:
		default:
			return det_result_name_1;
		case 1:
			if(sensor2_i2c < 0)
				return NULL;
			else
				return det_result_name_2;
	}
}

int nvt_set_det_get_result(int id, NVT_SEN_DET_CONFIG  *p_sen_config, UINT32 opt,UINT32 sen_tbl_idx)
{
	INT32 ret = HD_OK;
    PCTL_SEN_OBJ p_sen_ctl_obj = ctl_sen_get_object(id);
//    CTL_SEN_CHGMODE_OBJ chgmode_obj = {0};
	CTL_SEN_GET_PROBE_SEN_PARAM probe_para = {0};
    //UINT32 frame_rate;
    UINT32 ctl_sen_buf_size;

    ctl_sen_buf_size = ctl_sen_buf_query(2);
	printk("query size = %d\n",ctl_sen_buf_size);
    //set addr 0 to let ctl_set use internal kmalloc
    ret = ctl_sen_init((UINT32)ctl_sen_init_buf, ctl_sen_buf_size);
    if (ret) {
        DBG_ERR("ctl sen init fail!(%d)\r\n", ret);
        return -1;
    }

    if (p_sen_ctl_obj == NULL || p_sen_ctl_obj->open == NULL) {
        DBG_ERR("no ctl_sen obj(%d)\r\n", (UINT32)id);
        return -1;
    }
//  DBG_DUMP("id(%d) sen_mode=%d\r\n", (UINT32)id, (UINT32)p_user->sen_mode);
    ctl_sen_init_cfg(id,p_sen_ctl_obj, p_sen_config);
    //sensor open
	ret = p_sen_ctl_obj->open();

    if (ret != CTL_SEN_E_OK) {
        switch(ret) {
        case CTL_SEN_E_MAP_TBL:
            DBG_ERR("no sen driver\r\n");
            ret = HD_ERR_NOT_AVAIL;
            break;
        case CTL_SEN_E_SENDRV:
            DBG_ERR("sensor driver error\r\n");
            ret = HD_ERR_FAIL;//map to HD_ERR_FAIL
            break;
        default:
            DBG_ERR("sensor open fail(%d)\r\n", ret);
            ret = HD_ERR_INV;
            break;
        }
        goto s_close;
    }
    p_sen_ctl_obj->pwr_ctrl(CTL_SEN_PWR_CTRL_TURN_ON);
	//printk("power on\r\n");
	if( (opt & SEN_DET_OPTION_STREAM_ON) == 0)
	{
		ret = p_sen_ctl_obj->get_cfg(CTL_SEN_CFGID_GET_PROBE_SEN,(void*)(&probe_para));
		if(probe_para.probe_rst == 0 )
		{
			ret = HD_OK;
			goto s_power_down;
		}
		else
		{
			ret = HD_ERR_NOT_AVAIL;
        	goto s_power_down;
		}
	}
#if 0
    frame_rate = 1000;//100*GET_HI_UINT16(p_sen_config->frc)/GET_LO_UINT16(p_sen_config->frc);
    chgmode_obj.mode_sel = CTL_SEN_MODESEL_AUTO;
    chgmode_obj.manual_info.frame_rate = frame_rate;
    chgmode_obj.manual_info.sen_mode = p_sen_config->sen_mode;
    chgmode_obj.output_dest = 0;//p_sen_config->shdr_map&0xFFFF;
	printk("frame_rate=%d,_sen_mode=%d\r\n",chgmode_obj.manual_info.frame_rate,chgmode_obj.manual_info.sen_mode);
   // ret = p_sen_ctl_obj->chgmode(chgmode_obj);
#endif
	if(nvt_sen_det_init_write_tbl(p_sen_ctl_obj,sen_tbl_idx) != HD_OK)
	{
		ret = HD_ERR_NOT_AVAIL;
		goto s_power_down;
	}
	//printk("change mode =%d\r\n",ret);
	ret = p_sen_ctl_obj->get_cfg(CTL_SEN_CFGID_GET_PROBE_SEN,(void*)(&probe_para));
	//printk("2 get probe = %d ret = %d\r\n",probe_para.probe_rst,ret);
	if(probe_para.probe_rst == 0)
		ret = HD_OK;
	else
		ret = HD_ERR_NOT_AVAIL;
s_power_down:
	ret |= p_sen_ctl_obj->pwr_ctrl(CTL_SEN_PWR_CTRL_TURN_OFF);
//	printk("power off = %d\r\n",ret);
s_close:
	ret |= p_sen_ctl_obj->close();
//	printk("close = %d\r\n",ret);
//s_close:	
	ret |= ctl_sen_uninit();
//	printk("sen_uninit = %d\r\n",ret);
	
	return ret;
}

int nvt_sen_det_sen_list(void)
{
	int ret = HD_OK;
	int idx;
	printk("avalible sensor name =      \r\n");
    for(idx = 0;idx < SEN_DET_NUM ; idx ++)
    {
        if(sen_list[idx].option == SEN_DET_OPTION_END)
            break;
		printk("[%d] sensor name : %s\r\n",idx,sen_list[idx].sen_name);
    }
	return ret;
}

int nvt_sen_det_drv_start(void)
{
	int idx;
	NVT_SEN_DET_CONFIG det_config;
	det_config.frc = 30000;
	det_config.sen_mode = 0;
	for(idx = 0;idx < SEN_DET_NUM ; idx ++)
	{
		if(sen_list[idx].option == SEN_DET_OPTION_END)
			break;
		memcpy(det_config.sen_name,sen_list[idx].sen_name,32);
		det_config.sen_cfg_obj = sen_det_cfg; 
		printk("sensor 1: [%d] det name = %s\n",idx,sen_list[idx].sen_name);//det_config.sen_name);
		if(nvt_set_det_get_result(0,&det_config,sen_list[idx].option,sen_list[idx].sen_tbl_idx) == HD_OK)
		{
			memcpy(det_result_name_1,sen_list[idx].sen_name,32);
			printk("get sen = %s\n",det_result_name_1);
			break;
		}
	
	}

	if(sensor2_i2c < 0 )
		return 0;

    for(idx = 0;idx < SEN_DET_NUM ; idx ++)
    {
        if(sen_list[idx].option == SEN_DET_OPTION_END)
            break;

        memcpy(det_config.sen_name,sen_list[idx].sen_name,32);
        det_config.sen_cfg_obj = sen_det_cfg;
        printk("sensor 2:[%d] det name = %s\n",idx,sen_list[idx].sen_name);//det_config.sen_name);
        if(nvt_set_det_get_result(1,&det_config,sen_list[idx].option,sen_list[idx].sen_tbl_idx) == HD_OK)
        {
        	memcpy(det_result_name_2,sen_list[idx].sen_name,32);
           	printk("get sen = %s\n",det_result_name_2);
            break;
        }
    }	
	return 0;
}


int nvt_sen_det_drv_open(PSEN_DET_DRV_INFO psen_det_info)
{
	sen_list = psen_det_info->entry;
	sen_det_cfg = psen_det_info->sen_cfg_obj;
	return 0;
}

int nvt_sen_det_drv_release(PSEN_DET_DRV_INFO psen_det_module_info)
{

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_sen_det_drv_init(PSEN_DET_DRV_INFO psen_det_info)
{
	int i_ret = 0;

	return i_ret;
}
#if 0
int nvt_sen_det_drv_remove(SEN_DET_MODULE_INFO *psen_det_module_info)
{
	/* Add HW Moduel release operation here*/

	return 0;
}

int nvt_sen_det_drv_suspend(SEN_DET_MODULE_INFO *psen_det_module_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_sen_det_drv_resume(SEN_DET_MODULE_INFO *psen_det_module_info)
{
	nvt_dbg(IND, "\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_sen_det_drv_ioctl(unsigned char uc_if, SEN_DET_MODULE_INFO *psen_det_module_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	int i_ret = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);


	return i_ret;
}

#endif
int nvt_sen_det_drv_inital(PSEN_DET_DRV_INFO psen_det_module_info, unsigned int id)
{
	nvt_dbg(IND, "id %d\r\n", id);
	memset(det_result_name_1,0,sizeof(det_result_name_1));
	memset(det_result_name_2,0,sizeof(det_result_name_2));
	return 0;
}

