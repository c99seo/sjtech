#ifndef IMAGEAPP_COMMON_H
#define IMAGEAPP_COMMON_H

/// ========== Enum area ==========
typedef enum {
	IACOMM_RTSP_BS_VIDEO = 0,
	IACOMM_RTSP_BS_AUDIO,
	ENUM_DUMMY4WORD(IACOMM_RTSP_BS_TYPE)
} IACOMM_RTSP_BS_TYPE;

typedef enum {
	IACOMM_RTSP_OK                                  =  0,
	IACOMM_RTSP_FAIL                                = -1,
	IACOMM_RTSP_CHANNEL_EXCEED                      = -2,       // open
	IACOMM_RTSP_CHANNEL_IN_USE                      = -3,       // open
	IACOMM_RTSP_CODEC_UNKNOWN                       = -4,       // open
	IACOMM_RTSP_BS_OVERRUN                          = -5,       // getbs
	IACOMM_RTSP_BS_UNDERRUN                         = -6,       // getbs
	ENUM_DUMMY4WORD(IACOMM_RTSP_ERR_CODE)
} IACOMM_RTSP_ERR_CODE;

// =================== User Event Callback ===================
typedef enum {
	IACOMM_USER_CB_EVENT_START                = 0x0000E500,
	// EVENT
	IACOMM_USER_CB_EVENT_RTSP_OPEN            = 0x0000E500,
	IACOMM_USER_CB_EVENT_RTSP_CLOSE,
	// ERROR
	IACOMM_USER_CB_ERROR_RTSP_BS_ERR          = 0x0000E600,

	ENUM_DUMMY4WORD(IACOMM_USER_CB_EVENT)
} IACOMM_USER_CB_EVENT;

typedef void (IACommonUserEventCb)(UINT32 id, IACOMM_USER_CB_EVENT event_id, UINT32 value);

/// ========== Structure area ==========
typedef struct {
	UINT32                 id;
	char                   path[32];
	UINT32                 addr;
} SENSOR_PATH_INFO;

typedef struct {
	UINT32                 type;
	IACOMM_RTSP_ERR_CODE   err_code;
	UINT32                 resv[2];
} IACOMM_RTSP_CB_INFO;

#define HD_VIDEOPROC_CFG				0x000f0000	//vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK		0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1		0x00000000  //vprc "0: cut w>1280, GDC =  on, 2D_LUT off after cut (LL slow)
#define HD_VIDEOPROC_CFG_STRIP_LV2		0x00010000  //vprc "1: cut w>2048, GDC = off, 2D_LUT off after cut (LL fast)
#define HD_VIDEOPROC_CFG_STRIP_LV3		0x00020000  //vprc "2: cut w>2688, GDC = off, 2D_LUT off after cut (LL middle)(2D_LUT best)
#define HD_VIDEOPROC_CFG_STRIP_LV4		0x00030000  //vprc "3: cut w> 720, GDC =  on, 2D_LUT off after cut (LL not allow)(GDC best)

/// ImageApp_Common
extern ER ImageApp_Common_Init(void);
extern ER ImageApp_Common_Uninit(void);
extern ER ImageApp_Common_RtspStart(UINT32 id);
extern ER ImageApp_Common_RtspStop(UINT32 id);
/// ImageApp_Common_CB
extern ER ImageApp_Common_RegUserCB(IACommonUserEventCb *fncb);
/// ImageApp_Common_Param
extern ER ImageApp_Common_SetParam(UINT32 id, UINT32 param, UINT32 value);
extern ER ImageApp_Common_GetParam(UINT32 id, UINT32 param, UINT32* value);

// =================== Parameters ===================
enum {
	IACOMMON_PARAM_START = 0x0000CD000,
	// rtsp group
	IACOMMON_PARAM_RTSP_GROUP_BEGIN = IACOMMON_PARAM_START,
	IACOMMON_PARAM_SUPPORT_RTCP = IACOMMON_PARAM_RTSP_GROUP_BEGIN,                  /// var
	IACOMMON_PARAM_RTSP_GROUP_END,
};

#endif//IMAGEAPP_COMMON_H
