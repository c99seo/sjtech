/**
	@brief Sample code of video rtsp.\n

	@file plug_nn_sc.c

	@author Photon Lin

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "hdal.h"
#include "hd_debug.h"
#include "vendor_isp.h"

// NOTE: NNSC lib include
#include "nnsc_lib.h"
// NOTE: NNSC net include
#include "vendor_ai.h"
#include "vendor_ai_util.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai_cpu_postproc.h"

// platform dependent
#if defined(__LINUX)
#include <pthread.h> //for pthread API
#define MAIN(argc, argv) int main(int argc, char** argv)
#define GETCHAR() getchar()
#endif

///////////////////////////////////////////////////////////////////////////////
// NOTE: Function control of NNSC NET
#define CALC_NNSC_DURATION              14    // skip n frames and process once
#define ACCESS_NNSC_NET_EN               1
#define ACCESS_NNSC_LIB_EN               1
#define OSG_FUNC_EN                      1
#define SAVE_YUV_EN                      0

// NOTE: customer setting of NNSC NET
#define NNSC_NET_MODEL_NAME_V_2_02_000   "/mnt/sd/para/nvt_model_sc_2_02_220105.bin"
#define NNSC_NET_MODEL_SIZE_V_2_02_000   15315488

#define NNSC_NET_VERSION_V_2_02_000      3
#define NNSC_NET_VERSION                 NNSC_NET_VERSION_V_2_02_000

#if (NNSC_NET_VERSION == NNSC_NET_VERSION_V_2_02_000)
#define NNSC_NET_MODEL_NAME              NNSC_NET_MODEL_NAME_V_2_02_000
#define NNSC_NET_MODEL_SIZE              NNSC_NET_MODEL_SIZE_V_2_02_000
#endif
#define NNSC_NET_JOB_METHOD              1
#define NNSC_NET_WAIT_TIME               0
#define NNSC_NET_BUF_METHOD              0
//#define NNSC_NET_LABEL_NAME              "/mnt/sd/accuracy/labels.txt"
#define NNSC_NET_LABEL_NAME              "/mnt/sd/accuracy/labels_20211122.txt"

#define NNSC_NET_MODEL_IN_PATH           0
#define NNSC_NET_MODEL_PROC_PATH         0

///////////////////////////////////////////////////////////////////////////////
#define HD_VIDEOCAP_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define HD_VIDEOPROC_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define HD_VIDEOENC_PATH(dev_id, in_id, out_id) (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))

//header
#define DBGINFO_BUFSIZE() (0x200)
//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

typedef struct _VIDEO_PLUG_STREAM {
	// (1)
	HD_PATH_ID cap_path;

	// (2)
	HD_PATH_ID proc_path;

	// (3)
	HD_PATH_ID enc_path;
} VIDEO_PLUG_STREAM;

static BOOL is_nnsc_init;
static BOOL is_nnsc_run;
pthread_t  nnsc_thread_id;
static BOOL nnsc_thread_run;
static BOOL nnsc_net_msg_en;
static BOOL nnsc_lib_func_en;
static UINT32 nnsc_lib_msg_type;

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC net usage
#define VENDOR_AI_CFG 0x000F0000  //vendor ai config
#define AI_RGB_BUFSIZE(w, h) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))
#define NET_PATH_ID UINT32

typedef struct _MEM_PARM {
	UINT32 pa;
	UINT32 va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;

typedef struct _NET_IN_CONFIG {
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
} NET_IN_CONFIG;

typedef struct _NET_IN {
	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI_BUF src_img;
} NET_IN;

typedef struct _NET_PROC_CONFIG {
	CHAR model_filename[256];
	INT32 binsize;
	int job_method;
	int job_wait_ms;
	int buf_method;
	void *p_share_model;
	CHAR label_filename[256];
} NET_PROC_CONFIG;

typedef struct _NET_PROC {
	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;
	CHAR out_class_labels[MAX_CLASS_NUM * VENDOR_AIS_LBL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
} NET_PROC;

typedef struct _VIDEO_NN {
	// (1) input 
	HD_PATH_ID in_path;

	// (2) network 
	HD_PATH_ID net_path;
} VIDEO_NN;

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC lib usage
#if (ACCESS_NNSC_NET_EN)
#define TONE_LV_SMOOTH_FACTOR     7

static NNSC_PARAM nnsc_param = {
	{
		{5, 200}, // NNSC_TYPE_BACKLIGHT
		{5, 200}, // NNSC_TYPE_FOGGY
		{5, 200}, // NNSC_TYPE_GRASS
		{5, 200}, // NNSC_TYPE_SKIN_COLOR
		{5, 200}, // NNSC_TYPE_SNOWFIELD
	}
};

static NNSC_TEST test_param ={
	.enable = FALSE,
	.mode_type = NNSC_TYPE_BACKLIGHT,
};

static SCD_CONTROL scd_control = {
	.enable = TRUE,
	.auto_enable = TRUE,
	.manual_stable = FALSE,
};

static SCD_PARAM scd_param = {
	.stable_sensitive = 7,
	.change_sensitive = 5,
	.g_diff_num_th = 200,
};

NNSC_ADJ nnsc_adj = {0};
AET_EXP_RATIO exp_ratio = {0};
AWBT_GREEN_REMOVE green_remove = {0};
AWBT_SKIN_REMOVE skin_remove = {0};
IQT_DARK_ENH_RATIO dark_enh_ratio = {0};
IQT_CONTRAST_ENH_RATIO contrast_enh_ratio = {0};
IQT_GREEN_ENH_RATIO green_enh_ratio = {0};
IQT_SKIN_ENH_RATIO skin_enh_ratio = {0};
IQT_SHDR_TONE_LV shdr_tone_lv = {0};
#endif

///////////////////////////////////////////////////////////////////////////////
#if (OSG_FUNC_EN)
#define SHM_FILE_NAME "pq_osg"
#define SHM_FILE_SIZE 512
#define OSG_ITEM_NUM  8
#define OSG_TITLE_MAX 20

typedef struct _OSG_UPDATE_INFO {
	UINT16 value[OSG_ITEM_NUM];
	UINT8 title_text[OSG_ITEM_NUM][OSG_TITLE_MAX];
	UINT16 title_color[OSG_ITEM_NUM];
} OSG_UPDATE_INFO;

OSG_UPDATE_INFO osg_update_info = {
	.title_text = {
		{" BL Human"},
		{" BL Scene"},
		{"    Foggy"},
		{"    Grass"},
		{"GrassLand"},
		{"SkinColor"},
		{"   Normal"},
		{"SnowField"},
	},
	.title_color = {
		0xFFF0,     // BL_Human
		0xFEE0,     // BL_Scene
		0xF00F,     // Foggy 
		0xF0F0,     // Grass
		0xF0C0,     // GrassLand
		0xFC68,     // HandColor 
		0xFEEE,     // Normal
		0xFFFF,     // SnowField
	}
};
#endif

///////////////////////////////////////////////////////////////////////////////
// NOTE: NNSC net usage
#if (ACCESS_NNSC_NET_EN)
VIDEO_NN stream_nn = {NNSC_NET_MODEL_IN_PATH, NNSC_NET_MODEL_PROC_PATH}; // NN stream
// net in
NET_IN_CONFIG in_cfg = {0};
// net proc
NET_PROC_CONFIG net_cfg = {
	.model_filename = NNSC_NET_MODEL_NAME,
	.binsize =        NNSC_NET_MODEL_SIZE,
	.job_method =     NNSC_NET_JOB_METHOD,   // default 0: sequential
	.job_wait_ms =    NNSC_NET_WAIT_TIME,    // async mode
	.buf_method =     NNSC_NET_BUF_METHOD,   // 0: allocate each buffer, 1: shrink buffer space (default)
	.label_filename = NNSC_NET_LABEL_NAME,
};

static UINT32 nnsc_score_num = 0;
static NNSC_SCORE nnsc_score = {{0}};
static SCD_STATISTICS scd_statistic = {{0}};
static NET_IN g_in[16] = {0};
static NET_PROC g_net[16] = {0};
UINT32 outlayer_buf_pa = 0;
VENDOR_AI_BUF outlayer_buf_va = {0};
UINT32 softmax_loc_layer_float_pa = 0;
FLOAT *softmax_loc_layer_float_va = NULL;
UINT32 outlayer_path_list[256] = {0};
UINT32 outlayer_num = 0;
UINT32 outlayer_length;

static HD_RESULT NNSCnet_open_model_label(NET_PATH_ID net_path);
static HD_RESULT NNSCnet_close(NET_PATH_ID net_path);
static HD_RESULT NNSCnet_get_mem(MEM_PARM *mem_parm, UINT32 size);
static HD_RESULT NNSCnet_rel_mem(MEM_PARM *mem_parm);
static HD_RESULT NNSCnet_alloc_mem(MEM_PARM *mem_parm, CHAR* name, UINT32 size);
static HD_RESULT NNSCnet_free_mem(MEM_PARM *mem_parm);

static HD_RESULT NNSCnet_init_input(void)
{
	HD_RESULT ret = HD_OK;
	int  i;
	
	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

static HD_RESULT NNSCnet_uninit_input(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_set_input_config(NET_PATH_ID net_path, NET_IN_CONFIG* p_in_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	
	memcpy((void*)&p_net->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: buf=(%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	
	return ret;
}

static HD_RESULT NNSCnet_open_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_close_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_start_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_stop_input(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT NNSCnet_init_engine(void)
{
	HD_RESULT ret = HD_OK;
	int i;
	
	// config extend engine plugin, process scheduler
	{
		UINT32 schd = VENDOR_AI_PROC_SCHD_FAIR;
		vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE, vendor_ai_cpu1_get_engine());
		printf("vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE..\r\n");

		vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD, &schd);
		printf("vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD..\r\n");

	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		printf("vendor_ai_init fail (%d) \r\n", ret);
		return ret;
	}

	for (i = 0; i < 16; i++) {
		NET_PROC* p_net = g_net + i;
		p_net->proc_id = i;
	}
	return ret;
}

static HD_RESULT NNSCnet_uninit_engine(void)
{
	HD_RESULT ret;
	if ((ret =vendor_ai_uninit()) != HD_OK) {
		printf("vendor_ai_uninit fail (%d) \r\n", ret);
		return ret;
	}
	return HD_OK;
}

static HD_RESULT NNSCnet_init_module(void)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_init_input()) != HD_OK)
		return ret;
	if ((ret = NNSCnet_init_engine()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_uninit_module(void)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_uninit_input()) != HD_OK)
		return ret;
	if ((ret = NNSCnet_uninit_engine()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_open_module(VIDEO_NN *p_stream)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_open_input(p_stream->in_path)) != HD_OK)
		return ret;
	// load model
	if ((ret = NNSCnet_open_model_label(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT NNSCnet_close_module(VIDEO_NN *p_stream)
{
	HD_RESULT ret;
	if ((ret = NNSCnet_close_input(p_stream->in_path)) != HD_OK)
		return ret;
	if ((ret = NNSCnet_close(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static UINT32 NNSCnet_load_model(CHAR *filename, UINT32 va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINT32 model_addr = va;

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail \r\n", filename);
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	file_size = ALIGN_CEIL_4(ftell(fd));
	fseek (fd, 0, SEEK_SET);

	read_size = fread((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d \r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);

	printf("load model (%s) ok\r\n", filename);

	return read_size;
}

static HD_RESULT NNSCnet_load_label(UINT32 addr, UINT32 line_len, const CHAR *filename)
{
	FILE *fd;
	CHAR *p_line = (CHAR *)addr;

	fd = fopen(filename, "r");
	if (!fd) {
		printf("load label(%s) fail \r\n", filename);
		return HD_ERR_NG;
	}

	while (fgets(p_line, line_len, fd) != NULL) {
		p_line[strlen(p_line) - 1] = '\0'; // remove newline character
		p_line += line_len;
	}

	if (fd) {
		fclose(fd);
	}

	printf("load label(%s) ok\r\n", filename);

	return HD_OK;
}

static HD_RESULT NNSCnet_set_config(NET_PATH_ID net_path, NET_PROC_CONFIG* p_proc_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));

	printf("proc_id(%u) set net_cfg: job-opt=(%u,%d), buf-opt(%u), binsize = %d \r\n", 
		proc_id,
		p_net->net_cfg.job_method,
		(int)p_net->net_cfg.job_wait_ms,
		p_net->net_cfg.buf_method,
		p_net->net_cfg.binsize);
	
	// set buf opt
	{
		VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
		cfg_buf_opt.method = p_net->net_cfg.buf_method;
		cfg_buf_opt.ddr_id = DDR_ID0;
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);
	}

	// set job opt
	{
		VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
		cfg_job_opt.method = p_net->net_cfg.job_method;
		cfg_job_opt.wait_ms = p_net->net_cfg.job_wait_ms;
		cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);
	}

	return ret;
}

static HD_RESULT NNSCnet_alloc_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	VENDOR_AI_NET_CFG_WORKBUF wbuf = {0};

	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) get VENDOR_AI_NET_PARAM_CFG_WORKBUF fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}
	ret = NNSCnet_alloc_mem(&p_net->io_mem, "ai_io_buf", wbuf.size);
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc ai_io_buf fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}

	wbuf.pa = p_net->io_mem.pa;
	wbuf.va = p_net->io_mem.va;
	wbuf.size = p_net->io_mem.size;
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail \r\n", proc_id);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu \r\n", wbuf.pa, wbuf.va, wbuf.size);

	return ret;
}

static HD_RESULT NNSCnet_free_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	
	NET_PROC* p_net = g_net + net_path;
	
	if (p_net->io_mem.pa && p_net->io_mem.va) {
		NNSCnet_free_mem(&p_net->io_mem);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_open_model_label(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	UINT32 loadsize = 0;

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("proc_id(%u) input model is null\r\n", proc_id);
		return 0;
	}

	NNSCnet_get_mem(&p_net->proc_mem, p_net->net_cfg.binsize);
	//load model BIN
	loadsize = NNSCnet_load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);
	if (loadsize <= 0) {
		printf("proc_id (%u) input model load fail: %s\r\n", proc_id, p_net->net_cfg.model_filename);
		return 0;
	}

	// load label
	ret = NNSCnet_load_label((UINT32)p_net->out_class_labels, VENDOR_AIS_LBL_LEN, p_net->net_cfg.label_filename);
	if (ret != HD_OK) {
		printf("proc_id (%u), load_label (%d) \r\n", proc_id, ret);
		return HD_ERR_FAIL;
	}

	// set model
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL, (VENDOR_AI_NET_CFG_MODEL*)&p_net->proc_mem);

	// open
	vendor_ai_net_open(proc_id);

	return ret;
}

static HD_RESULT NNSCnet_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

	if ((ret = NNSCnet_free_io_buf(net_path)) != HD_OK) {
		return ret;
	}
	
	// close
	ret = vendor_ai_net_close(proc_id);
	
	NNSCnet_rel_mem(&p_net->proc_mem);

	return ret;
}

static HD_RESULT NNSCnet_get_mem(MEM_PARM *mem_parm, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINT32 pa = 0;
	void  *va = NULL;
	HD_COMMON_MEM_VB_BLK blk;

	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, size, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		printf("hd_common_mem_get_block fail \r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);

	/* Release buffer if fail*/
	if (va == 0) {
		ret = hd_common_mem_munmap(va, size);
		if (ret != HD_OK) {
			printf("mem unmap fail \r\n");
			return ret;
		}
	}

	mem_parm->pa = pa;
	mem_parm->va = (UINT32)va;
	mem_parm->size = size;
	mem_parm->blk = blk;

	return HD_OK;
}

