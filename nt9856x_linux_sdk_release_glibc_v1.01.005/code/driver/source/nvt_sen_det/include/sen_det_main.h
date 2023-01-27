#ifndef __SEN_DET_MAIN_H__
#define __SEN_DET_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "sen_det_drv.h"

#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_sen_det_module"

typedef struct sen_det_module_info {
	SEN_DET_DRV_INFO drv_info;

	struct device *pdevice[MODULE_MINOR_COUNT];
	dev_t dev_id;
	
	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
	struct proc_dir_entry *pproc_sen_name_entry;
} SEN_DET_MODULE_INFO, *PSEN_DET_MODULE_INFO;


#endif
