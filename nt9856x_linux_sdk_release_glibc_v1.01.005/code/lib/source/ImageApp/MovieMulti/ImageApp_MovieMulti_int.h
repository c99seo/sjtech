#ifndef IMAGEAPP_MOVIEMULTI_INT_H
#define IMAGEAPP_MOVIEMULTI_INT_H

#include "ImageApp/ImageApp_MovieMulti.h"
#include "../Common/ImageApp_Common_int.h"
#include "avfile/movieinterface_def.h"
#include <kwrap/cmdsys.h>
#include <kwrap/sxcmd.h>
#include <kwrap/stdio.h>
#include <kwrap/debug.h>
#include <kwrap/error_no.h>
#include <kwrap/task.h>
#include <kwrap/flag.h>
#include <kwrap/util.h>
#include <kwrap/perf.h>
#include <kwrap/cpu.h>
#include <kwrap/verinfo.h>
#include <string.h>
#include "hd_bsmux_lib.h"
#include "hd_fileout_lib.h"
#include "vendor_audiocapture.h"
#include "vendor_audioenc.h"
#include "vendor_videodec.h"
#include "vendor_videoenc.h"
#include <vf_gfx.h>
#include "SizeConvert.h"
#include "exif/Exif.h"
#include "GxImageFile.h"
#include "Utility/Color.h"
#include "ImageApp_MovieMulti_FileManage.h"

/// ========== Branch definition area ==========
#define IAMOVIE_BR_1_25                         125
#define IAMOVIE_BR_1_40                         140
#define IAMOVIE_BRANCH                          IAMOVIE_BR_1_40

/// ========== Debug macro area ==========
#define IAMOVIE_COMM_CB_TIMEOUT_CHECK           1000    // unit: ms
#define IAMOVIE_BS_CB_TIMEOUT_CHECK             100     // unit: ms
#define ImageApp_MovieMulti_DUMP(fmtstr, args...) do { if(g_ia_moviemulti_trace_on) DBG_DUMP(fmtstr, ##args); } while(0)

/// ========== Internal function configuation area ==========
#define _IAMOVIE_ONE_SEC_CB_USING_VENC          ENABLE         // Enable for using venc, else using bsmux

/// use filedb & naming                         default: ENABLE
#if (IAMOVIE_BRANCH != IAMOVIE_BR_1_25)
#define USE_FILEDB_AND_NAMING                   ENABLE
#else
#define USE_FILEDB_AND_NAMING                   DISABLE
#endif

#if (USE_FILEDB_AND_NAMING == ENABLE) // remove file manage
#include "FileDB.h"
#endif

/// alsa function
#define USE_ALSA_LIB                            DISABLE
#define ALSA_DEBUG                              ENABLE

#if (USE_ALSA_LIB == ENABLE)
#include <errno.h>
#include <alsa/asoundlib.h>
#endif

/// ========== Max path definition area ==========
#if defined(__FREERTOS)
#define MAX_IMG_PATH                2
#else       // Linux
#if (IAMOVIE_BRANCH != IAMOVIE_BR_1_25)
#if defined(_BSP_NA51055_)
#define MAX_IMG_PATH                3
#else
#define MAX_IMG_PATH                2
#endif
#else
#define MAX_IMG_PATH                2
#endif
#endif
#define MAX_DISP_PATH               1
#define MAX_WIFI_PATH               1
#define MAX_UVAC_PATH               MAX_WIFI_PATH
#define MAX_AUDCAP_PATH             1
#define MAX_ETHCAM_PATH             1
#define MAX_IMGCAP_PATH             1

#define MAX_BSMUX_PATH              12

#define MAX_GPS_DATA_SIZE           2048

/// ========== Common definition area ==========
#define MASK_EMR_MAIN              (_CFG_MAIN_EMR_BOTH  | _CFG_MAIN_EMR_PAUSE  | _CFG_MAIN_EMR_ONLY)
#define MASK_EMR_CLONE             (_CFG_CLONE_EMR_BOTH | _CFG_CLONE_EMR_PAUSE | _CFG_CLONE_EMR_ONLY)

enum {
	LINKTYPE_REC = 0,
	LINKTYPE_STRM,
	LINKTYPE_DISP,
	LINKTYPE_ETHCAM,
};

enum {
	IMGCAP_THUM      = MOVIE_IMGCAP_THUMB,
	IMGCAP_JSTM      = MOVIE_IMGCAP_RAWENC,
	IMGCAP_EXIF_THUM = MOVIE_IMGCAP_EXIF_THUMB,
};

enum {
	MOVIETYPE_MAIN = 0,
	MOVIETYPE_CLONE,
	MOVIETYPE_MAX,
};

