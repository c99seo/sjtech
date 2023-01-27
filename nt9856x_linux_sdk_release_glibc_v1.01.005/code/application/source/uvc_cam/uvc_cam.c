/**
	@brief Sample code of video record.\n

	@file VIDEO_STREAM.c

	@author Boyan Huang

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hd_debug.h"
#include "uvc_slice.h"
#include "vendor_videoenc.h"
#include "vendor_audiocapture.h"
#include "vendor_common.h"
#include "vendor_videoprocess.h"
#include "sys_mempool.h"
#include "uvc_debug_menu.h"
#include "uvc_cam.h"

// platform dependent
#if 1//defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#define msleep(x)				usleep(1000*x)
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(hd_video_record, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define UVC_ON 1
#define AUDIO_ON 1
#define WRITE_BS 0
#define UVC_SUPPORT_YUV_FMT 1
#define UAC_RX_ON 0
#define HID_FUNC 0
#define CDC_FUNC 0
#define UVAC_WAIT_RELEASE 1
#define MSDC_FUNC 0 // 0: DISABLE, 1: MSDC_DISK, 2: MSDC_IQ
#define UVC_CT 0
#define UVC_PU 0
#define UVC_EU 0

#define PERF_TEST 0

#define POOL_SIZE_USER_DEFINIED  0x1000000

#define COMMON_RECFRM_BUFFER 0
#define COMMON_RECFRM_FUNC_BASE   0
#define COMMON_RECFRM_FUNC_SVC    1
#define COMMON_RECFRM_FUNC_LTR    0

#if UVC_ON
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <compiler.h>
#include <usb2dev.h>
#include "UVAC.h"
#include "comm/timer.h"
#include "hd_common.h"
#include "FileSysTsk.h"
#include <sys/stat.h>
#include "ImageApp/ImageApp_UsbMovie.h"
#include "umsd.h"
#include "sdio.h"

#define BULK_PACKET_SIZE    512

#define USB_VID                         0x0603
#define USB_PID_PCCAM                   0x8612 // not support pc cam
#define USB_PID_WRITE                   0x8614
#define USB_PID_PRINT                   0x8613
#define USB_PID_MSDC                    0x8611

#define USB_PRODUCT_REVISION            '1', '.', '0', '0'
#define USB_VENDER_DESC_STRING          'N', 0x00,'O', 0x00,'V', 0x00,'A', 0x00,'T', 0x00,'E', 0x00,'K', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_VENDER_DESC_STRING_LEN      0x09
#define USB_PRODUCT_DESC_STRING         'D', 0x00,'E', 0x00,'M', 0x00,'O', 0x00,'1', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_PRODUCT_DESC_STRING_LEN     0x07
#define USB_PRODUCT_STRING              'N','v','t','-','D','S','C'
#define USB_SERIAL_NUM_STRING           '9', '8', '5', '2', '0','0', '0', '0', '0', '0','0', '0', '1', '0', '0'
#define USB_VENDER_STRING               'N','O','V','A','T','E','K'
#define USB_VENDER_SIDC_DESC_STRING     'N', 0x00,'O', 0x00,'V', 0x00,'A', 0x00,'T', 0x00,'E', 0x00,'K', 0x00, 0x20, 0x00,0x00, 0x00 // NULL
#define USB_VENDER_SIDC_DESC_STRING_LEN 0x09

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_720P    1280
#define VDO_SIZE_H_720P    720
//Max sensor size
#define MAX_CAP_SIZE_W cap_size_w
#define MAX_CAP_SIZE_H cap_size_h
//Max bitstream size
#define MAX_BS_W cap_size_w
#define MAX_BS_H cap_size_h
#define MAX_BS_FPS 30
#define NVT_UI_UVAC_RESO_CNT  2
#if UVC_SUPPORT_YUV_FMT
#define MAX_YUV_W 640
#define MAX_YUV_H 480
#define MAX_YUV_FPS 30
#endif
#define SEN_SEL_IMX290   0   //2M_SHDR
#define SEN_SEL_OS05A10  1   //5M_SHDR
#define SEN_SEL_GC4653   2   //4M
static UINT32 sensor_sel = SEN_SEL_IMX290;

static UINT32 cap_size_w = 0;
static UINT32 cap_size_h = 0;

#define UVC_MJPEG_STREAM_BUF_MARGIN 110 //%
#define UVC_MJPEG_ENC_BLK_CNT   3
#define UVC_MJPEG_ENC_BUF_MS(fps) (1000*UVC_MJPEG_ENC_BLK_CNT*UVC_MJPEG_STREAM_BUF_MARGIN/100/(fps))

#define UVC_H264_ENC_BUF_MS 2000

#define MJPEG_TBR_MAX   (20*8*1024*1024)
#define MJPEG_TBR_2M    (20*8*1024*1024)
#define MJPEG_TBR_720P  (7*8*1024*1024)

#define H264_TBR_MAX    (2*8*1024*1024)
#define H264_TBR_2M     (8*1024*1024)
#define H264_TBR_720P   (8*1024*1024)
#if 1
static UVAC_VID_RESO gUIUvacVidReso[NVT_UI_UVAC_RESO_CNT] = {
	{ VDO_SIZE_W_2M,   VDO_SIZE_H_2M,   1,      MAX_BS_FPS,      0,      0},
	{ VDO_SIZE_W_720P,   VDO_SIZE_H_720P,   1,   MAX_BS_FPS,      0,      0},
};
#else
static UVAC_VID_RESO gUIUvacVidResoMJPEG[3] = {
	{ VDO_SIZE_W_2M,   VDO_SIZE_H_2M,   1,      MAX_BS_FPS,      0,      0},
	{ VDO_SIZE_W_720P,   VDO_SIZE_H_720P,   1,   MAX_BS_FPS,      0,      0},
	{ 640,   480,   1,   MAX_BS_FPS,      0,      0},
};
static UVAC_VID_RESO gUIUvacVidResoH264[NVT_UI_UVAC_RESO_CNT] = {
	{ VDO_SIZE_W_2M,   VDO_SIZE_H_2M,   1,      MAX_BS_FPS,      0,      0},
	{ VDO_SIZE_W_720P,   VDO_SIZE_H_720P,   1,   MAX_BS_FPS,      0,      0},
};
#endif

#define NVT_UI_UVAC_AUD_SAMPLERATE_CNT                  1
#define NVT_UI_FREQUENCY_16K                    0x003E80   //16k
#define NVT_UI_FREQUENCY_32K                    0x007D00   //32k
#define NVT_UI_FREQUENCY_48K                    0x00BB80   //48k
#define UAC_RX_BLOCK_SIZE                       9600

static UINT32 gUIUvacAudSampleRate[NVT_UI_UVAC_AUD_SAMPLERATE_CNT] = {
	NVT_UI_FREQUENCY_16K
};
#if UVC_SUPPORT_YUV_FMT
static UVAC_VID_RESO gUIUvacVidYuvReso[] = {
	{MAX_YUV_W,	MAX_YUV_H,	1,		MAX_YUV_FPS,		 0,		 0},
	{320,	240,	1,		30,		 0,		 0},
};
#endif
static  UVAC_VIDEO_FORMAT m_VideoFmt[UVAC_VID_DEV_CNT_MAX] = {0}; //UVAC_VID_DEV_CNT_MAX=2
static BOOL m_bIsStaticPattern = FALSE;

static UINT32 uvac_pa = 0;
static UINT32 uvac_va = 0;
static UINT32 uvac_size = 0;
#if UVC_EU
static UINT32 g_capbind = 0;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_one_buf = 0;    //0x0: vprc noraml, 0x1: one buffer
#else
static UINT32 g_capbind = 1;  //0:D2D, 1:direct, 2: one-buf, 0xff: no-bind
static UINT32 g_one_buf = 1;    //0x0: vprc noraml, 0x1: one buffer
#endif
static UINT32 g_low_latency = 0;
static BOOL UVAC_enable(void);
static BOOL UVAC_disable(void);
#endif

#if UVC_CT
#include "hd_type.h"
#include "uvc_ct.h"
#endif
#if UVC_PU
#include "hd_type.h"
#include "ae_ui.h"
#include "uvc_pu.h"
#include "vendor_isp.h"
#endif

#if UVC_EU
#include "uvc_eu.h"
#include "vendor_isp.h"
#endif

///////////////////////////////////////////////////////////////////////////////

//header
#define DBGINFO_BUFSIZE()	(0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt)   (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 59
#define VDO_NRX_BUFSIZE(w, h)           (ALIGN_CEIL_4(ALIGN_CEIL_64(w) * 12 / 8 * RAW_COMPRESS_RATIO / 100 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)

//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt)	(VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

///////////////////////////////////////////////////////////////////////////////
#define SEN_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#if UVC_EU
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_RAW12
#else
#define CAP_OUT_FMT		HD_VIDEO_PXLFMT_NRX12
#endif

#define CA_WIN_NUM_W		32
#define CA_WIN_NUM_H		32
#define LA_WIN_NUM_W		32
#define LA_WIN_NUM_H		32
#define VA_WIN_NUM_W		16
#define VA_WIN_NUM_H		16
#define YOUT_WIN_NUM_W	128
#define YOUT_WIN_NUM_H	128
#define ETH_8BIT_SEL		0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL		1 //0: full, 1: subsample 1/2

#define VIDEOCAP_ALG_FUNC (HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB)
#define VIDEOPROC_ALG_FUNC (HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR)

///////////////////////////////////////////////////////////////////////////////
HD_VIDEOENC_BUFINFO phy_buf_main;
HD_VIDEOENC_BUFINFO phy_buf_sub1;
HD_VIDEOENC_BUFINFO phy_buf_sub2;
HD_AUDIOCAP_BUFINFO phy_buf_main2;
UINT32 vir_addr_main;
UINT32 vir_addr_sub1;
UINT32 vir_addr_sub2;
UINT32 vir_addr_main2;
VK_TASK_HANDLE cap_thread_id;
UINT32 flow_audio_start = 0, encode_start = 0, acquire_start = 0;
#define JPG_YUV_TRANS 1
pthread_mutex_t flow_start_lock;

#if  UAC_RX_ON
UINT32 vir_addr_main3;
pthread_t uac_thread_id;

HD_AUDIO_SOUND_MODE aud_rx_sound_mode = HD_AUDIO_SOUND_MODE_MONO;

static UINT32 gUIUvacAudRxSampleRate[NVT_UI_UVAC_AUD_SAMPLERATE_CNT] = {
	NVT_UI_FREQUENCY_48K
};
#endif

#include "vendor_isp.h"

VIDEO_STREAM stream[1] = {0}; //0: main stream, 1: sub stream

#if  AUDIO_ON
typedef struct _AUDIO_CAPONLY {
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;
	HD_AUDIO_SOUND_MODE sound_mode;

	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	UINT32 cap_exit;
} AUDIO_CAPONLY;

AUDIO_CAPONLY au_cap = {0};
#endif

#define CS_INTERFACE                        0x24
#define VC_EXTENSION_UNIT                   0x06

UINT8 get_eu_control_num(UINT8 *p_control, UINT8 size)
{
	UINT32 i, j;
	UINT8 num = 0, temp;

	for (j = 0; j < size; j++) {
		for (i = 0; i < 8; i++) {
			temp  = p_control[j] >> i;
			if (temp & 0x1) {
				num++;
			}
		}
	}
	return num;
}

//YUV definition for UVC
#define VDO_PXLFMT_YUV422_YUYV	(0x51102422) ///< 1 plane, pixel=YUYV(w,h), packed format with Y2U1V1
static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	HD_DIM buf_size;
	UINT32 id;

	// config common pool (cap)
	id = 0;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	if (g_capbind == 1) {
		//direct ... NOT require raw
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	} else {
		buf_size.w = MAX_CAP_SIZE_W;
		buf_size.h = MAX_CAP_SIZE_H;
		mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_RAW_BUFSIZE(buf_size.w, buf_size.h, CAP_OUT_FMT)
														+VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H)
														+VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H);
	}
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

	// config common pool (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_CAP_SIZE_W, ALIGN_CEIL_16(MAX_CAP_SIZE_H), HD_VIDEO_PXLFMT_YUV420);	//padding to 16x
	if (g_one_buf)
	mem_cfg.pool_info[id].blk_cnt = 1;
	else
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;

#if UVC_SUPPORT_YUV_FMT
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(MAX_YUV_W, ALIGN_CEIL_16(MAX_YUV_H), HD_VIDEO_PXLFMT_YUV422);	//padding to 16x
	mem_cfg.pool_info[id].blk_cnt = 6;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

#if AUDIO_ON
	// config common pool audio (main)
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = 0x10000;
	mem_cfg.pool_info[id].blk_cnt = 2;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

#if (MSDC_FUNC == 1)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = POOL_SIZE_USER_DEFINIED;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
#endif

#if COMMON_RECFRM_BUFFER
	id++;
	mem_cfg.pool_info[id].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[id].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(ALIGN_CEIL_64(MAX_BS_W), ALIGN_CEIL_64(MAX_BS_H), HD_VIDEO_PXLFMT_YUV420);
	mem_cfg.pool_info[id].blk_cnt = COMMON_RECFRM_FUNC_BASE + COMMON_RECFRM_FUNC_SVC + COMMON_RECFRM_FUNC_LTR;
	mem_cfg.pool_info[id].ddr_id = DDR_ID0;
#endif

	ret = hd_common_mem_init(&mem_cfg);

	return ret;
}

static HD_RESULT mem_exit(void)
{
	HD_RESULT ret = HD_OK;
	hd_common_mem_uninit();
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/*AUDIO */
#if AUDIO_ON
static HD_RESULT set_cap_cfg2(HD_PATH_ID *p_audio_cap_ctrl, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret;
	HD_PATH_ID audio_cap_ctrl = 0;
	HD_AUDIOCAP_DEV_CONFIG audio_dev_cfg = {0};
	HD_AUDIOCAP_DRV_CONFIG audio_drv_cfg = {0};
	INT32 prepwr_enable = TRUE;

	ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &audio_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}

	// set audiocap dev parameter
	audio_dev_cfg.in_max.sample_rate = sample_rate;
	audio_dev_cfg.in_max.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_dev_cfg.in_max.mode = mode;
	audio_dev_cfg.in_max.frame_sample = 1024;
	audio_dev_cfg.frame_num_max = 10;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DEV_CONFIG, &audio_dev_cfg);
	if (ret != HD_OK) {
		return ret;
	}

	// set audiocap drv parameter
	audio_drv_cfg.mono = HD_AUDIO_MONO_RIGHT;
	ret = hd_audiocap_set(audio_cap_ctrl, HD_AUDIOCAP_PARAM_DRV_CONFIG, &audio_drv_cfg);

	// set PREPWR
	vendor_audiocap_set(audio_cap_ctrl, VENDOR_AUDIOCAP_ITEM_PREPWR_ENABLE, &prepwr_enable);

	*p_audio_cap_ctrl = audio_cap_ctrl;

	return ret;
}

