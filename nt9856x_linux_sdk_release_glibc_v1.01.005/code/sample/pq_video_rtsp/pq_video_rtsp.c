/**
	@brief Sample code of video rtsp.\n

	@file pq_video_rtsp.c

	@author Photon Lin

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#include "hdal.h"
#include "hd_debug.h"
#include "nvtrtspd.h"
#include "vendor_isp.h"
#include "vendor_videocapture.h"


// platform dependent
#if defined(__LINUX)
#include <pthread.h> //for pthread API
#define MAIN(argc, argv) int main(int argc, char** argv)
#define GETCHAR() getchar()
#else
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h> //for sleep API
#define sleep(x) vos_util_delay_ms(1000*(x))
#define msleep(x) vos_util_delay_ms(x)
#define usleep(x) vos_util_delay_us(x)
#include <kwrap/examsys.h> //for MAIN(), GETCHAR() API
#define MAIN(argc, argv) EXAMFUNC_ENTRY(pq_video_rtsp, argc, argv)
#define GETCHAR() NVT_EXAMSYS_GETCHAR()
#endif

#define SEN_SEL_NONE            0
#define SEN_SEL_IMX290          1
#define SEN_SEL_OS02K10         2
#define SEL_SEL_AR0237IR        3
#define SEN_SEL_OV2718          4
#define SEN_SEL_OS05A10         5
#define SEN_SEL_IMX317          6
#define SEN_SEL_IMX335          7
#define SEN_SEL_F37             8
#define SEN_SEL_PS5268          9
#define SEN_SEL_SC4210         10
#define SEN_SEL_IMX307_SLAVE   11
#define SEN_SEL_F35            12
#define SEN_SEL_IMX415         13
#define SEN_SEL_SC500AI        14
#define SEN_SEL_SC401AI        15
#define SEN_SEL_OS04A10        16
#define SEN_SEL_GC4653         17
#define SEN_SEL_IMX307         18
#define SEN_SEL_SC4238         19
#define SEN_SEL_PATGEN         99

///////////////////////////////////////////////////////////////////////////////
//plug function
#define PLUG_NNSC_FUNC          0
#define PLUG_OSG_FUNC           0//w4000 0

#define NNSC_NET_MODEL_SIZE_V_2_02_000   15315488
#define NNSC_MODEL_SIZE                  NNSC_NET_MODEL_SIZE_V_2_02_000
#define VENDOR_AI_CFG                    0x000F0000

///////////////////////////////////////////////////////////////////////////////
//stripe level
#define HD_VIDEOPROC_CFG                0x000F0000 //vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK     0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1      0x00000000  //vprc "0: cut w>1280, GDC =  on, 2D_LUT off after cut (LL slow)
#define HD_VIDEOPROC_CFG_STRIP_LV2      0x00010000  //vprc "1: cut w>2048, GDC = off, 2D_LUT off after cut (LL fast)
#define HD_VIDEOPROC_CFG_STRIP_LV3      0x00020000  //vprc "2: cut w>2688, GDC = off, 2D_LUT off after cut (LL middle)(2D_LUT best)
#define HD_VIDEOPROC_CFG_STRIP_LV4      0x00030000  //vprc "3: cut w> 720, GDC =  on, 2D_LUT off after cut (LL not allow)(GDC best)
#define HD_VIDEOPROC_CFG_DISABLE_GDC    HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST        HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST     HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST       HD_VIDEOPROC_CFG_STRIP_LV4

///////////////////////////////////////////////////////////////////////////////
//pixel start for patgen
#define PATGEN_START_PIX_R  HD_VIDEO_PIX_RGGB_R
#define PATGEN_START_PIX_GR HD_VIDEO_PIX_RGGB_GR
#define PATGEN_START_PIX_GB HD_VIDEO_PIX_RGGB_GB
#define PATGEN_START_PIX_B  HD_VIDEO_PIX_RGGB_B

#define PATGEN_START_PIX HD_VIDEO_PIX_RGGB_R

///////////////////////////////////////////////////////////////////////////////
//header
#define DBGINFO_BUFSIZE() (0x200)

//RAW
#define VDO_RAW_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NRX: RAW compress: Only support 12bit mode
#define RAW_COMPRESS_RATIO 59
#define VDO_NRX_BUFSIZE(w, h) (ALIGN_CEIL_4(ALIGN_CEIL_64(w) * 12 / 8 * RAW_COMPRESS_RATIO / 100 * (h)))
//CA for AWB
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
//LA for AE
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
//VA for AF
#define VDO_VA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 4) << 1)

//YUV
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
//NVX: YUV compress
#define YUV_COMPRESS_RATIO 75
#define VDO_NVX_BUFSIZE(w, h, pxlfmt) (VDO_YUV_BUFSIZE(w, h, pxlfmt) * YUV_COMPRESS_RATIO / 100)

// MD
#define MD_HEAD_BUFSIZE()	            (0x40)
// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64(((((w >> 7) + 3) >> 2) << 2) * ((h + 15) >> 4)))

///////////////////////////////////////////////////////////////////////////////
#define ONEBUF_ENABLE       1
#define YUV_COMPRESS_ENABLE 0

#define SEN_OUT_FMT      HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT      HD_VIDEO_PXLFMT_RAW12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_NRX12_SHDR2
#define CA_WIN_NUM_W     32
#define CA_WIN_NUM_H     32
#define LA_WIN_NUM_W     32
#define LA_WIN_NUM_H     32
#define VA_WIN_NUM_W      8
#define VA_WIN_NUM_H      8
#define YOUT_WIN_NUM_W  128
#define YOUT_WIN_NUM_H  128
#define ETH_8BIT_SEL      0 //0: 2bit out, 1:8 bit out
#define ETH_OUT_SEL       1 //0: full, 1: subsample 1/2

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M_BIG  2880
#define VDO_SIZE_H_5M_BIG  1620
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_4M_BIG  2688
#define VDO_SIZE_H_4M_BIG  1520
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080

#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_AF | HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA

static UINT32 sensor_sel_1 = 0;
static UINT32 sensor_sel_2 = 0;
static UINT32 sensor_shdr_1 = 0;
static UINT32 sensor_shdr_2 = 0;
static UINT32 data_mode1 = 1; //option for vcap output directly to vprc
static UINT32 data_mode2 = 0; //option for vprc output lowlatency to venc
static UINT32 patgen_size_w = 1920, patgen_size_h = 1080;
static UINT32 vdo_size_w_1 = 1920, vdo_size_h_1 = 1080;
static UINT32 vdo_size_w_2 = 1920, vdo_size_h_2 = 1080;
static BOOL is_nt98528 = 0;

///////////////////////////////////////////////////////////////////////////////
#define PHY2VIRT_MAIN(pa) (vir_addr_main + (pa - phy_buf_main.buf_info.phy_addr))
#define PHY2VIRT_SUB(pa) (vir_addr_sub + (pa - phy_buf_sub.buf_info.phy_addr))

static UINT32 vir_addr_main;
static HD_VIDEOENC_BUFINFO phy_buf_main;
static UINT32 vir_addr_sub;
static HD_VIDEOENC_BUFINFO phy_buf_sub;

#if PLUG_OSG_FUNC
#define OSG_ICON_FILE_NAME      "/mnt/sd/osg/pq_osg_icon_w20_h32.dat"
#define OSG_ICON_FILE_SIZE      79360

#define OSG_ICON_WIDTH          20
#define OSG_ICON_HEIGHT         32

#define OSG_ICON_NUM_NUMBER     10
#define OSG_ICON_CHAR_NUMBER    26

#define OSG_TRANSP          0x0000
#define OSG_WHITE           0xFFFF
#define OSG_RED             0xFF00
#define OSG_GREEN           0xF0F0
#define OSG_BULE            0xF00F
#define OSG_YELLOW          0xFFF0
#define OSG_PURPLE          0xFF0F
#define OSG_GRAY            0xF888

#define OSG_BAR_MAX_COLOR   OSG_RED
#define OSG_BAR_NORM_COLOR  OSG_WHITE
#define OSG_BG_COLOR        0x4444

#define PQ_OSG_ITEM_NUM     8
#define OSG_BAR_MAX         100
#define OSG_TITLE_MAX       20
#define OSG_STRING_MAX      (OSG_TITLE_MAX<<1)

#define SHM_FILE_NAME "pq_osg"
#define SHM_FILE_SIZE 512

typedef struct _VIDEO_OSG {
	HD_PATH_ID stamp_path;
	UINT32 stamp_blk;
	UINT32 stamp_pa;
	UINT32 stamp_size;
	UINT16 *osg_buf;
	pthread_t thread_id;
	BOOL thread_run;
} VIDEO_OSG;

VIDEO_OSG stream_osg = {0};

typedef struct _OSG_UPDATE_INFO {
	UINT16 value[PQ_OSG_ITEM_NUM];
	UINT8 title_text[PQ_OSG_ITEM_NUM][OSG_TITLE_MAX];
	UINT16 title_color[PQ_OSG_ITEM_NUM];
} OSG_UPDATE_INFO;

OSG_UPDATE_INFO osg_update_info = {
	.value = {0},
	.title_text = {
		{"PQ Info 1"},
		{"PQ Info 2"},
		{"PQ Info 3"},
		{"PQ Info 4"},
		{"PQ Info 5"},
		{"PQ Info 6"},
		{"PQ Info 7"},
		{"PQ Info 8"},
	},
	.title_color = {
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
	}
};

typedef struct _PQ_OSG_DISPLAY_ {
	UINT16 icon_w, icon_h, icon_size;
	UINT16 bar_step;
	UINT16 value[PQ_OSG_ITEM_NUM];
	UINT8 title_text[PQ_OSG_ITEM_NUM][OSG_TITLE_MAX];
	UINT16 title_color[PQ_OSG_ITEM_NUM];
	UINT8 string[PQ_OSG_ITEM_NUM][OSG_STRING_MAX];
	UINT16 line_icon_num[PQ_OSG_ITEM_NUM];
	UINT16 line_w[PQ_OSG_ITEM_NUM];
	UINT16 width, height;
} PQ_OSG_DISPLAY;

PQ_OSG_DISPLAY pq_osg_display = {
	.icon_w = OSG_ICON_WIDTH,
	.icon_h = OSG_ICON_HEIGHT,
	.icon_size = (OSG_ICON_WIDTH * OSG_ICON_HEIGHT),
	.bar_step = 1,
	.value = {0},
	.title_text = {
		{"PQ Info 1"},
		{"PQ Info 2"},
		{"PQ Info 3"},
		{"PQ Info 4"},
		{"PQ Info 5"},
		{"PQ Info 6"},
		{"PQ Info 7"},
		{"PQ Info 8"},
	},
	.title_color = {
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
		OSG_GREEN,
	},
	.line_icon_num = {0},
	.line_w = {0},
	.width = VDO_SIZE_W_2M,
	.height = VDO_SIZE_H_2M
};

static UINT16 *pq_osg_number;
static UINT16 *pq_osg_uppercase;
static UINT16 *pq_osg_lowercase;

static HD_RESULT shm_init(void)
{
	char * data;
	int fd;

	fd = shm_open(SHM_FILE_NAME, O_CREAT | O_EXCL | O_RDWR, 0777);
	if (fd < 0)
	{
		printf("shm_init: %s open fail \n", SHM_FILE_NAME);
		return HD_ERR_NG;
	}

	ftruncate(fd, SHM_FILE_SIZE);

	data = (char*)mmap(NULL, SHM_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!data) {
		printf("shm_init: mmap fail \n");
		close(fd);
		return HD_ERR_NG;
	}

	memcpy(data, &osg_update_info, sizeof(OSG_UPDATE_INFO));

	munmap(data , SHM_FILE_SIZE);
	close(fd);

	return HD_OK;
}

static HD_RESULT shm_read(void)
{
	char* data;
	int fd;

	fd = shm_open(SHM_FILE_NAME, O_RDWR, 0777);
	if (fd < 0) {
		printf("shm_init: %s open fail \n", SHM_FILE_NAME);
		return HD_ERR_NG;
	}

	data = (char*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!data) {
		printf("shm_read: mmap fail \n");
		close(fd);
		return HD_ERR_NG;
	}

	memcpy(&osg_update_info, data, sizeof(OSG_UPDATE_INFO));

	munmap(data , SHM_FILE_SIZE);
	close(fd);

	return HD_OK;
}

static UINT32 osg_calc_buf_size(UINT32 osg_w, UINT32 osg_h)
{
	HD_VIDEO_FRAME frame = {0};
	int stamp_size;

	frame.sign   = MAKEFOURCC('O','S','G','P');
	frame.dim.w  = osg_w;
	frame.dim.h  = osg_h;
	frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;

	//get required buffer size for a single image
	stamp_size = hd_common_mem_calc_buf_size(&frame);
	if (!stamp_size) {
		printf("fail to query buffer size\n");
		return -1;
	}

	//ping pong buffer needs double size
	stamp_size *= 2;

	return stamp_size;
}

HD_RESULT osg_mem_alloc(void)
{
	UINT32 pa;
	HD_COMMON_MEM_VB_BLK blk;

	if(!stream_osg.stamp_size){
		printf("stamp_size is unknown\n");
		return HD_ERR_NG;
	}

	//get osd stamp's block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stream_osg.stamp_size, DDR_ID0);
	if (blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail\r\n");
		return HD_ERR_NG;
	}

	stream_osg.stamp_blk = blk;

	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("blk2pa fail, blk = 0x%x\r\n", blk);
		return HD_ERR_NG;
	}

	stream_osg.stamp_pa = pa;

	return HD_OK;
}

static HD_RESULT osg_icon_init(void)
{
	UINT32 i, ix, ix_img, iy, iy_img, iyw, iyw_img;
	int fd, ret;
	UINT16 *osg_icon_buf = NULL;

	pq_osg_display.icon_w = OSG_ICON_WIDTH;
	pq_osg_display.icon_h = OSG_ICON_HEIGHT;
	pq_osg_display.icon_size = OSG_ICON_WIDTH * OSG_ICON_HEIGHT;

	osg_icon_buf = malloc(OSG_ICON_FILE_SIZE);
	if(!osg_icon_buf){
		printf("fail to allocate pq osg icon buffer\n");
		return HD_ERR_NG;
	}

	fd = open(OSG_ICON_FILE_NAME, O_RDONLY);
	if(fd == -1){
		printf("fail to open %s \n", OSG_ICON_FILE_NAME);
		return HD_ERR_NG;
	}

	ret = read(fd, osg_icon_buf, OSG_ICON_FILE_SIZE);
	close(fd);
	if (ret != OSG_ICON_FILE_SIZE) {
		printf("fail to read %s (%d, %d) \n", OSG_ICON_FILE_NAME, ret, OSG_ICON_FILE_SIZE);
		return HD_ERR_NG;
	}

	pq_osg_number = malloc(OSG_ICON_NUM_NUMBER * pq_osg_display.icon_size * sizeof(UINT16));
	pq_osg_uppercase = malloc(OSG_ICON_CHAR_NUMBER * pq_osg_display.icon_size * sizeof(UINT16));
	pq_osg_lowercase = malloc(OSG_ICON_CHAR_NUMBER * pq_osg_display.icon_size * sizeof(UINT16));

	for (i = 0; i < OSG_ICON_NUM_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < pq_osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * pq_osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER) * pq_osg_display.icon_w);
			for (ix = 0, ix_img = (i * pq_osg_display.icon_w); ix < pq_osg_display.icon_w; ix++, ix_img++) {
				pq_osg_number[(i * pq_osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img + ix_img];
			}
		}
	}

	for (i = 0; i < OSG_ICON_CHAR_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < pq_osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * pq_osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER) * pq_osg_display.icon_w);
			for (ix = 0, ix_img = ((i + OSG_ICON_NUM_NUMBER) * pq_osg_display.icon_w); ix < pq_osg_display.icon_w; ix++, ix_img++) {
				pq_osg_uppercase[(i * pq_osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img + ix_img];
			}
		}
	}

	for (i = 0; i < OSG_ICON_CHAR_NUMBER; i++) {
		for (iy = 0, iy_img = 0; iy < pq_osg_display.icon_h; iy++, iy_img++) {
			iyw = iy * pq_osg_display.icon_w;
			iyw_img = iy_img * ((OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER + OSG_ICON_CHAR_NUMBER)*pq_osg_display.icon_w);
			for(ix = 0, ix_img = ((i + OSG_ICON_NUM_NUMBER + OSG_ICON_CHAR_NUMBER) * pq_osg_display.icon_w); ix < pq_osg_display.icon_w; ix++, ix_img++) {
				pq_osg_lowercase[(i * pq_osg_display.icon_size) + iyw + ix] = osg_icon_buf[iyw_img+ix_img];
			}
		}
	}

	free(osg_icon_buf);

	return 0;
}

static HD_RESULT osg_icon_uninit(void)
{
	if (pq_osg_number != NULL) {
		free(pq_osg_number);
	}
	if (pq_osg_uppercase != NULL) {
		free(pq_osg_uppercase);
	}
	if (pq_osg_lowercase != NULL) {
		free(pq_osg_lowercase);
	}

	return HD_OK;
}

static void osg_clear_buf(UINT16 *osg_buf)
{
	UINT32 i;

	for (i = 0; i < (pq_osg_display.width * pq_osg_display.height); i++) {
		osg_buf[i] = OSG_BG_COLOR;
	}
}

static void osg_param_init(void)
{
	UINT32 i, idx, cnt;
	UINT32 osg_buf_w = 0, osg_buf_h = 0;
	CHAR string[OSG_STRING_MAX] = {0};
	UINT32 osg_buf_w_max = 0;

	for (idx = 0; idx < PQ_OSG_ITEM_NUM; idx++) {

		for (i = 0; i < OSG_STRING_MAX; i++) {
			string[i] = 0;
		}

		sprintf((CHAR *)string, "%s%3d", pq_osg_display.title_text[idx], pq_osg_display.value[idx]);

		cnt = 0;
		for (i = 0; i < OSG_STRING_MAX; i++) {
			if (string[i] != 0) {
				pq_osg_display.string[idx][i] = string[i];
				cnt++;
			} else {
				pq_osg_display.line_icon_num[idx] = cnt;
				pq_osg_display.line_w[idx] = (pq_osg_display.line_icon_num[idx] * pq_osg_display.icon_w);
				if((cnt * pq_osg_display.icon_w) >= osg_buf_w_max) {
					osg_buf_w_max = (cnt * pq_osg_display.icon_w);
				}
				break;
			}
		}
	}

	osg_buf_w = osg_buf_w_max;
	osg_buf_w += (pq_osg_display.bar_step * OSG_BAR_MAX);
	osg_buf_h = (PQ_OSG_ITEM_NUM * pq_osg_display.icon_h);

	if ((osg_buf_h % 16) != 0) {
		osg_buf_h = (((osg_buf_h / 16) + 1) * 16);
	} else {
		osg_buf_h = ((osg_buf_h / 16) * 16);
	}

	pq_osg_display.width = osg_buf_w;
	pq_osg_display.height = osg_buf_h;
}

#if 0
static UINT32 pq_osg_get_max_ch(void)
{
	UINT32 i, max_value = 0, max_ch = PQ_OSG_ITEM_NUM;

	for (i = 0; i < PQ_OSG_ITEM_NUM; i++) {
		if (pq_osg_display.value[i] > max_value) {
			max_value = pq_osg_display.value[i];
			max_ch = i;
		}
	}

	return max_ch;
}
#endif

static void osg_draw(UINT16 *osg_buf)
{
	UINT32 osg_ix, osg_iy, osg_iyw;
	UINT32 idx;
	UINT8 char_idx;
	UINT32 line_ix, line_iy, line_idx;
	UINT32 line_iyw;
	UINT32 line_osg_ix, line_osg_iy;
	UINT32 ix, iy;

	for (idx = 0; idx < PQ_OSG_ITEM_NUM; idx++) {
		for (line_idx = 0; line_idx < pq_osg_display.line_icon_num[idx]; line_idx++) {
			char_idx = pq_osg_display.string[idx][line_idx];
			for (line_iy = 0, line_osg_iy = (idx * pq_osg_display.icon_h); line_iy < pq_osg_display.icon_h; line_iy++, line_osg_iy++) {
				line_iyw = line_iy * pq_osg_display.icon_w;
				for (line_ix = 0, line_osg_ix = (line_idx * pq_osg_display.icon_w); line_ix < pq_osg_display.icon_w; line_ix++, line_osg_ix++) {
					osg_ix = line_osg_ix;
					osg_iy = line_osg_iy;
					osg_iyw = (osg_iy * pq_osg_display.width);

					if ((char_idx >= 65) && (char_idx <= 90)) {
						osg_buf[osg_iyw+osg_ix] = (pq_osg_uppercase[((char_idx - 65) * pq_osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : pq_osg_uppercase[((char_idx - 65) * pq_osg_display.icon_size) + line_iyw + line_ix];
					} else if ((char_idx >= 97) && (char_idx <= 122)) {
						osg_buf[osg_iyw + osg_ix] = (pq_osg_lowercase[((char_idx - 97) * pq_osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : pq_osg_lowercase[((char_idx-97) * pq_osg_display.icon_size) + line_iyw + line_ix];
					} else if ((char_idx >= 48) && (char_idx <= 57)) {
						osg_buf[osg_iyw + osg_ix] = (pq_osg_number[((char_idx  - 48) * pq_osg_display.icon_size) + line_iyw + line_ix] == 0x0000) ? OSG_BG_COLOR : pq_osg_number[((char_idx-48) * pq_osg_display.icon_size) + line_iyw + line_ix];
					} else {
						osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
					}

					if (osg_buf[osg_iyw+osg_ix] != OSG_BG_COLOR) {
						osg_buf[osg_iyw+osg_ix] = pq_osg_display.title_color[idx];
					}
				}
			}

			#if 0
			for (iy = 0; iy < pq_osg_display.icon_h; iy++) {
				for (ix = 0; ix < (OSG_BAR_MAX * pq_osg_display.bar_step); ix++) {
					osg_ix = ix + pq_osg_display.line_w[idx];
					osg_iy = iy + (idx * pq_osg_display.icon_h);
					osg_iyw = (osg_iy * pq_osg_display.width);

					if (ix < (pq_osg_display.value[idx] * pq_osg_display.bar_step)) {
						if ((pq_osg_get_max_ch() == idx) && (pq_osg_display.value[idx] > 50)) {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_MAX_COLOR;
						} else {
							osg_buf[osg_iyw+osg_ix] = OSG_BAR_NORM_COLOR;
						}
					} else {
						osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
					}
				}
			}
			#else
			for (iy = 0; iy < pq_osg_display.icon_h; iy++) {
				for (ix = 0; ix < (OSG_BAR_MAX * pq_osg_display.bar_step); ix++) {
					osg_ix = ix + pq_osg_display.line_w[idx];
					osg_iy = iy + (idx * pq_osg_display.icon_h);
					osg_iyw = (osg_iy * pq_osg_display.width);
					osg_buf[osg_iyw+osg_ix] = OSG_BG_COLOR;
				}
			}
			#endif
		}
	}
}

static void osg_update(void)
{
	UINT32 i, idx;
	CHAR string[OSG_STRING_MAX] = {0};

	memcpy(&pq_osg_display.value, &osg_update_info, sizeof(OSG_UPDATE_INFO));

	for (idx = 0; idx < PQ_OSG_ITEM_NUM; idx++) {

		for (i = 0; i < OSG_STRING_MAX; i++) {
			string[i] = 0;
		}
		
		if (pq_osg_display.value[idx] > OSG_BAR_MAX) {
			pq_osg_display.value[idx] = OSG_BAR_MAX;
		}

		sprintf((CHAR *)string, "%s%3d", pq_osg_display.title_text[idx], pq_osg_display.value[idx]);

		for (i = 0; i < OSG_STRING_MAX; i++) {
			if (string[i] != 0) {
				pq_osg_display.string[idx][i] = string[i];
			} else {
				break;
			}
		}
	}
}

static HD_RESULT osg_set_enc_stamp(void)
{
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;
	HD_OSG_STAMP_ATTR attr;

	if(!stream_osg.stamp_pa){
		printf("stamp buffer is not allocated\n");
		return -1;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.p_addr    = stream_osg.stamp_pa;
	buf.size      = stream_osg.stamp_size;

	if(hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_BUF, &buf) != HD_OK){
		printf("fail to set stamp buffer\n");
		return -1;
	}

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = HD_VIDEO_PXLFMT_ARGB4444;
	img.dim.w      = pq_osg_display.width;
	img.dim.h      = pq_osg_display.height;
	img.p_addr     = (UINT32)stream_osg.osg_buf;

	if(hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_IMG, &img) != HD_OK){
		printf("fail to set stamp image\n");
		return -1;
	}

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	attr.position.x = 10;
	attr.position.y = (vdo_size_h_1 - pq_osg_display.height - 10);
	attr.alpha      = 0;
	attr.layer      = 0;
	attr.region     = 0;

	return hd_videoenc_set(stream_osg.stamp_path, HD_VIDEOENC_PARAM_IN_STAMP_ATTR, &attr);
}

static void *osg_thread(void *arg)
{
	while(1) {
		usleep(30000);  // 30ms

		if (!stream_osg.thread_run) {
			break;
		}

		shm_read();
		osg_update();
		osg_draw(stream_osg.osg_buf);
		osg_set_enc_stamp();
	}

	return 0;
}
#endif

static HD_RESULT mem_init(void)
{
	HD_RESULT              ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap)
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
	if (data_mode1 == 1) { // NOTE: direct mode
		mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() +
										VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H) +
										VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H) +
										VDO_VA_BUF_SIZE(VA_WIN_NUM_W, VA_WIN_NUM_H);
	} else { // NOTE: dram mode
		mem_cfg.pool_info[0].blk_size = DBGINFO_BUFSIZE() +
										VDO_RAW_BUFSIZE(vdo_size_w_1, vdo_size_h_1, CAP_OUT_FMT) +
										VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H) +
										VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H) +
										VDO_VA_BUF_SIZE(VA_WIN_NUM_W, VA_WIN_NUM_H);
	}
	if (is_nt98528) {
		mem_cfg.pool_info[0].blk_cnt = 8;
	} else {
		mem_cfg.pool_info[0].blk_cnt = 2;
		if (sensor_shdr_1 == 1) {
			mem_cfg.pool_info[0].blk_cnt += 2;
		}
	}
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	// config common pool (main)
	mem_cfg.pool_info[1].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[1].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(vdo_size_w_1, vdo_size_h_1, HD_VIDEO_PXLFMT_YUV420)
													+MD_HEAD_BUFSIZE()+MD_INFO_BUFSIZE(vdo_size_w_1, vdo_size_h_1);
	if (is_nt98528) {
		mem_cfg.pool_info[1].blk_cnt = 6;
	} else {
		#if (ONEBUF_ENABLE == 1)
		mem_cfg.pool_info[1].blk_cnt = 1;
		if (data_mode1 == 0) {
			mem_cfg.pool_info[1].blk_cnt += 1;
		}
		#else
		mem_cfg.pool_info[1].blk_cnt = 3;
		#endif
	}
	mem_cfg.pool_info[1].ddr_id = DDR_ID0;

	#if (PLUG_NNSC_FUNC)
	mem_cfg.pool_info[2].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[2].blk_size = NNSC_MODEL_SIZE;
	mem_cfg.pool_info[2].blk_cnt = 1;
	mem_cfg.pool_info[2].ddr_id = DDR_ID0;
	#endif

	#if (PLUG_OSG_FUNC)
	mem_cfg.pool_info[3].type = HD_COMMON_MEM_OSG_POOL;
	mem_cfg.pool_info[3].blk_size = osg_calc_buf_size(vdo_size_w_1, vdo_size_h_1);
	mem_cfg.pool_info[3].blk_cnt = 1;
	mem_cfg.pool_info[3].ddr_id = DDR_ID0;
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

static HD_RESULT get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	HD_RESULT ret = HD_OK;
	hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	return ret;
}

static HD_RESULT set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};
	char *chip_name = getenv("NVT_CHIP_ID");

	if (sensor_sel_1 == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
	} else if (sensor_sel_1 == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
	} else if (sensor_sel_1 == SEN_SEL_OS02K10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
	} else if (sensor_sel_1 == SEL_SEL_AR0237IR) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ar0237ir");
	} else if (sensor_sel_1 == SEN_SEL_OV2718) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ov2718");
	} else if (sensor_sel_1 == SEN_SEL_IMX317) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx317");
	} else if (sensor_sel_1 == SEN_SEL_IMX335) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx335");
	} else if (sensor_sel_1 == SEN_SEL_F37) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f37");
	}else if (sensor_sel_1 == SEN_SEL_PS5268) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ps5268");
	} else if (sensor_sel_1 == SEN_SEL_SC4210) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc4210");
	} else if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx307_slave");
	} else if (sensor_sel_1 == SEN_SEL_F35) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f35");
	} else if (sensor_sel_1 == SEN_SEL_IMX415) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx415");
	} else if (sensor_sel_1 == SEN_SEL_SC500AI) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc500ai");
	} else if (sensor_sel_1 == SEN_SEL_SC401AI) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc401ai");
	} else if (sensor_sel_1 == SEN_SEL_OS04A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os04a10");
	} else if (sensor_sel_1 == SEN_SEL_GC4653) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc4653");
	} else if (sensor_sel_1 == SEN_SEL_IMX307) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx307");
	} else if (sensor_sel_1 == SEN_SEL_PATGEN) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, HD_VIDEOCAP_SEN_PAT_GEN);
	} else if (sensor_sel_1 == SEN_SEL_SC4238) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc4238");
	} else {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "NULL");
	}

	printf("sensor 1 using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	if (sensor_sel_1 == SEL_SEL_AR0237IR) {
		// Parallel interface
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x204; //PIN_SENSOR_CFG_12BITS | PIN_SENSOR_CFG_MCLK
	} else if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x2A0; //PIN_SENSOR_CFG_LVDS_VDHD | PIN_SENSOR_CFG_MIPI | PIN_SENSOR_CFG_MCLK
	} else {
		// MIPI interface
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x220; //PIN_SENSOR_CFG_MIPI | PIN_SENSOR_CFG_MCLK
	}

	if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.tge_en = TRUE;
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.swap = FALSE;
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.vcap_vd_src = HD_VIDEOCAP_SEN_TGE_CH1_VD_TO_VCAP0;
		if (sensor_sel_2 == SEN_SEL_IMX307_SLAVE) {
			cap_cfg.sen_cfg.sen_dev.if_cfg.tge.vcap_sync_set = (HD_VIDEOCAP_0|HD_VIDEOCAP_1);
		}
	}

	//Sensor interface choice
	if (sensor_sel_2 != SEN_SEL_NONE) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0x301;
	} else if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0x301;
	} else if (sensor_sel_1 == SEL_SEL_AR0237IR) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0x0;
	} else {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0xF01;//PIN_MIPI_LVDS_CFG_CLK0 | PIN_MIPI_LVDS_CFG_DAT0 | PIN_MIPI_LVDS_CFG_DAT1 | PIN_MIPI_LVDS_CFG_DAT2 | PIN_MIPI_LVDS_CFG_DAT3
	}
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NA51089") == 0) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x01;//PIN_I2C_CFG_CH1
	} else {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x10;//PIN_I2C_CFG_CH2
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	if (sensor_sel_2 != SEN_SEL_NONE) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	} else if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = HD_VIDEOCAP_SEN_IGNORE;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = HD_VIDEOCAP_SEN_IGNORE;
	} else {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}

	if (sensor_shdr_1 == 1) {
		if (is_nt98528) {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_3));
		} else {
			cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
		}
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

	if (sensor_shdr_1 == 1) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}
	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_cfg2(HD_PATH_ID *p_video_cap_ctrl)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};
	char *chip_name = getenv("NVT_CHIP_ID");

	if (sensor_sel_2 == SEN_SEL_IMX290) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx290");
	} else if (sensor_sel_2 == SEN_SEL_OS05A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os05a10");
	} else if (sensor_sel_2 == SEN_SEL_OS02K10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os02k10");
	} else if (sensor_sel_2 == SEL_SEL_AR0237IR) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ar0237ir");
	} else if (sensor_sel_2 == SEN_SEL_OV2718) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ov2718");
	} else if (sensor_sel_2 == SEN_SEL_IMX317) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx317");
	} else if (sensor_sel_2 == SEN_SEL_IMX335) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx335");
	} else if (sensor_sel_2 == SEN_SEL_F37) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f37");
	} else if (sensor_sel_2 == SEN_SEL_PS5268) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_ps5268");
	} else if (sensor_sel_2 == SEN_SEL_SC4210) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc4210");
	} else if (sensor_sel_2 == SEN_SEL_IMX307_SLAVE) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx307_slave");
	} else if (sensor_sel_2 == SEN_SEL_F35) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_f35");
	} else if (sensor_sel_2 == SEN_SEL_IMX415) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_imx415");
	} else if (sensor_sel_2 == SEN_SEL_SC500AI) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc500ai");
	} else if (sensor_sel_2 == SEN_SEL_SC401AI) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc401ai");
	} else if (sensor_sel_2 == SEN_SEL_OS04A10) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_os04a10");
	}else if (sensor_sel_2 == SEN_SEL_GC4653) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_gc4653");
	} else if (sensor_sel_2 == SEN_SEL_SC4238) {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "nvt_sen_sc4238");
	} else {
		snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, "NULL");
	}

	printf("sensor 2 using %s\n", cap_cfg.sen_cfg.sen_dev.driver_name);

	if (sensor_sel_2 == SEL_SEL_AR0237IR) {
		// Parallel interface
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_P_RAW;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x204; //PIN_SENSOR_CFG_12BITS | PIN_SENSOR2_CFG_MCLK
	} else if (sensor_sel_2 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x2A0; //PIN_SENSOR_CFG_LVDS_VDHD | PIN_SENSOR_CFG_MIPI | PIN_SENSOR2_CFG_MCLK
	} else {
		// MIPI interface
		cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x1020; //PIN_SENSOR_CFG_MIPI | PIN_SENSOR2_CFG_MCLK
	}

	if (sensor_sel_2 == SEN_SEL_IMX307_SLAVE) {
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.tge_en = TRUE;
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.swap = FALSE;
		cap_cfg.sen_cfg.sen_dev.if_cfg.tge.vcap_vd_src = HD_VIDEOCAP_SEN_TGE_CH1_VD_TO_VCAP0;
		if (sensor_sel_1 == SEN_SEL_IMX307_SLAVE) {
			cap_cfg.sen_cfg.sen_dev.if_cfg.tge.vcap_sync_set = (HD_VIDEOCAP_0|HD_VIDEOCAP_1);
		}
	}

	//Sensor interface choice
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0xC02;//PIN_MIPI_LVDS_CFG_CLK1 | PIN_MIPI_LVDS_CFG_DAT0 | PIN_MIPI_LVDS_CFG_DAT1 | PIN_MIPI_LVDS_CFG_DAT2 | PIN_MIPI_LVDS_CFG_DAT3
	if (chip_name != NULL && strcmp(chip_name, "CHIP_NA51089") == 0) {
		// NOTE: NT98565
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x10;//PIN_I2C_CFG_CH2
	} else {
		cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x01;//PIN_I2C_CFG_CH1
	}
	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_SEL_CSI1_USE_C1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;
	ret = hd_videocap_open(0, HD_VIDEOCAP_1_CTRL, &video_cap_ctrl); //open this for device control

	if (ret != HD_OK) {
		return ret;
	}

	if (is_nt98528 && (sensor_shdr_2 == 1)) {
		cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, (HD_VIDEOCAP_1|HD_VIDEOCAP_4));
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	iq_ctl.func = VIDEOCAP_ALG_FUNC;

	if (is_nt98528 && (sensor_shdr_2 == 1)) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}

	ret |= hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);

	*p_video_cap_ctrl = video_cap_ctrl;
	return ret;
}

static HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 path)
{
	HD_RESULT ret = HD_OK;
	UINT32 color_bar_width = 200;
	{
		HD_VIDEOCAP_IN video_in_param = {0};

		if (sensor_sel_1 != SEN_SEL_PATGEN) {
			video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
			video_in_param.pxlfmt = SEN_OUT_FMT;
		} else {
			color_bar_width = (patgen_size_w >> 3) & 0xFFFFFFFE;
			video_in_param.sen_mode = HD_VIDEOCAP_PATGEN_MODE(HD_VIDEOCAP_SEN_PAT_COLORBAR, color_bar_width);
			video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);
			video_in_param.dim.w = p_dim->w;
			video_in_param.dim.h = p_dim->h;
		}

		if (((sensor_shdr_1 == 1) && (path == 0)) || ((sensor_shdr_2 == 1) && (path == 1))) {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		} else {
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;
		}

		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			return ret;
		}
	}
	//no crop, full frame
	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
	}
	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		if ((sensor_shdr_1 == 1) || (sensor_shdr_2 == 1)) {
			video_out_param.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_out_param.pxlfmt = CAP_OUT_FMT;
		}

		if (sensor_sel_1 == SEN_SEL_PATGEN) {
			video_out_param.pxlfmt |= PATGEN_START_PIX;
			printf("patgen pxlfmt: 0x%X \r\n", video_out_param.pxlfmt);
		}

		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
	}
	if (data_mode1) {//direct mode
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
	}

	if ((sensor_sel_1 == SEN_SEL_IMX307_SLAVE) ||(sensor_sel_1 == SEN_SEL_OS02K10) || (sensor_sel_2 != SEN_SEL_NONE)) {
		UINT32 data_lane = 2;
		vendor_videocap_set(video_cap_path, VENDOR_VIDEOCAP_PARAM_DATA_LANE, &data_lane);
	}

	return ret;
}

static HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if (ret != HD_OK)
		return ret;

	if (p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			video_cfg_param.isp_id = 0;
		} else {
			video_cfg_param.isp_id = 1;
		}

		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		if ((sensor_shdr_1 == 1) || (sensor_shdr_2 == 1)) {
			video_cfg_param.ctrl_max.func |= HD_VIDEOPROC_FUNC_SHDR;
		} else {
			video_cfg_param.ctrl_max.func &= ~HD_VIDEOPROC_FUNC_SHDR;
		}
		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;

		if ((sensor_shdr_1 == 1) || (sensor_shdr_2 == 1)) {
			video_cfg_param.in_max.pxlfmt = SHDR_CAP_OUT_FMT;
		} else {
			video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;
		}
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if (ret != HD_OK) {
			return HD_ERR_NG;
		}
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		if (data_mode1) {
			video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		}
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	if ((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_0_OUT_0;
	} else {
		video_ctrl_param.ref_path_3dnr = HD_VIDEOPROC_1_OUT_0;
	}
	if ((sensor_shdr_1 == 1) || (sensor_shdr_2 == 1)) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
	} else {
		video_ctrl_param.func &= ~HD_VIDEOPROC_FUNC_SHDR;
	}

	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);

	*p_video_proc_ctrl = video_proc_ctrl;

	return ret;
}

static HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim)
{
	HD_RESULT ret = HD_OK;

	if (p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
		HD_VIDEOPROC_OUT video_out_param = {0};
		video_out_param.func = 0;
		video_out_param.dim.w = p_dim->w;
		video_out_param.dim.h = p_dim->h;
		video_out_param.pxlfmt = HD_VIDEO_PXLFMT_YUV420;
		video_out_param.dir = HD_VIDEO_DIR_NONE;
		video_out_param.frc = HD_VIDEO_FRC_RATIO(1,1);
		#if (PLUG_NNSC_FUNC)
		video_out_param.depth = 1;
		#endif
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
	}
	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		if (data_mode2) {
			video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_LOWLATENCY; //enable low-latency
		}
		#if (ONEBUF_ENABLE == 1)
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_ONEBUF;
		#endif
		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}

	return ret;
}

static HD_RESULT set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};

	if (p_max_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
		video_path_config.max_mem.codec_type      = HD_CODEC_TYPE_H264;
		video_path_config.max_mem.max_dim.w       = p_max_dim->w;
		video_path_config.max_mem.max_dim.h       = p_max_dim->h;
		video_path_config.max_mem.bitrate         = max_bitrate;
		#if (ONEBUF_ENABLE == 1)
		video_path_config.max_mem.enc_buf_ms      = 2700;
		video_path_config.max_mem.svc_layer       = HD_SVC_DISABLE;
		#else
		video_path_config.max_mem.enc_buf_ms      = 3000;
		video_path_config.max_mem.svc_layer       = HD_SVC_4X;
		#endif
		video_path_config.max_mem.ltr             = TRUE;
		video_path_config.max_mem.rotate          = FALSE;
		video_path_config.max_mem.source_output   = FALSE;
		video_path_config.isp_id                  = isp_id;

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
		if (ret != HD_OK) {
			printf("set_enc_path_config = %d\r\n", ret);
			return HD_ERR_NG;
		}

		if (data_mode2) {
			video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY; //enable low-latency
		}
		#if (ONEBUF_ENABLE == 1)
		video_func_config.in_func |= HD_VIDEOENC_INFUNC_ONEBUF;
		#endif

		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
		if (ret != HD_OK) {
			printf("set_enc_func_config = %d\r\n", ret);
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
	HD_H26XENC_RATE_CONTROL rc_param = {0};
	HD_H26XENC_VUI vui = {0};

	if (p_dim != NULL) {

		//--- HD_VIDEOENC_PARAM_IN ---
		video_in_param.dir     = HD_VIDEO_DIR_NONE;
		#if (YUV_COMPRESS_ENABLE == 1)
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420_NVX2;
		#else
		video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
		#endif
		video_in_param.dim.w   = p_dim->w;
		video_in_param.dim.h   = p_dim->h;
		video_in_param.frc     = HD_VIDEO_FRC_RATIO(1,1);
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
		if (ret != HD_OK) {
			printf("set_enc_param_in = %d\r\n", ret);
			return ret;
		}

		if (enc_type == 0) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H265;
			video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
			video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
			video_out_param.h26x.gop_num       = 60;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
			video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
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
			if (is_nt98528) {
				if (vdo_size_w_1 <= VDO_SIZE_W_2M) {
					rc_param.cbr.ip_weight       = 0;
				} else if (vdo_size_w_1 <= VDO_SIZE_W_4M) {
					rc_param.cbr.ip_weight       = -30;
				} else {
					rc_param.cbr.ip_weight       = -40;
				}
			}
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}

			vui.vui_en = TRUE;
			vui.color_range = 1; // 0: Not full range, 1: Full range
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI , &vui);
			if (ret != HD_OK) {
				printf("set_enc_out_ui = %d\r\n", ret);
				return ret;
			}
		} else if (enc_type == 1) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_H264;
			video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
			video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
			video_out_param.h26x.gop_num       = 60;
			video_out_param.h26x.ltr_interval  = 0;
			video_out_param.h26x.ltr_pre_ref   = 0;
			video_out_param.h26x.gray_en       = 0;
			video_out_param.h26x.source_output = 0;
			video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
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
			if (is_nt98528) {
				if (vdo_size_w_1 <= VDO_SIZE_W_2M) {
					rc_param.cbr.ip_weight       = 0;
				} else if (vdo_size_w_1 <= VDO_SIZE_W_4M) {
					rc_param.cbr.ip_weight       = -30;
				} else {
					rc_param.cbr.ip_weight       = -40;
				}
			}
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
			if (ret != HD_OK) {
				printf("set_enc_rate_control = %d\r\n", ret);
				return ret;
			}

			vui.vui_en = TRUE;
			vui.color_range = 1; // 0: Not full range, 1: Full range
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_VUI , &vui);
			if (ret != HD_OK) {
				printf("set_enc_out_ui = %d\r\n", ret);
				return ret;
			}
		} else if (enc_type == 2) {
			//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
			video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
			video_out_param.jpeg.retstart_interval = 0;
			video_out_param.jpeg.image_quality = bitrate / (1024 * 1024);
			ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
			if (ret != HD_OK) {
				printf("set_enc_param_out = %d\r\n", ret);
				return ret;
			}
		} else {
			printf("not support enc_type\r\n");
			return HD_ERR_NG;
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_RECORD {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;

} VIDEO_RECORD;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = hd_videocap_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_init()) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_VIDEOENC_0_OUT_0, &p_stream->enc_path)) != HD_OK)
		return ret;
	#if (PLUG_OSG_FUNC)
	if((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_0, HD_STAMP_0, &stream_osg.stamp_path)) != HD_OK)
		return ret;
	#endif

	return HD_OK;
}

static HD_RESULT open_module_without_enc(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_0_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_0_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT open_module2(VIDEO_RECORD *p_stream, HD_DIM* p_proc_max_dim)
{
	HD_RESULT ret;

	// set videocap config
	ret = set_cap_cfg2(&p_stream->cap_ctrl);
	if (ret != HD_OK) {
		printf("set cap-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}
	// set videoproc config
	ret = set_proc_cfg(&p_stream->proc_ctrl, p_proc_max_dim, HD_VIDEOPROC_1_CTRL);
	if (ret != HD_OK) {
		printf("set proc-cfg fail=%d\n", ret);
		return HD_ERR_NG;
	}

	if ((ret = hd_videocap_open(HD_VIDEOCAP_1_IN_0, HD_VIDEOCAP_1_OUT_0, &p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_open(HD_VIDEOPROC_1_IN_0, HD_VIDEOPROC_1_OUT_0, &p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_open(HD_VIDEOENC_0_IN_1, HD_VIDEOENC_0_OUT_1, &p_stream->enc_path)) != HD_OK)
		return ret;

	return HD_OK;
}

static HD_RESULT close_module(VIDEO_RECORD *p_stream)
{
	HD_RESULT ret;

	if ((ret = hd_videocap_close(p_stream->cap_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoproc_close(p_stream->proc_path)) != HD_OK)
		return ret;
	if ((ret = hd_videoenc_close(p_stream->enc_path)) != HD_OK)
		return ret;
	#if (PLUG_OSG_FUNC)
	if((ret = hd_videoenc_close(stream_osg.stamp_path)) != HD_OK)
		return ret;
	#endif

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

typedef struct _VIDEO_INFO {
	VIDEO_RECORD *p_stream;
	HD_VIDEOENC_BS  data_pull;
	NVTLIVE555_CODEC codec_type;
	unsigned char vps[64];
	int vps_size;
	unsigned char sps[64];
	int sps_size;
	unsigned char pps[64];
	int pps_size;
	int ref_cnt;
} VIDEO_INFO;

typedef struct _ADUIO_INFO {
	int ref_cnt;
	NVTLIVE555_AUDIO_INFO info;
} AUDIO_INFO;

static VIDEO_INFO video_info[2] = { 0 };
//static AUDIO_INFO audio_info[1] = { 0 };

static int flush_video(VIDEO_INFO *p_video_info)
{
	while (hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, 0) == 0) {
		hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
	}

	HD_H26XENC_REQUEST_IFRAME req_i = {0};
	req_i.enable   = 1;
	int ret = hd_videoenc_set(p_video_info->p_stream->enc_path, HD_VIDEOENC_PARAM_OUT_REQUEST_IFRAME, &req_i);
	if (ret != HD_OK) {
		printf("set_enc_param_out_request_ifrm0 = %d\r\n", ret);
		return ret;
	}
	hd_videoenc_start(p_video_info->p_stream->enc_path);
	return ret;
}

static int on_open_video(int channel)
{
	if ((size_t)channel >= sizeof(video_info) / sizeof(video_info[0])) {
		printf("nvtrtspd video channel exceed\n");
		return 0;
	}
	if (video_info[channel].ref_cnt != 0) {
		printf("nvtrtspd video in use.\n");
		return 0;
	}

	//no video
	if (video_info[channel].codec_type == NVTLIVE555_CODEC_UNKNOWN) {
		return 0;
	}

	video_info[channel].ref_cnt++;


	flush_video(&video_info[channel]);
	return (int)&video_info[channel];
}

static int on_close_video(int handle)
{
	if (handle) {
		VIDEO_INFO *p_info = (VIDEO_INFO *)handle;
		p_info->ref_cnt--;
	}
	return 0;
}

static int refresh_video_info(int id, VIDEO_INFO *p_video_info)
{
	//while to get vsp, sps, pps
	int ret;
	p_video_info->codec_type = NVTLIVE555_CODEC_UNKNOWN;
	p_video_info->vps_size = p_video_info->sps_size = p_video_info->pps_size = 0;
	memset(p_video_info->vps, 0, sizeof(p_video_info->vps));
	memset(p_video_info->sps, 0, sizeof(p_video_info->sps));
	memset(p_video_info->pps, 0, sizeof(p_video_info->pps));
	while ((ret = hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, 500)) == 0) {
		if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_JPEG) {
			p_video_info->codec_type = NVTLIVE555_CODEC_MJPG;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		} else if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H264) {
			p_video_info->codec_type = NVTLIVE555_CODEC_H264;
			if (p_video_info->data_pull.pack_num != 3) {
				hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
				continue;
			}

			if (id == 0) {
				memcpy(p_video_info->sps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
				memcpy(p_video_info->pps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
			} else {
				memcpy(p_video_info->sps, (void *)PHY2VIRT_SUB(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
				memcpy(p_video_info->pps, (void *)PHY2VIRT_SUB(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
			}

			p_video_info->sps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[1].size;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		} else if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H265) {
			p_video_info->codec_type = NVTLIVE555_CODEC_H265;
			if (p_video_info->data_pull.pack_num != 4) {
				hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
				continue;
			}

			if (id == 0) {
				memcpy(p_video_info->vps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
				memcpy(p_video_info->sps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
				memcpy(p_video_info->pps, (void *)PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[2].phy_addr), p_video_info->data_pull.video_pack[2].size);
			} else {
				memcpy(p_video_info->vps, (void *)PHY2VIRT_SUB(p_video_info->data_pull.video_pack[0].phy_addr), p_video_info->data_pull.video_pack[0].size);
				memcpy(p_video_info->sps, (void *)PHY2VIRT_SUB(p_video_info->data_pull.video_pack[1].phy_addr), p_video_info->data_pull.video_pack[1].size);
				memcpy(p_video_info->pps, (void *)PHY2VIRT_SUB(p_video_info->data_pull.video_pack[2].phy_addr), p_video_info->data_pull.video_pack[2].size);
			}

			p_video_info->vps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->sps_size = p_video_info->data_pull.video_pack[1].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[2].size;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		} else {
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
		}
	}
	return 0;
}

static int on_get_video_info(int handle, int timeout_ms, NVTLIVE555_VIDEO_INFO *p_info)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;

	p_info->codec_type = p_video_info->codec_type;
	if (p_video_info->vps_size) {
		p_info->vps_size = p_video_info->vps_size;
		memcpy(p_info->vps, p_video_info->vps, p_info->vps_size);
	}

	if (p_video_info->sps_size) {
		p_info->sps_size = p_video_info->sps_size;
		memcpy(p_info->sps, p_video_info->sps, p_info->sps_size);
	}

	if (p_video_info->pps_size) {
		p_info->pps_size = p_video_info->pps_size;
		memcpy(p_info->pps, p_video_info->pps, p_info->pps_size);
	}

	return  0;
}

static int on_lock_video(int handle, int timeout_ms, NVTLIVE555_STRM_INFO *p_strm)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;
	int id = (p_video_info == &video_info[0])? 0: 1;

	//while to get vsp, sps, pps
	int ret = hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, -1);
	if (ret != 0) {
		return ret;
	}

	if (id == 0) {
		p_strm->addr = PHY2VIRT_MAIN(p_video_info->data_pull.video_pack[p_video_info->data_pull.pack_num-1].phy_addr);
	} else {
		p_strm->addr = PHY2VIRT_SUB(p_video_info->data_pull.video_pack[p_video_info->data_pull.pack_num-1].phy_addr);
	}
	p_strm->size = p_video_info->data_pull.video_pack[p_video_info->data_pull.pack_num-1].size;
	p_strm->timestamp = p_video_info->data_pull.timestamp;

	return 0;
}

static int on_unlock_video(int handle)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)handle;
	int ret = hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
	return ret;
}


static int on_open_audio(int channel)
{
	return 0;
#if 0
	if (channel >= sizeof(audio_info) / sizeof(audio_info[0])) {
		printf("nvtrtspd audio channel exceed\n");
		return 0;
	}
	audio_info[channel].ref_cnt++;
	return (int)&audio_info[channel];
#endif
}

static int on_close_audio(int handle)
{
	if (handle) {
		AUDIO_INFO *p_info = (AUDIO_INFO *)handle;
		p_info->ref_cnt--;
	}
	return 0;
}

static int on_get_audio_info(int handle, int timeout_ms, NVTLIVE555_AUDIO_INFO *p_info)
{
	return 0;
}

static int on_lock_audio(int handle, int timeout_ms, NVTLIVE555_STRM_INFO *p_strm)
{
	return 0;
}

static int on_unlock_audio(int handle)
{
	return 0;
}

static void *encode_thread(void *arg)
{
	VIDEO_INFO *p_video_info = (VIDEO_INFO *)arg;
	VIDEO_RECORD* p_stream0 = p_video_info[0].p_stream;
	VIDEO_RECORD* p_stream1 = p_video_info[1].p_stream;

	//------ wait flow_start ------
	if (sensor_sel_1 != SEN_SEL_NONE) {
		while (p_stream0->flow_start == 0) sleep(1);
	} else {
		while (p_stream1->flow_start == 0) sleep(1);
	}

	// query physical address of bs buffer ( this can ONLY query after hd_videoenc_start() is called !! )
	if (sensor_sel_1 != SEN_SEL_NONE) {
		hd_videoenc_get(p_stream0->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_main);
	}
	if (sensor_sel_2 != SEN_SEL_NONE) {
		hd_videoenc_get(p_stream1->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &phy_buf_sub);
	}

	// mmap for bs buffer (just mmap one time only, calculate offset to virtual address later)
	if (sensor_sel_1 != SEN_SEL_NONE) {
		vir_addr_main = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_main.buf_info.phy_addr, phy_buf_main.buf_info.buf_size);
	}
	if (sensor_sel_2 != SEN_SEL_NONE) {
		vir_addr_sub  = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, phy_buf_sub.buf_info.phy_addr, phy_buf_sub.buf_info.buf_size);
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		refresh_video_info(0, &p_video_info[0]);
	}
	if (sensor_sel_2 != SEN_SEL_NONE) {
		refresh_video_info(1, &p_video_info[1]);
	}

	//live555 setup
	NVTLIVE555_HDAL_CB hdal_cb;
	hdal_cb.open_video = on_open_video;
	hdal_cb.close_video = on_close_video;
	hdal_cb.get_video_info = on_get_video_info;
	hdal_cb.lock_video = on_lock_video;
	hdal_cb.unlock_video = on_unlock_video;
	hdal_cb.open_audio = on_open_audio;
	hdal_cb.close_audio = on_close_audio;
	hdal_cb.get_audio_info = on_get_audio_info;
	hdal_cb.lock_audio = on_lock_audio;
	hdal_cb.unlock_audio = on_unlock_audio;
	nvtrtspd_init(&hdal_cb);
	nvtrtspd_open();

	return 0;
}

MAIN(argc, argv)
{
	HD_RESULT ret;
	INT key;
	VIDEO_RECORD stream[2] = {0}; //0: main stream
	UINT32 enc_type = 0;
	UINT32 enc_bitrate = 4;
	HD_DIM main_dim;
	AET_CFG_INFO cfg_info = {0};
	AWBT_CT_TO_CGAIN ct_to_cgain = {0};
	ISPT_C_GAIN c_gain = {0};
	ISPT_CT ct = {0};

	if (argc == 1 || argc > 7) {
		printf("\033[1;37;43mUsage:\033[0m \033[1;32;40m<sensor_sel_1 > \033[1;31;40m<sensor_shdr_1> \033[1;32;40m<sensor_sel_2 > \033[1;31;40m<sensor_shdr_2> \033[1;37;40m<enc_type> <enc_bitrate>\033[0m.\r\n");
		printf("\r\n");
		printf("\033[1;32;40m  <sensor_sel_1 > :  0(NONE),   1(imx290), 2(os02k10), 3(ar0237ir), 4(ov2718),  5(os05a10)              \033[0m\r\n");
		printf("\033[1;32;40m                     6(imx317), 7(imx335), 8(f37),     9(ps5268),  10(sc4210), 11(imx307_slave)         \033[0m\r\n");
		printf("\033[1;32;40m                    12(f35),   13(imx415),14(sc500ai),15(sc401ai),16(os04a10), 17(gc4653)               \033[0m\r\n");
		printf("\033[1;32;40m                    18(imx307),19(sc4238),99(patgen)                                                               \033[0m\r\n");
		printf("\033[1;31;40m  <sensor_shdr_1> : 0(disable), 1(enable)                                                  \033[0m\r\n");
		printf("\033[1;32;40m  <sensor_sel_2 > : as <sensor_sel_1 >                                                     \033[0m\r\n");
		printf("\033[1;31;40m  <sensor_shdr_2> : 0(disable), 1(enable)                                                  \033[0m\r\n");
		printf("\033[1;37;40m  <enc_type     > : 0(H265), 1(H264), 2(MJPG)                                              \033[0m\r\n");
		printf("\033[1;37;40m  <enc_bitrate  > : Mbps for H265/H264, or Quality for MJPG                                \033[0m\r\n");
		printf("Remiding Notes for no sensor streaming:\r\n");
		printf("1.Please check data lanes wether need to swap or not.\r\n");
		return 0;
	}

	// query program options
	if (argc > 1) {
		sensor_sel_1 = atoi(argv[1]);
		switch (sensor_sel_1) {
			default:
			case SEN_SEL_NONE:
				printf("sensor 1: none \r\n");
			break;
			case SEN_SEL_IMX290:
				printf("sensor 1: imx290 w4000 \r\n");
				break;
			case SEN_SEL_OS02K10:
				printf("sensor 1: os02k10 \r\n");
				break;
			case SEL_SEL_AR0237IR:
				printf("sensor 1: ar0237ir \r\n");
				break;
			case SEN_SEL_OV2718:
				printf("sensor 1: ov2718 \r\n");
				break;
			case SEN_SEL_OS05A10:
				printf("sensor 1: os05a10 \r\n");
				break;
			case SEN_SEL_IMX317:
				printf("sensor 1: imx317 w4000 \r\n");
				break;
			case SEN_SEL_IMX335:
				printf("sensor 1: imx335 \r\n");
				break;
			case SEN_SEL_F37:
				printf("sensor 1: f37 \r\n");
				break;
			case SEN_SEL_PS5268:
				printf("sensor 1: ps5268 \r\n");
				break;
			case SEN_SEL_SC4210:
				printf("sensor 1: sc4210 \r\n");
				break;
			case SEN_SEL_IMX307_SLAVE:
				printf("sensor 1: imx307 slave \r\n");
				break;
			case SEN_SEL_F35:
				printf("sensor 1: f35 \r\n");
				break;
			case SEN_SEL_IMX415:
				printf("sensor 1: imx415 \r\n");
				break;
			case SEN_SEL_SC500AI:
				printf("sensor 1: sc500ai \r\n");
				break;
			case SEN_SEL_SC401AI:
				printf("sensor 1: sc401ai \r\n");
				break;
			case SEN_SEL_OS04A10:
				printf("sensor 1: os04a10 \r\n");
				break;
			case SEN_SEL_GC4653:
				printf("sensor 1: gc4653 \r\n");
				break;
			case SEN_SEL_IMX307:
				printf("sensor 1: imx307 \r\n");
				break;
			case SEN_SEL_SC4238:
				printf("sensor 1: sc4238 \r\n");
				break;
			case SEN_SEL_PATGEN:
				printf("sensor 1: patgen \r\n");
				break;
		}
	}

	if (argc > 2) {
		sensor_shdr_1 = atoi(argv[2]);
		if (sensor_shdr_1 == 0) {
			printf("sensor 1: shdr off \r\n");
		} else if (sensor_shdr_1 == 1) {
			if ((sensor_sel_1 == SEL_SEL_AR0237IR) || (sensor_sel_1 == SEN_SEL_OV2718)) {
				sensor_shdr_1 = 0;
				printf("sensor 1: shdr off, sensor not support. \r\n");
			} else {
				printf("sensor 1: shdr on \r\n");
			}
		} else {
			printf("error: not support shdr_mode! (%d) \r\n", sensor_shdr_1);
			return 0;
		}
	}

	if (argc > 3) {
		if (sensor_sel_1 == SEN_SEL_PATGEN) {
			if (atoi(argv[3]) > 0) {
				patgen_size_w = atoi(argv[3]);
			}
			sensor_sel_2 = SEN_SEL_NONE;
			printf("sensor 2: none, patgen w = %d \r\n", patgen_size_w);
		} else {
			sensor_sel_2 = atoi(argv[3]);
			switch (sensor_sel_2) {
				default:
				case SEN_SEL_NONE:
					printf("sensor 2: none \r\n");
				break;
				case SEN_SEL_IMX290:
					printf("sensor 2: imx290 \r\n");
					break;
				case SEN_SEL_OS02K10:
					printf("sensor 2: os02k10 \r\n");
					break;
				case SEL_SEL_AR0237IR:
					printf("sensor 2: ar0237ir \r\n");
					break;
				case SEN_SEL_OV2718:
					printf("sensor 2: ov2718 \r\n");
					break;
				case SEN_SEL_OS05A10:
					printf("sensor 2: os05a10 \r\n");
					break;
				case SEN_SEL_IMX317:
					printf("sensor 2: imx317 \r\n");
					break;
				case SEN_SEL_IMX335:
					printf("sensor 2: imx335 \r\n");
					break;
				case SEN_SEL_F37:
					printf("sensor 2: f37 \r\n");
					break;
				case SEN_SEL_PS5268:
					printf("sensor 2: ps5268 \r\n");
					break;
				case SEN_SEL_SC4210:
					printf("sensor 2: sc4210 \r\n");
					break;
				case SEN_SEL_IMX307_SLAVE:
					printf("sensor 2: imx307 slave \r\n");
					break;
				case SEN_SEL_F35:
					printf("sensor 2: f35 \r\n");
					break;
				case SEN_SEL_IMX415:
					printf("sensor 2: imx415 \r\n");
					break;
				case SEN_SEL_SC500AI:
					printf("sensor 2: sc500ai \r\n");
					break;
				case SEN_SEL_SC401AI:
					printf("sensor 2: sc401ai \r\n");
					break;
				case SEN_SEL_OS04A10:
					printf("sensor 2: os04a10 \r\n");
					break;
				case SEN_SEL_GC4653:
					printf("sensor 2: gc4653 \r\n");
					break;
				case SEN_SEL_SC4238:
					printf("sensor 2: sc4238 \r\n");
					break;
			}
		}
	}

	if (argc > 4) {
		if (sensor_sel_1 == SEN_SEL_PATGEN) {
			if (atoi(argv[4]) > 0) {
				patgen_size_h = atoi(argv[4]);
			}
			sensor_shdr_2 = 0;
			printf("sensor 2: shdr off, patgen h = %d \r\n", patgen_size_h);
		} else {
			sensor_shdr_2 = atoi(argv[4]);
			if (sensor_shdr_2 == 0) {
				printf("sensor 2: shdr off \r\n");
			} else if (sensor_shdr_2 == 1) {
				if ((sensor_sel_2 == SEL_SEL_AR0237IR) || (sensor_sel_2 == SEN_SEL_OV2718)) {
					sensor_shdr_2 = 0;
					printf("sensor 2: shdr off, sensor not support. \r\n");
				} else {
					printf("sensor 2: shdr on \r\n");
				}
			} else {
				printf("error: not support shdr_mode! (%d) \r\n", sensor_shdr_2);
				return 0;
			}
		}
	}

	if (argc > 5) {
		enc_type = atoi(argv[5]);
		if (enc_type == 0) {
			printf("Enc type: H.265\r\n", enc_type);
		} else if (enc_type == 1) {
			printf("Enc type: H.264\r\n", enc_type);
		} else if (enc_type == 2) {
			printf("Enc type: MJPG\r\n", enc_type);
		} else {
			printf("error: not support enc_type! (%d)\r\n", enc_type);
			return 0;
		}
	}

	if (argc > 6) {
		enc_bitrate = atoi(argv[6]);
		printf("Enc bitrate: %d Mbps\r\n", enc_bitrate);
	}

	printf("Dram size = 0x%X \r\n", _BOARD_DRAM_SIZE_);

	#if (ONEBUF_ENABLE)
	printf("One buffer enable \r\n");
	#else
	printf("One buffer disable \r\n");
	#endif

	if ((sensor_sel_1 == SEN_SEL_NONE) && (sensor_sel_2 == SEN_SEL_NONE)) {
		printf("sensor_sel_1 & sensor_sel_1 = SEN_SEL_NONE \n");
		return 0;
	}

	{
		char *chip_name = getenv("NVT_CHIP_ID");
		if (chip_name != NULL && strcmp(chip_name, "CHIP_NA51084") == 0) {
			printf("IS NT98528 \n");
			is_nt98528 = 1;
		} else if (chip_name != NULL && strcmp(chip_name, "CHIP_NA51089") == 0) {
			printf("IS NT9856x \n");
			is_nt98528 = 0;
		} else {
			printf("NOT NT98528 \n");
			is_nt98528 = 0;
		}
	}

	if ((!is_nt98528) && (sensor_sel_2 != 0) && ((sensor_shdr_1 == 1) || (sensor_shdr_2 == 1))) {
		printf("sensor_sel_2 != 0, sensor_shdr force to 0 \n");
		sensor_shdr_1 = 0;
		sensor_shdr_2 = 0;
	}

	if ((data_mode1 == 1) & (sensor_sel_2 != SEN_SEL_NONE)) {
		printf("sensor_sel_2 != 0, disable direct mode \n");
		data_mode1 = 0;
	}

	if (vendor_isp_init() != HD_ERR_NG) {
		if (sensor_sel_1 != SEN_SEL_NONE) {
			cfg_info.id = 0;
			if ((sensor_sel_1 == SEN_SEL_IMX290) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_IMX290) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_OS05A10) && (sensor_shdr_1 == 0) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_OS05A10) && (sensor_shdr_1 == 0) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_OS05A10) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_OS02K10) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os02k10_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_OS02K10) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os02k10_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEL_SEL_AR0237IR) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ar0237ir_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_OV2718) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ov2718_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_IMX317) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx317_0.cfg", CFG_NAME_LENGTH);
			//w4000 strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_IMX335 && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx335_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_IMX335) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_F37) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f37.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_F35) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f35.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_F35) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f35_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_PS5268) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ps5268.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4210) && (sensor_shdr_1 == 0) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4210) && (sensor_shdr_1 == 1) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_hdr_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4210) && (sensor_shdr_1 == 0) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4210) && (sensor_shdr_1 == 1) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_hdr_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_IMX415) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_IMX415) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_SC500AI) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc401ai_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_SC401AI) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc401ai_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_OS04A10) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_1 == SEN_SEL_IMX307) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx307_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4238) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4238_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_1 == SEN_SEL_SC4238) && (sensor_shdr_1 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4238_0_shdr.cfg", CFG_NAME_LENGTH);
			} else {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			}

			printf("sensor 1 load %s \n", cfg_info.path);

			vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
			vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
			vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

			ct_to_cgain.id = 0;
			ct_to_cgain.ct_to_cgain.ct = 4700;
			vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);

			c_gain.id = 0;
			c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
			c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
			c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

			ct.id = 0;
			ct.ct = 4700;
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
		}

		if (sensor_sel_2 != SEN_SEL_NONE) {
			cfg_info.id = 1;
			if ((sensor_sel_2 == SEN_SEL_IMX290) && (sensor_shdr_1 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_IMX290) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_OS05A10) && (sensor_shdr_1 == 0) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_OS05A10) && (sensor_shdr_1 == 0) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_OS02K10) && (sensor_shdr_2 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os02k10_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_OS02K10) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os02k10_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEL_SEL_AR0237IR) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ar0237ir_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_OV2718) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ov2718_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_IMX317) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_IMX335 && (sensor_shdr_2 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx335_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_IMX335) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_F37) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f37.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_F35) && (sensor_shdr_2 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f35.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_F35) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_f35_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_PS5268) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_ps5268.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4210) && (sensor_shdr_1 == 0) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4210) && (sensor_shdr_1 == 1) && (is_nt98528 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_hdr_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4210) && (sensor_shdr_1 == 0) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4210) && (sensor_shdr_1 == 1) && (is_nt98528 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4210_0_hdr_528.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_IMX415) && (sensor_shdr_2 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_IMX415) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx415_0_hdr.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_SC500AI) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc401ai_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_SC401AI) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc401ai_0.cfg", CFG_NAME_LENGTH);
			} else if (sensor_sel_2 == SEN_SEL_OS04A10) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_os05a10_0_52x.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4238) && (sensor_shdr_2 == 0)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4238_0.cfg", CFG_NAME_LENGTH);
			} else if ((sensor_sel_2 == SEN_SEL_SC4238) && (sensor_shdr_2 == 1)) {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_sc4238_0_shdr.cfg", CFG_NAME_LENGTH);
			} else {
				strncpy(cfg_info.path, "/mnt/app/isp/isp_imx290_0.cfg", CFG_NAME_LENGTH);
			}

			printf("sensor 2 load %s \n", cfg_info.path);

			vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
			vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
			vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);

			ct_to_cgain.id = 1;
			ct_to_cgain.ct_to_cgain.ct = 4700;
			vendor_isp_get_awb(AWBT_ITEM_CT_TO_CGAIN, &ct_to_cgain);

			c_gain.id = 1;
			c_gain.gain[0] = ct_to_cgain.ct_to_cgain.r_gain;
			c_gain.gain[1] = ct_to_cgain.ct_to_cgain.g_gain;
			c_gain.gain[2] = ct_to_cgain.ct_to_cgain.b_gain;
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);

			ct.id = 1;
			ct.ct = 4700;
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
		}

		vendor_isp_uninit();
	} else {
		printf("vendor_isp_init failed!\r\n");
	}

	if ((sensor_sel_1 == SEN_SEL_OS05A10) || (sensor_sel_1 == SEN_SEL_IMX335)){
		vdo_size_w_1 = VDO_SIZE_W_5M;
		vdo_size_h_1 = VDO_SIZE_H_5M;
	} else if (sensor_sel_1 == SEN_SEL_SC500AI) {
		vdo_size_w_1 = VDO_SIZE_W_5M_BIG;
		vdo_size_h_1 = VDO_SIZE_H_5M_BIG;
	
	} 
	#if 1//w4000 SEN_SEL_IMX327
	else if ((sensor_sel_1 == SEN_SEL_IMX317) || (sensor_sel_1 == SEN_SEL_IMX415)) {
		vdo_size_w_1 = VDO_SIZE_W_2M;
		vdo_size_h_1 = VDO_SIZE_H_2M;
	}
	#else
	else if ((sensor_sel_1 == SEN_SEL_IMX317) || (sensor_sel_1 == SEN_SEL_IMX415)) {
		vdo_size_w_1 = VDO_SIZE_W_8M;
		vdo_size_h_1 = VDO_SIZE_H_8M;
	}
	#endif
	else if ((sensor_sel_1 == SEN_SEL_SC4210) || (sensor_sel_1 == SEN_SEL_SC401AI) || (sensor_sel_1 == SEN_SEL_GC4653) || (sensor_sel_1 == SEN_SEL_SC4238)) {
		vdo_size_w_1 = VDO_SIZE_W_4M;
		vdo_size_h_1 = VDO_SIZE_H_4M;
	} else if (sensor_sel_1 == SEN_SEL_OS04A10) {
		vdo_size_w_1 = VDO_SIZE_W_4M_BIG;
		vdo_size_h_1 = VDO_SIZE_H_4M_BIG;
	} else if (sensor_sel_1 == SEN_SEL_PATGEN) {
		vdo_size_w_1 = patgen_size_w;
		vdo_size_h_1 = patgen_size_h;
	} else {
		vdo_size_w_1 = VDO_SIZE_W_2M;
		vdo_size_h_1 = VDO_SIZE_H_2M;
	}

	if ((sensor_sel_2 == SEN_SEL_OS05A10) || (sensor_sel_2 == SEN_SEL_IMX335)){
		vdo_size_w_2 = VDO_SIZE_W_5M;
		vdo_size_h_2 = VDO_SIZE_H_5M;
	} else if (sensor_sel_2 == SEN_SEL_SC500AI) {
		vdo_size_w_2 = VDO_SIZE_W_5M_BIG;
		vdo_size_h_2 = VDO_SIZE_H_5M_BIG;
	} 
	#if 1//w4000 SEN_SEL_IMX327
	else if ((sensor_sel_2 == SEN_SEL_IMX317) || (sensor_sel_2 == SEN_SEL_IMX415)) {
		vdo_size_w_2 = VDO_SIZE_W_2M;
		vdo_size_h_2 = VDO_SIZE_H_2M;
	}
	#else
	else if ((sensor_sel_2 == SEN_SEL_IMX317) || (sensor_sel_2 == SEN_SEL_IMX415)) {
		vdo_size_w_2 = VDO_SIZE_W_8M;
		vdo_size_h_2 = VDO_SIZE_H_8M;
	} 
	#endif
	else if ((sensor_sel_2 == SEN_SEL_SC4210) || (sensor_sel_2 == SEN_SEL_SC401AI) || (sensor_sel_2 == SEN_SEL_GC4653) || (sensor_sel_2 == SEN_SEL_SC4238)) {
		vdo_size_w_2 = VDO_SIZE_W_4M;
		vdo_size_h_2 = VDO_SIZE_H_4M;
	} else if (sensor_sel_2 == SEN_SEL_OS04A10) {
		vdo_size_w_2 = VDO_SIZE_W_4M_BIG;
		vdo_size_h_2 = VDO_SIZE_H_4M_BIG;
	} else {
		vdo_size_w_2 = VDO_SIZE_W_2M;
		vdo_size_h_2 = VDO_SIZE_H_2M;
	}

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
		goto exit;
	}

	hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV1, 0);
	#if (PLUG_NNSC_FUNC)
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable NN AI engine
	#endif

	// init memory
	ret = mem_init();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
		goto exit;
	}

	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init module fail=%d\n", ret);
		goto exit;
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		// open video_record modules (main)
		stream[0].proc_max_dim.w = vdo_size_w_1;
		stream[0].proc_max_dim.h = vdo_size_h_1;
		ret = open_module(&stream[0], &stream[0].proc_max_dim);
		if (ret != HD_OK) {
			printf("open module fail=%d\n", ret);
			goto exit;
		}

		// get videocap capability
		ret = get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail=%d\n", ret);
			goto exit;
		}

		// set videocap parameter
		stream[0].cap_dim.w = vdo_size_w_1;
		stream[0].cap_dim.h = vdo_size_h_1;
		ret = set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
		if (ret != HD_OK) {
			printf("set cap fail=%d\n", ret);
			goto exit;
		}

		// assign parameter by program options
		main_dim.w = vdo_size_w_1;
		main_dim.h = vdo_size_h_1;

		// set videoproc parameter (main)
		ret = set_proc_param(stream[0].proc_path, &main_dim);
		if (ret != HD_OK) {
			printf("set proc fail=%d\n", ret);
			goto exit;
		}

		// set videoenc config (main)
		stream[0].enc_max_dim.w = vdo_size_w_1;
		stream[0].enc_max_dim.h = vdo_size_h_1;
		ret = set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, 0);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			goto exit;
		}

		// set videoenc parameter (main)
		stream[0].enc_dim.w = vdo_size_w_1;
		stream[0].enc_dim.h = vdo_size_h_1;
		ret = set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc fail=%d\n", ret);
			goto exit;
		}

		// bind video_record modules (main)
		hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
		hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);

		video_info[0].p_stream = &stream[0];
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		// open video liview modules (sensor 2nd)
		stream[1].proc_max_dim.w = vdo_size_w_2; //assign by user
		stream[1].proc_max_dim.h = vdo_size_h_2; //assign by user
		ret = open_module2(&stream[1], &stream[1].proc_max_dim);
		if (ret != HD_OK) {
			printf("open module2 fail=%d\n", ret);
			goto exit;
		}

		// get videocap capability
		ret = get_cap_caps(stream[1].cap_ctrl, &stream[1].cap_syscaps);
		if (ret != HD_OK) {
			printf("get cap-caps fail=%d\n", ret);
			goto exit;
		}

		// set videocap parameter
		stream[1].cap_dim.w = vdo_size_w_2;
		stream[1].cap_dim.h = vdo_size_h_2;
		ret = set_cap_param(stream[1].cap_path, &stream[1].cap_dim, 1);
		if (ret != HD_OK) {
			printf("set cap2 fail=%d\n", ret);
			goto exit;
		}

		main_dim.w = vdo_size_w_2;
		main_dim.h = vdo_size_h_2;

		ret = set_proc_param(stream[1].proc_path, &main_dim);
		if (ret != HD_OK) {
			printf("set proc2 fail=%d\n", ret);
			goto exit;
		}

		stream[1].enc_max_dim.w = vdo_size_w_2;
		stream[1].enc_max_dim.h = vdo_size_h_2;
		ret = set_enc_cfg(stream[1].enc_path, &stream[1].enc_max_dim, enc_bitrate * 1024 * 1024, 1);
		if (ret != HD_OK) {
			printf("set enc-cfg fail=%d\n", ret);
			goto exit;
		}

		stream[1].enc_dim.w = vdo_size_w_2;
		stream[1].enc_dim.h = vdo_size_h_2;
		ret = set_enc_param(stream[1].enc_path, &stream[1].enc_dim, enc_type, enc_bitrate * 1024 * 1024);
		if (ret != HD_OK) {
			printf("set enc2 fail=%d\n", ret);
			goto exit;
		}

		hd_videocap_bind(HD_VIDEOCAP_1_OUT_0, HD_VIDEOPROC_1_IN_0);
		hd_videoproc_bind(HD_VIDEOPROC_1_OUT_0, HD_VIDEOENC_0_IN_1);

		video_info[1].p_stream = &stream[1];
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		ret = pthread_create(&stream[0].enc_thread_id, NULL, encode_thread, (void *)&video_info[0]);
	} else {
		ret = pthread_create(&stream[1].enc_thread_id, NULL, encode_thread, (void *)&video_info[0]);
	}

	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		if (data_mode1) {
			//direct NOTE: ensure videocap start after 1st videoproc phy path start
			hd_videoproc_start(stream[0].proc_path);
			hd_videocap_start(stream[0].cap_path);
		} else {
			hd_videocap_start(stream[0].cap_path);
			hd_videoproc_start(stream[0].proc_path);
		}

		// just wait ae/awb stable for auto-test, if don't care, user can remove it
		//sleep(1);

		hd_videoenc_start(stream[0].enc_path);

		#if (PLUG_OSG_FUNC)
		hd_videoenc_start(stream_osg.stamp_path);
		#endif
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		hd_videocap_start(stream[1].cap_path);
		hd_videoproc_start(stream[1].proc_path);
		// just wait ae/awb stable for auto-test, if don't care, user can remove it
		//sleep(1);

		hd_videoenc_start(stream[1].enc_path);
	}

	// let encode_thread start to work
	if (sensor_sel_1 != SEN_SEL_NONE) {
		stream[0].flow_start = 1;
	} else {
		stream[1].flow_start = 1;
	}

	#if (PLUG_OSG_FUNC)
	osg_param_init();

	shm_init();

	//allocate logo buffer
	stream_osg.osg_buf = malloc(vdo_size_w_1 * vdo_size_h_1 * sizeof(unsigned short));
	if(!stream_osg.osg_buf){
		printf("fail to allocate pq osg buffer\n");
		return -1;
	}

	osg_clear_buf(stream_osg.osg_buf);

	//load icon from sd card
	if(osg_icon_init()){
		printf("fail to load icon image\n");
		free(stream_osg.osg_buf);
		return -1;
	}

	osg_update();
	osg_draw(stream_osg.osg_buf);

	// init stamp data
	stream_osg.stamp_blk  = 0;
	stream_osg.stamp_pa   = 0;
	stream_osg.stamp_size = osg_calc_buf_size(vdo_size_w_1, vdo_size_h_1);
	if(stream_osg.stamp_size <= 0){
		printf("osg_calc_buf_size() fail\n");
		return -1;
	}

	ret = osg_mem_alloc();
	if (ret) {
		printf("fail to allocate stamp buffer\n");
		return -1;
	}

	//setup enc stamp parameter
	if (osg_set_enc_stamp() != HD_OK) {
		printf("osg_set_enc_stamp fial, path = 0x%X \r\n", stream_osg.stamp_path);
		return -1;
	}
	
	if (hd_videoenc_start(stream_osg.stamp_path) != HD_OK) {
		printf("hd_videoenc_start fail, path = 0x%X \r\n", stream_osg.stamp_path);
	}

	stream_osg.thread_run = TRUE;
	ret = pthread_create(&stream_osg.thread_id, NULL, osg_thread, NULL);
	if (ret < 0) {
		printf("create osg thread failed\n");
		goto exit;
	}
	#endif

	// query user key
	printf("Enter q to exit \n");

	while (1) {
		key = GETCHAR();
		if (key == 'q' || key == 0x3) {
			// let encode_thread stop loop and exit
			stream[0].enc_exit = 1;
			// quit program
			break;
		}

		// NOTE: Start of test path 1 linear/shdr switch
		if (key == 'x') {
			ISPT_D_GAIN d_gain = {0};
			ISPT_C_GAIN c_gain = {0};
			ISPT_TOTAL_GAIN total_gain = {0};
			ISPT_LV lv = {0};
			ISPT_CT ct = {0};
			VENDOR_VIDEOCAP_AE_PRESET ae_preset = {0};
			ISPT_SENSOR_EXPT sensor_expt = {0};
			ISPT_SENSOR_GAIN sensor_gain = {0};

			vendor_isp_init();
			d_gain.id = 0;
			c_gain.id = 0;
			total_gain.id = 0;
			lv.id = 0;
			ct.id = 0;
			vendor_isp_get_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_get_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_get_common(ISPT_ITEM_LV, &lv);
			vendor_isp_get_common(ISPT_ITEM_CT, &ct);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			printf("d gain = %d \n", d_gain.gain);
			printf("c gain = %d %d %d \n", c_gain.gain[0], c_gain.gain[1], c_gain.gain[2]);
			printf("total gain = %d \n", total_gain.gain);
			printf("lv = %d \n", lv.lv);
			printf("ct = %d \n", ct.ct);
			printf("expt = %d, ratio = %d \n", sensor_expt.time[0], sensor_gain.ratio[0]);

			sensor_shdr_1 = !sensor_shdr_1;
			printf("SHDR %d \n", sensor_shdr_1);

			if (data_mode1) {
				//direct NOTE: ensure videocap stop after all videoproc path stop
				hd_videoproc_stop(stream[0].proc_path);
				hd_videocap_stop(stream[0].cap_path);
				hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
				hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
			} else {
				hd_videocap_stop(stream[0].cap_path);
				hd_videoproc_stop(stream[0].proc_path);
			}
			hd_videocap_close(stream[0].cap_path);
			hd_videoproc_close(stream[0].proc_path);
			//hd_videoenc_stop(stream[0].enc_path);
			//hd_videoenc_close(stream[0].enc_path);
			stream[0].flow_start = 0;

			open_module_without_enc(&stream[0], &stream[0].proc_max_dim);
			get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
			set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
			set_proc_param(stream[0].proc_path, &main_dim);
			//set_enc_cfg(stream[0].enc_path, &stream[0].enc_max_dim, enc_bitrate * 1024 * 1024, 0);
			//set_enc_param(stream[0].enc_path, &stream[0].enc_dim, enc_type, enc_bitrate * 1024 * 1024);

			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);

			if (sensor_shdr_1 == 1) {
				d_gain.id = 1;
				c_gain.id = 1;
				total_gain.id = 1;
				lv.id = 1;
				ct.id = 1;
				vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
				vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
				vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
				vendor_isp_set_common(ISPT_ITEM_LV, &lv);
				vendor_isp_set_common(ISPT_ITEM_CT, &ct);
			}

			vendor_isp_uninit();

			ae_preset.enable = ENABLE;
			ae_preset.exp_time = sensor_expt.time[0];
			ae_preset.gain_ratio = sensor_gain.ratio[0];
			vendor_videocap_set(stream[0].cap_path, VENDOR_VIDEOCAP_PARAM_AE_PRESET, &ae_preset);

			if (data_mode1) {
				hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
				hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
				//direct NOTE: ensure videocap start after 1st videoproc phy path start
				hd_videoproc_start(stream[0].proc_path);
				hd_videocap_start(stream[0].cap_path);
			} else {
				hd_videocap_start(stream[0].cap_path);
				hd_videoproc_start(stream[0].proc_path);
			}
			//hd_videoenc_start(stream[0].enc_path);
			stream[0].flow_start = 1;
		}

		if (key == 's') {
			ISPT_D_GAIN d_gain = {0};
			ISPT_C_GAIN c_gain = {0};
			ISPT_TOTAL_GAIN total_gain = {0};
			ISPT_LV lv = {0};
			ISPT_CT ct = {0};
			VENDOR_VIDEOCAP_AE_PRESET ae_preset = {0};
			ISPT_SENSOR_EXPT sensor_expt = {0};
			ISPT_SENSOR_GAIN sensor_gain = {0};

			vendor_isp_init();
			d_gain.id = 0;
			c_gain.id = 0;
			total_gain.id = 0;
			lv.id = 0;
			ct.id = 0;
			vendor_isp_get_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_get_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_get_common(ISPT_ITEM_LV, &lv);
			vendor_isp_get_common(ISPT_ITEM_CT, &ct);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			printf("d gain = %d \n", d_gain.gain);
			printf("c gain = %d %d %d \n", c_gain.gain[0], c_gain.gain[1], c_gain.gain[2]);
			printf("total gain = %d \n", total_gain.gain);
			printf("lv = %d \n", lv.lv);
			printf("ct = %d \n", ct.ct);
			printf("expt = %d, ratio = %d \n", sensor_expt.time[0], sensor_gain.ratio[0]);

			sensor_shdr_1 = 1;
			printf("SHDR %d \n", sensor_shdr_1);

			if (data_mode1) {
				//direct NOTE: ensure videocap stop after all videoproc path stop
				hd_videoproc_stop(stream[0].proc_path);
				hd_videocap_stop(stream[0].cap_path);
				hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
				hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
			} else {
				hd_videocap_stop(stream[0].cap_path);
				hd_videoproc_stop(stream[0].proc_path);
			}
			hd_videocap_close(stream[0].cap_path);
			hd_videoproc_close(stream[0].proc_path);
			stream[0].flow_start = 0;

			open_module_without_enc(&stream[0], &stream[0].proc_max_dim);
			get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
			set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
			set_proc_param(stream[0].proc_path, &main_dim);

			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
			d_gain.id = 1;
			c_gain.id = 1;
			total_gain.id = 1;
			lv.id = 1;
			ct.id = 1;
			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
			vendor_isp_uninit();

			ae_preset.enable = ENABLE;
			ae_preset.exp_time = sensor_expt.time[0];
			ae_preset.gain_ratio = sensor_gain.ratio[0];
			vendor_videocap_set(stream[0].cap_path, VENDOR_VIDEOCAP_PARAM_AE_PRESET, &ae_preset);

			if (data_mode1) {
				hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
				hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
				//direct NOTE: ensure videocap start after 1st videoproc phy path start
				hd_videoproc_start(stream[0].proc_path);
				hd_videocap_start(stream[0].cap_path);
			} else {
				hd_videocap_start(stream[0].cap_path);
				hd_videoproc_start(stream[0].proc_path);
			}
			//hd_videoenc_start(stream[0].enc_path);
			stream[0].flow_start = 1;
		}

		if (key == 'l') {
			ISPT_D_GAIN d_gain = {0};
			ISPT_C_GAIN c_gain = {0};
			ISPT_TOTAL_GAIN total_gain = {0};
			ISPT_LV lv = {0};
			ISPT_CT ct = {0};
			VENDOR_VIDEOCAP_AE_PRESET ae_preset = {0};
			ISPT_SENSOR_EXPT sensor_expt = {0};
			ISPT_SENSOR_GAIN sensor_gain = {0};

			vendor_isp_init();
			d_gain.id = 0;
			c_gain.id = 0;
			total_gain.id = 0;
			lv.id = 0;
			ct.id = 0;
			vendor_isp_get_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_get_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_get_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_get_common(ISPT_ITEM_LV, &lv);
			vendor_isp_get_common(ISPT_ITEM_CT, &ct);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_EXPT, &sensor_expt);
			vendor_isp_get_common(ISPT_ITEM_SENSOR_GAIN, &sensor_gain);
			printf("d gain = %d \n", d_gain.gain);
			printf("c gain = %d %d %d \n", c_gain.gain[0], c_gain.gain[1], c_gain.gain[2]);
			printf("total gain = %d \n", total_gain.gain);
			printf("lv = %d \n", lv.lv);
			printf("ct = %d \n", ct.ct);
			printf("expt = %d, ratio = %d \n", sensor_expt.time[0], sensor_gain.ratio[0]);

			sensor_shdr_1 = 0;
			printf("SHDR %d \n", sensor_shdr_1);

			if (data_mode1) {
				//direct NOTE: ensure videocap stop after all videoproc path stop
				hd_videoproc_stop(stream[0].proc_path);
				hd_videocap_stop(stream[0].cap_path);
				hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
				hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
			} else {
				hd_videocap_stop(stream[0].cap_path);
				hd_videoproc_stop(stream[0].proc_path);
			}

			hd_videocap_close(stream[0].cap_path);
			hd_videoproc_close(stream[0].proc_path);
			stream[0].flow_start = 0;

			open_module_without_enc(&stream[0], &stream[0].proc_max_dim);
			get_cap_caps(stream[0].cap_ctrl, &stream[0].cap_syscaps);
			set_cap_param(stream[0].cap_path, &stream[0].cap_dim, 0);
			set_proc_param(stream[0].proc_path, &main_dim);

			vendor_isp_set_common(ISPT_ITEM_D_GAIN, &d_gain);
			vendor_isp_set_common(ISPT_ITEM_C_GAIN, &c_gain);
			vendor_isp_set_common(ISPT_ITEM_TOTAL_GAIN, &total_gain);
			vendor_isp_set_common(ISPT_ITEM_LV, &lv);
			vendor_isp_set_common(ISPT_ITEM_CT, &ct);
			vendor_isp_uninit();

			ae_preset.enable = ENABLE;
			ae_preset.exp_time = sensor_expt.time[0];
			ae_preset.gain_ratio = sensor_gain.ratio[0];
			vendor_videocap_set(stream[0].cap_path, VENDOR_VIDEOCAP_PARAM_AE_PRESET, &ae_preset);

			if (data_mode1) {
				hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
				hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
				//direct NOTE: ensure videocap start after 1st videoproc phy path start
				hd_videoproc_start(stream[0].proc_path);
				hd_videocap_start(stream[0].cap_path);
			} else {
				hd_videocap_start(stream[0].cap_path);
				hd_videoproc_start(stream[0].proc_path);
			}
			stream[0].flow_start = 1;
		}
		// NOTE:   End   of test path 1 linear/shdr switch
	}

	nvtrtspd_close();

	// stop video_record modules (main)
	if (sensor_sel_1 != SEN_SEL_NONE) {
		if (data_mode1) {
			//direct NOTE: ensure videocap stop after all videoproc path stop
			hd_videoproc_stop(stream[0].proc_path);
			hd_videocap_stop(stream[0].cap_path);
		} else {
			hd_videocap_stop(stream[0].cap_path);
			hd_videoproc_stop(stream[0].proc_path);
		}
		hd_videoenc_stop(stream[0].enc_path);

		// refresh to indicate no stream
		refresh_video_info(0, &video_info[0]);
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		hd_videocap_stop(stream[1].cap_path);
		hd_videoproc_stop(stream[1].proc_path);
		hd_videoenc_stop(stream[1].enc_path);

		// refresh to indicate no stream
		refresh_video_info(1, &video_info[1]);
	}

	// destroy encode thread
	if (sensor_sel_1 != SEN_SEL_NONE) {
		pthread_join(stream[0].enc_thread_id, NULL);
	} else {
		pthread_join(stream[1].enc_thread_id, NULL);
	}

	if (sensor_sel_1 != SEN_SEL_NONE) {
		// unbind video_record modules (main)
		hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
		hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);

		if (vir_addr_main) hd_common_mem_munmap((void *)vir_addr_main, phy_buf_main.buf_info.buf_size);
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		// unbind video_record modules (sub)
		hd_videocap_unbind(HD_VIDEOCAP_1_OUT_0);
		hd_videoproc_unbind(HD_VIDEOPROC_1_OUT_0);

		if (vir_addr_sub) hd_common_mem_munmap((void *)vir_addr_sub, phy_buf_sub.buf_info.buf_size);
	}

exit:
	if (sensor_sel_1 != SEN_SEL_NONE) {
		// close video_record modules (main)
		ret = close_module(&stream[0]);
		if (ret != HD_OK) {
			printf("close fail=%d\n", ret);
		}
	}

	if (sensor_sel_2 != SEN_SEL_NONE) {
		ret = close_module(&stream[1]);
		if (ret != HD_OK) {
			printf("close fail=%d\n", ret);
		}
	}

	#if (PLUG_OSG_FUNC)
	stream_osg.thread_run = FALSE;
	pthread_join(stream_osg.thread_id, NULL);

	osg_icon_uninit();

	if(stream_osg.stamp_blk) {
		if(hd_common_mem_release_block(stream_osg.stamp_blk) != HD_OK) {
			printf("hd_common_mem_release_block() fail\n");
		}
	}
	#endif

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}

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

	return 0;
}