typedef enum {
	IMGCAP_THUMB_W = 640,
	IMGCAP_THUMB_H = 480,
	IMGCAP_EXIF_W  = 160,
	IMGCAP_EXIF_H  = 120,
} IMGCAP_THUMB_SIZE;

typedef enum {
	CB_PATH_STATUS_INIT     = 0x0000,
	CB_PATH_STATUS_RECORD   = 0x0001,
	CB_PATH_STATUS_CUT      = 0x0002,
	CB_PATH_STATUS_CLOSE    = 0x0003,
} CB_PATH_STATUS;

#define AENC_BS_RESERVED_SIZE_MP4       0
#define AENC_BS_RESERVED_SIZE_TS        21
#define VENC_BS_RESERVED_SIZE_MP4       0
#define VENC_BS_RESERVED_SIZE_TS        21
#define VENC_BS_RESERVED_SIZE_JPG       65536

/// ========== Utilities definition area ==========
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define DBGINFO_BUFSIZE()	(0x200)
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	ALIGN_CEIL_4(((w) * (h) * HD_VIDEO_PXLFMT_BPP(pxlfmt)) / 8)

/// ========== Definition area ==========
#define UPDATE_FORWARD              0
#define UPDATE_REVERSE              1

/// ========== Flag id definition area ==========
//IAMOVIE_FLG_ID
#define FLGIAMOVIE_VE_M0            0x00000001      /// ImgLink0 main
#define FLGIAMOVIE_VE_C0            0x00000002      /// ImgLink0 clone
#define FLGIAMOVIE_VE_M1            0x00000004      /// ImgLink1 main
#define FLGIAMOVIE_VE_C1            0x00000008      /// ImgLink1 clone
#define FLGIAMOVIE_VE_M2            0x00000010      /// ImgLink2 main
#define FLGIAMOVIE_VE_C2            0x00000020      /// ImgLink2 clone
#define FLGIAMOVIE_VE_M3            0x00000040      /// ImgLink3 main
#define FLGIAMOVIE_VE_C3            0x00000080      /// ImgLink3 clone
#define FLGIAMOVIE_VE_S0            0x00000100      /// WifiLink
#define FLGIAMOVIE_VE_MASK          0x000001FF      /// All VE

#define FLGIAMOVIE_AE_AAC           0x00001000      /// AAC encode
#define FLGIAMOVIE_AE_MASK          0x00001000      /// All AE

#define FLGIAMOVIE_AE_S0            0x00010000

#define FLGIAMOVIE_AC_PCM           0x00008000      /// PCM CAP
#define FLGIAMOVIE_AC_MASK          0x00008000      /// All ACap

#define FLGIAMOVIE_AC_GET           0x01000000
#define FLGIAMOVIE_AC_GET_DONE      0x02000000

#define FLGIAMOVIE_FLOW_LINK_CFG    0x80000000      /// All link is eatablished

#define FLGIAMOVIE_MASK             FLGPTN_BIT_ALL

//IAMOVIE_FLG_ID2
#define FLGIAMOVIE2_JPG_M0_RDY      0x00000001      /// ImgLink0 main rawenc
#define FLGIAMOVIE2_JPG_C0_RDY      0x00000002      /// ImgLink0 clone rawenc
#define FLGIAMOVIE2_JPG_M1_RDY      0x00000004      /// ImgLink1 main rawenc
#define FLGIAMOVIE2_JPG_C1_RDY      0x00000008      /// ImgLink1 clone rawenc
#define FLGIAMOVIE2_JPG_M2_RDY      0x00000010      /// ImgLink2 main rawenc
#define FLGIAMOVIE2_JPG_C2_RDY      0x00000020      /// ImgLink2 clone rawenc
#define FLGIAMOVIE2_JPG_M3_RDY      0x00000040      /// ImgLink3 main rawenc
#define FLGIAMOVIE2_JPG_C3_RDY      0x00000080      /// ImgLink3 clone rawenc
#define FLGIAMOVIE2_THM_M0_RDY      0x00000100      /// ImgLink0 main thumbnail
#define FLGIAMOVIE2_THM_C0_RDY      0x00000200      /// ImgLink0 clone thumbnail
#define FLGIAMOVIE2_THM_M1_RDY      0x00000400      /// ImgLink1 main thumbnail
#define FLGIAMOVIE2_THM_C1_RDY      0x00000800      /// ImgLink1 clone thumbnail
#define FLGIAMOVIE2_THM_M2_RDY      0x00001000      /// ImgLink2 main thumbnail
#define FLGIAMOVIE2_THM_C2_RDY      0x00002000      /// ImgLink2 clone thumbnail
#define FLGIAMOVIE2_THM_M3_RDY      0x00004000      /// ImgLink3 main thumbnail
#define FLGIAMOVIE2_THM_C3_RDY      0x00008000      /// ImgLink3 clone thumbnail
#define FLGIAMOVIE2_RAWENC_RDY      0x0000FFFF      /// All rawenc path