static HD_RESULT set_cap_param2(HD_PATH_ID audio_cap_path, HD_AUDIO_SR sample_rate, HD_AUDIO_SOUND_MODE mode)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIOCAP_IN audio_cap_param = {0};

	// set audiocap input parameter
	audio_cap_param.sample_rate = sample_rate;
	audio_cap_param.sample_bit = HD_AUDIO_BIT_WIDTH_16;
	audio_cap_param.mode = mode;
	audio_cap_param.frame_sample = 1024;
	ret = hd_audiocap_set(audio_cap_path, HD_AUDIOCAP_PARAM_IN, &audio_cap_param);

	return ret;
}

static HD_RESULT open_module2(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
	ret = set_cap_cfg2(&p_caponly->cap_ctrl, p_caponly->sample_rate_max, p_caponly->sound_mode);
	if (ret != HD_OK) {
		printf("set cap-cfg fail\n");
		return HD_ERR_NG;
	}
	if((ret = hd_audiocap_open(HD_AUDIOCAP_0_IN_0, HD_AUDIOCAP_0_OUT_0, &p_caponly->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module2(AUDIO_CAPONLY *p_caponly)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_close(p_caponly->cap_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module2(void)
{
	HD_RESULT ret;
	if((ret = hd_audiocap_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}


THREAD_DECLARE(capture_thread, arg)
{
	HD_RESULT ret = HD_OK;
	HD_AUDIO_FRAME  data_pull;
	int get_phy_flag=0;
#if WRITE_BS
	char file_path_main[64], file_path_len[64];
	FILE *f_out_main, *f_out_len;
#endif
	AUDIO_CAPONLY *p_cap_only = (AUDIO_CAPONLY *)arg;
#if UVC_ON
	UVAC_STRM_FRM strmFrm = {0};
#endif
	#define PHY2VIRT_MAIN2(pa) (vir_addr_main2 + (pa - phy_buf_main2.buf_info.phy_addr))

#if WRITE_BS
	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/audio_bs_%d_%d_%d_pcm.dat", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->sample_rate);
	snprintf(file_path_len, sizeof(file_path_len), "/mnt/sd/audio_bs_%d_%d_%d_pcm.len", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, p_cap_only->sample_rate);
#endif

#if WRITE_BS
	/* open output files */
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	if ((f_out_len = fopen(file_path_len, "wb")) == NULL) {
		printf("open len file (%s) fail....\r\n", file_path_len);
	}
#endif

	/* pull data test */
	while (p_cap_only->cap_exit == 0) {

		if (!flow_audio_start){
			if (get_phy_flag){
				/* mummap for bs buffer */
				//printf("release common buffer2\r\n");
				if (vir_addr_main2)hd_common_mem_munmap((void *)vir_addr_main2, phy_buf_main2.buf_info.buf_size);

				get_phy_flag = 0;
			}
			/* wait flow_audio_start */
			msleep(10);
			continue;
		}else{
			if (!get_phy_flag){

				/* query physical address of bs buffer	(this can ONLY query after hd_audiocap_start() is called !!) */
				hd_audiocap_get(au_cap.cap_ctrl, HD_AUDIOCAP_PARAM_BUFINFO, &phy_buf_main2);

				/* mmap for bs buffer  (just mmap one time only, calculate offset to virtual address later) */
				vir_addr_main2 = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main2.buf_info.phy_addr, phy_buf_main2.buf_info.buf_size);
				if (vir_addr_main2 == 0) {
					printf("mmap error\r\n");
					return 0;
				}

				get_phy_flag = 1;
			}
		}

		// pull data
		ret = hd_audiocap_pull_out_buf(p_cap_only->cap_path, &data_pull, 200); // >1 = timeout mode

		if (ret == HD_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN2(data_pull.phy_addr[0]);
			UINT32 size = data_pull.size;
#if WRITE_BS
			UINT32 timestamp = hd_gettime_ms();
#endif
			//printf("audio fram size =%u\r\n",size);
#if UVC_ON
			hd_common_mem_flush_cache((void *)ptr, size);

			strmFrm.path = UVAC_STRM_AUD;
			strmFrm.addr = data_pull.phy_addr[0];
			strmFrm.size = data_pull.size;
			strmFrm.timestamp = data_pull.timestamp;
			UVAC_SetEachStrmInfo(&strmFrm);

			#if UVAC_WAIT_RELEASE
			UVAC_WaitStrmDone(UVAC_STRM_AUD);
			#endif
#endif

#if WRITE_BS
			// write bs
			if (f_out_main) fwrite(ptr, 1, size, f_out_main);
			if (f_out_main) fflush(f_out_main);

			// write bs len
			if (f_out_len) fprintf(f_out_len, "%d %d\n", size, timestamp);
			if (f_out_len) fflush(f_out_len);
#endif
			// release data
			ret = hd_audiocap_release_out_buf(p_cap_only->cap_path, &data_pull);
			if (ret != HD_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	if (get_phy_flag){
		/* mummap for bs buffer */
		//printf("release common buffer2\r\n");
		if (vir_addr_main2)hd_common_mem_munmap((void *)vir_addr_main2, phy_buf_main2.buf_info.buf_size);
	}

#if WRITE_BS
	/* close output file */
	if (f_out_main) fclose(f_out_main);
	if (f_out_len) fclose(f_out_len);
#endif
	return 0;
}
#endif // AUDIO_ON
/*AUDIO end*/

#if  UAC_RX_ON
THREAD_DECLARE(uac_thread, arg)
{

#if UVC_ON
	HD_RESULT ret = HD_OK;
	int get_phy_flag=0;
	char file_path_main[64];
	FILE *f_out_main;
	AUDIO_CAPONLY *p_cap_only = (AUDIO_CAPONLY *)arg;
	UVAC_STRM_FRM strmFrm = {0};

	#define PHY2VIRT_MAIN3(pa) (vir_addr_main3 + (pa - uvac_pa))

	/* config pattern name */
	snprintf(file_path_main, sizeof(file_path_main), "/mnt/sd/uac_bs_%d_%d_%ld_pcm.dat", HD_AUDIO_BIT_WIDTH_16, HD_AUDIO_SOUND_MODE_MONO, gUIUvacAudRxSampleRate[0]);

	/* open output files */
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		printf("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}

	/* pull data test */
	while (p_cap_only->cap_exit == 0) {

		if (!uvac_pa){
			/* wait flow_start */
			printf("\r\nwait flow_start\r\n");
			usleep(10);
			continue;
		}else{
			if (!get_phy_flag){

				/* mmap for bs buffer  (just mmap one time only, calculate offset to virtual address later) */
				vir_addr_main3 = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, uvac_pa, uvac_size);
				if (vir_addr_main3 == 0) {
					printf("mmap error\r\n");
					return 0;
				}

				get_phy_flag = 1;
			}
		}

		// pull data
		ret = UVAC_PullOutStrm(&strmFrm, 300); // >1 = timeout mode

		if (ret == E_OK) {
			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN3(strmFrm.addr);
			UINT32 size = strmFrm.size;

			// write bs
			if (f_out_main) fwrite(ptr, 1, size, f_out_main);
			if (f_out_main) fflush(f_out_main);

			// release data
			ret = UVAC_ReleaseOutStrm(&strmFrm);
			if (ret != E_OK) {
				printf("release buffer failed. ret=%x\r\n", ret);
			}
		}
	}

	if (get_phy_flag){
		/* mummap for bs buffer */
		//printf("release common buffer2\r\n");
		if (vir_addr_main3)hd_common_mem_munmap((void *)vir_addr_main3, uvac_size);
	}

	/* close output file */
	if (f_out_main) fclose(f_out_main);
#endif

	return 0;
}
#endif

static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	return ret;
}

HD_RESULT get_cap_sysinfo(HD_PATH_ID video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_SYSINFO sys_info = {0};

	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSINFO, &sys_info);
	printf("sys_info.devid =0x%X, cur_fps[0]=%d/%d, vd_count=%llu\r\n", sys_info.dev_id, GET_HI_UINT16(sys_info.cur_fps[0]), GET_LO_UINT16(sys_info.cur_fps[0]), sys_info.vd_count);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	if (sensor_sel == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
		printf("Using nvt_sen_imx290\n");
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
		printf("Using nvt_sen_os05a10\n");
	} else if (sensor_sel == SEN_SEL_GC4653) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc4653");
		printf("Using nvt_sen_gc4653\n");
	}
	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux =  0x220; //PIN_SENSOR_CFG_MIPI | PIN_SENSOR_CFG_MCLK
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0xF01;//PIN_MIPI_LVDS_CFG_CLK2 | PIN_MIPI_LVDS_CFG_DAT0|PIN_MIPI_LVDS_CFG_DAT1 | PIN_MIPI_LVDS_CFG_DAT2 | PIN_MIPI_LVDS_CFG_DAT3
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x01;//PIN_I2C_CFG_CH1
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;//HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;//HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control
	if (ret != HD_OK) {
		return ret;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, int fps, HD_DIM *p_out_dim)
{
	HD_RESULT ret = HD_OK;
	{//select sensor mode, manually or automatically
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(fps,1);
		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		printf("set_cap_param fps=%d\r\n", fps);
		if (ret != HD_OK) {
			return ret;
		}
	}
	#if 1 //no crop, full frame
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP NONE=%d\r\n", ret);
	}
	#else //HD_CROP_ON
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_ON;
		video_crop_param.win.rect.x = 0;
		video_crop_param.win.rect.y = 0;
		video_crop_param.win.rect.w = MAX_CAP_SIZE_W/2;
		video_crop_param.win.rect.h= MAX_CAP_SIZE_H/2;
		video_crop_param.align.w = 4;
		video_crop_param.align.h = 4;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		//printf("set_cap_param CROP ON=%d\r\n", ret);
	}
	#endif
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		video_out_param.dim.w = p_out_dim->w;
		video_out_param.dim.h = p_out_dim->h;
		video_out_param.pxlfmt = CAP_OUT_FMT;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		if (g_capbind == 0) //D2D mode, for pull raw frame
			video_out_param.depth = 1;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		//printf("set_cap_param OUT=%d\r\n", ret);
	}

	{
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = 0;
		if (g_capbind == 1) //direct mode
			video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_cap_param PATH_CONFIG=0x%X\r\n", ret);
	}

	return ret;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, HD_VIDEOPROC_0_CTRL, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;


	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		video_cfg_param.isp_id = 0;
		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		printf("vin w= %d, h = %d\r\n", p_max_dim->w, p_max_dim->h);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (g_capbind == 1)
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct

		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
		//printf("set_proc_param PATH_CONFIG=0x%X\r\n", ret);
	}
	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}
	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 pull_allow, UINT32 b_one_buf)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = pxlfmt;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = pull_allow; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}

	HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};
	video_path_param.out_func = 0;
	if (g_low_latency == 1 && video_proc_path == stream[0].proc_main_path){
		//only the proc_path should be set
		printf("g_low_latency = %d\r\n", g_low_latency);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY;
	}
	if (b_one_buf == 1){
		printf("g_one_buf = %d\r\n", g_one_buf);
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
	}
	ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	if (ret != HD_OK) {
		return HD_ERR_NG;
	}

	if (stream[0].codec_type == HD_CODEC_TYPE_JPEG){
		UINT32 h_align = 16;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	} else {
		UINT32 h_align = 2;
		//to avoid MJPEG might encode read YUV out of range
		ret = vendor_videoproc_set(video_proc_path, VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN, &h_align);
		if (ret != HD_OK) {
			printf("VENDOR_VIDEOPROC_PARAM_HEIGHT_ALIGN failed!(%d)\r\n", ret);
		}
	}
	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, HD_VIDEO_CODEC codec_type, UINT32 max_bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	VENDOR_VIDEOENC_H26X_ENC_COLMV h26x_enc_colmv = {0};
	VENDOR_VIDEOENC_FIT_WORK_MEMORY fit_work_mem_cfg = {0};

	//fit_work_mem_cfg.b_enable = TRUE;
	fit_work_mem_cfg.b_enable = FALSE;
    vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_FIT_WORK_MEMORY, &fit_work_mem_cfg);

	h26x_enc_colmv.h26x_colmv_en = 0;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COLMV, &h26x_enc_colmv);

