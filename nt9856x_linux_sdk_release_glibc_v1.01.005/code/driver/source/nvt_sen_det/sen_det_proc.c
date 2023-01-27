#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "sen_det_proc.h"
#include "sen_det_main.h"


//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     10

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(SEN_DET_DRV_INFO *drv_info, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
PSEN_DET_MODULE_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================

static int nvt_sen_det_start(SEN_DET_DRV_INFO *drv_info, unsigned char argc, char **argv);
static int nvt_set_det_tbl(SEN_DET_DRV_INFO *drv_info, unsigned char argc, char **argv);
//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================

static PROC_CMD cmd_list[] = {
	// keyword          function name
	{ "sen_det",        nvt_sen_det_start           },
	{ "sen_tbl",		nvt_set_det_tbl				}};
//	{ "unreg",          nvt_sen_ext_api_unreg_sendrv         }

#define NUM_OF_CMD (sizeof(cmd_list) / sizeof(PROC_CMD))
static int nvt_set_det_tbl(SEN_DET_DRV_INFO *drv_info, unsigned char argc, char **argv)
{
	nvt_sen_det_sen_list();	
	return 0;
}
static int nvt_sen_det_start(SEN_DET_DRV_INFO *drv_info, unsigned char argc, char **argv)
{
	nvt_sen_det_drv_start();
	return 0;
}

static int nvt_sen_det_proc_cmd_show(struct seq_file *sfile, void *v)
{
	nvt_dbg(IND, "\n");
	return 0;
}

static int nvt_sen_det_proc_cmd_open(struct inode *inode, struct file *file)
{
	nvt_dbg(IND, "\n");
	return single_open(file, nvt_sen_det_proc_cmd_show, &pdrv_info_data->drv_info);
}

static ssize_t nvt_sen_det_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned char loop;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	} else if (len < 1) {
		nvt_dbg(ERR, "Command length is too short!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	nvt_dbg(IND, "CMD:%s\n", cmd_line);

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	for (loop = 0 ; loop < NUM_OF_CMD; loop++) {
		if (strncmp(argv[0], cmd_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
			ret = cmd_list[loop].execute(&pdrv_info_data->drv_info, ucargc - 1, &argv[1]);
			break;
		}
	}

	if (loop >= NUM_OF_CMD) {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
	return -1;
}

static struct file_operations proc_cmd_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_sen_det_proc_cmd_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = nvt_sen_det_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_sen_det_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, "Press ( echo sen_det > /proc/sen_det/cmd ) trigger sen detection \n");
	seq_printf(sfile, "Press ( echo sen_tbl > /proc/sen_det/cmd ) show avalible sensor table \n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int nvt_sen_det_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_sen_det_proc_help_show, NULL);
}

static struct file_operations proc_help_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_sen_det_proc_help_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};


//=============================================================================
// proc "sen_name" file operation functions
//=============================================================================
static int nvt_sen_det_proc_sen_name_show(struct seq_file *sfile, void *v)
{	
	char *sensor_name;
	sensor_name = nvt_set_det_drv_get_result(1);

	if(sensor_name == NULL)
	{	
		seq_printf(sfile, "[1] %s \r\n",nvt_set_det_drv_get_result(0));
	}
	else
	{	
		sensor_name = NULL;
		sensor_name = nvt_set_det_drv_get_result(0);
		if(sensor_name == NULL)
			seq_printf(sfile, "[2] %s\r\n",nvt_set_det_drv_get_result(1));
		else
			seq_printf(sfile, "[1] %s [2] %s\r\n",nvt_set_det_drv_get_result(0),nvt_set_det_drv_get_result(1));
	}
    return 0;
}

static int nvt_sen_det_proc_sen_name_open(struct inode *inode, struct file *file)
{
    return single_open(file, nvt_sen_det_proc_sen_name_show, NULL);
}

static struct file_operations proc_sen_name_fops = {
    .owner  = THIS_MODULE,
    .open   = nvt_sen_det_proc_sen_name_open,
    .release = single_release,
    .read   = seq_read,
    .llseek = seq_lseek,
};

int nvt_sen_det_proc_init(PSEN_DET_MODULE_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("sen_det", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;


	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_help_entry = pentry;


    pentry = proc_create("sen_name", S_IRUGO | S_IXUGO, pmodule_root, &proc_sen_name_fops);
    if (pentry == NULL) {
        nvt_dbg(ERR, "failed to create proc sen_name!\n");
        ret = -EINVAL;
        goto remove_cmd;
    }
    pdrv_info->pproc_sen_name_entry = pentry;	

	pdrv_info_data = pdrv_info;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_sen_det_proc_remove(PSEN_DET_MODULE_INFO pdrv_info)
{
	proc_remove(pdrv_info->pproc_help_entry);
	proc_remove(pdrv_info->pproc_cmd_entry);
	proc_remove(pdrv_info->pproc_sen_name_entry);
	proc_remove(pdrv_info->pproc_module_root);
	return 0;
}