#define FLGIAMOVIE2_VE_IMGCAP       0x80000000      /// ImgCapLink

#define FLGIAMOVIE2_MASK            FLGPTN_BIT_ALL

/// ========== Link data structures area ==========
/// ===== AudCapLink =====
typedef struct {
	// audiocap
	HD_PATH_ID           acap_p_ctrl;
	HD_CTRL_ID           acap_ctrl;
	HD_IN_ID             acap_i[1];
	HD_OUT_ID            acap_o[1];
	HD_PATH_ID           acap_p[1];
	// audioenc
	HD_IN_ID             aenc_i[1];
	HD_OUT_ID            aenc_o[1];             // aac
	HD_PATH_ID           aenc_p[1];
} MOVIE_AUDCAP_LINK;

typedef struct {
	// audiocap
	UINT32               acap_p[1];
	// audioenc
	UINT32               aenc_p[1];             // aac
} MOVIE_AUDCAP_LINK_STATUS;

/// ===== DispLink =====
typedef struct {
	// videoout
	HD_VIDEOOUT_SYSCAPS  vout_syscaps;
	HD_PATH_ID           vout_p_ctrl;
	// VdoOut flow refine: vout is opened in project
	#if 0
	HD_IN_ID             vout_i[1];
	HD_OUT_ID            vout_o[1];
	#endif
	HD_PATH_ID           vout_p[1];
	HD_DIM               vout_ratio;
	HD_URECT             vout_win;
	HD_VIDEO_DIR         vout_dir;
} MOVIE_DISP_LINK;

typedef struct {
	// videoout
	UINT32               vout_p[1];
} MOVIE_DISP_LINK_STATUS;

/// ===== EthCamLink =====
typedef struct {
	// video decode
	HD_IN_ID             vdec_i[1];
	HD_OUT_ID            vdec_o[1];
	HD_PATH_ID           vdec_p[1];
	// bsmux
	HD_IN_ID             bsmux_i[2];
	HD_OUT_ID            bsmux_o[2];
	HD_PATH_ID           bsmux_p[2];
	// fileout
	HD_IN_ID             fileout_i[2];
	HD_OUT_ID            fileout_o[2];
	HD_PATH_ID           fileout_p[2];
} MOVIE_ETHCAM_LINK;

typedef struct {
	// video decode
	UINT32               vdec_p[1];
	// bsmux
	UINT32               bsmux_p[2];
	// fileout
	UINT32               fileout_p[2];
} MOVIE_ETHCAM_LINK_STATUS;

/// ===== ImgCapLink =====
typedef struct {
	// video encode
	HD_IN_ID             venc_i[1];
	HD_OUT_ID            venc_o[1];
	HD_PATH_ID           venc_p[1];
} MOVIE_IMGCAP_LINK;

typedef struct {
	// video encode
	UINT32               venc_p[1];
} MOVIE_IMGCAP_LINK_STATUS;

/// ===== ImgLink =====
typedef struct {
	// video capture
	HD_VIDEOCAP_SYSCAPS  vcap_syscaps;
	HD_CTRL_ID           vcap_ctrl;
	HD_PATH_ID           vcap_p_ctrl;
	HD_IN_ID             vcap_i[1];
	HD_OUT_ID            vcap_o[1];
	HD_PATH_ID           vcap_p[1];
	// video process
	HD_VIDEOPROC_SYSCAPS vproc_syscaps;
	HD_CTRL_ID           vproc_ctrl;
	HD_PATH_ID           vproc_p_ctrl;
	HD_IN_ID             vproc_i[1];
	HD_OUT_ID            vproc_o_phy[5];
	HD_PATH_ID           vproc_p_phy[5];
	HD_OUT_ID            vproc_o[7];            // main / clone / disp / wifi / alg / disp2 / alg(y)
	HD_PATH_ID           vproc_p[7];
	// video encode
	HD_IN_ID             venc_i[2];
	HD_OUT_ID            venc_o[2];
	HD_PATH_ID           venc_p[2];
	// bsmux
	HD_IN_ID             bsmux_i[4];
	HD_OUT_ID            bsmux_o[4];
	HD_PATH_ID           bsmux_p[4];
	// fileout
	HD_IN_ID             fileout_i[4];
	HD_OUT_ID            fileout_o[4];
	HD_PATH_ID           fileout_p[4];
} MOVIE_IMAGE_LINK;

typedef struct {
	// video capture
	UINT32               vcap_p[1];
	// video process
	UINT32               vproc_p_phy[5];
	UINT32               vproc_p[7];            // main / clone / disp / wifi / alg / disp2 / alg(y)
	// video encode
	UINT32               venc_p[2];
	// bsmux
	UINT32               bsmux_p[4];
	// fileout
	UINT32               fileout_p[4];
} MOVIE_IMAGE_LINK_STATUS;