#if COMMON_RECFRM_BUFFER
	VENDOR_VIDEOENC_H26X_COMM_RECFRM h26x_comm_recfrm = {0};

	h26x_comm_recfrm.enable = 1;
	h26x_comm_recfrm.h26x_comm_base_recfrm_en = COMMON_RECFRM_FUNC_BASE;
	h26x_comm_recfrm.h26x_comm_svc_recfrm_en = COMMON_RECFRM_FUNC_SVC;
	h26x_comm_recfrm.h26x_comm_ltr_recfrm_en = COMMON_RECFRM_FUNC_LTR;
	vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_COMM_RECFRM, &h26x_comm_recfrm);
#endif

	if (p_max_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type = codec_type;
		video_path_config.max_mem.max_dim.w  = p_max_dim->w;
		video_path_config.max_mem.max_dim.h  = p_max_dim->h;
		video_path_config.max_mem.bitrate    = max_bitrate;
		video_path_config.max_mem.enc_buf_ms = UVC_H264_ENC_BUF_MS;
		video_path_config.max_mem.svc_layer  = HD_SVC_4X;
		video_path_config.max_mem.ltr        = FALSE;
		video_path_config.max_mem.rotate     = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		video_path_config.isp_id             = 0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};
			video_func_config.in_func = 0;
		if (g_low_latency == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY;
		else if (g_one_buf == 1)
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_ONEBUF;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}
	}
	return ret;
}

static HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_VIDEOENC_OUT2 video_out2_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir           = HD_VIDEO_DIR_NONE;
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		if (enc_type == HD_CODEC_TYPE_H264) {
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			min_ratio.min_i_ratio = UVC_H264_ENC_BUF_MS/2;
			min_ratio.min_p_ratio = UVC_H264_ENC_BUF_MS/3;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

			//printf("enc_type = HD_CODEC_TYPE_H264\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 15;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_4X;
			video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}

			//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
			rc_param.rc_mode             = HD_RC_MODE_CBR;
			rc_param.cbr.bitrate         = bitrate;
			rc_param.cbr.frame_rate_base = 30;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 45;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 45;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = 0;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}

		} else if (enc_type == HD_CODEC_TYPE_JPEG) {
			//printf("enc_type = HD_CODEC_TYPE_JPEG\r\n");
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			VENDOR_VIDEOENC_MIN_RATIO_CFG min_ratio = {0};

			//ms*(PATH CONFIG target bitrate) to check if remaining buffer size is enough
			//only min_i_ratio is valid in MJPEG encode. bitstream buffer is designed to be less than (ms/1000)*target bitrate
			min_ratio.min_i_ratio = 1000*UVC_MJPEG_STREAM_BUF_MARGIN/100;//ms
			min_ratio.min_p_ratio = min_ratio.min_i_ratio;
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_MIN_RATIO, &min_ratio);

#if 0	//fixed quality
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			if (p_dim->w >= VDO_SIZE_W_4M ) {
				video_out_param.jpeg.image_quality = 90;
			} else {
				video_out_param.jpeg.image_quality = 95;
			}
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#else	//MJPEG CBR
			video_out2_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out2_param.jpeg.retstart_interval = 0;
			video_out2_param.jpeg.image_quality = 80;
			video_out2_param.jpeg.bitrate = bitrate;
			video_out2_param.jpeg.frame_rate_base = MAX_BS_FPS;
			video_out2_param.jpeg.frame_rate_incr = 1;
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM2, &video_out2_param);

			VENDOR_VIDEOENC_JPG_RC_CFG jpg_rc_param = {0};
			jpg_rc_param.vbr_mode_en = 0;  // 0: CBR, 1: VBR
			jpg_rc_param.min_quality = 1; // min quality
			jpg_rc_param.max_quality = 100; // max quality
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_RC, &jpg_rc_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
#endif
			//MJPG YUV trans(YUV420 to YUV422)
      #if JPG_YUV_TRANS
			VENDOR_VIDEOENC_JPG_YUV_TRANS_CFG yuv_trans_cfg = {0};
			yuv_trans_cfg.jpg_yuv_trans_en = JPG_YUV_TRANS;
			//printf("MJPG YUV TRANS(%d)\r\n", JPG_YUV_TRANS);
			vendor_videoenc_set(video_enc_path, VENDOR_VIDEOENC_PARAM_OUT_JPG_YUV_TRANS, &yuv_trans_cfg);
      #endif

		} else {

			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
    if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
#if AUDIO_ON
	if((ret = hd_audiocap_init()) != HD_OK)
        return ret;
#endif
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_STREAM *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;
	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1,  &p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_main_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_STREAM *p_stream)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_main_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_sub_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_main_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_uninit()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_uninit()) != HD_OK)
		return ret;

	return HD_OK;
}

#if UVC_SUPPORT_YUV_FMT
THREAD_DECLARE(acquire_yuv_thread, arg)
{
	VIDEO_STREAM* p_stream0 = (VIDEO_STREAM *)arg;
	HD_RESULT ret = HD_OK;
	HD_VIDEO_FRAME video_frame = {0};
	UINT32 phy_addr_main, vir_addr_main;
	UINT32 yuv_size;

	HD_DIM dim;

	dim.w = MAX_YUV_W;
	dim.h = MAX_YUV_H;

	#define PHY2VIRT_MAIN_YUV(pa) (vir_addr_main + ((pa) - phy_addr_main))

	printf("acquire_yuv_thread enter!!\r\n");
	//--------- pull data test ---------
	while (!p_stream0->acquire_exit) {
		if(acquire_start) {
			dim.w = stream[0].enc_main_dim.w;
			dim.h = stream[0].enc_main_dim.h;
			ret = hd_videoproc_pull_out_buf(p_stream0->proc_sub_path, &video_frame, 1000); // -1 = blocking mode, 0 = non-blocking mode, >0 = blocking-timeout mode
			if (ret != HD_OK) {
				printf("pull_out(1000) error = %d!!\r\n", ret);
				msleep(10);
				goto skip;
			}

			phy_addr_main = hd_common_mem_blk2pa(video_frame.blk); // Get physical addr
			if (phy_addr_main == 0) {
				printf("blk2pa fail, blk = 0x%x\r\n", video_frame.blk);
				goto skip;
			}

			yuv_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(dim.w, dim.h, HD_VIDEO_PXLFMT_YUV422);

			// mmap for frame buffer (just mmap one time only, calculate offset to virtual address later)
			//printf("phy_addr_main: 0x%x, 0x%x\n", phy_addr_main, yuv_size);
			vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_addr_main, yuv_size);
			if (vir_addr_main == 0) {
				printf("mmap error !!\r\n\r\n");
				goto skip;
			}

			UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN_YUV(video_frame.phy_addr[0]);
			//UINT32 len = video_frame.loff[0]*video_frame.ph[0] + video_frame.loff[1]*video_frame.ph[1];
			yuv_push_to_usb((UINT32)ptr, &video_frame);

			// mummap for frame buffer
			ret = hd_common_mem_munmap((void *)vir_addr_main, yuv_size);
			if (ret != HD_OK) {
				printf("mnumap error !!\r\n\r\n");
				goto skip;
			}

			ret = hd_videoproc_release_out_buf(p_stream0->proc_sub_path, &video_frame);
			if (ret != HD_OK) {
				printf("release_out() error !!\r\n\r\n");
				goto skip;
			}
		} else {
			msleep(10);
		}
skip:
		;
		//msleep(10);
	}
	return 0;
}
#endif // UVC_SUPPORT_YUV_FMT

THREAD_DECLARE(encode_thread, arg)
{

	VIDEO_STREAM* p_stream0 = (VIDEO_STREAM *)arg;
	HD_RESULT ret_main = HD_OK;
	HD_VIDEOENC_BS  data_pull_main;
	int get_phy_flag=0;

#if WRITE_BS
	char file_path_main[32] = "/mnt/sd/dump_bs_main.dat";
	FILE *f_out_main;
	UINT32 j;
#endif
	#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
#if UVC_ON
	UVAC_STRM_FRM      strmFrm = {0};
	UINT32             loop =0;
#endif
	//UINT32 temp=1;
#if WRITE_BS
	//----- open output files -----
	if ((f_out_main = fopen(file_path_main, "wb")) == NULL) {
		HD_VIDEOENC_ERR("open file (%s) fail....\r\n", file_path_main);
	} else {
		printf("\r\ndump main bitstream to file (%s) ....\r\n", file_path_main);
	}
#endif

	//--------- pull data test ---------
	while (p_stream0->enc_exit == 0) {
		if (!get_phy_flag){
			get_phy_flag = 1;
			// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
			ret_main = hd_videoenc_get(stream[0].enc_main_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
			// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
			vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
		}

		if (!encode_start){
			//------ wait encode_start ------
			msleep(10);
			continue;
		}

		//pull data
		ret_main = hd_videoenc_pull_out_buf(p_stream0->enc_main_path, &data_pull_main, -1); // -1 = blocking mode

		{
			//UINT64 cur_time = hd_gettime_us();

			//printf("vd = %lld, encout = %lld, diff = %lld\r\n", data_pull.timestamp, cur_time, cur_time - data_pull.timestamp);
		}

		if (ret_main == HD_OK) {
#if UVC_ON
			strmFrm.path = UVAC_STRM_VID ;
			strmFrm.addr = data_pull_main.video_pack[data_pull_main.pack_num-1].phy_addr;
			strmFrm.size = data_pull_main.video_pack[data_pull_main.pack_num-1].size;
			strmFrm.pStrmHdr = 0;
			strmFrm.strmHdrSize = 0;
			strmFrm.timestamp = data_pull_main.timestamp;
			if (p_stream0->codec_type == HD_CODEC_TYPE_H264){
				if (data_pull_main.pack_num > 1) {
					UINT32 header_size = 0;
					//if strmFrm.addr contains SPS/PPS, there is no need of pStrmHdr
					for (loop = 0; loop < data_pull_main.pack_num - 1; loop ++) {
						header_size += data_pull_main.video_pack[loop].size;
					}
					strmFrm.size += header_size;
					strmFrm.addr -= header_size;
				}
			}
			strmFrm.va = PHY2VIRT_MAIN(strmFrm.addr);
			//printf("++++va_main=0x%08X\n", strmFrm.va);
			if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
				UVAC_SetEachStrmInfo(&strmFrm);

				#if UVAC_WAIT_RELEASE
				#if PERF_TEST
				VOS_TICK t1 = 0, t2 = 0;
				UINT32 diff_us;
				static UINT32 cnt = 0;
				vos_perf_mark(&t1);
				#endif
				UVAC_WaitStrmDone(UVAC_STRM_VID);
				#if PERF_TEST
				vos_perf_mark(&t2);
				if (cnt%15 == 0) {
					diff_us = vos_perf_duration(t1, t2);
					printf("%dKB %dus(%dKB/sec)\r\n", strmFrm.size/1024, diff_us, strmFrm.size*1000/1024*1000/diff_us);
				}
				cnt++;
				#endif
				#endif
			}
			/*timstamp for test
			if (temp){
				system("echo \"uvc\" > /proc/nvt_info/bootts");
				temp=0;
			}
			*/
#endif

#if WRITE_BS
			for (j=0; j< data_pull_main.pack_num; j++) {
				UINT8 *ptr = (UINT8 *)PHY2VIRT_MAIN(data_pull_main.video_pack[j].phy_addr);
				UINT32 len = data_pull_main.video_pack[j].size;
				if (f_out_main) fwrite(ptr, 1, len, f_out_main);
				if (f_out_main) fflush(f_out_main);
			}

#endif

			// release data
			ret_main = hd_videoenc_release_out_buf(p_stream0->enc_main_path, &data_pull_main);
			if (ret_main != HD_OK) {
				printf("enc_release main error=%d !!\r\n", ret_main);
			}
		}
	}
	if (get_phy_flag){
		// mummap for bs buffer
		//printf("release common buffer\r\n");
		if (vir_addr_main) hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	}

#if WRITE_BS
	// close output file
	if (f_out_main) fclose(f_out_main);
#endif

	return 0;
}

