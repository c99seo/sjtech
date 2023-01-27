#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <kdrv_builtin/kdrv_builtin.h>
#include "isp_builtin.h"
#else
#include "plat/gpio.h"
#endif
#include "kwrap/error_no.h"
#include "kwrap/type.h"
#include "kwrap/util.h"
#include <kwrap/verinfo.h>
#include "kflow_videocapture/ctl_sen.h"
#include "isp_api.h"

#include "sen_cfg.h"
#include "sen_common.h"
#include "sen_inc.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#if defined(__KERNEL__)
char *sen_cfg_path = "null";
module_param_named(sen_cfg_path, sen_cfg_path, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sen_cfg_path, "Path of cfg file");

#ifdef DEBUG
unsigned int sen_debug_level = THIS_DBGLVL;
module_param_named(sen_debug_level, sen_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sen_debug_level, "Debug message level");
#endif
#endif

//=============================================================================
// version
//=============================================================================
VOS_MODULE_VERSION(nvt_sen_sc3336, 1, 45, 000, 00);

//=============================================================================
// information
//=============================================================================
#define SEN_SC3336_MODULE_NAME      "sen_sc3336"
#define SEN_MAX_MODE                1
#define MAX_VD_PERIOD               0x7FFF
#define MAX_EXPOSURE_LINE           0xFFFF
#define MIN_EXPOSURE_LINE           1
#define NON_EXPOSURE_LINE           8
#define SENSOR_ID 					0xcc41

#define SEN_I2C_ADDR 0x60 >> 1
#define SEN_I2C_COMPATIBLE "nvt,sen_sc3336"

#include "sen_i2c.c"

//=============================================================================
// function declaration
//=============================================================================
static CTL_SEN_DRV_TAB *sen_get_drv_tab_sc3336(void);
static void sen_pwr_ctrl_sc3336(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb);
static ER sen_open_sc3336(CTL_SEN_ID id);
static ER sen_close_sc3336(CTL_SEN_ID id);
static ER sen_sleep_sc3336(CTL_SEN_ID id);
static ER sen_wakeup_sc3336(CTL_SEN_ID id);
static ER sen_write_reg_sc3336(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_read_reg_sc3336(CTL_SEN_ID id, CTL_SEN_CMD *cmd);
static ER sen_chg_mode_sc3336(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj);
static ER sen_chg_fps_sc3336(CTL_SEN_ID id, UINT32 fps);
static ER sen_set_info_sc3336(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static ER sen_get_info_sc3336(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data);
static UINT32 sen_calc_chgmode_vd_sc3336(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_calc_exp_vd_sc3336(CTL_SEN_ID id, UINT32 fps);
static void sen_set_gain_sc3336(CTL_SEN_ID id, void *param);
static void sen_set_expt_sc3336(CTL_SEN_ID id, void *param);
static void sen_set_preset_sc3336(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl);
static void sen_set_flip_sc3336(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
static ER sen_get_flip_sc3336(CTL_SEN_ID id, CTL_SEN_FLIP *flip);
#if defined(__FREERTOS)
void sen_get_gain_sc3336(CTL_SEN_ID id, void *param);
void sen_get_expt_sc3336(CTL_SEN_ID id, void *param);
#else
static void sen_get_gain_sc3336(CTL_SEN_ID id, void *param);
static void sen_get_expt_sc3336(CTL_SEN_ID id, void *param);
#endif
static void sen_get_min_expt_sc3336(CTL_SEN_ID id, void *param);
static void sen_get_mode_basic_sc3336(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic);
static void sen_get_attr_basic_sc3336(CTL_SENDRV_GET_ATTR_BASIC_PARAM *data);
static void sen_get_attr_signal_sc3336(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data);
static ER sen_get_attr_cmdif_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data);
static ER sen_get_attr_if_sc3336(CTL_SENDRV_GET_ATTR_IF_PARAM *data);
static void sen_get_fps_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data);
static void sen_get_speed_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data);
static void sen_get_mode_mipi_sc3336(CTL_SENDRV_GET_MODE_MIPI_PARAM *data);
static void sen_get_modesel_sc3336(CTL_SENDRV_GET_MODESEL_PARAM *data);
static UINT32 sen_calc_rowtime_sc3336(CTL_SEN_ID id, CTL_SEN_MODE mode);
static UINT32 sen_calc_rowtime_step_sc3336(CTL_SEN_ID id, CTL_SEN_MODE mode);
static void sen_get_rowtime_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data);
static void sen_set_cur_fps_sc3336(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_cur_fps_sc3336(CTL_SEN_ID id);
static void sen_set_chgmode_fps_sc3336(CTL_SEN_ID id, UINT32 fps);
static UINT32 sen_get_chgmode_fps_sc3336(CTL_SEN_ID id);
static void sen_ext_get_probe_sen_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_PROBE_SEN_PARAM *data);
//=============================================================================
// global variable
//=============================================================================
static UINT32 sen_map = SEN_PATH_1;

static SEN_PRESET sen_preset[CTL_SEN_ID_MAX] = {
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000},
	{10000, 1000}
};

static SEN_DIRECTION sen_direction[CTL_SEN_ID_MAX] = {
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE},
	{FALSE, FALSE}
};

static SEN_POWER sen_power[CTL_SEN_ID_MAX] = {
	//C_GPIO:+0x0; P_GPIO:+0x20; S_GPIO:+0x40; L_GPIO:0x60
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1},
	{CTL_SEN_CLK_SEL_SIEMCLK, CTL_SEN_IGNORE, 0x44, 1, 1}
};

static SEN_I2C sen_i2c[CTL_SEN_ID_MAX] = {
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_1, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR},
	{SEN_I2C_ID_2, SEN_I2C_ADDR}
};

static CTL_SENDRV_GET_ATTR_BASIC_PARAM basic_param = {
	SEN_SC3336_MODULE_NAME,
	CTL_SEN_VENDOR_OTHERS,
	SEN_MAX_MODE,
	CTL_SEN_SUPPORT_PROPERTY_MIRROR|CTL_SEN_SUPPORT_PROPERTY_FLIP|CTL_SEN_SUPPORT_PROPERTY_CHGFPS,
	0
};

static CTL_SENDRV_GET_ATTR_SIGNAL_PARAM signal_param = {
	CTL_SEN_SIGNAL_MASTER,
	{CTL_SEN_ACTIVE_HIGH, CTL_SEN_ACTIVE_HIGH, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING, CTL_SEN_PHASE_RISING}
};

static CTL_SENDRV_I2C i2c = {
	{
		{CTL_SEN_I2C_W_ADDR_DFT,     0x60},
		{CTL_SEN_I2C_W_ADDR_OPTION1, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION2, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION3, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION4, 0xFF},
		{CTL_SEN_I2C_W_ADDR_OPTION5, 0xFF}
	}
};

static CTL_SENDRV_GET_SPEED_PARAM speed_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_SIEMCLK_SRC_DFT,
		27000000,
		101250000,
		50625000
	}
};

static CTL_SENDRV_GET_MODE_MIPI_PARAM mipi_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_CLKLANE_1,
		CTL_SEN_DATALANE_2,
		{ {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0}, {CTL_SEN_MIPI_MANUAL_NONE, 0} },
		0,
		{0, 0, 0, 0},
		SEN_BIT_OFS_NONE
	}
};