/// ===== WifiLink =====
typedef struct {
	// video encode
	HD_IN_ID             venc_i[1];
	HD_OUT_ID            venc_o[1];
	HD_PATH_ID           venc_p[1];
} MOVIE_WIFI_LINK;

typedef struct {
	// video encode
	UINT32               venc_p[1];
} MOVIE_WIFI_LINK_STATUS;

/// ========== Internal data structures area ==========
typedef struct {
	USIZE size;
	UINT32 fmt;
	UINT32 fps;
	USIZE ratio;
} MOVIE_IMG_IME_SETTING;

#if (USE_FILEDB_AND_NAMING == ENABLE) // remove file manage
typedef struct {
	CHAR path_prev[132];
	UINT32 is_crash_curr;
	UINT32 is_crash_prev;
	UINT32 is_crash_next;
	UINT32 is_crash_next_old; //is_crash_next of the last time
} MOVIE_CB_CRASH_INFO;
#endif

typedef struct {
	CB_PATH_STATUS status;
	CHAR *p_fpath;
	UINT32 duration;
} MOVIE_CB_PATH_INFO;

typedef struct {
	UINT32 idx;
	UINT32 tbl;
	UINT32 link_type;
} MOVIE_TBL_IDX;

/// ========== ImaCap queue area ==========
#define IMGCAP_JOBQUEUENUM      4

typedef struct {
	QUEUE_BUFFER_ELEMENT q;             // should be the first element of this structure
	UINT32               type;
	UINT32               exif;
	HD_PATH_ID           path;
	UINT32               is_emr;        // for thumb only, to record normal or emr path
} IMGCAP_JOB_ELEMENT;