#if UVC_ON
_ALIGNED(4) static UINT16 m_UVACSerialStrDesc3[] = {
	0x0320,                             // 20: size of String Descriptor = 32 bytes
	// 03: String Descriptor type
	'5', '1', '0', '5', '5',            // 96611-00000-001 (default)
	'0', '0', '0', '0', '0',
	'0', '0', '1', '0', '0'
};
_ALIGNED(4) const static UINT8 m_UVACManuStrDesc[] = {
	USB_VENDER_DESC_STRING_LEN * 2 + 2, // size of String Descriptor = 6 bytes
	0x03,                       // 03: String Descriptor type
	USB_VENDER_DESC_STRING
};

_ALIGNED(4) const static UINT8 m_UVACProdStrDesc[] = {
	USB_PRODUCT_DESC_STRING_LEN * 2 + 2, // size of String Descriptor = 6 bytes
	0x03,                       // 03: String Descriptor type
	USB_PRODUCT_DESC_STRING
};

#if HID_FUNC
#include "hid_def.h"

#define HID_READ_BUFFER_POOL     HD_COMMON_MEM_USER_DEFINIED_POOL
#define HID_WRITE_BUFFER_POOL	(HID_READ_BUFFER_POOL+1)

#define HID_READ_BUFFER_SIZE    (512*1024)
#define HID_WRITE_BUFFER_SIZE	(512*1024)

typedef struct _HID_MEM_PARM {
	UINT32 pa;
	UINT32 va;
	UINT32 size;
} HID_MEM_PARM;

static HID_MEM_PARM hid_read_buf = {0};
static HID_MEM_PARM hid_write_buf = {0};

//just a sample for report descriptor
static UINT8 report_desc[] = {
	HID_USAGE_PAGE_16(VENDOR_USAGE_PAGE_CFU_TLC&0xFF, VENDOR_USAGE_PAGE_CFU_TLC>>8),
	HID_USAGE_16(VENDOR_USAGE_CFU&0xFF, VENDOR_USAGE_CFU>>8),
	HID_COLLECTION(APPLICATION),
		HID_USAGE_8(VERSION_FEATURE_USAGE),
		HID_REPORT_ID(CFU_GET_REATURE_REPORT_ID),
		HID_LOGICAL_MIN_8(0x00),//a dummy item for CV test
		HID_LOGICAL_MAX_8(0x7F),//a dummy item for CV test
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(sizeof(CFU_FW_VERSION)),
		HID_FEATURE_8(Data_Var_Abs),

		HID_USAGE_8(CONTENT_OUTPUT_USAGE),
		HID_REPORT_ID(CFU_UPDATE_CONTENT_REPORT_ID),
		HID_OUTPUT_16(Data_Var_Abs, BuffBytes),
		HID_USAGE_8(CONTENT_RESPONSE_INPUT_USAGE),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(sizeof(CFU_UPDATE_CONTENT_RESP)),
		HID_INPUT_8(Data_Var_Abs),

		HID_USAGE_8(OFFER_OUTPUT_USAGE),
		HID_REPORT_ID(CFU_UPDATE_OFFER_REPORT_ID),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(sizeof(CFU_UPDATE_OFFER)),
		HID_OUTPUT_8(Data_Var_Abs),

		HID_USAGE_8(OFFER_RESPONSE_INPUT_USAGE),
		HID_REPORT_SIZE(8),
		HID_REPORT_COUNT(sizeof(CFU_UPDATE_OFFER_RESP)),
		HID_INPUT_8(Data_Var_Abs),
	HID_END_COLLECTION(),
	HID_USAGE_PAGE_16(VENDOR_USAGE_PAGE_VC_TLC&0xFF, VENDOR_USAGE_PAGE_VC_TLC>>8),
	HID_USAGE_16(VENDOR_USAGE_VC&0xFF, VENDOR_USAGE_VC>>8),
	HID_COLLECTION(APPLICATION),
		HID_USAGE_8(VC_REQUEST_FEATURE_USAGE),
		HID_REPORT_ID(VC_REQUEST_REPORT_ID),
		HID_FEATURE_16(Data_Var_Abs, BuffBytes),

		HID_USAGE_8(VC_RESPONSE_INPUT_USAGE),
		HID_REPORT_ID(VC_RESPONSE_REPORT_ID),
		HID_INPUT_8(Data_Var_Abs),
	HID_END_COLLECTION()
};
static BOOL UvacHidCB(UINT8 request, UINT16 value, UINT8 *p_data, UINT32 *p_dataLen)
{
	BOOL ret = TRUE;
	UINT8 report_type, report_id;

	report_type = value >> 8;
	report_id = value & 0xFF;

	printf("%s:request = 0x%X, ReportType=0x%X, ReportID=0x%X\r\n", __func__, request, report_type, report_id);

	//just a sample
	switch(request)
	{
		case SET_REPORT:
	    case SET_IDLE:
	    case SET_PROTOCOL:
	        ret = TRUE;
            break;
		case GET_REPORT:
		default:
			ret = FALSE;
			break;
	}
	return ret;
}
#endif


static void USBMakerInit_UVAC(UVAC_VEND_DEV_DESC *pUVACDevDesc)
{

	pUVACDevDesc->pManuStringDesc = (UVAC_STRING_DESC *)m_UVACManuStrDesc;
	pUVACDevDesc->pProdStringDesc = (UVAC_STRING_DESC *)m_UVACProdStrDesc;

	pUVACDevDesc->pSerialStringDesc = (UVAC_STRING_DESC *)m_UVACSerialStrDesc3;

	pUVACDevDesc->VID = USB_VID;
	pUVACDevDesc->PID = USB_PID_PCCAM;

}

static UINT32 xUvacStartVideoCB(UVAC_VID_DEV_CNT vidDevIdx, UVAC_STRM_INFO *pStrmInfo)
{
	int ret=0;
	int fps;
	int bitrate;
	//HD_VIDEOPROC_IN video_in_param = {0};

	if (pStrmInfo) {
		printf("UVAC Start[%d] resoIdx=%d,W=%d,H=%d,codec=%d,fps=%d,path=%d,tbr=0x%x\r\n", vidDevIdx, pStrmInfo->strmResoIdx, pStrmInfo->strmWidth, pStrmInfo->strmHeight, pStrmInfo->strmCodec, pStrmInfo->strmFps, pStrmInfo->strmPath, pStrmInfo->strmTBR);

		//stop poll data
		pthread_mutex_lock(&flow_start_lock);
		encode_start = 0;
		acquire_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

		printf("g_capbind = %d\r\n", g_capbind);
		// stop VIDEO_STREAM modules (main)
		if (g_capbind == 1) {
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoproc_stop(stream[0].proc_sub_path);
			hd_videocap_stop(stream[0].cap_path);
		} else {
			hd_videocap_stop(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoproc_stop(stream[0].proc_sub_path);

		}
		if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
			hd_videoenc_stop(stream[0].enc_main_path);
		}
		m_VideoFmt[vidDevIdx] = pStrmInfo->strmCodec;

		//set codec,resolution
		switch (pStrmInfo->strmCodec){
			case UVAC_VIDEO_FORMAT_H264:
				stream[0].codec_type = HD_CODEC_TYPE_H264;
				//g_low_latency = 1;
				break;
			case UVAC_VIDEO_FORMAT_MJPG:
				stream[0].codec_type = HD_CODEC_TYPE_JPEG;
				//g_low_latency = 0;
				break;
			case UVAC_VIDEO_FORMAT_YUV:
				stream[0].codec_type = HD_CODEC_TYPE_RAW;
				//g_low_latency = 0;
				break;
			default:
				printf("unrecognized codec(%d)\r\n",pStrmInfo->strmCodec);
				break;
		}
		printf("strmCodec = %d\r\n", pStrmInfo->strmCodec);
		//set encode resolution to device
		stream[0].enc_main_dim.w = pStrmInfo->strmWidth;
		stream[0].enc_main_dim.h = pStrmInfo->strmHeight;

		//one buffer limitation
		hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
		hd_videoproc_close(stream[0].proc_main_path);
		hd_videoproc_close(stream[0].proc_sub_path);
		set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim);
		#if 0
		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV) {
			//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].enc_main_dim);
			hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
			video_in_param.dim.w = stream[0].enc_main_dim.w;
			video_in_param.dim.h = stream[0].enc_main_dim.h;
			hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		} else {
			//set_proc_cfg(&stream[0].proc_ctrl, &stream[0].cap_dim);
			hd_videoproc_get(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
			video_in_param.dim.w = stream[0].cap_dim.w;
			video_in_param.dim.h = stream[0].cap_dim.h;
			hd_videoproc_set(stream[0].proc_main_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
		}
		#endif
		hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &stream[0].proc_main_path);
		hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_1, &stream[0].proc_sub_path);
		hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);

		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			//YUV format: stop vdoenc and allow pull from vprc sub path
			ret = set_proc_param(stream[0].proc_main_path, &stream[0].cap_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
			ret = set_proc_param(stream[0].proc_sub_path, &stream[0].enc_main_dim, VDO_PXLFMT_YUV422_YUYV, 1, 0);
		} else {
			ret = set_proc_param(stream[0].proc_main_path, &stream[0].enc_main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
		}
		if (ret != HD_OK) {
			printf("set proc fail=%d\n", ret);
		}
		if (stream[0].codec_type == HD_CODEC_TYPE_JPEG) {
			//MJPG bitrate
			if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M ) {
				bitrate = MJPEG_TBR_MAX;
			} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
				bitrate = MJPEG_TBR_2M;
			} else {
				bitrate = MJPEG_TBR_720P;
			}
		} else if (stream[0].codec_type == HD_CODEC_TYPE_RAW) {
			//YUV bitrate
			bitrate = 8*12*1024*1024;
		} else {
			//H264 bitrate
			if (stream[0].enc_main_dim.w > VDO_SIZE_W_2M) {
				bitrate = H264_TBR_MAX;
			} else if(stream[0].enc_main_dim.w > VDO_SIZE_W_720P) {
				bitrate = H264_TBR_2M;
			} else {
				bitrate = H264_TBR_720P;
			}
		}
		// set videoenc parameter (main)
		if (stream[0].codec_type != HD_CODEC_TYPE_RAW){
			ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, stream[0].codec_type, bitrate);
		}
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
		}
		fps = pStrmInfo->strmFps;
		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].cap_dim);
		} else {
			ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, fps, &stream[0].enc_main_dim);
		}
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
		}
		if (g_capbind == 0) {
			//start engine(modules)
			hd_videocap_start(stream[0].cap_path);
		}
		hd_videoproc_start(stream[0].proc_main_path);
		if (pStrmInfo->strmCodec == UVAC_VIDEO_FORMAT_YUV){
			hd_videoproc_start(stream[0].proc_sub_path);
		}
		if (g_capbind == 1) {
			//start engine(modules)
			hd_videocap_start(stream[0].cap_path);
		}
		// just wait ae/awb stable for auto-test, if don't care, user can remove it
		//sleep(1);
		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV){
			hd_videoenc_start(stream[0].enc_main_path);
		}

		if (pStrmInfo->strmCodec != UVAC_VIDEO_FORMAT_YUV){
			pthread_mutex_lock(&flow_start_lock);
			encode_start = 1;
			acquire_start = 0;
			pthread_mutex_unlock(&flow_start_lock);
		} else {
			pthread_mutex_lock(&flow_start_lock);
			encode_start = 0;
			acquire_start = 1;
			pthread_mutex_unlock(&flow_start_lock);
		}
	}
	return E_OK;
}

