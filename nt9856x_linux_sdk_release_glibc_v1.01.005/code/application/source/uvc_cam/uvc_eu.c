#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hd_type.h"
#include "uvc_eu.h"
#include "uvc_cam.h"
#include "hdal.h"
#include "hd_debug.h"
#include "UVAC.h"
#include <kwrap/debug.h>
#include <kwrap/util.h>
#include <kwrap/task.h>
#include "kwrap/semaphore.h"

extern VIDEO_STREAM stream[1];
static UINT32 raw_seq = NUM_PACK;
UINT32 raw_offser = 0;
static HD_VIDEO_FRAME raw_data = {0};
UINT32 raw_phy_addr, raw_vir_addr;
 #define PHY2VIRT_MAIN(pa) (raw_vir_addr + ((pa) - raw_phy_addr))

static BOOL xUvac_EU_GetRawCB(UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	static UINT8 buf[SIZE_PACK];	//maximum 20KB per packet
	HD_RESULT ret = HD_OK;

	switch (request){
		case GET_LEN:
			pData[0] = 0x00;
			pData[1] = 0x50;	//20 KB
			*pDataLen = 2;		//must be 2 according to UVC spec
			return TRUE;
		case GET_INFO:
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			*pDataLen = 1;		//must be 1 according to UVC spec
			return TRUE;
		case GET_MIN:
			memset(buf, 0x00, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_MAX:
			memset(buf, 0xFF, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_RES:
			memset(buf, 0x01, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_DEF:
			memset(buf, 0x00, sizeof(buf));
			*(UINT32 *)pData = (UINT32)buf;
			*pDataLen = sizeof(buf);
			return TRUE;
		case GET_CUR:
			if (raw_seq > 0){
				raw_seq--;
			} else {
				raw_seq = NUM_PACK;//reset raw_seq
			}
			*(UINT32 *)pData = (UINT32)PHY2VIRT_MAIN(raw_data.phy_addr[0] + (raw_offser * SIZE_PACK));
			*pDataLen = SIZE_PACK;
			raw_offser++;
			
			if (raw_offser == NUM_PACK){
				raw_offser = 0;	
				ret = hd_videocap_release_out_buf(stream[0].cap_path, &raw_data);
				if (ret != HD_OK) {
					printf("cap_release error=%d !!\r\n\r\n", ret);
				}

				hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);
				hd_videoproc_bind(HD_VIDEOPROC_0_OUT_0, HD_VIDEOENC_0_IN_0);
				hd_videoproc_start(stream[0].proc_main_path);
				hd_videoenc_start(stream[0].enc_main_path);
				printf("HDAL bind/start! \r\n\r\n");
			}
			return TRUE;
		case SET_CUR:
			//UVAC lib does not support XU set_cur with data larger than 64 bytes
			DBG_ERR("not support Data = 0x%02X for SET_CUR\r\n", *pData);
			return FALSE;
	}
	DBG_ERR("get raw not support!\r\n");
	return FALSE;
}

static BOOL xUvac_EU_SetRawSeqCB(UINT8 request, UINT8 *pData, UINT32 *pDataLen){

	HD_RESULT ret = HD_OK;

	switch (request){
		case GET_LEN:
			pData[0] = 0x02;
			pData[1] = 0x00;
			*pDataLen = 2;		//must be 2 according to UVC spec
			return TRUE;
		case GET_INFO:
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			*pDataLen = 1;		//must be 1 according to UVC spec
			return TRUE;
		case GET_MIN:
			pData[0] = 0x00;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_MAX:
			pData[0] = 0xFF;
			pData[1] = 0xFF;
			*pDataLen = 2;
			return TRUE;
		case GET_RES:
			pData[0] = 0x01;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_DEF:
			pData[0] = 0x00;
			pData[1] = 0x00;
			*pDataLen = 2;
			return TRUE;
		case GET_CUR:
			pData[0] = raw_seq & 0xFF;
			pData[1] = (raw_seq>>8) & 0xFF;
			*pDataLen = 2;
			return TRUE;
		case SET_CUR:
			hd_videoproc_stop(stream[0].proc_main_path);
			hd_videoenc_stop(stream[0].enc_main_path);
			hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
			hd_videoproc_unbind(HD_VIDEOPROC_0_OUT_0);
			printf("HDAL unbind/stop! \r\n\r\n");

			raw_seq = pData[0] + (pData[1]<<8);
			ret = hd_videocap_pull_out_buf(stream[0].cap_path, &raw_data, -1); // -1 = blocking mode
			if (ret != HD_OK) {
				printf("cap_pull error=%d !!\r\n\r\n", ret);
				return FALSE;
			}
			
			raw_phy_addr = hd_common_mem_blk2pa(raw_data.blk);
			raw_vir_addr = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, raw_phy_addr, SIZE_RAW);
			return TRUE;
	}
	DBG_ERR("set raw_seq not support!\r\n");
	return FALSE;
}

BOOL xUvacEU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	BOOL ret = FALSE;

	if (request == SET_CUR){
		printf("%s: SET_CUR, CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}
	switch (CS){
		case EU_CONTROL_ID_01:
		case EU_CONTROL_ID_02:
		case EU_CONTROL_ID_03:
		case EU_CONTROL_ID_04:
		case EU_CONTROL_ID_05:
			ret = xUvac_EU_GetRawCB(request, pData, pDataLen);
			break;
		case EU_CONTROL_ID_06:
			ret = xUvac_EU_SetRawSeqCB(request, pData, pDataLen);
			break;
		case EU_CONTROL_ID_07:
		case EU_CONTROL_ID_08:
		case EU_CONTROL_ID_09:
		case EU_CONTROL_ID_10:
		case EU_CONTROL_ID_11:
		case EU_CONTROL_ID_12:
		case EU_CONTROL_ID_13:
		case EU_CONTROL_ID_14:
		case EU_CONTROL_ID_15:
		case EU_CONTROL_ID_16:
			//not supported now
			break;
		default:
			break;
	}

	if (request != SET_CUR) {
		printf("%s: CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}

	if (ret && request == SET_CUR){
		*pDataLen = 0;
	}
	
	return ret;
}

#if 0 //sample code for customized 2nd EU callback
BOOL xUvacEU2_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen){
	BOOL ret = FALSE;

	if (request == SET_CUR){
		printf("%s: SET_CUR, CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}
	switch (CS){
		case EU_CONTROL_ID_01:
		case EU_CONTROL_ID_02:
		case EU_CONTROL_ID_03:
		case EU_CONTROL_ID_04:
		case EU_CONTROL_ID_05:
		case EU_CONTROL_ID_06:
		case EU_CONTROL_ID_07:
		case EU_CONTROL_ID_08:
		case EU_CONTROL_ID_09:
		case EU_CONTROL_ID_10:
		case EU_CONTROL_ID_11:
		case EU_CONTROL_ID_12:
		case EU_CONTROL_ID_13:
		case EU_CONTROL_ID_14:
		case EU_CONTROL_ID_15:
		case EU_CONTROL_ID_16:
			//not supported now
			break;
		default:
			break;
	}

	if (request != SET_CUR) {
		printf("%s: CS=0x%02X, request = 0x%X,  pData=0x%X, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	}

	if (ret && request == SET_CUR){
		*pDataLen = 0;
	}

	return ret;
}
#endif