/// ========== Internal cross file variable area ==========
/// ImageApp_MovieMulti
extern UINT32 IsMovieMultiOpened;
extern MOVIEMULTI_MAX_LINK_INFO MaxLinkInfo;
extern UINT32 iamovie_need_reopen;
extern MOVIEMULTI_DRAM_CFG dram_cfg;
// Important notice!!! If enlarge MAX_IMG_PATH, following variables shoule also update initial value too !!!
extern UINT32 img_vcap_capout_dir[MAX_IMG_PATH];
extern UINT32 img_sensor_mapping[MAX_IMG_PATH];
extern UINT32 img_bsmux_bufsec[MAX_IMG_PATH][4];
extern UINT32 ImgLinkVdoEncBufMs[MAX_IMG_PATH][2];
extern MOVIE_AD_MAP ad_sensor_map[MAX_IMG_PATH];
/// ImageApp_MovieMulti_AudCapLink
extern MOVIEMULTI_AUDIO_INFO g_AudioInfo;
extern UINT32 IsAudCapLinkOpened[MAX_AUDCAP_PATH];
extern MOVIE_AUDCAP_LINK AudCapLink[MAX_AUDCAP_PATH];
extern MOVIE_AUDCAP_LINK_STATUS AudCapLinkStatus[MAX_AUDCAP_PATH];
extern HD_AUDIOCAP_DEV_CONFIG aud_acap_dev_cfg[MAX_AUDCAP_PATH];
extern HD_AUDIOCAP_DRV_CONFIG aud_acap_drv_cfg[MAX_AUDCAP_PATH];
extern HD_AUDIOCAP_IN aud_acap_param[MAX_AUDCAP_PATH];
extern HD_AUDIOCAP_ANR aud_acap_anr_param[MAX_AUDCAP_PATH];
extern HD_AUDIOENC_PATH_CONFIG2 aud_aenc_cfg[MAX_AUDCAP_PATH];
extern HD_AUDIOENC_IN aud_aenc_in_param[MAX_AUDCAP_PATH];
extern HD_AUDIOENC_OUT aud_aenc_out_param[MAX_AUDCAP_PATH];
extern HD_AUDIOENC_BUFINFO aud_aenc_bs_buf[MAX_AUDCAP_PATH];
extern UINT32 AudEncBufMs[MAX_AUDCAP_PATH];
extern UINT32 aud_cap_volume;
extern UINT32 aud_cap_alc_en;
extern BOOL aud_mute_enc_func;
extern BOOL aud_mute_enc;
extern UINT32 aud_mute_enc_buf_pa, aud_mute_enc_buf_va, aud_mute_enc_buf_size;
extern MovieAudCapCb *aud_cap_cb;
extern BOOL aud_acap_by_hdal;
#if (USE_ALSA_LIB == ENABLE)
extern snd_pcm_t *alsa_hdl;
extern UINT32 alsabuf_pa, alsabuf_va, alsabuf_size;
#endif
/// ImageApp_MovieMulti_CB
extern MovieUserEventCb *g_ia_moviemulti_usercb;
extern UINT32 g_vdo_rec_sec;
extern HD_BSMUX_PUT_DATA gps_data;
/// ImageApp_MovieMulti_DispLink
extern UINT32 IsDispLinkOpened[MAX_DISP_PATH];
extern UINT32 IsDispLinkStarted[MAX_DISP_PATH];
extern MOVIE_DISP_LINK DispLink[MAX_DISP_PATH];
extern MOVIE_DISP_LINK_STATUS DispLinkStatus[MAX_DISP_PATH];
extern UINT32 g_DispFps[MAX_DISP_PATH];
extern UINT32 g_DispMirrorFlip[MAX_DISP_PATH];
extern MOVIE_RAWPROC_CB  g_DispCb;
extern HD_URECT user_disp_win[MAX_DISP_PATH];
extern HD_VIDEOOUT_FUNC_CONFIG disp_func_cfg[MAX_DISP_PATH];
/// ImageApp_MovieMulti_EthCamLink
extern UINT32 EthCamRxFuncEn;
extern MOVIE_ETHCAM_LINK EthCamLink[MAX_ETHCAM_PATH];
extern MOVIE_ETHCAM_LINK_STATUS EthCamLinkStatus[MAX_ETHCAM_PATH];
extern UINT32 is_ethcamlink_opened[MAX_ETHCAM_PATH];
extern UINT32 is_ethcamlink_for_disp_enabled[MAX_ETHCAM_PATH][MAX_DISP_PATH];
extern HD_VIDEODEC_PATH_CONFIG ethcam_vdec_cfg[MAX_ETHCAM_PATH][1];
extern HD_VIDEODEC_IN ethcam_vdec_in_param[MAX_ETHCAM_PATH][1];
extern HD_H26XDEC_DESC ethcam_vdec_desc[MAX_ETHCAM_PATH][1];
extern MOVIEMULTI_ETHCAM_REC_INFO EthCamLinkRecInfo[MAX_ETHCAM_PATH];
extern HD_BSMUX_VIDEOINFO ethcam_bsmux_vinfo[MAX_ETHCAM_PATH][2];
extern HD_BSMUX_AUDIOINFO ethcam_bsmux_ainfo[MAX_ETHCAM_PATH][2];
extern HD_BSMUX_FILEINFO ethcam_bsmux_finfo[MAX_ETHCAM_PATH][2];
extern UINT32 ethcam_bsmux_front_moov_en[MAX_ETHCAM_PATH][2];
extern UINT32 ethcam_bsmux_front_moov_flush_sec[MAX_ETHCAM_PATH][2];
extern UINT32 ethcam_bsmux_bufsec[MAX_ETHCAM_PATH][2];
/// ImageApp_MovieMulti_id
extern ID IAMOVIE_FLG_ID, IAMOVIE_FLG_ID2;
extern ID IAMOVIE_SEMID_FM;
extern MovieAudBsCb  *aud_bs_cb[MAX_AUDCAP_PATH];
extern MovieVdoBsCb  *vdo_bs_rec_cb[MAX_IMG_PATH][2];
extern MovieVdoBsCb  *vdo_bs_strm_cb[MAX_WIFI_PATH];
/// ImageApp_MovieMulti_ImgCapLink
extern UINT32 IsImgCapLinkOpened[MAX_IMGCAP_PATH];
extern MOVIE_IMGCAP_LINK ImgCapLink[MAX_IMGCAP_PATH];
extern MOVIE_IMGCAP_LINK_STATUS ImgCapLinkStatus[MAX_IMGCAP_PATH];
extern HD_VIDEOENC_PATH_CONFIG imgcap_venc_path_cfg[MAX_IMGCAP_PATH];
extern HD_VIDEOENC_IN  imgcap_venc_in_param[MAX_IMGCAP_PATH];
extern HD_VIDEOENC_OUT2 imgcap_venc_out_param[MAX_IMGCAP_PATH];
extern HD_VIDEOENC_BUFINFO imgcap_venc_bs_buf[MAX_IMGCAP_PATH];
extern UINT32 imgcap_venc_bs_buf_va[MAX_IMGCAP_PATH];
extern QUEUE_BUFFER_QUEUE ImgCapJobQueueClear;
extern QUEUE_BUFFER_QUEUE ImgCapJobQueueReady;
extern UINT32 ImgCapExifFuncEn;
extern USIZE imgcap_thum_size;
extern UINT32 imgcap_rawenc_pa[MAX_IMG_PATH], imgcap_rawenc_va[MAX_IMG_PATH], imgcap_rawenc_size[MAX_IMG_PATH];
extern UINT32 imgcap_use_venc_buf;
/// ImageApp_MovieMulti_ImgLink
extern UINT32 IsImgLinkOpened[MAX_IMG_PATH];
extern MOVIE_IMAGE_LINK ImgLink[MAX_IMG_PATH];
extern MOVIE_IMAGE_LINK_STATUS ImgLinkStatus[MAX_IMG_PATH];
extern UINT32 gImgLinkAlgFuncEn[MAX_IMG_PATH];
extern UINT32 gImeMaxOutPath[MAX_IMG_PATH];
extern UINT32 gImgLinkForcedSizePath[MAX_IMG_PATH];
extern UINT32 gSwitchLink[MAX_IMG_PATH][6];
extern UINT32 gUserProcMap[MAX_IMG_PATH][4];
extern UINT32 fps_vcap_out[MAX_IMG_PATH], fps_vprc_p_out[MAX_IMG_PATH][3], fps_vprc_e_out[MAX_IMG_PATH][6];
extern MOVIEMULTI_IPL_SIZE_INFO IPLUserSize[MAX_IMG_PATH];
extern UINT32 IsSetImgLinkRecInfo[MAX_IMG_PATH][2];
extern MOVIE_RECODE_INFO ImgLinkRecInfo[MAX_IMG_PATH][2];
extern UINT32 ImgLinkTimelapseTime[MAX_IMG_PATH][2];
extern UINT32 ImgLinkTimelapsePlayRate;
extern UINT32 IsSetImgLinkAlgInfo[MAX_IMG_PATH];
extern MOVIE_ALG_INFO ImgLinkAlgInfo[MAX_IMG_PATH];
extern MOVIEMULTI_IME_CROP_INFO DispIMECropInfo[MAX_IMG_PATH];
extern UINT32 DispIMECropNoAutoScaling[MAX_IMG_PATH];
extern UINT32 IsImgLinkForEthCamTxEnabled[MAX_IMG_PATH][2];
extern HD_VIDEOCAP_DRV_CONFIG img_vcap_cfg[MAX_IMG_PATH];
extern HD_VIDEO_PXLFMT img_vcap_senout_pxlfmt[MAX_IMG_PATH];
extern HD_VIDEO_PXLFMT img_vcap_capout_pxlfmt[MAX_IMG_PATH];
extern UINT32 img_vcap_data_lane[MAX_IMG_PATH];
extern HD_VIDEOCAP_CTRL img_vcap_ctrl[MAX_IMG_PATH];
extern HD_VIDEOCAP_IN img_vcap_in_param[MAX_IMG_PATH];
extern HD_VIDEOCAP_CROP img_vcap_crop_param[MAX_IMG_PATH];
extern HD_VIDEOCAP_OUT img_vcap_out_param[MAX_IMG_PATH];
extern HD_VIDEOCAP_FUNC_CONFIG img_vcap_func_cfg[MAX_IMG_PATH];
extern VENDOR_VIDEOCAP_CCIR_FMT_SEL img_vcap_ccir_fmt[MAX_IMG_PATH];
extern UINT32 img_vcap_ccir_mode[MAX_IMG_PATH];
extern MOVIEMULTI_VCAP_CROP_INFO img_vcap_crop_win[MAX_IMG_PATH];
extern HD_VIDEOPROC_DEV_CONFIG img_vproc_cfg[MAX_IMG_PATH];
extern HD_VIDEOPROC_CTRL img_vproc_ctrl[MAX_IMG_PATH];
extern HD_VIDEOPROC_IN img_vproc_in_param[MAX_IMG_PATH];
extern HD_VIDEOPROC_FUNC_CONFIG img_vproc_func_cfg[MAX_IMG_PATH];
extern UINT32 img_vproc_yuv_compress[MAX_IMG_PATH];
extern HD_VIDEOENC_PATH_CONFIG img_venc_path_cfg[MAX_IMG_PATH][2];
extern HD_VIDEOENC_IN  img_venc_in_param[MAX_IMG_PATH][2];
extern HD_VIDEOENC_OUT2 img_venc_out_param[MAX_IMG_PATH][2];
extern HD_H26XENC_RATE_CONTROL2 img_venc_rc_param[MAX_IMG_PATH][2];
extern HD_VIDEOENC_BUFINFO img_venc_bs_buf[MAX_IMG_PATH][2];
extern UINT32 img_venc_quality_base_mode_en[MAX_IMG_PATH][2];
extern HD_BSMUX_VIDEOINFO img_bsmux_vinfo[MAX_IMG_PATH][4];
extern HD_BSMUX_AUDIOINFO img_bsmux_ainfo[MAX_IMG_PATH][4];
extern HD_BSMUX_FILEINFO img_bsmux_finfo[MAX_IMG_PATH][4];
extern UINT32 img_bsmux_front_moov_en[MAX_IMG_PATH][4];
extern UINT32 img_bsmux_front_moov_flush_sec[MAX_IMG_PATH][4];
extern UINT32 img_bsmux_2v1a_mode[MAX_IMG_PATH];
extern MOVIE_RECODE_FILE_OPTION ImgLinkFileInfo;
extern UINT32 use_unique_3dnr_path[MAX_IMG_PATH];
#if (_IAMOVIE_ONE_SEC_CB_USING_VENC == ENABLE)
extern UINT32 img_venc_frame_cnt[MAX_IMG_PATH][2];
extern UINT32 img_venc_1st_path_idx;
extern UINT32 img_venc_1st_path_ch;
#endif
extern UINT32 trig_once_limited[MAX_IMG_PATH][2];
extern UINT32 trig_once_cnt[MAX_IMG_PATH][2];
extern UINT32 trig_once_first_i[MAX_IMG_PATH][2];
extern UINT32 trig_once_2v1a_stop_path[MAX_IMG_PATH][2];
extern UINT32 gpsdata_pa, gpsdata_va, gpsdata_size, gpsdata_offset;
extern UINT32 gpsdata_rec_pa[MAX_IMG_PATH][2], gpsdata_rec_va[MAX_IMG_PATH][2], gpsdata_rec_size[MAX_IMG_PATH][2], gpsdata_rec_offset[MAX_IMG_PATH][2];
extern UINT32 gpsdata_eth_pa[MAX_ETHCAM_PATH][1], gpsdata_eth_va[MAX_ETHCAM_PATH][1], gpsdata_eth_size[MAX_ETHCAM_PATH][1], gpsdata_eth_offset[MAX_ETHCAM_PATH][1];
extern UINT32 gpsdata_rec_disable[MAX_IMG_PATH][2], gpsdata_rec_rate[MAX_IMG_PATH][2];
extern UINT32 gpsdata_eth_disable[MAX_ETHCAM_PATH][1], gpsdata_eth_rate[MAX_ETHCAM_PATH][1];
extern UINT32 img_pseudo_rec_mode[MAX_IMG_PATH][2];
/// ImageApp_MovieMulti_Param
/// ImageApp_MovieMulti_Operation
extern UINT32 IsImgLinkForDispEnabled[MAX_IMG_PATH][MAX_DISP_PATH];
extern UINT32 IsImgLinkForStreamingEnabled[MAX_IMG_PATH][MAX_WIFI_PATH];
extern UINT32 IsImgLinkForAlgEnabled[MAX_IMG_PATH][_CFG_ALG_PATH_MAX];
/// ImageApp_MovieMulti_Param
#if (USE_FILEDB_AND_NAMING == ENABLE) // remove file manage
extern UINT32 g_ia_moviemulti_filedb_filter;
extern UINT32 g_ia_moviemulti_filedb_max_num;
extern UINT32 g_ia_moviemulti_delete_filter;
#endif
/// ImageApp_MovieMulti_SxCmd
extern BOOL g_ia_moviemulti_trace_on;
/// ImageApp_MovieMulti_Utilities
#if (_IAMOVIE_ONE_SEC_CB_USING_VENC == DISABLE)
extern UINT32 FirstBsMuxPath;
#endif
extern UINT32 BsMuxFrmCnt[MAX_BSMUX_PATH];
/// ImageApp_MovieMulti_WifiLink
extern MOVIE_STRM_INFO WifiLinkStrmInfo[MAX_WIFI_PATH];
extern UINT32 IsWifiLinkOpened[MAX_WIFI_PATH];
extern UINT32 IsWifiLinkStarted[MAX_WIFI_PATH];
extern MOVIE_WIFI_LINK WifiLink[MAX_WIFI_PATH];
extern MOVIE_WIFI_LINK_STATUS WifiLinkStatus[MAX_WIFI_PATH];
extern HD_VIDEOENC_PATH_CONFIG wifi_venc_path_cfg[MAX_WIFI_PATH];
extern HD_VIDEOENC_IN  wifi_venc_in_param[MAX_WIFI_PATH];
extern HD_VIDEOENC_OUT2 wifi_venc_out_param[MAX_WIFI_PATH];
extern HD_H26XENC_RATE_CONTROL2 wifi_venc_rc_param[MAX_WIFI_PATH];
extern UINT32 WifiLinkVdoEncBufMs[MAX_WIFI_PATH];
extern MOVIE_RAWPROC_CB  g_WifiCb;

