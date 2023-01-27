#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hdal.h>
#include <kwrap/perf.h>
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include "UVAC.h"
#include "vendor_videoprocess.h"

#define ISF_LATENCY_DEBUG 0
#define UVAC_WAIT_RELEASE 1
#define PERF_TEST         0

#if 0
typedef struct {
	BOOL            slice_start;
	UINT32          frame_count;
	UINT32          slice_processed_h;      // for slice colortrans
	UINT8           slice_cnt;
	UINT8           slice_idx;
	UINT8           prev_slice_idx;
} UVC_SLICE_INFO;


UVC_SLICE_INFO slice_info;
#endif

#if 0
HD_RESULT yuv_transform(UINT32 buf_va, UINT32 buf_size, HD_DIM *main_dim)
{
	int                       fd;
	HD_GFX_COLOR_TRANSFORM    param;
	void                      *va;
	HD_RESULT                 ret;
	HD_VIDEO_FRAME            frame;
	UINT32                    src_size, dst_size, temp_size;
	int                       len;
	int                       w = 0, h = 0;
	UINT32                    src_pa, src_va, dst_pa, dst_va, temp_pa;
	static UINT32             gfx_qcnt = 0;

	UVAC_STRM_FRM      strmFrm = {0};

	w = main_dim->w;
	h = main_dim->h;
	//calculate yuv420 image's buffer size
	frame.sign = MAKEFOURCC('V','F','R','M');
	frame.pxlfmt  = HD_VIDEO_PXLFMT_YUV420;
	frame.dim.h   = h;
	frame.loff[0] = w;
	frame.loff[1] = w;
	src_size = hd_common_mem_calc_buf_size(&frame);
	if(!src_size){
		printf("hd_common_mem_calc_buf_size() fail to calculate src size\n");
		return -1;
	}

	//calculate yuyv image's buffer size
	frame.sign = MAKEFOURCC('V','F','R','M');
	frame.pxlfmt  = HD_VIDEO_PXLFMT_YUV422_ONE;
	frame.dim.h   = h;
	frame.loff[0] = w;
	dst_size = hd_common_mem_calc_buf_size(&frame);
	if(!dst_size){
		printf("hd_common_mem_calc_buf_size() fail to calculate yuv size\n");
		return -1;
	}
	temp_size = dst_size;

	if((src_size + dst_size + temp_size) > gfx[gfx_qcnt].buf_size){
		printf("required size(%d) > allocated size(%d)\n", (src_size + dst_size + temp_size), gfx[gfx_qcnt].buf_size);
		return -1;
	}

	src_pa  = gfx[gfx_qcnt].buf_pa;
	dst_pa  = src_pa + src_size;
	temp_pa = dst_pa + dst_size;

	// cpoy video stream to gfx buffer
	if (gfx[gfx_qcnt].buf_va){
		memcpy((void*)gfx[gfx_qcnt].buf_va,(const void *)buf_va, src_size);//gfx[gfx_qcnt].buf_size);
	} else {
		printf("gfx[%d].buf_va NULL!\n, gfx_qcnt");
		return -1;
	}

	src_va  = gfx[gfx_qcnt].buf_va;
	dst_va  = src_va + src_size;

	//use gfx engine to transform yuv420 image to yuyv
	memset(&param, 0, sizeof(HD_GFX_COLOR_TRANSFORM));
	param.src_img.dim.w            = w;
	param.src_img.dim.h            = h;
	param.src_img.format           = HD_VIDEO_PXLFMT_YUV420;
	param.src_img.p_phy_addr[0]    = src_pa;
	param.src_img.p_phy_addr[1]    = src_pa + w*h;
	param.src_img.lineoffset[0]    = w;
	param.src_img.lineoffset[1]    = w;
	param.dst_img.dim.w            = w;
	param.dst_img.dim.h            = h;
	param.dst_img.format           = HD_VIDEO_PXLFMT_YUV422_ONE;
	param.dst_img.p_phy_addr[0]    = dst_pa;
	param.dst_img.lineoffset[0]    = w * 2;
	param.p_tmp_buf                = temp_pa;
	param.tmp_buf_size             = temp_size;

	ret = hd_gfx_color_transform(&param);
	if(ret != HD_OK){
		printf("hd_gfx_color_transform fail=%d\n", ret);
		goto exit;
	}

#if UVC_ON
	strmFrm.path = UVAC_STRM_VID ;
	strmFrm.addr = dst_pa;
	strmFrm.size = dst_size;
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = dst_va;
	if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
		UVAC_SetEachStrmInfo(&strmFrm);

		#if UVAC_WAIT_RELEASE
		UVAC_WaitStrmDone(UVAC_STRM_VID);
		#endif
	}
	gfx_qcnt++;
	if (gfx_qcnt >= GFX_QUEUE_MAX_NUM)
		gfx_qcnt = 0; // This state machine does not consider queue overwrite issue.
#endif
	ret = HD_OK;