static CTL_SENDRV_GET_MODE_BASIC_PARAM mode_basic_param[SEN_MAX_MODE] = {
	{
		CTL_SEN_MODE_1,
		CTL_SEN_IF_TYPE_MIPI,
		CTL_SEN_DATA_FMT_RGB,
		CTL_SEN_MODE_LINEAR,
		3000,
		1,
		CTL_SEN_STPIX_R,
		CTL_SEN_PIXDEPTH_10BIT,
		CTL_SEN_FMT_POGRESSIVE,
		{2304, 1298},
		{{0, 2, 2304, 1296}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
		{2304, 1296},
		{0, 1250, 0, 1350},//HTS,VTS
		CTL_SEN_RATIO(16, 9),
		{1000, 766080},
		100
	}
};

static CTL_SEN_CMD sc3336_mode_1[] = {
    {0x0103, 1, {0x01, 0x0}},
    {0x301f, 1, {0x07, 0x0}},
    {0x36e9, 1, {0x80, 0x0}},
    {0x37f9, 1, {0x80, 0x0}},
    {0x3200, 1, {0x00, 0x0}},
    {0x3201, 1, {0x02, 0x0}},
    {0x3202, 1, {0x00, 0x0}},
    {0x3203, 1, {0x00, 0x0}},
    {0x3204, 1, {0x09, 0x0}},
    {0x3205, 1, {0x0d, 0x0}},
    {0x3206, 1, {0x05, 0x0}},
    {0x3207, 1, {0x1b, 0x0}},
    {0x3208, 1, {0x09, 0x0}},
    {0x3209, 1, {0x08, 0x0}},
    {0x320a, 1, {0x05, 0x0}},
    {0x320b, 1, {0x18, 0x0}},
    {0x3210, 1, {0x00, 0x0}},
    {0x3211, 1, {0x03, 0x0}},//start pixel R
    {0x3212, 1, {0x00, 0x0}},
    {0x3213, 1, {0x03, 0x0}},//start pixel R
    {0x3212, 1, {0x00, 0x0}},
    {0x3253, 1, {0x10, 0x0}},
    {0x325f, 1, {0x20, 0x0}},
    {0x3301, 1, {0x04, 0x0}},
    {0x3306, 1, {0x50, 0x0}},
    {0x330a, 1, {0x00, 0x0}},
    {0x330b, 1, {0xd8, 0x0}},
    {0x3333, 1, {0x10, 0x0}},
    {0x3334, 1, {0x40, 0x0}},
    {0x335e, 1, {0x06, 0x0}},
    {0x335f, 1, {0x0a, 0x0}},
    {0x3364, 1, {0x5e, 0x0}},
    {0x337c, 1, {0x02, 0x0}},
    {0x337d, 1, {0x0e, 0x0}},
    {0x3390, 1, {0x01, 0x0}},
    {0x3391, 1, {0x03, 0x0}},
    {0x3392, 1, {0x07, 0x0}},
    {0x3393, 1, {0x04, 0x0}},
    {0x3394, 1, {0x04, 0x0}},
    {0x3395, 1, {0x04, 0x0}},
    {0x3396, 1, {0x08, 0x0}},
    {0x3397, 1, {0x0b, 0x0}},
    {0x3398, 1, {0x1f, 0x0}},
    {0x3399, 1, {0x04, 0x0}},
    {0x339a, 1, {0x0a, 0x0}},
    {0x339b, 1, {0x3a, 0x0}},
    {0x339c, 1, {0xb0, 0x0}},
    {0x33a2, 1, {0x04, 0x0}},
    {0x33ac, 1, {0x08, 0x0}},
    {0x33ad, 1, {0x1c, 0x0}},
    {0x33ae, 1, {0x10, 0x0}},
    {0x33af, 1, {0x30, 0x0}},
    {0x33b1, 1, {0x80, 0x0}},
    {0x33b3, 1, {0x48, 0x0}},
    {0x33f9, 1, {0x60, 0x0}},
    {0x33fb, 1, {0x74, 0x0}},
    {0x33fc, 1, {0x4b, 0x0}},
    {0x33fd, 1, {0x5f, 0x0}},
    {0x349f, 1, {0x03, 0x0}},
    {0x34a6, 1, {0x4b, 0x0}},
    {0x34a7, 1, {0x5f, 0x0}},
    {0x34a8, 1, {0x20, 0x0}},
    {0x34a9, 1, {0x18, 0x0}},
    {0x34ab, 1, {0xe8, 0x0}},
    {0x34ad, 1, {0xf8, 0x0}},
    {0x34f8, 1, {0x5f, 0x0}},
    {0x34f9, 1, {0x18, 0x0}},
    {0x3630, 1, {0xc0, 0x0}},
    {0x3631, 1, {0x84, 0x0}},
    {0x3633, 1, {0x32, 0x0}},
    {0x363b, 1, {0x26, 0x0}},
    {0x363c, 1, {0x08, 0x0}},
    {0x3641, 1, {0x38, 0x0}},
    {0x3670, 1, {0x4e, 0x0}},
    {0x3674, 1, {0xc0, 0x0}},
    {0x3675, 1, {0xc0, 0x0}},
    {0x3676, 1, {0xc0, 0x0}},
    {0x3677, 1, {0x84, 0x0}},
    {0x3678, 1, {0x8a, 0x0}},
    {0x3679, 1, {0x8c, 0x0}},
    {0x367c, 1, {0x48, 0x0}},
    {0x367d, 1, {0x49, 0x0}},
    {0x367e, 1, {0x4b, 0x0}},
    {0x367f, 1, {0x5f, 0x0}},
    {0x3690, 1, {0x33, 0x0}},
    {0x3691, 1, {0x43, 0x0}},
    {0x3692, 1, {0x54, 0x0}},
    {0x369c, 1, {0x4b, 0x0}},
    {0x369d, 1, {0x5f, 0x0}},
    {0x36b0, 1, {0x87, 0x0}},
    {0x36b1, 1, {0x90, 0x0}},
    {0x36b2, 1, {0xa1, 0x0}},
    {0x36b3, 1, {0xc2, 0x0}},
    {0x36b4, 1, {0x49, 0x0}},
    {0x36b5, 1, {0x4b, 0x0}},
    {0x36b6, 1, {0x4f, 0x0}},
    {0x370f, 1, {0x01, 0x0}},
    {0x3722, 1, {0x09, 0x0}},
    {0x3724, 1, {0x41, 0x0}},
    {0x3725, 1, {0xc1, 0x0}},
    {0x3771, 1, {0x09, 0x0}},
    {0x3772, 1, {0x09, 0x0}},
    {0x3773, 1, {0x05, 0x0}},
    {0x377a, 1, {0x48, 0x0}},
    {0x377b, 1, {0x5f, 0x0}},
    {0x3905, 1, {0x8c, 0x0}},
    {0x391d, 1, {0x04, 0x0}},
    {0x3921, 1, {0x20, 0x0}},
    {0x3926, 1, {0x21, 0x0}},
    {0x3933, 1, {0x80, 0x0}},
    {0x3934, 1, {0x03, 0x0}},
    {0x3935, 1, {0x00, 0x0}},
    {0x3936, 1, {0x12, 0x0}},
    {0x3937, 1, {0x77, 0x0}},
    {0x3938, 1, {0x74, 0x0}},
    {0x39dc, 1, {0x02, 0x0}},
    {0x3e01, 1, {0x53, 0x0}},
    {0x3e02, 1, {0xe0, 0x0}},
    {0x3e09, 1, {0x00, 0x0}},
    {0x4509, 1, {0x20, 0x0}},
    {0x5ae0, 1, {0xfe, 0x0}},
    {0x5ae1, 1, {0x40, 0x0}},
    {0x5ae2, 1, {0x38, 0x0}},
    {0x5ae3, 1, {0x30, 0x0}},
    {0x5ae4, 1, {0x28, 0x0}},
    {0x5ae5, 1, {0x38, 0x0}},
    {0x5ae6, 1, {0x30, 0x0}},
    {0x5ae7, 1, {0x28, 0x0}},
    {0x5ae8, 1, {0x3f, 0x0}},
    {0x5ae9, 1, {0x34, 0x0}},
    {0x5aea, 1, {0x2c, 0x0}},
    {0x5aeb, 1, {0x3f, 0x0}},
    {0x5aec, 1, {0x34, 0x0}},
    {0x5aed, 1, {0x2c, 0x0}},
    //2304X1298
    {0x3208, 1, {0x09, 0x0}},
	{0x3209, 1, {0x00, 0x0}},
	{0x320a, 1, {0x05, 0x0}},
	{0x320b, 1, {0x12, 0x0}},
	{0x3210, 1, {0x00, 0x0}},
	{0x3211, 1, {0x05, 0x0}},
	{0x3212, 1, {0x00, 0x0}},
	{0x3213, 1, {0x03, 0x0}},
	/////////////
    {0x36e9, 1, {0x54, 0x0}},
    {0x37f9, 1, {0x27, 0x0}},
    {SEN_CMD_SETVD, 1, {0x00, 0x0}},
	{SEN_CMD_PRESET, 1, {0x00, 0x0}},
	{SEN_CMD_DIRECTION, 1, {0x00, 0x0}},	
    {0x0100, 1, {0x01, 0x0}},
	
};

static UINT32 cur_sen_mode[CTL_SEN_ID_MAX] = {CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1, CTL_SEN_MODE_1};
static UINT32 cur_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 chgmode_fps[CTL_SEN_ID_MAX] = {0};
static UINT32 power_ctrl_mclk[CTL_SEN_CLK_SEL_MAX] = {0};
static UINT32 reset_ctrl_count[CTL_SEN_ID_MAX] = {0};
static UINT32 pwdn_ctrl_count[CTL_SEN_ID_MAX] = {0};
static ISP_SENSOR_CTRL sensor_ctrl_last[CTL_SEN_ID_MAX] = {0};
static ISP_SENSOR_PRESET_CTRL preset_ctrl[CTL_SEN_ID_MAX] = {0};
static UINT32 compensation_ratio[CTL_SEN_ID_MAX][ISP_SEN_MFRAME_MAX_NUM] = {0};
static UINT32 first_flag[CTL_SEN_ID_MAX] = {0};
static INT32 is_fastboot[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_id[CTL_SEN_ID_MAX];
static UINT32 fastboot_i2c_addr[CTL_SEN_ID_MAX];
static BOOL i2c_valid[CTL_SEN_ID_MAX];

static CTL_SEN_DRV_TAB sc3336_sen_drv_tab = {
	sen_open_sc3336,
	sen_close_sc3336,
	sen_sleep_sc3336,
	sen_wakeup_sc3336,
	sen_write_reg_sc3336,
	sen_read_reg_sc3336,
	sen_chg_mode_sc3336,
	sen_chg_fps_sc3336,
	sen_set_info_sc3336,
	sen_get_info_sc3336,
};

static CTL_SEN_DRV_TAB *sen_get_drv_tab_sc3336(void)
{
	return &sc3336_sen_drv_tab;
}

static void sen_pwr_ctrl_sc3336(CTL_SEN_ID id, CTL_SEN_PWR_CTRL_FLAG flag, CTL_SEN_CLK_CB clk_cb)
{
	UINT32 i = 0;
	UINT32 reset_count = 0, pwdn_count = 0;	
	DBG_IND("enter flag %d \r\n", flag);

	if ((flag == CTL_SEN_PWR_CTRL_TURN_ON) && ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr))) {

		if (sen_power[id].pwdn_pin != CTL_SEN_IGNORE) {
			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( pwdn_ctrl_count[i] == (sen_power[id].pwdn_pin)) {
					pwdn_count++;
				}
			}
			pwdn_ctrl_count[id] = (sen_power[id].pwdn_pin);

			if (!pwdn_count) {
				gpio_direction_output((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 1);
			}
		}

		if (clk_cb != NULL) {
			if (sen_power[id].mclk != CTL_SEN_IGNORE) {
				if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK ) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK] += 1;
				} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK2] += 1;
				} else { //CTL_SEN_CLK_SEL_SIEMCLK3
					power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK3] += 1;
				}
				if (1 == (power_ctrl_mclk[sen_power[id].mclk])) {
					clk_cb(sen_power[id].mclk, TRUE);
				}
			}
		}

		if (sen_power[id].rst_pin != CTL_SEN_IGNORE) {
			vos_util_delay_ms(sen_power[id].stable_time);
			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( reset_ctrl_count[i] == (sen_power[id].rst_pin)) {
					reset_count++;
				}
			}
			reset_ctrl_count[id] = (sen_power[id].rst_pin);

			if (!reset_count) {
				gpio_direction_output((sen_power[id].rst_pin), 0);
				gpio_set_value((sen_power[id].rst_pin), 0);
				vos_util_delay_ms(sen_power[id].rst_time);
				gpio_set_value((sen_power[id].rst_pin), 1);
				vos_util_delay_ms(sen_power[id].stable_time);
			}
		}
	}

	if (flag == CTL_SEN_PWR_CTRL_TURN_OFF) {
		
		if (sen_power[id].pwdn_pin != CTL_SEN_IGNORE) {
			pwdn_ctrl_count[id] = 0;

			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( pwdn_ctrl_count[i] == (sen_power[id].pwdn_pin)) {
					pwdn_count++;
				}
			}

			if (!pwdn_count) {
				gpio_direction_output((sen_power[id].pwdn_pin), 0);
				gpio_set_value((sen_power[id].pwdn_pin), 0);
			}
		}
	
		if (sen_power[id].rst_pin != CTL_SEN_IGNORE) {
			reset_ctrl_count[id] = 0;

			for ( i = 0; i < CTL_SEN_ID_MAX ; i++ ) {
				if ( reset_ctrl_count[i] == (sen_power[id].rst_pin)) {
					reset_count++;
				}
			}

			if (!reset_count) {
				gpio_direction_output((sen_power[id].rst_pin), 0);
				gpio_set_value((sen_power[id].rst_pin), 0);
				vos_util_delay_ms(sen_power[id].stable_time);
			}
		}		
		
		if (clk_cb != NULL) {
			if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK ) {
				power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK] -= 1;
			} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
				power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK2] -= 1;
			} else { //CTL_SEN_CLK_SEL_SIEMCLK3
				power_ctrl_mclk[CTL_SEN_CLK_SEL_SIEMCLK3] -= 1;
			}
			if (!power_ctrl_mclk[sen_power[id].mclk]) {
				clk_cb(sen_power[id].mclk, FALSE);
			}
		}
	}
}