/// ========== Internal function area ==========
/// ImageApp_MovieMulti
extern ER _ImageApp_MovieMulti_LinkCfg(void);
extern ER _ImageApp_MovieMulti_RecidGetTableIndex(MOVIE_CFG_REC_ID id, MOVIE_TBL_IDX *p);
extern ER _ImageApp_MovieMulti_BsPortGetTableIndex(UINT32 id, MOVIE_TBL_IDX *p);
extern ER _ImageApp_MovieMulti_CheckEMRMode(void);
/// ImageApp_MovieMulti_AudCapLink
extern ER _ImageApp_MovieMulti_AudCapLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_AudSetAEncParam(UINT32 idx);
extern ER _ImageApp_MovieMulti_AudReopenAEnc(UINT32 idx);
extern ER _ImageApp_MovieMulti_AudCapLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_AudCapLinkClose(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_AudCapLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_AudRecStart(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_AudRecStop(MOVIE_CFG_REC_ID id);
extern void _ImageApp_MovieMulti_AudCapFrameProc(HD_AUDIO_FRAME *pdata);
/// ImageApp_MovieMulti_CB
extern INT32 _ImageApp_MovieMulti_BsMux_CB(CHAR *p_name, HD_BSMUX_CBINFO *cbinfo, UINT32 *param);
extern INT32 _ImageApp_MovieMulti_FileOut_CB(CHAR *p_name, HD_FILEOUT_CBINFO *cbinfo, UINT32 *param);
/// ImageApp_MovieMulti_DispLink
extern ER _ImageApp_MovieMulti_DispLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_DispLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_DispLinkClose(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_DispLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_DispGetSize(MOVIE_CFG_REC_ID id, USIZE *sz);
/// ImageApp_MovieMulti_EthCamLink
extern ER _ImageApp_MovieMulti_EthCamLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_EthCamLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_EthCamLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_EthCamLinkClose(MOVIE_CFG_REC_ID id);
/// ImageApp_MovieMulti_id
extern ER _ImageApp_MovieMulti_InstallID(void);
extern ER _ImageApp_MovieMulti_UninstallID(void);
/// ImageApp_MovieMulti_ImgCapLink
extern ER _ImageApp_MovieMulti_ImgCapLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_ImgCapLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_ImgCapLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_ImgCapLinkClose(MOVIE_CFG_REC_ID id);
/// ImageApp_MovieMulti_ImgLink
extern ER _ImageApp_MovieMulti_ImgLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_ImgSetVEncParam(UINT32 idx, UINT32 tbl);
extern ER _ImageApp_MovieMulti_ImgReopenVEnc(UINT32 idx, UINT32 tbl);
extern ER _ImageApp_MovieMulti_ImgLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_ImgLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_ImgLinkClose(MOVIE_CFG_REC_ID id);
/// ImageApp_MovieMulti_Operation
extern ER _ImageApp_MovieMulti_TriggerThumb(MOVIE_CFG_REC_ID id, UINT32 is_emr);
/// ImageApp_MovieMulti_Param
extern ER _ImageApp_MovieMulti_SetupAudioParam(void);
extern ER _ImageApp_MovieMulti_SetupRecParam(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_SetupFileOption(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_SetupStrmParam(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_SetupImgCapParam(MOVIE_CFG_REC_ID id);
/// ImageApp_MovieMulti_Utilities
#if (_IAMOVIE_ONE_SEC_CB_USING_VENC == DISABLE)
extern UINT32 _ImageApp_MovieMulti_GetFirstBsMuxPort(void);
#endif
extern UINT32 _ImageApp_MovieMulti_IsRecording(void);
/// ImageApp_MovieMulti_WifiLink
extern ER _ImageApp_MovieMulti_WifiLinkCfg(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_WifiSetVEncParam(UINT32 idx);
extern ER _ImageApp_MovieMulti_WifiReopenVEnc(UINT32 idx);
extern ER _ImageApp_MovieMulti_WifiLinkStatusUpdate(MOVIE_CFG_REC_ID id, UINT32 direction);
extern ER _ImageApp_MovieMulti_WifiLinkOpen(MOVIE_CFG_REC_ID id);
extern ER _ImageApp_MovieMulti_WifiLinkClose(MOVIE_CFG_REC_ID id);

/// ========== Switch sorting function area ==========
#define SWITCH_IN_MAX       6
#define SWITCH_OUT_MAX      6

typedef struct _SW_PORT_SPEC {
	UINT32 w;
	UINT32 h;
	UINT32 fmt;
	UINT32 frc;
} SW_PORT_SPEC;

typedef struct _SW_PORT_TABLE {
	UINT32 BeginCollect;
	UINT32 AutoInNum;
	SW_PORT_SPEC OUT[SWITCH_OUT_MAX];
	SW_PORT_SPEC IN[SWITCH_IN_MAX];
	UINT32 InStart;
	UINT32 InEnd;
} SW_PORT_TABLE;

extern ER _Switch_Auto_Begin(SW_PORT_TABLE *tbl);
extern ER _Switch_Auto_End(SW_PORT_TABLE *tbl);
extern UINT32 _Switch_Auto_GetConnect(SW_PORT_TABLE *tbl, UINT32 o);

#endif//IMAGEAPP_MOVIEMULTI_INT_H