static UINT32 xUvacStopVideoCB(UINT32 isClosed)
{
	//printf(":isClosed=%d\r\n", isClosed);
	printf("stop encode flow\r\n");
	pthread_mutex_lock(&flow_start_lock);
	encode_start = 0;
	acquire_start = 0;
	pthread_mutex_unlock(&flow_start_lock);
	return E_OK;
}

static UINT32 xUvacStartAudioCB(UVAC_AUD_DEV_CNT audDevIdx, UVAC_STRM_INFO *pStrmInfo)
{
	UINT32 stop_acap = FALSE;

	if (pStrmInfo->isAudStrmOn) {
		//stop poll data
		pthread_mutex_lock(&flow_start_lock);
		flow_audio_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

#if AUDIO_ON
		//start audio capture module
		hd_audiocap_start(au_cap.cap_path);
#endif
		pthread_mutex_lock(&flow_start_lock);
		flow_audio_start = 1;
		pthread_mutex_unlock(&flow_start_lock);
	} else {
		pthread_mutex_lock(&flow_start_lock);
		if (flow_audio_start) {
			stop_acap = TRUE;
		}
		flow_audio_start = 0;
		pthread_mutex_unlock(&flow_start_lock);

#if AUDIO_ON
		//stop audio capture module
		if (stop_acap) {
			hd_audiocap_stop(au_cap.cap_path);
		}
#endif

		return E_OK;
	}

	return E_OK;
}

#if (MSDC_FUNC == 1)
static UINT32 user_va = 0;
static UINT32 user_pa = 0;
static HD_COMMON_MEM_VB_BLK msdc_blk;
static PMSDC_OBJ p_msdc_object;
static _ALIGNED(64) UINT8 InquiryData[36] = {
	0x00, 0x80, 0x05, 0x02, 0x20, 0x00, 0x00, 0x00,
//    //Vendor identification, PREMIER
	'N', 'O', 'V', 'A', 'T', 'E', 'K', '-',
//    //product identification, DC8365
	'N', 'T', '9', '8', '5', '2', '8', '-',
	'D', 'S', 'P', ' ', 'I', 'N', 'S', 'I',
//    //product revision level, 1.00
	'D', 'E', ' ', ' '
};

static BOOL emuusb_msdc_strg_detect(void)
{
	return 1;
}
#endif

#if CDC_FUNC
typedef _PACKED_BEGIN struct {
	UINT32   uiBaudRateBPS; ///< Data terminal rate, in bits per second.
	UINT8    uiCharFormat;  ///< Stop bits.
	UINT8    uiParityType;  ///< Parity.
	UINT8    uiDataBits;    ///< Data bits (5, 6, 7, 8 or 16).
} _PACKED_END CDCLineCoding;

typedef enum _CDC_PSTN_REQUEST {
	REQ_SET_LINE_CODING         =    0x20,
	REQ_GET_LINE_CODING         =    0x21,
	REQ_SET_CONTROL_LINE_STATE  =    0x22,
	REQ_SEND_BREAK              =    0x23,
	ENUM_DUMMY4WORD(CDC_PSTN_REQUEST)
} CDC_PSTN_REQUEST;

#define CDC_READ_BUFFER_POOL     HD_COMMON_MEM_USER_DEFINIED_POOL
#define CDC_WRITE_BUFFER_POOL	(CDC_READ_BUFFER_POOL+1)

#define CDC_READ_BUFFER_SIZE    (512*1024)
#define CDC_WRITE_BUFFER_SIZE	(512*1024)

typedef struct _CDC_MEM_PARM {
	UINT32 pa;
	UINT32 va;
	UINT32 size;
} CDC_MEM_PARM;

static CDC_MEM_PARM cdc_read_buf = {0};
static CDC_MEM_PARM cdc_write_buf = {0};

static BOOL CdcPstnReqCB(CDC_COM_ID ComID, UINT8 Code, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL bSupported = TRUE;
	CDCLineCoding LineCoding;

	switch (Code) {
	case REQ_GET_LINE_CODING:
		LineCoding.uiBaudRateBPS = 115200;
		LineCoding.uiCharFormat = 0;//CDC_LINEENCODING_OneStopBit;
		LineCoding.uiParityType = 0;//CDC_PARITY_None;
		LineCoding.uiDataBits = 8;
		*pDataLen = sizeof(LineCoding);

		memcpy(pData, &LineCoding, *pDataLen);
		break;
	case REQ_SET_LINE_CODING:
		if (*pDataLen == sizeof(LineCoding)) {
			memcpy(&LineCoding, pData, *pDataLen);
		} else {
			bSupported = FALSE;
		}
		break;
	case REQ_SET_CONTROL_LINE_STATE:
		//debug console test
		if (*(UINT16 *)pData == 0x3) { //console ready
		}
		break;
	default:
		bSupported = FALSE;
		break;
	}
	return bSupported;
}
#endif

static BOOL UVAC_enable(void)
{

	UVAC_INFO       UvacInfo = {0};
	UINT32          retV = 0;

	UVAC_VEND_DEV_DESC UIUvacDevDesc = {0};
	UVAC_AUD_SAMPLERATE_ARY uvacAudSampleRateAry = {0};
	UINT32 bVidFmtType = UVAC_VIDEO_FORMAT_H264_MJPEG;
	m_bIsStaticPattern = FALSE;

	printf("###Real Open UVAC-lib\r\n");

	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	UINT32 pa;
	void *va;
	HD_RESULT hd_ret;

	hd_ret = hd_gfx_init();
	if(hd_ret != HD_OK) {
        printf("init gfx fail=%d\n", hd_ret);
	}

	#if (MSDC_FUNC == 1)
	{
		STORAGE_OBJ *pStrg = sdio_getStorageObject(STRG_OBJ_FAT1);
		USB_MSDC_INFO       MSDCInfo;
		UINT32 i;
		HD_RESULT                 ret;
		UVAC_MSDC_INFO       uvac_msdc_info;

		if (pStrg == NULL) {
			printf("failed to get STRG_OBJ_FAT1.n");
			return -1;
		}

		msdc_blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, POOL_SIZE_USER_DEFINIED, DDR_ID0);
	    if (HD_COMMON_MEM_VB_INVALID_BLK == msdc_blk) {
	            printf("hd_common_mem_get_block fail\r\n");
	            return HD_ERR_NG;
	    }
	    user_pa = hd_common_mem_blk2pa(msdc_blk);
	    if (user_pa == 0) {
	            printf("not get buffer, pa=%08x\r\n", (int)user_pa);
	            return HD_ERR_NG;
	    }
	    user_va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, user_pa, POOL_SIZE_USER_DEFINIED);
	    /* Release buffer */
	    if (user_va == 0) {
	            printf("mem map fail\r\n");
	            ret = hd_common_mem_munmap((void *)user_va, POOL_SIZE_USER_DEFINIED);
	            if (ret != HD_OK) {
	                    printf("mem unmap fail\r\n");
	                    return ret;
	            }
	            return HD_ERR_NG;
	    }

		p_msdc_object = Msdc_getObject(MSDC_ID_USB20);
		p_msdc_object->SetConfig(USBMSDC_CONFIG_ID_SELECT_POWER,  USBMSDC_POW_SELFPOWER);
		p_msdc_object->SetConfig(USBMSDC_CONFIG_ID_COMPOSITE,  TRUE);

		MSDCInfo.uiMsdcBufAddr_va= (UINT32)user_va;
		MSDCInfo.uiMsdcBufAddr_pa= (UINT32)user_pa;
		if (MSDCInfo.uiMsdcBufAddr_pa == 0) {
			printf("malloc buffer failed\n");
		}
		MSDCInfo.uiMsdcBufSize = MSDC_MIN_BUFFER_SIZE;
		MSDCInfo.pInquiryData = (UINT8 *)&InquiryData[0];

		MSDCInfo.msdc_check_cb = NULL;
		MSDCInfo.msdc_vendor_cb = NULL;

		MSDCInfo.msdc_RW_Led_CB = NULL;
		MSDCInfo.msdc_StopUnit_Led_CB = NULL;
		MSDCInfo.msdc_UsbSuspend_Led_CB = NULL;

		for (i = 0; i < MAX_LUN; i++) {
			MSDCInfo.msdc_type[i] = MSDC_STRG;
		}

		MSDCInfo.pStrgHandle[0] = pStrg;//pSDIOObj
		MSDCInfo.msdc_strgLock_detCB[0] = NULL;
		MSDCInfo.msdc_storage_detCB[0] = emuusb_msdc_strg_detect;
		MSDCInfo.LUNs = 1;

		if (p_msdc_object->Open(&MSDCInfo) != E_OK) {
			printf("msdc open failed\r\n ");
		}

		uvac_msdc_info.en = TRUE;
		UVAC_SetConfig(UVAC_CONFIG_MSDC_INFO, (UINT32) &uvac_msdc_info);
	}
	#elif (MSDC_FUNC == 2)
	{
		UVAC_MSDC_INFO uvac_msdc_info;
		uvac_msdc_info.en = TRUE;
		nvt_cmdsys_runcmd("msdcnvt open");
		UVAC_SetConfig(UVAC_CONFIG_MSDC_INFO, (UINT32) &uvac_msdc_info);
	}
	#endif
#if  UAC_RX_ON
	UVAC_SetConfig(UVAC_CONFIG_UAC_RX_ENABLE, TRUE);

	uvacAudSampleRateAry.aryCnt = NVT_UI_UVAC_AUD_SAMPLERATE_CNT;
	uvacAudSampleRateAry.pAudSampleRateAry = &gUIUvacAudRxSampleRate[0];
	UVAC_SetConfig(UVAC_CONFIG_AUD_RX_SAMPLERATE, (UINT32)&uvacAudSampleRateAry);
	UVAC_SetConfig(UVAC_CONFIG_AUD_RX_CHANNEL_NUM, aud_rx_sound_mode);
	UVAC_SetConfig(UVAC_CONFIG_UAC_RX_BLK_SIZE, UAC_RX_BLOCK_SIZE);