exit:
	return ret;
}
#endif
#if 0
void uvc_slice_push(HD_DIM *main_dim)
{
	UVAC_STRM_FRM      strmFrm = {0};
	UVC_SLICE_INFO     *pObj = &slice_info;
	BOOL                  isFinalpart = FALSE;
	UINT32                proc_h, image_h;

	image_h = main_dim->h;
	isFinalpart = TRUE;
	proc_h = image_h - pObj->slice_processed_h;
	if (proc_h == 0) {
		return 0;
	}
	pVStrmBuf->flag = MAKEFOURCC('P', 'V', 'S', 'T');
	pVStrmBuf->DataSize = p_tmp_dst_img->Height * p_tmp_dst_img->LineOffset[0];
	pVStrmBuf->Resv[6]  = isFinalpart;
	pVStrmBuf->Resv[7]  = p_dst_img->PxlAddr[0];
	pVStrmBuf->DataAddr = p_tmp_dst_img->PxlAddr[0];
	DBG_IND("[colortrans] Final. slice proc_h=%d, slice_processed_h=%d, size = %d, isFinalpart = %d, prev_slice=%d\r\n",
			proc_h, pObj->slice_processed_h, pVStrmBuf->DataSize, isFinalpart , pObj->prev_slice_idx);
	pObj->prev_slice_idx = 0;
	pObj->slice_processed_h += proc_h;
	pObj->slice_start = FALSE;
	p_out_isf->TimeStamp = p_in_isf->TimeStamp;
	#if ISF_LATENCY_DEBUG
	p_out_isf->vd_timestamp        = p_in_isf->vd_timestamp;
	p_out_isf->dramend_timestamp   = p_in_isf->dramend_timestamp;
	p_out_isf->ipl_timestamp_start = p_in_isf->ipl_timestamp_start;
	p_out_isf->ipl_timestamp_end   = p_in_isf->ipl_timestamp_end;
	p_out_isf->enc_timestamp_start = p_in_isf->enc_timestamp_start;
	p_out_isf->enc_timestamp_end   = p_in_isf->enc_timestamp_end;
	p_out_isf->rtsp_timestamp_start = p_in_isf->rtsp_timestamp_start;
	p_out_isf->uvc_timestamp_start = p_in_isf->uvc_timestamp_start;
	p_out_isf->uvc_timestamp_last_part_start = p_in_isf->uvc_timestamp_last_part_start;
	#endif
	ISF_ImgTrans.pBase->Push(&ISF_ImgTrans, pDest, (ISF_DATA *)p_out_isf, 0);
	ISF_ImgTrans.pBase->Release(&ISF_ImgTrans, pSrc, p_out_isf, ISF_OK);



	strmFrm.path = UVAC_STRM_VID ;
	strmFrm.addr = dst_pa;
	strmFrm.size = dst_size;
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = dst_va;
	if (stream[0].codec_type == HD_CODEC_TYPE_RAW){
		UVAC_SetEachStrmInfo(&strmFrm);

		#if UVAC_WAIT_RELEASE
		UVAC_WaitStrmDone(UVAC_STRM_VID);
		#endif
	}
}
#endif
/*
typedef struct _UVAC_STRM_FRM {
	UINT32              addr;       ///< Address of stream.
	UINT32              size;       ///< Size of stream.
	UVAC_STRM_PATH      path;       ///< Stream path.
	UINT8               *pStrmHdr;  ///< VS header address.
	UINT32              strmHdrSize;///< VS header size.
	UINT32              va;         ///< Virtual address of stream, for YUV frame only.
	UINT64              timestamp;  ///< Timestamp.
	UVAC_VIDEO_FRM_TYPE frame_type; ///< Video frame type.
} UVAC_STRM_FRM, *PUVAC_STRM_FRM;
*/

HD_RESULT yuv_push_to_usb(UINT32 buf_va, HD_VIDEO_FRAME *pVideoFrame)
{
	UVAC_STRM_FRM      strmFrm = {0};

	#if 0
	if (pVideoFrame->pxlfmt != VDO_PXLFMT_YUV422_YVYU) {
		DBG_ERR("pxlfmt = 0x%x\r\n", pVideoFrame->pxlfmt);
		return HD_ERR_PARAM;
	}
	#endif
	strmFrm.path = UVAC_STRM_VID;
	strmFrm.addr = pVideoFrame->phy_addr[0];
	strmFrm.size = pVideoFrame->loff[0]*pVideoFrame->ph[0] + pVideoFrame->loff[1]*pVideoFrame->ph[1];
	strmFrm.pStrmHdr = 0;
	strmFrm.strmHdrSize = 0;
	strmFrm.va = buf_va;
	strmFrm.timestamp = pVideoFrame->timestamp;
	//strmFrm.frame_type = UVAC_VIDEO_FRM_NORMAL;
	//printf("framecount = %lld, va = 0x%x, size = 0x%x, timestamp = %lld\r\n", pVideoFrame->count, strmFrm.va, strmFrm.size, strmFrm.timestamp);
	//printf("loff[0]= %d, loff[1]= %d, height = %d  \r\n", pVideoFrame->loff[0], pVideoFrame->loff[1], pVideoFrame->ph[0]);
	//printf("pa = 0x%x, size = 0x%x\r\n", strmFrm.addr, strmFrm.size);
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

	return HD_OK;
}