static CTL_SEN_CMD sen_set_cmd_info_sc3336(UINT32 addr, UINT32 data_length, UINT32 data0, UINT32 data1)
{
	CTL_SEN_CMD cmd;

	cmd.addr = addr;
	cmd.data_len = data_length;
	cmd.data[0] = data0;
	cmd.data[1] = data1;
	return cmd;
}

#if defined(__KERNEL__)
static void sen_load_cfg_from_compatible_sc3336(struct device_node *of_node)
{
	DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
	sen_common_load_cfg_preset_compatible(of_node, &sen_preset);
	sen_common_load_cfg_direction_compatible(of_node, &sen_direction);
	sen_common_load_cfg_power_compatible(of_node, &sen_power);
	sen_common_load_cfg_i2c_compatible(of_node, &sen_i2c);
}
#endif
static ER sen_open_sc3336(CTL_SEN_ID id)
{
	ER rt = E_OK;

	#if defined(__KERNEL__)
	sen_i2c_reg_cb(sen_load_cfg_from_compatible_sc3336);
	#endif

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_DEFAULT;
	i2c_valid[id] = TRUE;
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		rt = sen_i2c_init_driver(id, &sen_i2c[id]);

		if (rt != E_OK) {
			i2c_valid[id] = FALSE;

			DBG_ERR("init. i2c driver fail (%d) \r\n", id);
		}
	}

	return rt;
}