#endif
	//UVAC_SetConfig(UVAC_CONFIG_MAX_FRAME_SIZE, 4*1024*1024); //default 800KB, MAX:3MB

	uvac_size = UVAC_GetNeedMemSize();
	//printf("uvac_size = %u\r\n", uvac_size);
	if ((hd_ret = hd_common_mem_alloc("usbmovie", &pa, (void **)&va, uvac_size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc failed(%d)\r\n", hd_ret);
		uvac_va = 0;
		uvac_pa = 0;
		uvac_size = 0;
	} else {
		uvac_va = (UINT32)va;
		uvac_pa = (UINT32)pa;
	}

	UvacInfo.UvacMemAdr    = uvac_pa;
	UvacInfo.UvacMemSize   = uvac_size;

	UvacInfo.fpStartVideoCB  = (UVAC_STARTVIDEOCB)xUvacStartVideoCB;
	UvacInfo.fpStopVideoCB  = (UVAC_STOPVIDEOCB)xUvacStopVideoCB;

	USBMakerInit_UVAC(&UIUvacDevDesc);

	UVAC_SetConfig(UVAC_CONFIG_VEND_DEV_DESC, (UINT32)&UIUvacDevDesc);
	UVAC_SetConfig(UVAC_CONFIG_HW_COPY_CB, (UINT32)hd_gfx_memcpy);

	UVAC_SetConfig(UVAC_CONFIG_AUD_START_CB, (UINT32)xUvacStartAudioCB);
	//printf("%s:uvacAddr=0x%x,s=0x%x;IPLAddr=0x%x,s=0x%x\r\n", __func__, UvacInfo.UvacMemAdr, UvacInfo.UvacMemSize, m_VideoBuf.addr, m_VideoBuf.size);

	#if 0 //sample code for customized EU descriptor
	{
		static UINT8 baSourceID[] = {0}; //this should be a static array, set 0 for default
		static UINT8 bmControls[] = {0xFF, 0x00}; //this should be a static array
		UVAC_EU_DESC eu_desc = {0};
		//UINT8 guid[16] = {0x29, 0xa7, 0x87, 0xc9, 0xd3, 0x59, 0x69, 0x45, 0x84, 0x67, 0xff, 0x08, 0x49, 0xfc, 0x19, 0xe8};
		UINT8 guid[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

		eu_desc.bUnitID = 5;
		eu_desc.bDescriptorType = CS_INTERFACE;
		eu_desc.bDescriptorSubtype = VC_EXTENSION_UNIT;
		memcpy(eu_desc.guidExtensionCode, guid, sizeof(eu_desc.guidExtensionCode));
		eu_desc.bNrInPins = sizeof(baSourceID);
		eu_desc.baSourceID = baSourceID;
		eu_desc.bControlSize = sizeof(bmControls);
		eu_desc.bmControls = bmControls;
		eu_desc.bNumControls = get_eu_control_num(eu_desc.bmControls, eu_desc.bControlSize);
		eu_desc.bLength = 24 + eu_desc.bNrInPins + eu_desc.bControlSize;
		UVAC_SetConfig(UVAC_CONFIG_EU_DESC, (UINT32)&eu_desc);
	}
	#endif

	#if 0 //sample code for customized EU descriptor
	{
		static UINT8 baSourceID[] = {0}; //this should be a static array, set 0 for default
		static UINT8 bmControls[] = {0xFF, 0x00}; //this should be a static array
		UVAC_EU_DESC eu_desc = {0};
		//UINT8 guid[16] = {0x29, 0xa7, 0x87, 0xc9, 0xd3, 0x59, 0x69, 0x45, 0x84, 0x67, 0xff, 0x08, 0x49, 0xfc, 0x19, 0xe8};
		UINT8 guid[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

		eu_desc.bUnitID = 5;
		eu_desc.bDescriptorType = CS_INTERFACE;
		eu_desc.bDescriptorSubtype = VC_EXTENSION_UNIT;
		memcpy(eu_desc.guidExtensionCode, guid, sizeof(eu_desc.guidExtensionCode));
		eu_desc.bNrInPins = sizeof(baSourceID);
		eu_desc.baSourceID = baSourceID;
		eu_desc.bControlSize = sizeof(bmControls);
		eu_desc.bmControls = bmControls;
		eu_desc.bNumControls = get_eu_control_num(eu_desc.bmControls, eu_desc.bControlSize);
		eu_desc.bLength = 24 + eu_desc.bNrInPins + eu_desc.bControlSize;
		eu_desc.eu_cb = (UVAC_UNIT_CB)xUvacEU2_CB;
		UVAC_SetConfig(UVAC_CONFIG_EU_DESC_2ND, (UINT32)&eu_desc);
	}
	#endif

	#if 0 //sample code for setting UVC and UAC string
	{
		//this should be a static array
		static UINT16 uvc_string_descriptor[] = {
			0x0316,                             // 16: size of String Descriptor = 22 bytes
												// 03: String Descriptor type
			'U', 'V', 'C', '_', 'D',            // UVC_DEVICE
			'E', 'V', 'I', 'C', 'E'
		};
		//this should be a static array
		static UINT16 uac_string_descriptor[] = {
			0x0316,                             // 16: size of String Descriptor = 22 bytes
												// 03: String Descriptor type
			'U', 'A', 'C', '_', 'D',            // UAC_DEVICE
			'E', 'V', 'I', 'C', 'E'
		};
		UVAC_SetConfig(UVAC_CONFIG_UVC_STRING, (UINT32)uvc_string_descriptor);
		UVAC_SetConfig(UVAC_CONFIG_UAC_STRING, (UINT32)uac_string_descriptor);
	}
	#endif
	#if HID_FUNC//sample code for setting HID
	{
		//this should be a static array
		static UINT16 hid_string_descriptor[] = {
			0x0316,                             // 16: size of String Descriptor = 22 bytes
												// 03: String Descriptor type
			'H', 'I', 'D', '_', 'D',            // UAC_DEVICE
			'E', 'V', 'I', 'C', 'E'
		};
		UVAC_HID_INFO hid_info = {0};

		hid_info.en = TRUE;
		hid_info.cb = UvacHidCB;
		hid_info.hid_desc.bLength = 9;
		hid_info.hid_desc.bHidDescType = 0x21;
		hid_info.hid_desc.bcdHID = 0x0111;
		hid_info.hid_desc.bCountryCode = 0;
		hid_info.hid_desc.bNumDescriptors = 1;
		hid_info.hid_desc.bDescriptorType = 0x22;//Report Descriptor
		hid_info.hid_desc.wDescriptorLength = sizeof(report_desc);
		hid_info.p_report_desc = report_desc;
		hid_info.intr_out = TRUE;
		hid_info.intr_interval = 3;
		hid_info.p_vendor_string = (UVAC_STRING_DESC *)hid_string_descriptor;
		UVAC_SetConfig(UVAC_CONFIG_HID_INFO, (UINT32)&hid_info);
	}
	#endif

	//must set to 1I14P ratio
	UvacInfo.strmInfo.strmWidth = MAX_BS_W;
	UvacInfo.strmInfo.strmHeight = MAX_BS_H;
	UvacInfo.strmInfo.strmFps = MAX_BS_FPS;
	//UvacInfo.strmInfo.strmCodec = MEDIAREC_ENC_H264;
	//w=1280,720,30,2
#if 1 //MJPG/H264 can only share the same resolution array if use this API.
	gUIUvacVidReso[0].width = cap_size_w;
	gUIUvacVidReso[0].height = cap_size_h;
	UVAC_ConfigVidReso(gUIUvacVidReso, NVT_UI_UVAC_RESO_CNT);
#else //if you want to set differnet resolution for MJPG/H264, use this API.
	UVAC_VID_RESO_ARY uvacVidYuvResoAry = {0};

	uvacVidYuvResoAry.aryCnt = sizeof(gUIUvacVidResoMJPEG)/sizeof(UVAC_VID_RESO);
	uvacVidYuvResoAry.pVidResAry = &gUIUvacVidResoMJPEG[0];
	UVAC_SetConfig(UVAC_CONFIG_UVC_MJPG_FRM_INFO, (UINT32)&uvacVidYuvResoAry);

	uvacVidYuvResoAry.aryCnt = sizeof(gUIUvacVidResoH264)/sizeof(UVAC_VID_RESO);
	uvacVidYuvResoAry.pVidResAry = &gUIUvacVidResoH264[0];
	UVAC_SetConfig(UVAC_CONFIG_UVC_H264_FRM_INFO, (UINT32)&uvacVidYuvResoAry);
#endif
	//set default frame index, starting from 1
	//UVAC_SetConfig(UVAC_CONFIG_MJPG_DEF_FRM_IDX, 1);
	//UVAC_SetConfig(UVAC_CONFIG_H264_DEF_FRM_IDX, 1);

	#if UVC_SUPPORT_YUV_FMT
	{
		UVAC_VID_RESO_ARY uvacVidYuvResoAry = {0};

		uvacVidYuvResoAry.aryCnt = sizeof(gUIUvacVidYuvReso)/sizeof(UVAC_VID_RESO);
		uvacVidYuvResoAry.pVidResAry = &gUIUvacVidYuvReso[0];
		//set default frame index, starting from 1
		//uvacVidYuvResoAry.bDefaultFrameIndex = 1;
		UVAC_SetConfig(UVAC_CONFIG_YUV_FRM_INFO, (UINT32)&uvacVidYuvResoAry);
	}
	#endif
	uvacAudSampleRateAry.aryCnt = NVT_UI_UVAC_AUD_SAMPLERATE_CNT;
	uvacAudSampleRateAry.pAudSampleRateAry = &gUIUvacAudSampleRate[0];
	UVAC_SetConfig(UVAC_CONFIG_AUD_SAMPLERATE, (UINT32)&uvacAudSampleRateAry);
	UvacInfo.channel = 1;
	//printf("w=%d,h=%d,fps=%d,codec=%d\r\n", UvacInfo.strmInfo.strmWidth, UvacInfo.strmInfo.strmHeight, UvacInfo.strmInfo.strmFps, UvacInfo.strmInfo.strmCodec);
	UVAC_SetConfig(UVAC_CONFIG_AUD_CHANNEL_NUM, au_cap.sound_mode);
	UVAC_SetConfig(UVAC_CONFIG_VIDEO_FORMAT_TYPE, bVidFmtType);

#if 0
	//Please see comment in lib/include/UVAC.h to understand this setting
	{
		UVAC_PAYLOAD_TX_SIZE payload_tx_size;
		static UINT32 h264_payload_size[] = {
			0x800,
			0x400,
		};
		static UINT32 mjpg_payload_size[] = {
			0xC00,
			0x400,
		};
		static UINT32 yuv_payload_size[] = {
			0xC00,
			0x400,
		};

		memset((void *)(&payload_tx_size), 0, sizeof(UVAC_PAYLOAD_TX_SIZE));
		payload_tx_size.vidDevIdx = UVAC_VID_DEV_CNT_1;
		payload_tx_size.video_fmt = UVAC_VIDEO_FORMAT_H264;
		payload_tx_size.p_payload_tx_size = h264_payload_size;
		payload_tx_size.cnt = sizeof(h264_payload_size)/sizeof(UINT32);
		UVAC_SetConfig(UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE, (UINT32)&payload_tx_size);

		memset((void *)(&payload_tx_size), 0, sizeof(UVAC_PAYLOAD_TX_SIZE));
		payload_tx_size.vidDevIdx = UVAC_VID_DEV_CNT_1;
		payload_tx_size.video_fmt = UVAC_VIDEO_FORMAT_MJPG;
		payload_tx_size.p_payload_tx_size = mjpg_payload_size;
		payload_tx_size.cnt = sizeof(mjpg_payload_size)/sizeof(UINT32);
		UVAC_SetConfig(UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE, (UINT32)&payload_tx_size);

		memset((void *)(&payload_tx_size), 0, sizeof(UVAC_PAYLOAD_TX_SIZE));
		payload_tx_size.vidDevIdx = UVAC_VID_DEV_CNT_1;
		payload_tx_size.video_fmt = UVAC_VIDEO_FORMAT_YUV;
		payload_tx_size.p_payload_tx_size = yuv_payload_size;
		payload_tx_size.cnt = sizeof(yuv_payload_size)/sizeof(UINT32);
		UVAC_SetConfig(UVAC_CONFIG_MAX_PAYLOAD_TX_SIZE, (UINT32)&payload_tx_size);
	}
#endif

	//Extension Unit
	#if UVC_EU
	UVAC_SetConfig(UVAC_CONFIG_XU_CTRL, (UINT32)xUvacEU_CB);
	#else
	//deprecated, backward compatible
	//UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID01, (UINT32)xUvacEUVendCmdCB_Idx1);
	//UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID02, (UINT32)xUvacEUVendCmdCB_Idx2);
	//UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID03, (UINT32)xUvacEUVendCmdCB_Idx3);
	//UVAC_SetConfig(UVAC_CONFIG_EU_VENDCMDCB_ID04, (UINT32)xUvacEUVendCmdCB_Idx4);
	#endif

	//CT & PU
	#if UVC_CT
	UVAC_SetConfig(UVAC_CONFIG_CT_CONTROLS, uvc_ct_controls);
	UVAC_SetConfig(UVAC_CONFIG_CT_CB, (UINT32)xUvacCT_CB);
	#endif
	#if UVC_PU
	UVAC_SetConfig(UVAC_CONFIG_PU_CONTROLS, uvc_pu_controls);
	UVAC_SetConfig(UVAC_CONFIG_PU_CB, (UINT32)xUvacPU_CB);
	#endif

	#if 0	//sample code for setting UAC wMaxPacketSize and bInterval
	UVAC_SetConfig(UVAC_CONFIG_UAC_PACKETSIZE, 68);
	UVAC_SetConfig(UVAC_CONFIG_UAC_INTERVAL, 4);
	#endif

	#if CDC_FUNC
	UVAC_SetConfig(UVAC_CONFIG_CDC_PSTN_REQUEST_CB, (UINT32)CdcPstnReqCB);
	UVAC_SetConfig(UVAC_CONFIG_CDC_ENABLE, TRUE);
	#endif
	retV = UVAC_Open(&UvacInfo);
	if (retV != E_OK) {
		printf("Error open UVAC task(%u)\r\n", retV);
	}

	return TRUE;
}

static BOOL UVAC_disable(void)
{
	HD_RESULT    ret = HD_OK;

	#if (MSDC_FUNC == 1)
	{
		p_msdc_object->Close();

		if(msdc_blk) {
			hd_common_mem_release_block(msdc_blk);
		}
		if(user_va) {
			if(hd_common_mem_munmap((void *)user_va, POOL_SIZE_USER_DEFINIED)) {
				printf("fail to unmap va(%x)\n", (int)user_va);
			}
		}
	}
	#elif (MSDC_FUNC == 2)
	{
		nvt_cmdsys_runcmd("msdcnvt close");
	}
	#endif

	ret = hd_common_mem_free(uvac_pa, &uvac_va);
	if (HD_OK == ret) {
		printf("Error handle test for free an invalid address: fail %d\r\n", (int)(ret));
	}
	UVAC_Close();

	return TRUE;
}
#endif
static HD_RESULT set_proc_ct_crop_param(HD_PATH_ID video_proc_path, HD_IRECT *af_crop)
{
	HD_RESULT ret = HD_OK;
	double	ratio_w = 0;
	double	ratio_h = 0;
	HD_VIDEOPROC_CROP video_out_crop_param = {0};
	HD_VIDEOPROC_OUT video_out_param = {0};

	ratio_w = (double)cap_size_w / (double)af_crop->w;
	ratio_h = (double)cap_size_h / (double)af_crop->h;
	if (af_crop != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		video_out_param.func = 0;
		video_out_param.dim.w = cap_size_w * ratio_w;
		video_out_param.dim.h = cap_size_h * ratio_h;
		video_out_param.dim.w = ALIGN_CEIL_4(video_out_param.dim.w);
		video_out_param.dim.h = ALIGN_CEIL_4(video_out_param.dim.h);
		//printf ("video_out_param.dim.w: %u video_out_param.dim.h: %u\r\n", video_out_param.dim.w, video_out_param.dim.h);
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		video_out_param.depth = 0; //set 1 to allow pull
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
		video_out_crop_param.mode = HD_CROP_ON;
		video_out_crop_param.win.rect.x = af_crop->x * ratio_w;
		video_out_crop_param.win.rect.y = af_crop->y * ratio_h;

		video_out_crop_param.win.rect.x = ALIGN_CEIL_4(video_out_crop_param.win.rect.x);
		video_out_crop_param.win.rect.y = ALIGN_CEIL_4(video_out_crop_param.win.rect.y);
		video_out_crop_param.win.rect.w = cap_size_w;
		video_out_crop_param.win.rect.h = cap_size_h;
		//boundary checking (TODO)
#if 0
		if (video_out_crop_param.win.rect.x + video_out_crop_param.win.rect.w > 1920){
			video_out_crop_param.win.rect.x = video_out_crop_param.win.rect.x - (video_out_crop_param.win.rect.x + video_out_crop_param.win.rect.w - 1920);
		}
		if (video_out_crop_param.win.rect.y + video_out_crop_param.win.rect.h > 1080){
			video_out_crop_param.win.rect.y = video_out_crop_param.win.rect.y - (video_out_crop_param.win.rect.y + video_out_crop_param.win.rect.h - 1080);
		}
#endif
		if (video_out_crop_param.win.rect.x < 0)
			video_out_crop_param.win.rect.x = 0;
		if (video_out_crop_param.win.rect.y < 0)
			video_out_crop_param.win.rect.y = 0;

		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_CROP, &video_out_crop_param);
	} else {
		video_out_crop_param.mode = HD_CROP_OFF;
		video_out_crop_param.win.rect.x = 0;
		video_out_crop_param.win.rect.y = 0;
		video_out_crop_param.win.rect.w = 0;
		video_out_crop_param.win.rect.h = 0;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_CROP, &video_out_crop_param);
	}

	return ret;
}

void Set_CT_Zoom_Absolute(UINT32 indx)
{
	int ret;
	static HD_IRECT ct_crop_size = {0};
	ct_crop_size.w = cap_size_w - (indx * 20);
	ct_crop_size.h = cap_size_h - (indx * 12);
	ct_crop_size.x = (cap_size_w - ct_crop_size.w) / 2;
	ct_crop_size.y = (cap_size_h - ct_crop_size.h) / 2;

	ret = set_proc_ct_crop_param(stream[0].proc_main_path, &ct_crop_size);
	if (ret != HD_OK) {
		printf("fail to set vp afmask attr\r\n");
	}
	ret = hd_videoproc_start(stream[0].proc_main_path);
	if (ret != HD_OK) {
		printf("fail to start proc_path %d\n", ret);
	}
}

#if HID_FUNC
void write_hid_test(UINT8 *p_buf, UINT32 buf_size, INT32 timeout)
{
	INT32 ret_value;

	memcpy((void *)hid_write_buf.va, p_buf, buf_size);
	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)hid_write_buf.va, buf_size);
	printf("Write HID %d byte(%d)\r\n", buf_size, timeout);
	ret_value = UVAC_WriteHidData((void *)hid_write_buf.pa, buf_size, timeout);
	printf("Write HID ret %d\r\n", ret_value);
}
void read_hid_test(UINT32 buf_size, INT32 timeout)
{
	INT32 i;
	UINT8 *pBuf;
	INT32 ret_value;

	printf("Read HID %d byte(%d)\r\n", buf_size, timeout);
	ret_value = UVAC_ReadHidData((UINT8 *)hid_read_buf.pa, buf_size, timeout);
	printf("Read HID ret %d\r\n", ret_value);

	if (ret_value > 0) {
		vendor_common_mem_cache_sync((void *)hid_read_buf.va, ret_value, VENDOR_COMMON_MEM_DMA_FROM_DEVICE);
		//dump buffer
		pBuf = (UINT8 *)hid_read_buf.va;
		for (i = 0; i < ret_value; i++) {
			if (i % 16 == 0) {
				printf("\r\n");
			}
			printf("0x%02X ", *(pBuf + i));
		}
		printf("\r\n");
	}
}
static INT32 get_hid_mem_block(VOID)
{
	HD_RESULT hd_ret;
	HD_COMMON_MEM_DDR_ID      ddr_id = DDR_ID0;
	UINT32 pa;
	void *va;

	hid_read_buf.size = HID_READ_BUFFER_SIZE;
	if ((hd_ret = hd_common_mem_alloc("hid_read", &pa, (void **)&va, hid_read_buf.size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc hid_read_buf failed(%d)\r\n", hd_ret);
		hid_read_buf.va = 0;
		hid_read_buf.pa = 0;
		hid_read_buf.size = 0;
	} else {
		hid_read_buf.va = (UINT32)va;
		hid_read_buf.pa = (UINT32)pa;
	}

	hid_write_buf.size = HID_WRITE_BUFFER_SIZE;
	if ((hd_ret |= hd_common_mem_alloc("hid_write", &pa, (void **)&va, hid_write_buf.size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc hid_write_buf failed(%d)\r\n", hd_ret);
		hid_write_buf.va = 0;
		hid_write_buf.pa = 0;
		hid_write_buf.size = 0;
	} else {
		hid_write_buf.va = (UINT32)va;
		hid_write_buf.pa = (UINT32)pa;
	}

	return hd_ret;
}

static HD_RESULT release_hid_mem_block(VOID)
{
	HD_RESULT ret = HD_OK;

	/* Release in buffer */
	if (hid_read_buf.va) {
		ret = hd_common_mem_free(hid_read_buf.pa, (void *) hid_read_buf.va);
		if (ret != HD_OK) {
			printf("mem_uninit : (hid_read_buf.va)hd_common_mem_free fail.\r\n");
			return ret;
		}
	}
	if (hid_write_buf.va) {
		ret = hd_common_mem_free(hid_write_buf.pa, (void *) hid_write_buf.va);
		if (ret != HD_OK) {
			printf("mem_uninit : (hid_write_buf.va)hd_common_mem_free fail.\r\n");
			return ret;
		}
	}


	return ret;
}
#endif// HID_FUNC

#if CDC_FUNC
static void write_cdc_test(UINT8 *p_buf, UINT32 buf_size, INT32 timeout)
{
	INT32 ret_value;

	memcpy((void *)cdc_write_buf.va, p_buf, buf_size);
	//--- data is written by CPU, flush CPU cache to PHY memory ---
	hd_common_mem_flush_cache((void *)cdc_write_buf.va, buf_size);
	printf("Write CDC %d byte(%d)\r\n", buf_size, timeout);
	ret_value = UVAC_WriteCdcData(CDC_COM_1ST, (void *)cdc_write_buf.pa, buf_size, timeout);
	printf("Write CDC ret %d\r\n", ret_value);
}
static void read_cdc_test(UINT32 buf_size, INT32 timeout)
{
	INT32 i;
	UINT8 *pBuf;
	INT32 ret_value;

	printf("Read CDC %d byte(%d)\r\n", buf_size, timeout);
	ret_value = UVAC_ReadCdcData(CDC_COM_1ST, (UINT8 *)cdc_read_buf.pa, buf_size, timeout);
	printf("Read CDC ret %d\r\n", ret_value);

	if (ret_value > 0) {
		vendor_common_mem_cache_sync((void *)cdc_read_buf.va, ret_value, VENDOR_COMMON_MEM_DMA_FROM_DEVICE);
		//dump buffer
		pBuf = (UINT8 *)cdc_read_buf.va;
		*(pBuf + ret_value) = '\0';
		printf("%s ", pBuf);
		printf("\r\n");
	}
}
static INT32 get_cdc_mem_block(VOID)
{
	HD_RESULT hd_ret;
	HD_COMMON_MEM_DDR_ID      ddr_id = DDR_ID0;
	UINT32 pa;
	void *va;

	cdc_read_buf.size = CDC_READ_BUFFER_SIZE;
	if ((hd_ret = hd_common_mem_alloc("cdc_read", &pa, (void **)&va, cdc_read_buf.size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc cdc_read_buf failed(%d)\r\n", hd_ret);
		cdc_read_buf.va = 0;
		cdc_read_buf.pa = 0;
		cdc_read_buf.size = 0;
	} else {
		cdc_read_buf.va = (UINT32)va;
		cdc_read_buf.pa = (UINT32)pa;
	}

	cdc_write_buf.size = CDC_WRITE_BUFFER_SIZE;
	if ((hd_ret |= hd_common_mem_alloc("cdc_write", &pa, (void **)&va, cdc_write_buf.size, ddr_id)) != HD_OK) {
		printf("hd_common_mem_alloc cdc_write_buf failed(%d)\r\n", hd_ret);
		cdc_write_buf.va = 0;
		cdc_write_buf.pa = 0;
		cdc_write_buf.size = 0;
	} else {
		cdc_write_buf.va = (UINT32)va;
		cdc_write_buf.pa = (UINT32)pa;
	}

	return hd_ret;
}

static HD_RESULT release_cdc_mem_block(VOID)
{
	HD_RESULT ret = HD_OK;

	/* Release in buffer */
	if (cdc_read_buf.va) {
		ret = hd_common_mem_free(cdc_read_buf.pa, (void *) cdc_read_buf.va);
		if (ret != HD_OK) {
			printf("mem_uninit : (cdc_read_buf.va)hd_common_mem_free fail.\r\n");
			return ret;
		}
	}
	if (cdc_write_buf.va) {
		ret = hd_common_mem_free(cdc_write_buf.pa, (void *) cdc_write_buf.va);
		if (ret != HD_OK) {
			printf("mem_uninit : (cdc_write_buf.va)hd_common_mem_free fail.\r\n");
			return ret;
		}
	}


	return ret;
}
#endif// CDC_FUNC

static void app_main(int argc, char* argv[])
{
	HD_RESULT ret;
	//INT key;

	UINT32 enc_type = HD_CODEC_TYPE_H264;
	HD_DIM main_dim;

	stream[0].codec_type = enc_type;
	// query program options
	if (argc > 1) {
		sensor_sel = atoi(argv[1]);
		printf("sensor_sel %d\r\n", sensor_sel);
	}
	if (sensor_sel == SEN_SEL_IMX290) {
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	} else if (sensor_sel == SEN_SEL_OS05A10) {
		cap_size_w = VDO_SIZE_W_5M;
		cap_size_h = VDO_SIZE_H_5M;
	} else if (sensor_sel == SEN_SEL_GC4653) {
		cap_size_w = VDO_SIZE_W_4M;
		cap_size_h = VDO_SIZE_H_4M;
	} else {
		printf("Not support sensor_sel %d\r\n", sensor_sel);
		sensor_sel = SEN_SEL_IMX290;
		cap_size_w = VDO_SIZE_W_2M;
		cap_size_h = VDO_SIZE_H_2M;
	}

	AET_CFG_INFO cfg_info = {0};
	if (vendor_isp_init() != HD_ERR_NG) {
		cfg_info.id = 0;
		//strncpy(cfg_info.path, "/mnt/app/isp/sc8238_20200916-9.cfg", CFG_NAME_LENGTH);
		//printf("sensor 1 load /mnt/app/isp/sc8238_20200916-9.cfg\n");
		vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
		vendor_isp_uninit();
	}
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	// init memory
#if (MSDC_FUNC == 2)
	mempool_init();
#endif
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

#if HID_FUNC
	ret = get_hid_mem_block();
	if (ret != HD_OK) {
		printf("hid_mem_init fail=%d\n", ret);
		goto exit;
	}
#endif

#if CDC_FUNC
	ret = get_cdc_mem_block();
	if (ret != HD_OK) {
		printf("cdc_mem_init fail=%d\n", ret);
		goto exit;
	}
#endif

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

	// open VIDEO_STREAM modules (main)
	stream[0].proc_max_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].proc_max_dim.h = MAX_CAP_SIZE_H; //assign by user

	ret = open_module(&stream[0], &stream[0].proc_max_dim);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

#if AUDIO_ON
	//open capture module
	au_cap.sample_rate_max = gUIUvacAudSampleRate[0];
	au_cap.sound_mode = HD_AUDIO_SOUND_MODE_STEREO;		//UAC sound mode: HD_AUDIO_SOUND_MODE_STEREO or HD_AUDIO_SOUND_MODE_MONO
	ret = open_module2(&au_cap);
	if(ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	//set audiocap parameter
	au_cap.sample_rate = gUIUvacAudSampleRate[0];
	ret = set_cap_param2(au_cap.cap_path, au_cap.sample_rate, au_cap.sound_mode);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}
#endif

	// get videocap capability
	ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
	if (ret != HD_OK) {
		printf("get cap-caps fail=%d\n", ret);
		goto exit;
	}

	// set videocap parameter
	stream[0].cap_dim.w = MAX_CAP_SIZE_W; //assign by user
	stream[0].cap_dim.h = MAX_CAP_SIZE_H; //assign by user
	ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, MAX_BS_FPS, &stream[0].cap_dim);
	if (ret != HD_OK) {
		printf("set cap fail=%d\n", ret);
		goto exit;
	}

	// assign parameter by program options
	main_dim.w = MAX_BS_W;
	main_dim.h = MAX_BS_H;
	// set videoproc parameter (main)
	ret = set_proc_param(stream[0].proc_main_path, &main_dim, HD_VIDEO_PXLFMT_YUV420, 0, 1);
	if (ret != HD_OK) {
		printf("1111set proc fail=%d\n", ret);
		goto exit;
	}

	// set videoenc config (main), enc_cfg uses H264 to define bitstream buffer
	stream[0].enc_main_max_dim.w = MAX_BS_W;
	stream[0].enc_main_max_dim.h = MAX_BS_H;
	if ((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(30) < ((H264_TBR_MAX)/(1024*1024))*UVC_H264_ENC_BUF_MS){	//H264 max bs buffer > MJPEG max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, H264_TBR_MAX);
	} else {		//MJPEG max bs buffer > H264 max bs buffer
		ret = set_enc_cfg(stream[0].enc_main_path, &stream[0].enc_main_max_dim, enc_type, (((MJPEG_TBR_MAX/(1024*1024))*UVC_MJPEG_ENC_BUF_MS(MAX_BS_FPS))/UVC_H264_ENC_BUF_MS)*1024*1024);
	}

	if (ret != HD_OK) {
		printf("set enc-cfg fail=%d\n", ret);
		goto exit;
	}

	// set videoenc parameter (main)
	stream[0].enc_main_dim.w = main_dim.w;
	stream[0].enc_main_dim.h = main_dim.h;
	ret = set_enc_param(stream[0].enc_main_path, &stream[0].enc_main_dim, enc_type, H264_TBR_MAX);
	if (ret != HD_OK) {
		printf("set enc fail=%d\n", ret);
		goto exit;
	}
	// bind VIDEO_STREAM modules (main)
	hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
	hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

#if AUDIO_ON
	//audio create capture thread
	THREAD_CREATE(cap_thread_id, capture_thread, (void *)&au_cap, "capture_thread");
	if (0 == cap_thread_id) {
		printf("create encode thread failed");
		goto exit;
	}
	THREAD_RESUME(cap_thread_id);
	//start audio capture module
	hd_audiocap_start(au_cap.cap_path);
#endif

#if  UAC_RX_ON
	ret = pthread_create(&uac_thread_id, NULL, uac_thread, (void *)&au_cap);
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}
#endif

	// start VIDEO_STREAM modules (main)
	if (g_capbind == 1) {
		//direct NOTE: ensure videocap start after 1st videoproc phy path start
		hd_videoproc_start(stream[0].proc_main_path);
		hd_videocap_start(stream[0].cap_path);
	} else {
		hd_videocap_start(stream[0].cap_path);
		hd_videoproc_start(stream[0].proc_main_path);
	}

	// just wait ae/awb stable for auto-test, if don't care, user can remove it
	//sleep(1);
	hd_videoenc_start(stream[0].enc_main_path);
	// create encode_thread (pull_out bitstream)
	THREAD_CREATE(stream[0].enc_thread_id, encode_thread, (void *)stream, "encode_thread");
	if (0 == stream[0].enc_thread_id) {
		printf("create encode thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].enc_thread_id);

#if UVC_SUPPORT_YUV_FMT
	THREAD_CREATE(stream[0].acquire_thread_id, acquire_yuv_thread, (void *)stream, "acquire_yuv_thread");
	if (0 == stream[0].acquire_thread_id) {
		printf("create acquire thread failed");
		goto exit;
	}
	THREAD_RESUME(stream[0].acquire_thread_id);
#endif

	pthread_mutex_lock(&flow_start_lock);
	flow_audio_start = 0;
	encode_start = 0;
	acquire_start = 0;
	pthread_mutex_unlock(&flow_start_lock);

#if UVC_ON
	sleep(1);
	UVAC_enable(); //init usb device, start to prepare video/audio buffer to send
#endif

	// query user key
	printf("Enter q to exit\n");
#if 0
	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			pthread_mutex_lock(&flow_start_lock);
			flow_audio_start = 0;
			encode_start = 0;
			acquire_start = 0;
			pthread_mutex_unlock(&flow_start_lock);
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1; //stop video

#if AUDIO_ON
			au_cap.cap_exit = 1; //stop audio
#endif

			// quit program
			break;
		}

		#if (DEBUG_MENU == 1)
		if (key == 'd') {
			// enter debug menu
			hd_debug_run_menu();
			printf("\r\nEnter q to exit, Enter d to debug\r\n");
		}
		#endif
		if (key == '0') {
			get_cap_sysinfo(stream[0].cap_ctrl);
		}
		if (key == 'u') {
			uvc_debug_menu_entry_p(uvc_debug_menu, "UVAC DEBUG MAIN");
		}
#if HID_FUNC
		if (key == '1') {
			UINT8 intr_in_test[8] = {1, 2, 3, 4, 5, 6, 7, 8};

			write_hid_test(intr_in_test, sizeof(intr_in_test), 0);
		}
		if (key == '2') {
			UINT8 intr_in_test[8] = {8, 7, 6, 4, 5, 6, 7, 8};

			write_hid_test(intr_in_test, sizeof(intr_in_test), 5000);
		}
		if (key == '3') {
			UINT8 intr_in_test[8] = {0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1};

			write_hid_test(intr_in_test, sizeof(intr_in_test), -1);
		}
		if (key == '4') {
			read_hid_test(8, 0);
		}
		if (key == '5') {
			read_hid_test(100, 5000);
		}
		if (key == '6') {
			read_hid_test(8, -1);
		}
#else //internal test only
		if (key == '1') {
			printf("Stop video ++\r\n");
			_UVAC_stream_enable(0, 0);
			printf("Stop video --\r\n");
		}
		if (key == '2') {
			printf("Start video ++\r\n");
			_UVAC_stream_enable(0, 1);
			printf("Start video --\r\n");
		}
		if (key == '3') {
			printf("Stop aud ++\r\n");
			_UVAC_stream_enable(1, 0);
			printf("Stop aud --\r\n");
		}
		if (key == '4') {
			printf("Start aud ++\r\n");
			_UVAC_stream_enable(1, 1);
			printf("Start aud --\r\n");
		}
#endif

#if CDC_FUNC
		if (key == 'z') {
			UINT8 in_test[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

			write_cdc_test(in_test, sizeof(in_test), -1);
		}

		if (key == 'x') {
			read_cdc_test(5, -1);
		}
#endif
	}
#else // Reduce CPU loading. 8x% ---> 1x%.
	while(1) {sleep(1);}
#endif
	// destroy encode thread
	THREAD_DESTROY(stream[0].enc_thread_id);
#if UVC_SUPPORT_YUV_FMT
	THREAD_DESTROY(stream[0].acquire_thread_id);
#endif
#if AUDIO_ON
	THREAD_DESTROY(cap_thread_id); // stop audio

#if  UAC_RX_ON
	THREAD_DESTROY(uac_thread_id); // stop RX audio

#endif
#endif

#if UVC_ON
	UVAC_disable();
#endif

#if AUDIO_ON
	//stop audio capture module
	hd_audiocap_stop(au_cap.cap_path);
#endif
	// stop VIDEO_STREAM modules (main)
	if (g_capbind == 1){
		hd_videoproc_stop(stream[0].proc_main_path);
		hd_videocap_stop(stream[0].cap_path);
	} else {
		hd_videocap_stop(stream[0].cap_path);
		hd_videoproc_stop(stream[0].proc_main_path);
	}
	hd_videoenc_stop(stream[0].enc_main_path);
	// unbind VIDEO_STREAM modules (main)
	hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
	hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

exit:
	// close VIDEO_STREAM modules (main)
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

#if AUDIO_ON
	//close audio module
	ret = close_module2(&au_cap);
	if(ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}
#endif

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

#if AUDIO_ON
	// uninit all modules
	ret = exit_module2();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}
#endif

    ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("hd_gfx_uninit fail=%d\n", ret);
	}

#if HID_FUNC
	ret = release_hid_mem_block();
	if (ret != HD_OK) {
		printf("mem_hid_uninit fail=%d\n", ret);
	}
#endif

#if CDC_FUNC
	ret = release_cdc_mem_block();
	if (ret != HD_OK) {
		printf("mem_cdc_uninit fail=%d\n", ret);
	}
#endif

	// uninit memory
	ret = mem_exit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "hdal.h"
#include <kwrap/examsys.h>
#include <kwrap/cmdsys.h>

static int check_if_multiple_instance(char *p_name)
{
	char buf[128];
	FILE *pp;

	snprintf(buf, sizeof(buf), "ps | grep %s | wc -l", p_name);
	if ((pp = popen(buf, "r")) == NULL) {
		printf("popen() error!\n");
		exit(1);
	}

	while (fgets(buf, sizeof buf, pp)) {
	}

	pclose(pp);

	if(atoi(buf) > 3) {
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (check_if_multiple_instance(argv[0]) == 1) {
		if (argc > 1) {
			nvt_cmdsys_ipc_cmd(argc, argv);
			return 0;
		}
		printf("%s has already in running. quit.\n", argv[0]);
		return -1;
	}

	nvt_cmdsys_init();      // command system
	nvt_examsys_init();     // exam system

	//start do your program
	app_main(argc, argv);
	return 0;
}
