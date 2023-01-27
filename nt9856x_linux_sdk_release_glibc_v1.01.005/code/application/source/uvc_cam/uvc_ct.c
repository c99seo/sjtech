#include "hd_type.h"
#include "uvc_ct.h"
#include "hdal.h"
#include "hd_debug.h"
#include "UVAC.h"
#include "ae_ui.h"
#include "vendor_isp.h"
UINT32 uvc_ct_controls = (CT_SCANNING_MODE_MSK|CT_AE_MODE_MSK|CT_AE_PRIORITY_MSK|CT_EXPOSURE_TIME_ABSOLUTE_MSK|
                          CT_EXPOSURE_TIME_RELATIVE_MSK|CT_FOCUS_ABSOLUTE_MSK|CT_FOCUS_RELATIVE_MSK|
                          CT_ZOOM_ABSOLUTE_MSK|CT_ZOOM_RELATIVE_MSK|CT_PANTILT_ABSOLUTE_MSK|CT_PRIVACY_CONTROL_MSK|
                          CT_AUTO_FOCUS_MSK|CT_REGION_OF_INTEREST_MSK);

extern void Set_CT_Zoom_Absolute(UINT32 indx);
UINT32 ct_zoom_indx = 0;

static BOOL xUvac_CT_AE_Mode(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	AET_MANUAL AEInfo = {0};
	
	printf("%s: request(0x%x)\r\n", __func__, request);
	switch(request)
	{
		//No need to support GET_LEN, GET_MAX, GET_MIN
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_RES:
			*pDataLen = 1;
			pData[0] = 0x06;
			break;
		case GET_DEF:
			*pDataLen = 1;
			pData[0] = 0x02;
			break;
		case GET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			*pDataLen = 1;
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			if (AEInfo.manual.mode == MANUAL_MODE){
				pData[0] = 0x04;
			} else if (AEInfo.manual.mode == AUTO_MODE){
				pData[0] = 0x02;
			} else {
				printf("AE mode(%d) not supprt in UVC CT\n", AEInfo.manual.mode);
				return FALSE;
			}
			vendor_isp_uninit();
			break;
		case SET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			AEInfo.id = 0;
			if (pData[0] == 0x04){
				vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
				AEInfo.manual.mode = MANUAL_MODE;
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else if (pData[0] == 0x02){
				vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
				AEInfo.manual.mode = AUTO_MODE;
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else {
				printf("CT AE mode not support (%u)\r\n", pData[0]);
			}
			vendor_isp_uninit();
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}

static BOOL xUvac_CT_Exposure_Time_Absolute(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;
	AET_MANUAL AEInfo = {0};

	printf("%s: request(0x%x)\r\n", __func__, request);

	switch(request)
	{
		case GET_LEN:
			*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 4;
			pData[1] = 0;
			break;
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] =  SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_MIN:
			*pDataLen = 4;
			pData[0] = 625 & 0xFF;
			pData[1] = (625 >> 8) & 0xFF;
			pData[2] = (625 >> 16) & 0xFF;
			pData[3] = (625 >> 24) & 0xFF;
			break;
		case GET_MAX:
			*pDataLen = 4;
			pData[0] = 160000 & 0xFF;
			pData[1] = (160000 >> 8) & 0xFF;
			pData[2] = (160000 >> 16) & 0xFF;
			pData[3] = (160000 >> 24) & 0xFF;
			break;
		case GET_RES:
			*pDataLen = 4;
			pData[0] = 1;
			pData[1] = 0;
			pData[2] = 0;
			pData[3] = 0;
			break;
		case GET_DEF:
			*pDataLen = 4;
			pData[0] = 40000 & 0xFF;;
			pData[1] = (40000 >> 8) & 0xFF;
			pData[2] = (40000 >> 16) & 0xFF;
			pData[3] = (40000 >> 24) & 0xFF;
			break;
		case GET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			*pDataLen = 4;
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			if (AEInfo.manual.expotime==0 || AEInfo.manual.iso_gain==0){
				pData[0] = 625 & 0xFF;
				pData[1] = (625 >> 8) & 0xFF;
				pData[2] = (625 >> 16) & 0xFF;
				pData[3] = (625 >> 24) & 0xFF;
				AEInfo.manual.expotime = 625;       //set default value
				AEInfo.manual.iso_gain = 100;		//fixed to 100
				vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			} else {
				pData[0] = AEInfo.manual.expotime & 0xFF;
				pData[1] = (AEInfo.manual.expotime >> 8) & 0xFF;
				pData[2] = (AEInfo.manual.expotime >> 16) & 0xFF;
				pData[3] = (AEInfo.manual.expotime >> 24) & 0xFF;
			}
			vendor_isp_uninit();
			break;
		case SET_CUR:
			if (vendor_isp_init() == HD_ERR_NG) {
				return FALSE;
			}
			AEInfo.id = 0;
			vendor_isp_get_ae(AET_ITEM_MANUAL, &AEInfo);
			AEInfo.manual.expotime = pData[0] + (pData[1]<<8) + (pData[2]<<16) + (pData[3] << 24);
			AEInfo.manual.iso_gain = 100;
			vendor_isp_set_ae(AET_ITEM_MANUAL, &AEInfo);
			vendor_isp_uninit();
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}

static BOOL xUvac_CT_Zoom_Absolute(UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = TRUE;

	printf("%s: request(0x%x)\r\n", __func__, request);

	switch(request)
	{
		case GET_LEN:
			*pDataLen = 2;	//must be 2 according to UVC spec
			pData[0] = 2;
			pData[1] = 0;
			break;
		case GET_INFO:
			*pDataLen = 1;	//must be 1 according to UVC spec
			pData[0] = SUPPORT_GET_REQUEST | SUPPORT_SET_REQUEST;
			break;
		case GET_MIN:
			*pDataLen = 2;
			pData[0] = 0;
			pData[1] = 0;
			break;
		case GET_MAX:
			*pDataLen = 2;
			pData[0] = 0x28;	//40
			pData[1] = 0;
			break;
		case GET_RES:
			*pDataLen = 2;
			pData[0] = 1;
			pData[1] = 0;
			break;
		case GET_DEF:
			*pDataLen = 2;
			pData[0] = 0;		//0
			pData[1] = 0;
			break;
		case GET_CUR:
			printf("%s: GET_CUR\r\n", __func__);
			*pDataLen = 2;
			pData[0] = ct_zoom_indx;
			pData[1] = 0;
			break;
		case SET_CUR:
			//printf("%s: SET_CUR\r\n", __func__);
			printf("%s: SET_CUR pData=0x%X 0x%X =%d\r\n", __func__, pData[1], pData[0], (pData[1]<<8)+pData[0]);
			ct_zoom_indx = *pData;		// 0 ~ 40->depending on max dzoom factor
			Set_CT_Zoom_Absolute(ct_zoom_indx);
			break;
		default:
			ret = FALSE;
			break;
	}
	return ret;
}

BOOL xUvacCT_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen)
{
	BOOL ret = FALSE;

	printf("%s: CS=0x%x, request = 0x%X\r\n", __func__, CS, request);
//	printf("%s: CS=0x%x, request = 0x%X,  pData=0x%x, len=%d\r\n", __func__, CS, request, pData, *pDataLen);
	switch (CS){
		case CT_SCANNING_MODE_CONTROL:
			//not supported now
			break;
		case CT_AE_MODE_CONTROL:
			ret = xUvac_CT_AE_Mode(request, pData, pDataLen);
			break;
		case CT_AE_PRIORITY_CONTROL:
			//not supported now
			break;
		case CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
			ret = xUvac_CT_Exposure_Time_Absolute(request, pData, pDataLen);
			break;
		case CT_ZOOM_ABSOLUTE_CONTROL:
			ret = xUvac_CT_Zoom_Absolute(request, pData, pDataLen);
			break;
		case CT_EXPOSURE_TIME_RELATIVE_CONTROL:
		case CT_FOCUS_ABSOLUTE_CONTROL:
		case CT_FOCUS_RELATIVE_CONTROL:
		case CT_FOCUS_AUTO_CONTROL:
		case CT_IRIS_ABSOLUTE_CONTROL:
		case CT_IRIS_RELATIVE_CONTROL:
		case CT_ZOOM_RELATIVE_CONTROL:
		case CT_PANTILT_ABSOLUTE_CONTROL:
		case CT_PANTILT_RELATIVE_CONTROL:
		case CT_ROLL_ABSOLUTE_CONTROL:
		case CT_ROLL_RELATIVE_CONTROL:
		case CT_PRIVACY_CONTROL:
			//not supported now
			break;
		default:
			break;
	}
	if (ret && request == SET_CUR) {
		*pDataLen = 0;
	}
	return ret;
}