static ER sen_close_sc3336(CTL_SEN_ID id)
{
	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		if (i2c_valid[id]) {
			sen_i2c_remove_driver(id);
		}
	} else {
		is_fastboot[id] = 0;
		#if defined(__KERNEL__)
		isp_builtin_uninit_i2c(id);
		#endif
	}

	i2c_valid[id] = FALSE;

	return E_OK;
}

static ER sen_sleep_sc3336(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_wakeup_sc3336(CTL_SEN_ID id)
{
	DBG_IND("enter \r\n");
	return E_OK;
}

static ER sen_write_reg_sc3336(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg msgs;
	unsigned char buf[3];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}

	buf[0]     = (cmd->addr >> 8) & 0xFF;
	buf[1]     = cmd->addr & 0xFF;
	buf[2]     = cmd->data[0] & 0xFF;
	msgs.addr  = sen_i2c[id].addr;
	msgs.flags = 0;
	msgs.len   = 3;
	msgs.buf   = buf;

	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		i = 0;
		while(1){
			if (sen_i2c_transfer(id, &msgs, 1) == 0)
				break;
			i++;
			if (i == 5)
				return E_SYS;
		}
	} else {
		#if defined(__KERNEL__)
		isp_builtin_set_transfer_i2c(id, &msgs, 1);
		#endif
	}

	return E_OK;
}

static ER sen_read_reg_sc3336(CTL_SEN_ID id, CTL_SEN_CMD *cmd)
{
	struct i2c_msg  msgs[2];
	unsigned char   tmp[2], tmp2[2];
	int i;

	if (!i2c_valid[id]) {
		return E_NOSPT;
	}

	tmp[0]        = (cmd->addr >> 8) & 0xFF;
	tmp[1]        = cmd->addr & 0xFF;
	msgs[0].addr  = sen_i2c[id].addr;
	msgs[0].flags = 0;
	msgs[0].len   = 2;
	msgs[0].buf   = tmp;

	tmp2[0]       = 0;
	msgs[1].addr  = sen_i2c[id].addr;
	msgs[1].flags = 1;
	msgs[1].len   = 1;
	msgs[1].buf   = tmp2;

	if ((!is_fastboot[id]) || (fastboot_i2c_id[id] != sen_i2c[id].id) || (fastboot_i2c_addr[id] != sen_i2c[id].addr)) {
		i = 0;
		while(1){
			if (sen_i2c_transfer(id, msgs, 2) == 0)
				break;
			i++;
			if (i == 5)
				return E_SYS;
		}
	} else {
		#if defined(__KERNEL__)
		isp_builtin_set_transfer_i2c(id, msgs, 2);
		#endif
	}

	cmd->data[0] = tmp2[0];

	return E_OK;
}

static UINT32 sen_get_cmd_tab_sc3336(CTL_SEN_MODE mode, CTL_SEN_CMD **cmd_tab)
{
	switch (mode) {
	case CTL_SEN_MODE_1:
		*cmd_tab = sc3336_mode_1;
		return sizeof(sc3336_mode_1) / sizeof(CTL_SEN_CMD);

	default:
		DBG_ERR("sensor mode %d no cmd table\r\n", mode);
		*cmd_tab = NULL;
		return 0;
	}
}

static ER sen_chg_mode_sc3336(CTL_SEN_ID id, CTL_SENDRV_CHGMODE_OBJ chgmode_obj)
{
	ISP_SENSOR_CTRL sensor_ctrl = {0};
	CTL_SEN_CMD *p_cmd_list = NULL, cmd;
	CTL_SEN_FLIP flip = CTL_SEN_FLIP_NONE;
	UINT32 sensor_vd;
	UINT32 idx, cmd_num = 0;
	ER rt = E_OK;

	cur_sen_mode[id] = chgmode_obj.mode;

	if (is_fastboot[id]) {
		#if defined(__KERNEL__)
		ISP_BUILTIN_SENSOR_CTRL *p_sensor_ctrl_temp;

		p_sensor_ctrl_temp = isp_builtin_get_sensor_gain(id);
		sensor_ctrl.gain_ratio[0] = p_sensor_ctrl_temp->gain_ratio[0];
		sensor_ctrl.gain_ratio[1] = p_sensor_ctrl_temp->gain_ratio[1];
		p_sensor_ctrl_temp = isp_builtin_get_sensor_expt(id);
		sensor_ctrl.exp_time[0] = p_sensor_ctrl_temp->exp_time[0];
		sensor_ctrl.exp_time[1] = p_sensor_ctrl_temp->exp_time[1];
		sen_set_chgmode_fps_sc3336(id, isp_builtin_get_chgmode_fps(id));
		sen_set_cur_fps_sc3336(id, isp_builtin_get_chgmode_fps(id));
		sen_set_gain_sc3336(id, &sensor_ctrl);
		sen_set_expt_sc3336(id, &sensor_ctrl);
		#endif
		preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;

		return E_OK;
	}

	// get & set sensor cmd table
	cmd_num = sen_get_cmd_tab_sc3336(chgmode_obj.mode, &p_cmd_list);
	if (p_cmd_list == NULL) {
		DBG_ERR("%s: SenMode(%d) out of range!!! \r\n", __func__, chgmode_obj.mode);
		return E_SYS;
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_sc3336(id, chgmode_obj.frame_rate);
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	first_flag[id] = 0;

	for (idx = 0; idx < cmd_num; idx++) {
		if (p_cmd_list[idx].addr == SEN_CMD_DELAY) {
			vos_util_delay_ms((p_cmd_list[idx].data[0] & 0xFF) | ((p_cmd_list[idx].data[1] & 0xFF) << 8));
		} else if (p_cmd_list[idx].addr == SEN_CMD_SETVD) {
			cmd = sen_set_cmd_info_sc3336(0x320E, 1, (sensor_vd >> 8)& 0x7F, 0x0);
			rt |= sen_write_reg_sc3336(id, &cmd);
			cmd = sen_set_cmd_info_sc3336(0x320F, 1, sensor_vd & 0xFF, 0x0);
			rt |= sen_write_reg_sc3336(id, &cmd);
		} else if (p_cmd_list[idx].addr == SEN_CMD_PRESET) {
			switch (preset_ctrl[id].mode) {
				default:
				case ISP_SENSOR_PRESET_DEFAULT:
					sensor_ctrl.gain_ratio[0] = sen_preset[id].gain_ratio;
					sensor_ctrl.exp_time[0] = sen_preset[id].expt_time;
					if (mode_basic_param[cur_sen_mode[id]].frame_num == 2) {
						sensor_ctrl.exp_time[1] = sen_preset[id].expt_time >> 3;
					}
					break;

				case ISP_SENSOR_PRESET_CHGMODE:
					memcpy(&sensor_ctrl, &sensor_ctrl_last[id], sizeof(ISP_SENSOR_CTRL));
					break;

				case ISP_SENSOR_PRESET_AE:
					sensor_ctrl.exp_time[0] = preset_ctrl[id].exp_time[0];
					sensor_ctrl.exp_time[1] = preset_ctrl[id].exp_time[1];
					sensor_ctrl.gain_ratio[0] = preset_ctrl[id].gain_ratio[0];
					sensor_ctrl.gain_ratio[1] = preset_ctrl[id].gain_ratio[1];
				break;
			}

			sen_set_gain_sc3336(id, &sensor_ctrl);
			sen_set_expt_sc3336(id, &sensor_ctrl);
		} else if (p_cmd_list[idx].addr == SEN_CMD_DIRECTION) {
			if (sen_direction[id].mirror) {
				flip |= CTL_SEN_FLIP_H;
			}
			if (sen_direction[id].flip) {
				flip |= CTL_SEN_FLIP_V;
			}
			sen_set_flip_sc3336(id, &flip);
		} else {
			cmd = sen_set_cmd_info_sc3336(p_cmd_list[idx].addr, p_cmd_list[idx].data_len, p_cmd_list[idx].data[0], p_cmd_list[idx].data[1]);
			rt |= sen_write_reg_sc3336(id, &cmd);
			//DBG_ERR("write addr=0x%x, data0=0x%x, data1=0x%x\r\n", p_cmd_list[idx].addr, p_cmd_list[idx].data[0], p_cmd_list[idx].data[1]);
		}
	}

	preset_ctrl[id].mode = ISP_SENSOR_PRESET_CHGMODE;
	first_flag[id] = 1;

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
		return rt;
	}

	return E_OK;
}

