#ifndef __SEN_DET_DRV_H__
#define __SEN_DET_DRV_H__
#if defined __KERNEL__
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/clk.h>
#endif

#include "kwrap/debug.h"
#include "kflow_common/isf_flow_def.h"
#include "kflow_common/isf_flow_ioctl.h"
#include "kflow_videocapture/isf_vdocap.h"
#include "kflow_videocapture/ctl_sen.h"
#include "kflow_videocapture/ctl_sie.h"

#define SEN_SUPPORT_IMX415	1
#define SEN_SUPPORT_OS05A	1
#define SEN_SUPPORT_OS12D	0
#define SEN_SUPPORT_GC4653	1
#define SEN_SUPPORT_GC4663  0
#define SEN_SUPPORT_OS08A	0
#define SEN_SUPPORT_SC401AI 1
#define SEN_SUPPORT_OS04C	1
#define SEN_SUPPORT_SC3336  1

enum SEN_DET_TBL {
#if SEN_SUPPORT_IMX415
	SEN_DET_TBL_IMX415,
#endif
#if SEN_SUPPORT_OS05A
	SEN_DET_TBL_OS05A,
#endif
#if SEN_SUPPORT_OS12D
	SEN_DET_TBL_OS12D,
#endif
#if SEN_SUPPORT_GC4653
	SEN_DET_TBL_GC4653,
#endif
#if SEN_SUPPORT_OS08A
	SEN_DET_TBL_OS08A,
#endif
#if SEN_SUPPORT_GC4663
	SEN_DET_TBL_GC4663,
#endif
#if SEN_SUPPORT_SC401AI
	SEN_DET_TBL_SC401AI,
#endif
#if SEN_SUPPORT_OS04C
	SEN_DET_TBL_OS04C,
#endif
#if SEN_SUPPORT_SC3336
	SEN_DET_TBL_SC3336,
#endif
};

#define SEN_DET_TBL_NONE	0xffffffff

#define SEN_DET_NUM		32

typedef enum _HD_RESULT {
    HD_OK =                             0,
	HD_ERR_FAIL =                       -19, ///< already executed and failed
    HD_ERR_INV =                        -35, ///< invalid argument passed
	HD_ERR_NOT_AVAIL =                  -41, ///< resources not available
} HD_RESULT;

typedef struct sen_det_entry{
    char sen_name[32];
    UINT32 option;
	UINT32 sen_tbl_idx;
} SEN_DET_ENTRY;

typedef struct sen_det_info {
	SEN_DET_ENTRY *entry; // sensor name
	CTL_SEN_INIT_CFG_OBJ *sen_cfg_obj;
} SEN_DET_DRV_INFO, *PSEN_DET_DRV_INFO;

typedef enum _SEN_DET_OPTION {
	SEN_DET_OPTION_NONE       = 0,
	SEN_DET_OPTION_STREAM_ON  = 1,
	SEN_DET_OPTION_END		  = 0xffffffff,
} SEN_DET_OPTION;


extern int nvt_sen_det_drv_start(void);
extern char *nvt_set_det_drv_get_result(int id);
extern int nvt_sen_det_drv_release(PSEN_DET_DRV_INFO psen_det_info);
extern int nvt_sen_det_drv_open(PSEN_DET_DRV_INFO psen_det_info);
extern int nvt_sen_det_drv_init(PSEN_DET_DRV_INFO psen_det_info);
extern int nvt_sen_det_init_write_tbl(PCTL_SEN_OBJ p_sen_Ctl_obj,int tbl_idx);
extern int nvt_sen_det_sen_list(void);
//int nvt_sen_det_drv_remove(PSEN_DET_MODULE_INFO psen_det_info);
//int nvt_sen_det_drv_suspend(SEN_DET_MODULE_INFO *psen_det_info);
//int nvt_sen_det_drv_resume(SEN_DET_MODULE_INFO *psen_det_info);
//int nvt_sen_det_drv_inital(PSEN_DET_MODULE_INFO psen_det_info);
#endif

