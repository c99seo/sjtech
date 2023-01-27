#ifndef _UVC_CAM_H_
#include "hdal.h"
#include "kwrap/stdio.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/examsys.h"
#include "kwrap/sxcmd.h"
#include <kwrap/cmdsys.h>
#include "kwrap/error_no.h"
#include <kwrap/util.h>
#include "kwrap/perf.h"

#define _UVC_CAM_H_
// Define Bits Containing Capabilities of the control for the Get_Info request (section 4.1.2 in the UVC spec 1.1)
#define SUPPORT_GET_REQUEST							0x01
#define SUPPORT_SET_REQUEST							0x02
#define DISABLED_DUE_TO_AUTOMATIC_MODE				0x04
#define AUTOUPDATE_CONTROL							0x08
#define ASNCHRONOUS_CONTROL							0x10
#define RESERVED_BIT5								0x20
#define RESERVED_BIT6								0x40
#define RESERVED_BIT7								0x80

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
	HD_PATH_ID proc_main_path;
	HD_PATH_ID proc_sub_path;

	HD_DIM  enc_main_max_dim;
	HD_DIM  enc_main_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_main_path;

	// (4) user pull
	VK_TASK_HANDLE enc_thread_id;
	UINT32         enc_exit;
	HD_VIDEO_CODEC codec_type;

	// (5) YUV pull
	VK_TASK_HANDLE  acquire_thread_id;
	UINT32     acquire_exit;

} VIDEO_STREAM;

#endif	/* _UVC_CAM_H_ */