static void sen_ext_get_probe_sen_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_PROBE_SEN_PARAM *data)
{

    CTL_SEN_CMD cmd;
    UINT32 read_data_h = 0,read_data_l = 0,sensor_id = 0;
    ER rt = E_OK;

    cmd = sen_set_cmd_info_sc3336(0x3107, 1,0x0, 0x0);
    rt |= sen_read_reg_sc3336(id, &cmd);
    read_data_h = cmd.data[0];
    if( rt != E_OK) {
        data->probe_rst = rt;
        return;
    }

    cmd = sen_set_cmd_info_sc3336(0x3108, 1, 0x0, 0x0);
    rt |= sen_read_reg_sc3336(id, &cmd);
    read_data_l = cmd.data[0];

    if( rt != E_OK) {
        data->probe_rst = rt;
        return;
    }

    sensor_id = ((read_data_h & 0xFF) << 8) | (read_data_l & 0xFF);

    printk("sensor id = %x\r\n",sensor_id);
    if( rt != E_OK) {
        data->probe_rst = rt;
    }

    else if (SENSOR_ID == sensor_id) {
        data->probe_rst = 0;
    } else if (SENSOR_ID != sensor_id) {
        data->probe_rst = -1;
    }
}

static ER sen_chg_fps_sc3336(CTL_SEN_ID id, UINT32 fps)
{
	CTL_SEN_CMD cmd;
	UINT32 sensor_vd;
	ER rt = E_OK;

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_CHGFPS) {
		sensor_vd = sen_calc_chgmode_vd_sc3336(id, fps);
	} else {
		DBG_WRN(" not support fps adjust \r\n");
		sen_set_cur_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_chgmode_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

    cmd = sen_set_cmd_info_sc3336(0x320E, 1, (sensor_vd >> 8) & 0x7F, 0x0);
    rt |= sen_write_reg_sc3336(id, &cmd);
    cmd = sen_set_cmd_info_sc3336(0x320F, 1, sensor_vd & 0xFF, 0x0);
    rt |= sen_write_reg_sc3336(id, &cmd);
	return rt;
}

static ER sen_set_info_sc3336(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_SET_EXPT:
		sen_set_expt_sc3336(id, data);
		break;
	case CTL_SENDRV_CFGID_SET_GAIN:
		sen_set_gain_sc3336(id, data);
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		sen_set_flip_sc3336(id, (CTL_SEN_FLIP *)(data));
		break;
	case CTL_SENDRV_CFGID_USER_DEFINE1:
		sen_set_preset_sc3336(id, (ISP_SENSOR_PRESET_CTRL *)(data));
		break;
	default:
		return E_NOSPT;
	}
	return E_OK;
}