static HD_RESULT NNSCnet_rel_mem(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	/* Release in buffer */
	if (mem_parm->va) {
		ret = hd_common_mem_munmap((void *)mem_parm->va, mem_parm->size);
		if (ret != HD_OK) {
			printf("mem_uninit : (mem_parm->va)hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	ret = hd_common_mem_release_block(mem_parm->blk);
	if (ret != HD_OK) {
		printf("mem_uninit : (mem_parm->pa)hd_common_mem_release_block fail.\r\n");
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT NNSCnet_alloc_mem(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINT32 pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINT32)va;
	mem_parm->size = size;
	mem_parm->blk  = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT NNSCnet_free_mem(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	//free private pool
	ret =  hd_common_mem_free(mem_parm->pa, (void *)mem_parm->va);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT NNSCnet_start(VIDEO_NN *p_stream)
{
	HD_RESULT ret = HD_OK;

	ret = vendor_ai_net_start(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) start fail !!\r\n", p_stream->net_path);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_stop(VIDEO_NN *p_stream)
{
	HD_RESULT ret = HD_OK;
	
	//stop: should be call after last time proc
	ret = vendor_ai_net_stop(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id (%u) stop fail \n", p_stream->net_path);
	}
	
	return ret;
}

static HD_RESULT NNSCnet_get_ai_outlayer_list(UINT32 proc_id, UINT32 *outlayer_num, UINT32 *outlayer_path_list)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI_NET_INFO net_info = {0};

	// get output layer number
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
	if (HD_OK != ret) {
		printf("proc_id (%lu) get info fail \n", proc_id);
		return ret;
	}

	*outlayer_num = net_info.out_buf_cnt;

	// get path_list
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, outlayer_path_list);
	if (HD_OK != ret) {
		printf("proc_id(%u) get outlayer_path_list fail \n", proc_id);
	}

	return ret;
}

static HD_RESULT NNSCnet_get_ai_outlayer_by_path_id(INT32 proc_id, UINT32 path_id, VENDOR_AI_BUF *p_outbuf)
{
	HD_RESULT ret = HD_OK;
	// get out buf by path_id
	ret = vendor_ai_net_get(proc_id, path_id, p_outbuf);
	if (HD_OK != ret) {
		printf("proc_id(%u) get AI_OUTBUF fail \n", proc_id);
		return ret;
	}

	return ret;
}

static HD_RESULT NNSCnet_assign_frame(NET_PATH_ID net_path, UINT32 pa, UINT32 va, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	p_net->input_mem.pa = pa;
	p_net->input_mem.va = va;
	p_net->input_mem.size = size;
	p_net->src_img.width = p_net->in_cfg.w;
	p_net->src_img.height = p_net->in_cfg.h;
	p_net->src_img.channel = p_net->in_cfg.c;
	p_net->src_img.line_ofs= p_net->in_cfg.loff;
	p_net->src_img.fmt= p_net->in_cfg.fmt;
	
	p_net->src_img.pa = pa;
	p_net->src_img.va = va;
	p_net->src_img.sign = MAKEFOURCC('A','B','U','F');
	p_net->src_img.size = p_net->in_cfg.loff * p_net->in_cfg.h * 3 / 2;

	// set net input image
	ret = vendor_ai_net_set(net_path, VENDOR_AI_NET_PARAM_IN(0, 0), (void *)&(p_net->src_img));
	if (HD_OK != ret) {
		printf("proc_id (%u) push input fail \n", net_path);
		return ret;
	}

	return HD_OK;
}
#endif

#if (OSG_FUNC_EN)
static HD_RESULT shm_write(void)
{
	char *data;
	int fd;

	fd = shm_open(SHM_FILE_NAME, O_RDWR, 0777);
	if (fd < 0)
	{
		printf("shm_write: %s open fail \n", SHM_FILE_NAME);
		return HD_ERR_NG;
	}

	ftruncate(fd, SHM_FILE_SIZE);

	data = (char*)mmap(NULL, SHM_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!data) {
		printf("shm_write: mmap fail \n");
		close(fd);
		return HD_ERR_NG;
	}

	memcpy(data, &osg_update_info, sizeof(OSG_UPDATE_INFO));

	munmap(data , SHM_FILE_SIZE);
	close(fd);

	return HD_OK;
}
#endif

///////////////////////////////////////////////////////////////////////////////
static INT32 get_choose_int(void)
{
	char buf[256];
	INT val, rt;

	rt = scanf("%d", &val);

	if (rt != 1) {
		printf("Invalid option. Try again.\n");
		clearerr(stdin);
		fgets(buf, sizeof(buf), stdin);
		val = -1;
	}

	return val;
}

static void *access_nn_thread(void *arg)
{
	VIDEO_PLUG_STREAM* p_stream0 = (VIDEO_PLUG_STREAM *)arg;
	HD_VIDEO_FRAME video_frame = {0};
	HD_RESULT ret = HD_OK;
	ISPT_WAIT_VD wait_vd = {0};
	UINT32 new_shdr_tone_lv;
	static UINT32 pre_shdr_tone_lv = 50;
	UINT32 i;
	#if (SAVE_YUV_EN || ACCESS_NNSC_NET_EN)
	UINT32 phy_addr_yuv, vir_addr_yuv;
	UINT32 yuv_size;
	#define PHY2VIRT_YUV(pa) (vir_addr_yuv + ((pa) - phy_addr_yuv))
	#endif
	static UINT32 pre_exp_ratio = 65535;

	#if (ACCESS_NNSC_NET_EN)
	ISPT_CA_DATA isp_ca = {0};
	INT rt = 0;
	#endif

	#if (SAVE_YUV_EN)
	CHAR save_yuv_path[100];
	FILE *fp = NULL;
	static UINT32 save_yuv_cnt = 0;
	#endif

	wait_vd.id = 0;
	wait_vd.timeout = 100;

	while (nnsc_thread_run) {
		for (i = 0; i < CALC_NNSC_DURATION; i++) {
			vendor_isp_get_common(ISPT_ITEM_WAIT_VD, &wait_vd);
		}

		ret = hd_videoproc_pull_out_buf(p_stream0->proc_path, &video_frame, -1); // -1 = blocking mode
		if (ret != HD_OK) {
			if (ret != HD_ERR_UNDERRUN)
				printf("proc_pull fial (%d) \r\n", ret);
			return 0;
		}

		// NOTE: access nnsc net here
		#if (ACCESS_NNSC_NET_EN)
		phy_addr_yuv = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_yuv == 0) {
			printf("blk2pa fail, blk = 0x%x \n", video_frame.blk);
			goto skip;
		}
		yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(video_frame.pw[0], video_frame.ph[0], HD_VIDEO_PXLFMT_YUV420);
		vir_addr_yuv = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_yuv, yuv_size);
		if (vir_addr_yuv == 0) {
			printf("nnsc net, mmap fail. \n");
			goto skip;
		}

		ret = NNSCnet_assign_frame(stream_nn.net_path, phy_addr_yuv + DBGINFO_BUFSIZE(), PHY2VIRT_YUV(video_frame.phy_addr[0]), video_frame.loff[0]*video_frame.ph[0] + video_frame.loff[1]*video_frame.ph[1]);
		if (ret != HD_OK) {
			printf("NNSCnet_assign_frame fail, path = %u \n", stream_nn.net_path);
			goto skip;
		}

		// do net proc
		ret = vendor_ai_net_proc(stream_nn.net_path);
		if (ret != HD_OK) {
			printf("vendor_ai_net_proc, path = %u \n", stream_nn.net_path);
			goto skip;
		}

		// get net result
		NNSCnet_get_ai_outlayer_by_path_id(stream_nn.net_path, outlayer_path_list[0], &outlayer_buf_va);
		hd_common_mem_flush_cache((VOID *)(outlayer_buf_va.va), outlayer_buf_va.size);
		ret = vendor_ai_cpu_util_fixed2float((VOID *)(outlayer_buf_va.va), outlayer_buf_va.fmt, softmax_loc_layer_float_va, outlayer_buf_va.scale_ratio, outlayer_length);
		nnsc_score_num = outlayer_buf_va.channel;
		if (nnsc_score_num == NNSC_SCORE_MAX_NUM) {
			for(UINT32 f_cnt = 0; f_cnt < nnsc_score_num; f_cnt++) {
				nnsc_score.score[f_cnt] = (UINT32)((softmax_loc_layer_float_va[f_cnt]* 100 + 0.5));
				if (nnsc_net_msg_en) {
					if (f_cnt == 0) printf("score[%2u] = %3d - backlight_human \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 1) printf("score[%2u] = %3d - backlight_scene \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 2) printf("score[%2u] = %3d - foggy \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 3) printf("score[%2u] = %3d - grass \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 4) printf("score[%2u] = %3d - grass_landscape \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 5) printf("score[%2u] = %3d - hand_color \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 6) printf("score[%2u] = %3d - normal \n", f_cnt, nnsc_score.score[f_cnt]);
					else if (f_cnt == 7) printf("score[%2u] = %3d - snowfield \n", f_cnt, nnsc_score.score[f_cnt]);
					else printf("score[%u] = %d - unknown\n", f_cnt, nnsc_score.score[f_cnt]);
				}
			}
			nnsc_set_score(&nnsc_score);
		} else {
			printf("score number mismatch. NN (%d) and NNSC lib (%d)\r\n", nnsc_score_num, NNSC_SCORE_MAX_NUM);
		}

		// mummap for frame buffer
		ret = hd_common_mem_munmap((void *)vir_addr_yuv, yuv_size);
		if (ret != HD_OK) {
			printf("nnsc net, mnumap fail. \n");
			return 0;
		}
		#endif

		// NOTE: access nnsc lib here
		#if (ACCESS_NNSC_NET_EN)
		if (nnsc_lib_func_en == TRUE) {
			isp_ca.id = 0;
			vendor_isp_get_common(ISPT_ITEM_CA_DATA, &isp_ca);
			memcpy(scd_statistic.ca_r, isp_ca.ca_rslt.r, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			memcpy(scd_statistic.ca_g, isp_ca.ca_rslt.g, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			memcpy(scd_statistic.ca_b, isp_ca.ca_rslt.b, sizeof(UINT16) * NNSC_CA_MAX_WINNUM);
			nnsc_set_scd_statistic(&scd_statistic);

			rt = nnsc_adj_cal(&nnsc_adj);
			if ((rt != HD_OK) && (rt != HD_ERR_ABORT)) {
				if (nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) {
					printf("nnsc_adj_cal fail=%d !! \r\n", rt);
					printf("NNSC score :\r\n");
					#if (ACCESS_NNSC_NET_EN)
					printf("  NNSC_SCORE_BACKLIGHT_HUMAN =      %3d \r\n", nnsc_score.score[NNSC_SCORE_BACKLIGHT_HUMAN]);
					printf("  NNSC_SCORE_BACKLIGHT =            %3d \r\n", nnsc_score.score[NNSC_SCORE_BACKLIGHT]);
					printf("  NNSC_SCORE_FOGGY =                %3d \r\n", nnsc_score.score[NNSC_SCORE_FOGGY]);
					printf("  NNSC_SCORE_GRASS_A =              %3d \r\n", nnsc_score.score[NNSC_SCORE_GRASS_A]);
					printf("  NNSC_SCORE_GRASS_B =              %3d \r\n", nnsc_score.score[NNSC_SCORE_GRASS_B]);
					printf("  NNSC_SCORE_HAND_COLOR =           %3d \r\n", nnsc_score.score[NNSC_SCORE_HAND_COLOR]);
					printf("  NNSC_SCORE_SNOWFIELD =            %3d \r\n", nnsc_score.score[NNSC_SCORE_SNOWFIELD]);
					#else
					printf("  ACCESS_NNSC_NET_EN 0, no nnsc_score \r\n");
					#endif
			
					printf("NNSC param :\r\n");
					printf("  NNSC_TYPE_BACKLIGHT =       {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_BACKLIGHT].sensitive, nnsc_param.param[NNSC_TYPE_BACKLIGHT].adj_strength);
					printf("  NNSC_TYPE_FOGGY =           {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_FOGGY].sensitive, nnsc_param.param[NNSC_TYPE_FOGGY].adj_strength);
					printf("  NNSC_TYPE_GRASS =           {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_GRASS].sensitive, nnsc_param.param[NNSC_TYPE_GRASS].adj_strength);
					printf("  NNSC_TYPE_SKIN_COLOR        {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_SKIN_COLOR].sensitive, nnsc_param.param[NNSC_TYPE_SKIN_COLOR].adj_strength);
					printf("  NNSC_TYPE_SNOWFIELD =       {%3d, %3d} \r\n", nnsc_param.param[NNSC_TYPE_SNOWFIELD].sensitive, nnsc_param.param[NNSC_TYPE_SNOWFIELD].adj_strength);
				}
			} else {
				if ((nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) && (rt == HD_OK)) {
					printf("NNSC output :\r\n");
					printf("NNSC_ADJ_EXP_ENH_RATIO =      %3d \r\n", nnsc_adj.exp_ratio);
					printf("NNSC_ADJ_GREEN_REMOVE_RATIO = %3d \r\n", nnsc_adj.green_remove);
					printf("NNSC_ADJ_SKIN_REMOVE_RATIO =  %3d \r\n", nnsc_adj.skin_remove);
					printf("NNSC_ADJ_DARK_ENH_RATIO =     %3d \r\n", nnsc_adj.dark_enh_ratio);
					printf("NNSC_ADJ_CONTRAST_ENH_RATIO = %3d \r\n", nnsc_adj.contrast_enh_ratio);
					printf("NNSC_ADJ_GREEN_ENH_RATIO =    %3d \r\n", nnsc_adj.green_enh_ratio);
					printf("NNSC_ADJ_SKIN_ENH_RATIO =     %3d \r\n", nnsc_adj.skin_enh_ratio);
				}

				exp_ratio.id = 0;
				exp_ratio.ratio = nnsc_adj.exp_ratio;
				
				if (pre_exp_ratio != exp_ratio.ratio) {
					vendor_isp_set_ae(AET_ITEM_NNSC_EXP_RATIO, &exp_ratio);
					pre_exp_ratio = exp_ratio.ratio;
				}
				
				green_remove.id = 0;
				green_remove.enable = nnsc_adj.green_remove;
				vendor_isp_set_awb(AWBT_ITEM_NNSC_GREEN_REMOVE, &green_remove);
				skin_remove.id = 0;
				skin_remove.enable = nnsc_adj.skin_remove;
				vendor_isp_set_awb(AWBT_ITEM_NNSC_SKIN_REMOVE, &skin_remove);
				dark_enh_ratio.id = 0;
				dark_enh_ratio.ratio = nnsc_adj.dark_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
				contrast_enh_ratio.id = 0;
				contrast_enh_ratio.ratio = nnsc_adj.contrast_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
				green_enh_ratio.id = 0;
				green_enh_ratio.ratio = nnsc_adj.green_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
				skin_enh_ratio.id = 0;
				skin_enh_ratio.ratio = nnsc_adj.skin_enh_ratio;
				vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);

				new_shdr_tone_lv = 50 + dark_enh_ratio.ratio / 2;
				if (new_shdr_tone_lv > 100) {
					new_shdr_tone_lv = 100;
				}
				if (pre_shdr_tone_lv <= new_shdr_tone_lv) {
					pre_shdr_tone_lv = (pre_shdr_tone_lv * TONE_LV_SMOOTH_FACTOR + new_shdr_tone_lv * 1 + TONE_LV_SMOOTH_FACTOR) / (TONE_LV_SMOOTH_FACTOR + 1); // Unconditional carry
				} else {
					pre_shdr_tone_lv = (pre_shdr_tone_lv * TONE_LV_SMOOTH_FACTOR + new_shdr_tone_lv * 1) / (TONE_LV_SMOOTH_FACTOR + 1); // Unconditional chop
				}
				shdr_tone_lv.id = 0;
				shdr_tone_lv.lv = pre_shdr_tone_lv;
				vendor_isp_set_iq(IQT_ITEM_SHDR_TONE_LV, &shdr_tone_lv);
				if (nnsc_lib_msg_type == NNSC_DBG_TYPE_MAIN) {
					printf("shdr_tone_lv =                %3d \r\n", shdr_tone_lv.lv);
				}
			}
		} else {
			exp_ratio.id = 0;
			exp_ratio.ratio = 0;
			if (pre_exp_ratio != exp_ratio.ratio) {
				vendor_isp_set_ae(AET_ITEM_NNSC_EXP_RATIO, &exp_ratio);
				pre_exp_ratio = exp_ratio.ratio;
			}
			green_remove.id = 0;
			green_remove.enable = 0;
			vendor_isp_set_awb(AWBT_ITEM_NNSC_GREEN_REMOVE, &green_remove);
			skin_remove.id = 0;
			skin_remove.enable = 0;
			vendor_isp_set_awb(AWBT_ITEM_NNSC_SKIN_REMOVE, &skin_remove);
			dark_enh_ratio.id = 0;
			dark_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_DARK_ENH_RATIO, &dark_enh_ratio);
			contrast_enh_ratio.id = 0;
			contrast_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_CONTRAST_ENH_RATIO, &contrast_enh_ratio);
			green_enh_ratio.id = 0;
			green_enh_ratio.ratio = 0;
			vendor_isp_set_iq(IQT_ITEM_NNSC_GREEN_ENH_RATIO, &green_enh_ratio);
			skin_enh_ratio.id = 0;
			skin_enh_ratio.ratio = 50;
			vendor_isp_set_iq(IQT_ITEM_NNSC_SKIN_ENH_RATIO, &skin_enh_ratio);

			shdr_tone_lv.id = 0;
			shdr_tone_lv.lv = 50;
			vendor_isp_set_iq(IQT_ITEM_SHDR_TONE_LV, &shdr_tone_lv);
			pre_shdr_tone_lv = 50;
		}
		#endif

		#if (OSG_FUNC_EN)
		for(i = 0; i < NNSC_SCORE_MAX_NUM; i++) {
			#if (ACCESS_NNSC_NET_EN)
			osg_update_info.value[i] = nnsc_score.score[i];
			#else
			osg_update_info.value[i] = 0;
			#endif
		}

		shm_write();

		#endif

		// NOTE: save yuv here
		#if (SAVE_YUV_EN)
		save_yuv_cnt++;
		printf("cnt = %d, w = %d, h = %d, loff = %d \n", save_yuv_cnt, video_frame.pw[0], video_frame.ph[0], video_frame.loff[0]);
		phy_addr_yuv = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
		if (phy_addr_yuv == 0) {
			printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
			return 0;
		}
		yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(video_frame.pw[0], video_frame.ph[0], HD_VIDEO_PXLFMT_YUV420);
		vir_addr_yuv = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_yuv, yuv_size);
		if (vir_addr_yuv == 0) {
			printf("save yuv, mmap fail. \n");
			return 0;
		}

		sprintf(save_yuv_path, "/mnt/sd/save_%d_yuv420_w%d_h%d.yuv", (int)save_yuv_cnt, (int)video_frame.loff[0], (int)video_frame.ph[0]);
		fp = fopen(save_yuv_path, "wb");
		if (fp == NULL) {
			printf("fail to open %s \n", save_yuv_path);
			return 0;
		}

		//save Y plane
		{
			UINT8 *ptr = (UINT8 *)PHY2VIRT_YUV(video_frame.phy_addr[0]);
			UINT32 len = video_frame.loff[0]*video_frame.ph[0];
			if (fp) fwrite(ptr, 1, len, fp);
			if (fp) fflush(fp);
		}
		//save UV plane
		{
			UINT8 *ptr = (UINT8 *)PHY2VIRT_YUV(video_frame.phy_addr[1]);
			UINT32 len = video_frame.loff[1]*video_frame.ph[1];
			if (fp) fwrite(ptr, 1, len, fp);
			if (fp) fflush(fp);
		}

		fclose(fp);

		// mummap for frame buffer
		ret = hd_common_mem_munmap((void *)vir_addr_yuv, yuv_size);
		if (ret != HD_OK) {
			printf("save yuv, mnumap fail. \n");
			return 0;
		}
		#endif

		ret = hd_videoproc_release_out_buf(p_stream0->proc_path, &video_frame);
		if (ret != HD_OK) {
				printf("proc_release fail (%d) \r\n\r\n", ret);
			return 0;
		}
	}

	#if (ACCESS_NNSC_NET_EN)
skip:
	hd_common_mem_free(outlayer_buf_pa, &outlayer_buf_va);
	hd_common_mem_free(softmax_loc_layer_float_pa, softmax_loc_layer_float_va);
	#endif

	#if (SAVE_YUV_EN)
	save_yuv_cnt = 0;
	#endif

	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT32 option, option_1;
	UINT32 trig = 1;
	#if (ACCESS_NNSC_NET_EN)
	ISPT_YUV_INFO yuv_info = {0};
	#endif
	VIDEO_PLUG_STREAM stream[1] = {0}; //0: main stream
	UINT32 nnsc_lib_version;

	if (vendor_isp_init() == HD_ERR_NG) {
		printf("init vendor isp fail \r\n\r\n");
		return 0;
	}

	hd_common_init(1);
	hd_common_mem_init(NULL);

	nnsc_net_msg_en =    FALSE;
	nnsc_lib_func_en =   TRUE;
	nnsc_lib_msg_type =  0;

	while (trig) {
		printf("----------------------------------------\n");
		printf("   1.  Init & config NNSC net \n");
		printf("   3.  Run NNSC \n");
		printf("   5.  Stop & close NNSC net \n");
		printf("----------------------------------------\n");
		printf("   10. Config NNSC NET message \n");
		printf("----------------------------------------\n");
		printf("   20. Get NNSC LIB version \n");
		printf("   21. Config NNSC LIB function \n");
		printf("   22. Config NNSC LIB message \n");
		printf("   23. Config NNSC LIB param \n");
		printf("   24. Config NNSC LIB test mode \n");
		printf("   25. Config NNSC LIB SCD control \n");
		printf("   26. Config NNSC LIB SCD param \n");
		printf("----------------------------------------\n");
		printf("   0.  Quit\n");
		printf("----------------------------------------\n");
		do {
			printf(">> ");
			option = get_choose_int();
		} while (0);

		switch (option) {
		case 1:
			if (is_nnsc_init) {
				is_nnsc_run = FALSE;
				printf("NNSC NET init yet \n");
				break;
			}

			printf("Start of init NNSC NET \n");

			#if (ACCESS_NNSC_NET_EN)
			// print info
			printf("nnsc net, model name: %s, size: %d \n", NNSC_NET_MODEL_NAME, NNSC_NET_MODEL_SIZE);

			// init network modules
			ret = NNSCnet_init_module();
			if (ret != HD_OK) {
				printf("init module for NN fail (%d) \n", ret);
				goto exit;
			}

			// get yuv info
			yuv_info.id = 0;
			yuv_info.yuv_info.pid = 0;
			vendor_isp_get_common(ISPT_ITEM_YUV, &yuv_info);
			vendor_isp_set_common(ISPT_ITEM_YUV, &yuv_info.id);

			// setup with queried yuv info
			in_cfg.w = yuv_info.yuv_info.pw[0];
			in_cfg.h = yuv_info.yuv_info.ph[0];
			in_cfg.c = 2; // 2: YUV420
			in_cfg.loff = yuv_info.yuv_info.loff[0];
			in_cfg.fmt = HD_VIDEO_PXLFMT_YUV420;

			ret = NNSCnet_set_input_config(stream_nn.in_path, &in_cfg);
			if (HD_OK != ret) {
				printf("proc_id(%u) NNSCnet input set config (%d) \n", stream_nn.in_path, ret);
				goto exit;
			}

			// do NN proc config
			ret = NNSCnet_set_config(stream_nn.net_path, &net_cfg);
			if (HD_OK != ret) {
				printf("proc_id(%u) NNSCnet set config (%d) \n", stream_nn.net_path, ret);
				goto exit;
			}

			// in NNSCnet_open_module(), we do:
			// load input image from SD card for test (optional)
			// load network model bin
			// load network label txt
			ret = NNSCnet_open_module(&stream_nn);
			if (ret != HD_OK) {
				printf("nnsc net open module (%d) \n", ret);
				goto exit;
			}
			
			ret = NNSCnet_alloc_io_buf(stream_nn.net_path);
			if (ret != HD_OK) {
				printf("nnsc net alloc io buf (%d) \n", ret);
				goto exit;
			}

			NNSCnet_start_input(stream_nn.in_path);

			// let scene classification network start to work
			ret = NNSCnet_start(&stream_nn);
			if (ret != HD_OK)
			{
				printf("network_user_start() fail (%d) \r\n", ret);
				goto exit;
			}


			NNSCnet_get_ai_outlayer_list(stream_nn.net_path, &outlayer_num, outlayer_path_list);
			printf("output layer number: %d\r\n", outlayer_num);
			if (outlayer_num!=1) {
				printf("output layer number should be 1.\r\n");
				return 0;
			}

			ret = hd_common_mem_alloc("ai_layer_structure", &outlayer_buf_pa, (void*)&outlayer_buf_va, outlayer_num * sizeof(VENDOR_AI_BUF), DDR_ID0);
			if (ret != HD_OK)
			{
				printf("hd_common_mem_alloc() fail (%d) \r\n", ret);
				goto exit;
			}

			NNSCnet_get_ai_outlayer_by_path_id(stream_nn.net_path, outlayer_path_list[0], &outlayer_buf_va);
			outlayer_length = outlayer_buf_va.width * outlayer_buf_va.height * outlayer_buf_va.channel * outlayer_buf_va.batch_num;

			ret = hd_common_mem_alloc("ai_outlayer_softmax", &softmax_loc_layer_float_pa, (void**)&softmax_loc_layer_float_va, outlayer_length * sizeof(FLOAT), DDR_ID0);
			if (ret != HD_OK)
			{
				printf("hd_common_mem_alloc() fail (%d) \r\n", ret);
				goto exit;
			}
			#endif

			printf("End of init NNSC NET \n");
			is_nnsc_init = TRUE;
			is_nnsc_run = FALSE;

			break;

		case 3:
			if (!is_nnsc_init) {
				printf("NNSC not init yet\n");
				break;
			}
			if (is_nnsc_run) {
				printf("NNSC NET run yet \n");
				break;
			}

			stream[0].cap_path = HD_VIDEOCAP_PATH(HD_DAL_VIDEOCAP(0), HD_IN(0), HD_OUT(0));
			stream[0].proc_path = HD_VIDEOPROC_PATH(HD_DAL_VIDEOPROC(0), HD_IN(0), HD_OUT(0));
			stream[0].enc_path = HD_VIDEOENC_PATH(HD_DAL_VIDEOENC(0), HD_IN(0), HD_OUT(0));

			printf("Start of NNSC \n");

			nnsc_set_param(&nnsc_param);
			nnsc_thread_run = 1;
			ret = pthread_create(&nnsc_thread_id, NULL, access_nn_thread, (void *)&stream[0]);
			if (ret < 0) {
				printf("create access_nn_thread thread failed");
				goto exit;
			}

			is_nnsc_run = TRUE;
			break;

		case 5:
			if (!is_nnsc_init) {
				printf("nnsc net not init yet\n");
				is_nnsc_run = FALSE;
				break;
			}
			printf("Start of uninit NNSC NET \n");

			nnsc_thread_run = 0;
			sleep(1);
			pthread_join(nnsc_thread_id, NULL);

			#if (ACCESS_NNSC_NET_EN)
			NNSCnet_stop_input(stream_nn.in_path); 
			NNSCnet_stop(&stream_nn);
			ret = NNSCnet_close_module(&stream_nn);
			if (ret != HD_OK) {
				printf("net close module fail (%d) \n", ret);
			}
			ret = NNSCnet_uninit_module();
			if (ret != HD_OK) {
				printf("net unint module fail (%d) \n", ret);
			}
			#endif

			is_nnsc_init = FALSE;
			is_nnsc_run = FALSE;
			printf("End of uninit NNSC NET \n");

			break;

		case 10:
			do {
				printf("Config nnsc net msg(0: disable; 1: enable)>> \n");
				nnsc_net_msg_en = (UINT32)get_choose_int();
			} while (0);
			break;

		case 20:
			nnsc_lib_version = nnsc_get_version();
			printf(" nnsc lib version : v%d.%d \n", (nnsc_lib_version >> 8) & 0xff, (nnsc_lib_version >> 0) & 0xff);
			break;

		case 21:
			do {
				printf("Config nnsc lib func(0: disable; 1: enable)>> \n");
				nnsc_lib_func_en = (UINT32)get_choose_int();
			} while (0);
			break;

		case 22:
			do {
				printf("Config nnsc lib msg type>> \n");
				printf("---------------------------------------- \r\n");
				printf("%4d >> NNSC_DBG_TYPE_NULL \n", NNSC_DBG_TYPE_NULL);
				printf("%4d >> NNSC_DBG_TYPE_MAIN \n", NNSC_DBG_TYPE_MAIN);
				printf("%4d >> NNSC_DBG_TYPE_SCD \n", NNSC_DBG_TYPE_SCD);
				printf("%4d >> NNSC_DBG_TYPE_AE \n", NNSC_DBG_TYPE_AE);
				printf("%4d >> NNSC_DBG_TYPE_AWB \n", NNSC_DBG_TYPE_AWB);
				printf("%4d >> NNSC_DBG_TYPE_IQ \n", NNSC_DBG_TYPE_IQ);
				printf("%4d >> NNSC_DBG_TYPE_PARAM \n", NNSC_DBG_TYPE_PARAM);
				printf("---------------------------------------- \r\n");
				nnsc_lib_msg_type = (UINT32)get_choose_int();
				nnsc_set_dbg_out(nnsc_lib_msg_type);
			} while (0);
			break;

		case 23:
			do {
				printf("Config nnsc lib nnsc param item (0 ~ %d)>> \r\n", NNSC_TYPE_MAX_NUM - 1);
				printf("---------------------------------------- \r\n");
				printf("%2d >> NNSC_TYPE_BACKLIGHT \n", NNSC_TYPE_BACKLIGHT);
				printf("%2d >> NNSC_TYPE_FOGGY \n", NNSC_TYPE_FOGGY);
				printf("%2d >> NNSC_TYPE_GRASS \n", NNSC_TYPE_GRASS);
				printf("%2d >> NNSC_TYPE_SKIN_COLOR \n", NNSC_TYPE_SKIN_COLOR);
				printf("%2d >> NNSC_TYPE_SNOWFIELD \n", NNSC_TYPE_SNOWFIELD);
				printf("---------------------------------------- \r\n");
				option_1 = (UINT32)get_choose_int();
			} while (0);
			printf("Config nnsc lib param sensitive (0 ~ %d)>> \r\n", NNSC_PARAM_SENSITIVE_MAX);
			nnsc_param.param[option_1].sensitive = get_choose_int();
			if (nnsc_param.param[option_1].sensitive > NNSC_PARAM_SENSITIVE_MAX) {
				nnsc_param.param[option_1].sensitive = NNSC_PARAM_SENSITIVE_MAX;
			}
			printf("Config nnsc lib param strength (0 ~ %d)>> \r\n", NNSC_PARAM_STRENGTH_MAX);
			nnsc_param.param[option_1].adj_strength = get_choose_int();
			if (nnsc_param.param[option_1].adj_strength > NNSC_PARAM_STRENGTH_MAX) {
				nnsc_param.param[option_1].adj_strength = NNSC_PARAM_STRENGTH_MAX;
			}
			nnsc_set_param(&nnsc_param);
			break;

		case 24:
			do {
				printf("Config nnsc lib test enable (0: disable; 1: enable)>> \n");
				test_param.enable = (UINT32)get_choose_int();
			} while (0);
			do {
				printf("Config nnsc lib test mode (0 ~ %d)>> \n", NNSC_TYPE_MAX_NUM - 1);
				printf("---------------------------------------- \r\n");
				printf("%2d >> NNSC_TYPE_BACKLIGHT \n", NNSC_TYPE_BACKLIGHT);
				printf("%2d >> NNSC_TYPE_FOGGY \n", NNSC_TYPE_FOGGY);
				printf("%2d >> NNSC_TYPE_GRASS \n", NNSC_TYPE_GRASS);
				printf("%2d >> NNSC_TYPE_SKIN_COLOR \n", NNSC_TYPE_SKIN_COLOR);
				printf("%2d >> NNSC_TYPE_SNOWFIELD \n", NNSC_TYPE_SNOWFIELD);
				printf("---------------------------------------- \r\n");
				test_param.mode_type = (UINT32)get_choose_int();
			} while (0);
			nnsc_set_test_mode(&test_param);
			break;

		case 25:
			do {
				printf("Config nnsc lib scd control : \r\n");
			} while (0);
			printf("Config scd enable (0: disable; 1: enable)>> \n");
			scd_control.enable = (UINT32)get_choose_int();
			printf("Config scd auto_enable (0: disable; 1: enable)>> \n");
			scd_control.auto_enable = (UINT32)get_choose_int();
			printf("Config scd manual_stable (0: change; 1: stable)>> \n");
			scd_control.manual_stable = (UINT32)get_choose_int();
			nnsc_set_scd_control(&scd_control);
			break;

		case 26:
			do {
				printf("Config nnsc lib scd param : \r\n");
			} while (0);
			printf("Config stable_sensitive (0 ~ 10)>> \r\n");
			scd_param.stable_sensitive = get_choose_int();
			printf("Config change_sensitive (0 ~ 10)>> \r\n");
			scd_param.change_sensitive = get_choose_int();
			printf("Config g_diff_num_th (0 ~ 1024)>> \r\n");
			scd_param.g_diff_num_th = get_choose_int();
			nnsc_set_scd_param(&scd_param);
			break;

		case 0:
		//default:
			trig = 0;
			break;
		}
	}

exit:
	vendor_isp_uninit();
	hd_common_mem_uninit();
	hd_common_uninit();

	return 0;
}