static ER sen_get_info_sc3336(CTL_SEN_ID id, CTL_SENDRV_CFGID drv_cfg_id, void *data)
{
	ER rt = E_OK;

	switch (drv_cfg_id) {
	case CTL_SENDRV_CFGID_GET_EXPT:
		sen_get_expt_sc3336(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_GAIN:
		sen_get_gain_sc3336(id, data);
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_BASIC:
		sen_get_attr_basic_sc3336((CTL_SENDRV_GET_ATTR_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_SIGNAL:
		sen_get_attr_signal_sc3336((CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_CMDIF:
		rt = sen_get_attr_cmdif_sc3336(id, (CTL_SENDRV_GET_ATTR_CMDIF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_ATTR_IF:
		rt = sen_get_attr_if_sc3336((CTL_SENDRV_GET_ATTR_IF_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_FPS:
		sen_get_fps_sc3336(id, (CTL_SENDRV_GET_FPS_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_SPEED:
		sen_get_speed_sc3336(id, (CTL_SENDRV_GET_SPEED_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_BASIC:
		sen_get_mode_basic_sc3336((CTL_SENDRV_GET_MODE_BASIC_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_MIPI:
		sen_get_mode_mipi_sc3336((CTL_SENDRV_GET_MODE_MIPI_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODESEL:
		sen_get_modesel_sc3336((CTL_SENDRV_GET_MODESEL_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_GET_MODE_ROWTIME:
		sen_get_rowtime_sc3336(id, (CTL_SENDRV_GET_MODE_ROWTIME_PARAM *)(data));
		break;
	case CTL_SENDRV_CFGID_FLIP_TYPE:
		rt = sen_get_flip_sc3336(id, (CTL_SEN_FLIP *)(data));
		break;
    case CTL_SENDRV_CFGID_GET_PROBE_SEN:
        sen_ext_get_probe_sen_sc3336(id, (CTL_SENDRV_GET_PROBE_SEN_PARAM *)(data));
        break;
	case CTL_SENDRV_CFGID_USER_DEFINE2:
		sen_get_min_expt_sc3336(id, data);
		break;
	default:
		rt = E_NOSPT;
	}
	return rt;
}

static UINT32 sen_calc_chgmode_vd_sc3336(CTL_SEN_ID id, UINT32 fps)
{
	UINT32 sensor_vd;

	if (1 > fps) {
		DBG_ERR("sensor fps can not small than (%d),change to dft sensor fps (%d) \r\n", fps, mode_basic_param[cur_sen_mode[id]].dft_fps);
		fps = mode_basic_param[cur_sen_mode[id]].dft_fps;
	}
	sensor_vd = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / fps;

	sen_set_chgmode_fps_sc3336(id, fps);
	sen_set_cur_fps_sc3336(id, fps);

	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = MAX_VD_PERIOD;
		fps = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / sensor_vd;
		sen_set_chgmode_fps_sc3336(id, fps);
		sen_set_cur_fps_sc3336(id, fps);
	}

	if(sensor_vd < (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period)) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
		sen_set_chgmode_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
		sen_set_cur_fps_sc3336(id, mode_basic_param[cur_sen_mode[id]].dft_fps);
	}

	return sensor_vd;
}

static UINT32 sen_calc_exp_vd_sc3336(CTL_SEN_ID id, UINT32 fps)
{
	UINT32 sensor_vd;

	if (1 > fps) {
		DBG_ERR("sensor fps can not small than (%d),change to dft sensor fps (%d) \r\n", fps, mode_basic_param[cur_sen_mode[id]].dft_fps);
		fps = mode_basic_param[cur_sen_mode[id]].dft_fps;
	}
	sensor_vd = (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period) * (mode_basic_param[cur_sen_mode[id]].dft_fps) / fps;

	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = MAX_VD_PERIOD;

	}

	if(sensor_vd < (mode_basic_param[cur_sen_mode[id]].signal_info.vd_period)) {
		DBG_ERR("sensor vd out of sensor driver range (%d) \r\n", sensor_vd);
		sensor_vd = mode_basic_param[cur_sen_mode[id]].signal_info.vd_period;
	}

	return sensor_vd;
}

static void sen_set_gain_sc3336(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	UINT32 data1[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data2[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 data3[ISP_SEN_MFRAME_MAX_NUM] = {0};
	UINT32 frame_cnt, total_frame;
	CTL_SEN_CMD cmd;
	UINT32 a_gain = 0, d_gain_h = 0, d_gain_l = 0, temp_dgain_h = 0;
	ER rt = E_OK;

	sensor_ctrl_last[id].gain_ratio[0] = sensor_ctrl->gain_ratio[0];
	sensor_ctrl_last[id].gain_ratio[1] = sensor_ctrl->gain_ratio[1];

	// Calculate sensor gain
	if (mode_basic_param[cur_sen_mode[id]].frame_num == 0) {
		DBG_WRN("total_frame = 0, force to 1 \r\n");
		total_frame = 1;
	} else {
		total_frame = mode_basic_param[cur_sen_mode[id]].frame_num;
	}

	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		if (100 <= (compensation_ratio[id][frame_cnt])) {
			sensor_ctrl->gain_ratio[frame_cnt] = (sensor_ctrl->gain_ratio[frame_cnt]) * (compensation_ratio[id][frame_cnt]) / 100;
		}		
		if (sensor_ctrl->gain_ratio[frame_cnt] < (mode_basic_param[cur_sen_mode[id]].gain.min)) {
			sensor_ctrl->gain_ratio[frame_cnt] = (mode_basic_param[cur_sen_mode[id]].gain.min);
		} else if (sensor_ctrl->gain_ratio[frame_cnt] > (mode_basic_param[cur_sen_mode[id]].gain.max)) {
			sensor_ctrl->gain_ratio[frame_cnt] = (mode_basic_param[cur_sen_mode[id]].gain.max);
		}
	

		if (1520 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x00;
			d_gain_h = 0;
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 1000;
		} else if (3040 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x40;
			d_gain_h = 0;			
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 1520;			
		} else if (6080 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x48;
			d_gain_h = 0;			
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 3040;			
		} else if (12160 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x49;
			d_gain_h = 0;			
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 6080;			
		} else if (24320 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x4B;
			d_gain_h = 0;			
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 12160;			
		} else if (48640 > (sensor_ctrl->gain_ratio[frame_cnt])) {
			a_gain = 0x4F;
			d_gain_h = 0;			
			d_gain_l = (sensor_ctrl->gain_ratio[frame_cnt]) * 128 / 24320;				
		} else {
			a_gain = 0x5F;
			temp_dgain_h = (sensor_ctrl->gain_ratio[frame_cnt]) * 100 / 4864;
			if (2000 > temp_dgain_h) {
				d_gain_h = 0;
				d_gain_l = temp_dgain_h * 128 / 1000;				
			} else if (4000 > temp_dgain_h){
				d_gain_h = 1;
				d_gain_l = temp_dgain_h * 128 / 2 / 1000;					
			} else if (8000 > temp_dgain_h){
				d_gain_h = 3;
				d_gain_l = temp_dgain_h * 128 / 4 / 1000;						
			} else {
				d_gain_h = 7;
				d_gain_l = temp_dgain_h * 128 / 8 / 1000;									
			}
		}
		
		if (((d_gain_l - 128) % 4) != 0) {
			d_gain_l = (d_gain_l - 128) / 4 * 4 + 128;
		}

		if (0xFC < d_gain_l) {
			d_gain_l = 0xFC;
		} else if (0x80 > d_gain_l) {
			d_gain_l = 0x80;
		}

		data1[frame_cnt] = a_gain;
		data2[frame_cnt] = d_gain_h;
		data3[frame_cnt] = d_gain_l;
	}

	//Set analog coarse gain
	cmd = sen_set_cmd_info_sc3336(0x3E09, 1, data1[0] & 0xFF, 0x0);
	rt |= sen_write_reg_sc3336(id, &cmd);
	//Set digital gain
	cmd = sen_set_cmd_info_sc3336(0x3E06, 1, data2[0] & 0xFF, 0x0);
	rt |= sen_write_reg_sc3336(id, &cmd);
	//Set digiral fine gain
	cmd = sen_set_cmd_info_sc3336(0x3E07, 1, data3[0] & 0xFF, 0x0);
	rt |= sen_write_reg_sc3336(id, &cmd);

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}

}

static void sen_set_expt_sc3336(CTL_SEN_ID id, void *param)
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;
	UINT32 line[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 frame_cnt, total_frame;
	CTL_SEN_CMD cmd;
	UINT32 expt_time = 0, sensor_vd = 0, chgmode_fps = 0, cur_fps = 0, clac_fps = 0, t_row = 0;
	UINT32 temp_line[ISP_SEN_MFRAME_MAX_NUM] = {0};
	ER rt = E_OK;

	sensor_ctrl_last[id].exp_time[0] = sensor_ctrl->exp_time[0];
	sensor_ctrl_last[id].exp_time[1] = sensor_ctrl->exp_time[1];

	if (mode_basic_param[cur_sen_mode[id]].frame_num == 0) {
		DBG_WRN("total_frame = 0, force to 1 \r\n");
		total_frame = 1;
	} else {
		total_frame = mode_basic_param[cur_sen_mode[id]].frame_num;
	}

	// Calculate exposure line
	for (frame_cnt = 0; frame_cnt < total_frame; frame_cnt++) {
		// Calculates the exposure setting
		t_row = sen_calc_rowtime_sc3336(id, cur_sen_mode[id]);
		if (0 == t_row) {
			DBG_WRN("t_row  = 0, must >= 1 \r\n");
			t_row = 1;
		}
		line[frame_cnt] = (sensor_ctrl->exp_time[frame_cnt]) * 10 / t_row;
		
		// Limit minimun exposure line
		if (line[frame_cnt] < MIN_EXPOSURE_LINE) {
			line[frame_cnt] = MIN_EXPOSURE_LINE;
		}
	}

	// Write exposure line
	// Get fps
	chgmode_fps = sen_get_chgmode_fps_sc3336(id);

	// Calculate exposure time
	t_row = sen_calc_rowtime_sc3336(id, cur_sen_mode[id]);
	expt_time = (line[0]) * t_row / 10;		
	temp_line[0] = line[0];

	// Calculate fps
	if (0 == expt_time) {
		DBG_WRN("expt_time  = 0, must >= 1 \r\n");		
		expt_time = 1;
	}
	clac_fps = 100000000 / expt_time;

	cur_fps = (clac_fps < chgmode_fps) ? clac_fps : chgmode_fps;
	sen_set_cur_fps_sc3336(id, cur_fps);

	// Calculate new vd
	sensor_vd = sen_calc_exp_vd_sc3336(id, cur_fps);

	//Check max vts
	if (sensor_vd > MAX_VD_PERIOD) {
		DBG_ERR("max vts overflow\r\n");
		sensor_vd = MAX_VD_PERIOD;
	}

	// Set vts to sensor
	cmd = sen_set_cmd_info_sc3336(0x320E, 1,( sensor_vd >> 8 ) & 0xFF, 0);
	rt |= sen_write_reg_sc3336(id, &cmd);
	cmd = sen_set_cmd_info_sc3336(0x320F, 1, sensor_vd & 0xFF, 0);
	rt |= sen_write_reg_sc3336(id, &cmd);


	//Check max exp line reg
	if (line[0] > (MAX_EXPOSURE_LINE)) {
		DBG_ERR("max line overflow \r\n");
		line[0] = MAX_EXPOSURE_LINE;
	}
	
	//Check max exp time
	if ((line[0]) > (sensor_vd - NON_EXPOSURE_LINE)) {
		line[0] = sensor_vd - NON_EXPOSURE_LINE;
	}		
	compensation_ratio[id][0] = 100 * temp_line[0] / line[0];

	//Set exposure line to sensor
	cmd = sen_set_cmd_info_sc3336(0x3E00, 1, ( line[0] >> 12 ) & 0x0F, 0);
	rt |= sen_write_reg_sc3336(id, &cmd);
	cmd = sen_set_cmd_info_sc3336(0x3E01, 1, ( line[0] >> 4 ) & 0xFF, 0);
	rt |= sen_write_reg_sc3336(id, &cmd);
	cmd = sen_set_cmd_info_sc3336(0x3E02, 1, ( line[0] & 0x0F ) << 4, 0);
	rt |= sen_write_reg_sc3336(id, &cmd);

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static void sen_set_preset_sc3336(CTL_SEN_ID id, ISP_SENSOR_PRESET_CTRL *ctrl)
{
	memcpy(&preset_ctrl[id], ctrl, sizeof(ISP_SENSOR_PRESET_CTRL));
}

static void sen_set_flip_sc3336(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	CTL_SEN_CMD cmd;
	ER rt = E_OK;

	cmd = sen_set_cmd_info_sc3336(0x3221, 1, 0x0, 0x0);
	rt |= sen_read_reg_sc3336(id, &cmd);
  
	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		if (*flip & CTL_SEN_FLIP_H) {
			cmd.data[0] |= 0x06;
		} else {
			cmd.data[0] &= (~0x06);
		}
	} else {
		DBG_WRN("no support mirror \r\n");
	}
	
	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (*flip & CTL_SEN_FLIP_V) {
			cmd.data[0] |= 0x60;
		} else {
			cmd.data[0] &= (~0x60);
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}

	rt |= sen_write_reg_sc3336(id, &cmd);
	
	if(cmd.data[0] == 0x66){		
		cmd = sen_set_cmd_info_sc3336(0x3211, 1, 0x09, 0);
		rt |= sen_write_reg_sc3336(id, &cmd);
	}else if(cmd.data[0] == 0x00){
		cmd = sen_set_cmd_info_sc3336(0x3211, 1, 0x05, 0);
		rt |= sen_write_reg_sc3336(id, &cmd);
	}else if(cmd.data[0] == 0x06){
		cmd = sen_set_cmd_info_sc3336(0x3211, 1, 0x09, 0);
		rt |= sen_write_reg_sc3336(id, &cmd);
	}else if(cmd.data[0] == 0x60){
		cmd = sen_set_cmd_info_sc3336(0x3211, 1, 0x05, 0);
		rt |= sen_write_reg_sc3336(id, &cmd);
	}

	if (rt != E_OK) {
		DBG_ERR("write register error %d \r\n", (INT)rt);
	}
}

static ER sen_get_flip_sc3336(CTL_SEN_ID id, CTL_SEN_FLIP *flip)
{
	CTL_SEN_CMD cmd;
	ER rt = E_OK;

	cmd = sen_set_cmd_info_sc3336(0x3221, 1, 0x0, 0x0);
	rt |= sen_read_reg_sc3336(id, &cmd);

	*flip = CTL_SEN_FLIP_NONE;
	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_MIRROR) {
		if (cmd.data[0] & 0x6) {
			*flip |= CTL_SEN_FLIP_H;
		}
	} else {
		DBG_WRN("no support mirror \r\n");
	}

	if (basic_param.property & CTL_SEN_SUPPORT_PROPERTY_FLIP) {
		if (cmd.data[0] & 0x60) {
			*flip |= CTL_SEN_FLIP_V;
		}
	} else {
		DBG_WRN("no support flip \r\n");
	}

	return rt;
}

#if defined(__FREERTOS)
void sen_get_gain_sc3336(CTL_SEN_ID id, void *param)
#else
static void sen_get_gain_sc3336(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->gain_ratio[0] = sensor_ctrl_last[id].gain_ratio[0];
	sensor_ctrl->gain_ratio[1] = sensor_ctrl_last[id].gain_ratio[1];
}

#if defined(__FREERTOS)
void sen_get_expt_sc3336(CTL_SEN_ID id, void *param)
#else
static void sen_get_expt_sc3336(CTL_SEN_ID id, void *param)
#endif
{
	ISP_SENSOR_CTRL *sensor_ctrl = (ISP_SENSOR_CTRL *)param;

	sensor_ctrl->exp_time[0] = sensor_ctrl_last[id].exp_time[0];
	sensor_ctrl->exp_time[1] = sensor_ctrl_last[id].exp_time[1];
}

static void sen_get_min_expt_sc3336(CTL_SEN_ID id, void *param)
{
	UINT32 *min_exp_time = (UINT32 *)param;
	UINT32 t_row;

	t_row = sen_calc_rowtime_sc3336(id, cur_sen_mode[id]);
	*min_exp_time = t_row * MIN_EXPOSURE_LINE / 10 + 1; // if linear and shdr mode is different,the value need to separate.

}

static void sen_get_mode_basic_sc3336(CTL_SENDRV_GET_MODE_BASIC_PARAM *mode_basic)
{
	UINT32 mode = mode_basic->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(mode_basic, &mode_basic_param[mode], sizeof(CTL_SENDRV_GET_MODE_BASIC_PARAM));
}

static void sen_get_attr_basic_sc3336(CTL_SENDRV_GET_ATTR_BASIC_PARAM *data)
{
	memcpy(data, &basic_param, sizeof(CTL_SENDRV_GET_ATTR_BASIC_PARAM));
}

static void sen_get_attr_signal_sc3336(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM *data)
{
	memcpy(data, &signal_param, sizeof(CTL_SENDRV_GET_ATTR_SIGNAL_PARAM));
}

static ER sen_get_attr_cmdif_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_ATTR_CMDIF_PARAM *data)
{
	data->type = CTL_SEN_CMDIF_TYPE_I2C;
	memcpy(&data->info, &i2c, sizeof(CTL_SENDRV_I2C));
	data->info.i2c.ch = sen_i2c[id].id;
	data->info.i2c.w_addr_info[0].w_addr = sen_i2c[id].addr;
	data->info.i2c.cur_w_addr_info.w_addr_sel = data->info.i2c.w_addr_info[0].w_addr_sel;
	data->info.i2c.cur_w_addr_info.w_addr = data->info.i2c.w_addr_info[0].w_addr;
	return E_OK;
}

static ER sen_get_attr_if_sc3336(CTL_SENDRV_GET_ATTR_IF_PARAM *data)
{
	#if 1
	if (data->type == CTL_SEN_IF_TYPE_MIPI) {
		return E_OK;
	}
	return E_NOSPT;
	#else
	if (data->type == CTL_SEN_IF_TYPE_MIPI) {
		memcpy(&data->info.mipi, &mipi, sizeof(CTL_SENDRV_MIPI));
		return E_OK;
	}
	return E_NOSPT;
	#endif
}

static void sen_get_fps_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_FPS_PARAM *data)
{
	data->cur_fps = sen_get_cur_fps_sc3336(id);
	data->chg_fps = sen_get_chgmode_fps_sc3336(id);
}

static void sen_get_speed_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_SPEED_PARAM *data)
{
	UINT32 mode = data->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(data, &speed_param[mode], sizeof(CTL_SENDRV_GET_SPEED_PARAM));

	if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK ) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK;
	} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK2) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK2;
	} else if (sen_power[id].mclk == CTL_SEN_CLK_SEL_SIEMCLK3) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_MCLK3;
	} else if (sen_power[id].mclk == CTL_SEN_IGNORE) {
		data->mclk_src = CTL_SEN_SIEMCLK_SRC_IGNORE;
	} else {
		DBG_ERR("mclk source is fail \r\n");
	}
}

static void sen_get_mode_mipi_sc3336(CTL_SENDRV_GET_MODE_MIPI_PARAM *data)
{
	UINT32 mode = data->mode;

	if (mode >= SEN_MAX_MODE) {
		mode = 0;
	}
	memcpy(data, &mipi_param[mode], sizeof(CTL_SENDRV_GET_MODE_MIPI_PARAM));
}

static void sen_get_modesel_sc3336(CTL_SENDRV_GET_MODESEL_PARAM *data)
{
	if (data->if_type != CTL_SEN_IF_TYPE_MIPI) {
		DBG_ERR("if_type %d N.S. \r\n", data->if_type);
		return;
	}

	if (data->data_fmt != CTL_SEN_DATA_FMT_RGB) {
		DBG_ERR("data_fmt %d N.S. \r\n", data->data_fmt);
		return;
	}

	if (data->frame_num == 1) {
		if ((data->size.w <= 2312) && (data->size.h <= 1304)) {
			if (data->frame_rate <= 3000) {
				data->mode = CTL_SEN_MODE_1;
				//DBG_DUMP("data->mode=%d\r\n", data->mode);  
				return;
			}
		}
	}

	DBG_ERR("fail (frame_rate%d,size%d*%d,if_type%d,data_fmt%d,frame_num%d) \r\n"
			, data->frame_rate, data->size.w, data->size.h, data->if_type, data->data_fmt, data->frame_num);
	data->mode = CTL_SEN_MODE_1;
}

static UINT32 sen_calc_rowtime_step_sc3336(CTL_SEN_ID id, CTL_SEN_MODE mode)
{
	UINT32 div_step = 0;

	if (mode >= SEN_MAX_MODE) {
		mode = cur_sen_mode[id];
	}

	if (mode_basic_param[mode].mode_type == CTL_SEN_MODE_LINEAR) {
		div_step = 1;
	}

	return div_step;
}

static UINT32 sen_calc_rowtime_sc3336(CTL_SEN_ID id, CTL_SEN_MODE mode)
{
	UINT32 row_time = 0;

	if (mode >= SEN_MAX_MODE) {
		mode = cur_sen_mode[id];
	}

	//Precision * 10
	row_time = 1000 * 2 *(mode_basic_param[mode].signal_info.hd_period) / ((speed_param[mode].pclk) / 10000);
	
	//DBG_DUMP("row_time=%d\n", row_time);  

	return row_time;
}

static void sen_get_rowtime_sc3336(CTL_SEN_ID id, CTL_SENDRV_GET_MODE_ROWTIME_PARAM *data)
{
	data->row_time_step = sen_calc_rowtime_step_sc3336(id, data->mode);
	data->row_time = sen_calc_rowtime_sc3336(id, data->mode) * (data->row_time_step);
}

static void sen_set_cur_fps_sc3336(CTL_SEN_ID id, UINT32 fps)
{
	cur_fps[id] = fps;
}

static UINT32 sen_get_cur_fps_sc3336(CTL_SEN_ID id)
{
	return cur_fps[id];
}

static void sen_set_chgmode_fps_sc3336(CTL_SEN_ID id, UINT32 fps)
{
	chgmode_fps[id] = fps;
}

static UINT32 sen_get_chgmode_fps_sc3336(CTL_SEN_ID id)
{
	return chgmode_fps[id];
}

#if defined(__FREERTOS)
void sen_get_i2c_id_sc3336(CTL_SEN_ID id, UINT32 *i2c_id)
{
	*i2c_id = sen_i2c[id].id;
}

void sen_get_i2c_addr_sc3336(CTL_SEN_ID id, UINT32 *i2c_addr)
{
	*i2c_addr = sen_i2c[id].addr;
}
int sen_init_sc3336(SENSOR_DTSI_INFO *info)
{
	CTL_SEN_REG_OBJ reg_obj;
	CHAR node_path[64];
	CHAR compatible[64];
	UINT32 id;
	ER rt = E_OK;

	for (id = 0; id < CTL_SEN_ID_MAX; id++ ) {
		is_fastboot[id] = 0;
		fastboot_i2c_id[id] = 0xFFFFFFFF;
		fastboot_i2c_addr[id] = 0x0;
	}

	sprintf(compatible, "nvt,sen_sc3336");
	if (sen_common_check_compatible(compatible)) {
		DBG_DUMP("compatible valid, using peri-dev.dtsi \r\n");
		sen_common_load_cfg_preset_compatible(compatible, &sen_preset);
		sen_common_load_cfg_direction_compatible(compatible, &sen_direction);
		sen_common_load_cfg_power_compatible(compatible, &sen_power);
		sen_common_load_cfg_i2c_compatible(compatible, &sen_i2c);
	} else if (info->addr != NULL) {
		DBG_DUMP("compatible not valid, using sensor.dtsi \r\n");
		sprintf(node_path, "/sensor/sen_cfg/sen_sc3336");
		sen_common_load_cfg_map(info->addr, node_path, &sen_map);
		sen_common_load_cfg_preset(info->addr, node_path, &sen_preset);
		sen_common_load_cfg_direction(info->addr, node_path, &sen_direction);
		sen_common_load_cfg_power(info->addr, node_path, &sen_power);
		sen_common_load_cfg_i2c(info->addr, node_path, &sen_i2c);
	} else {
		DBG_WRN("DTSI addr is NULL \r\n");
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_sc3336;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_sc3336();
	rt = ctl_sen_reg_sendrv("nvt_sen_sc3336", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

void sen_exit_sc3336(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_sc3336");
}

#else
static int __init sen_init_sc3336(void)
{
	INT8 cfg_path[MAX_PATH_NAME_LENGTH+1] = { '\0' };
	CFG_FILE_FMT *pcfg_file;
	CTL_SEN_REG_OBJ reg_obj;
	UINT32 id;
	ER rt = E_OK;

	for (id = 0; id < ISP_BUILTIN_ID_MAX_NUM; id++ ) {
		is_fastboot[id] = kdrv_builtin_is_fastboot();
		fastboot_i2c_id[id] = isp_builtin_get_i2c_id(id);
		fastboot_i2c_addr[id] = isp_builtin_get_i2c_addr(id);
	}

	// Parsing cfc file if exist
	if ((strstr(sen_cfg_path, "null")) || (strstr(sen_cfg_path, "NULL"))) {
		DBG_WRN("cfg file no exist \r\n");
		cfg_path[0] = '\0';
	} else {
		if ((sen_cfg_path != NULL) && (strlen(sen_cfg_path) <= MAX_PATH_NAME_LENGTH)) {
			strncpy((char *)cfg_path, sen_cfg_path, MAX_PATH_NAME_LENGTH);
		}

		if ((pcfg_file = sen_common_open_cfg(cfg_path)) != NULL) {
			DBG_MSG("load %s success \r\n", sen_cfg_path);
			sen_common_load_cfg_map(pcfg_file, &sen_map);
			sen_common_load_cfg_preset(pcfg_file, &sen_preset);
			sen_common_load_cfg_direction(pcfg_file, &sen_direction);
			sen_common_load_cfg_power(pcfg_file, &sen_power);
			sen_common_load_cfg_i2c(pcfg_file, &sen_i2c);
			sen_common_close_cfg(pcfg_file);
		} else {
			DBG_WRN("load cfg fail \r\n");
		}
	}

	memset((void *)(&reg_obj), 0, sizeof(CTL_SEN_REG_OBJ));
	reg_obj.pwr_ctrl = sen_pwr_ctrl_sc3336;
	reg_obj.det_plug_in = NULL;
	reg_obj.drv_tab = sen_get_drv_tab_sc3336();
	rt = ctl_sen_reg_sendrv("nvt_sen_sc3336", &reg_obj);
	if (rt != E_OK) {
		DBG_WRN("register sensor driver fail \r\n");
	}

	return rt;
}

static void __exit sen_exit_sc3336(void)
{
	ctl_sen_unreg_sendrv("nvt_sen_sc3336");
}

module_init(sen_init_sc3336);
module_exit(sen_exit_sc3336);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION(SEN_SC3336_MODULE_NAME);
MODULE_LICENSE("GPL");
#endif

