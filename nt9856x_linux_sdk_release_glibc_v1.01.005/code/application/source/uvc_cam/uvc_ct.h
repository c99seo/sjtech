
#include "hd_type.h"
#ifndef ENABLE
#define ENABLE 1
#define DISABLE 0
#endif

// CT control
#define CT_SCANNING_MODE_EN                         DISABLE
#define CT_AE_MODE_EN                               ENABLE
#define CT_AE_PRIORITY_EN                           DISABLE
#define CT_EXPOSURE_TIME_ABSOLUTE_EN                ENABLE
#define CT_EXPOSURE_TIME_RELATIVE_EN                DISABLE
#define CT_FOCUS_RELATIVE_EN                        DISABLE
#define CT_ZOOM_ABSOLUTE_EN                         ENABLE
#define CT_ZOOM_RELATIVE_EN                         DISABLE
#define CT_PANTILT_ABSOLUTE_EN                      DISABLE
#define CT_PANTILT_RELATIVE_EN                      DISABLE
#define CT_FOCUS_ABSOLUTE_EN                        DISABLE
#define CT_PRIVACY_CONTROL_EN                       DISABLE
#define CT_AUTO_FOCUS_EN                            DISABLE
#define CT_REGION_OF_INTEREST_EN                    DISABLE
#if CT_SCANNING_MODE_EN
#define CT_SCANNING_MODE_MSK                        0x0001
#else
#define CT_SCANNING_MODE_MSK                        0x0000
#endif

#if CT_AE_MODE_EN
#define CT_AE_MODE_MSK                              0x0002
#else
#define CT_AE_MODE_MSK                              0x0000
#endif

#if CT_AE_PRIORITY_EN
#define CT_AE_PRIORITY_MSK                          0x0004
#else
#define CT_AE_PRIORITY_MSK                          0x0000
#endif

#if CT_EXPOSURE_TIME_ABSOLUTE_EN
#define CT_EXPOSURE_TIME_ABSOLUTE_MSK               0x0008
#else
#define CT_EXPOSURE_TIME_ABSOLUTE_MSK               0x0000
#endif

#if CT_EXPOSURE_TIME_RELATIVE_EN
#define CT_EXPOSURE_TIME_RELATIVE_MSK               0x0010
#else
#define CT_EXPOSURE_TIME_RELATIVE_MSK               0x0000
#endif

#if CT_FOCUS_ABSOLUTE_EN
#define CT_FOCUS_ABSOLUTE_MSK                       0x0020
#else
#define CT_FOCUS_ABSOLUTE_MSK                       0x0000
#endif

#if CT_FOCUS_RELATIVE_EN
#define CT_FOCUS_RELATIVE_MSK                       0x0040
#else
#define CT_FOCUS_RELATIVE_MSK                       0x0000
#endif

#if CT_ZOOM_ABSOLUTE_EN
#define CT_ZOOM_ABSOLUTE_MSK                        0x0200
#else
#define CT_ZOOM_ABSOLUTE_MSK                        0x0000
#endif

#if CT_ZOOM_RELATIVE_EN
#define CT_ZOOM_RELATIVE_MSK                        0x0400
#else
#define CT_ZOOM_RELATIVE_MSK                        0x0000
#endif

#if CT_PANTILT_ABSOLUTE_EN
#define CT_PANTILT_ABSOLUTE_MSK                     0x0800
#else
#define CT_PANTILT_ABSOLUTE_MSK                     0x0000
#endif

#if CT_PANTILT_RELATIVE_EN
#define CT_PANTILT_RELATIVE_MSK                     0x1000
#else
#define CT_PANTILT_RELATIVE_MSK                     0x0000
#endif

#if CT_PRIVACY_CONTROL_EN
#define CT_PRIVACY_CONTROL_MSK                      0x40000
#else
#define CT_PRIVACY_CONTROL_MSK                      0x00000
#endif

#if CT_AUTO_FOCUS_EN
#define CT_AUTO_FOCUS_MSK                           0x20000
#else
#define CT_AUTO_FOCUS_MSK                           0x00000
#endif

#if CT_REGION_OF_INTEREST_EN
#define CT_REGION_OF_INTEREST_MSK                   0x200000
#else
#define CT_REGION_OF_INTEREST_MSK                   0x00000
#endif


// Define Bits Containing Capabilities of the control for the Get_Info request (section 4.1.2 in the UVC spec 1.1)
#define SUPPORT_GET_REQUEST                                                     0x01
#define SUPPORT_SET_REQUEST                                                     0x02
#define DISABLED_DUE_TO_AUTOMATIC_MODE                          0x04
#define AUTOUPDATE_CONTROL                                                      0x08
#define ASNCHRONOUS_CONTROL                                                     0x10
#define RESERVED_BIT5                                                           0x20
#define RESERVED_BIT6                                                           0x40
#define RESERVED_BIT7                                                           0x80


//Camera Terminal Control Selectors
#define CT_CONTROL_UNDEFINED                                            0x00
#define CT_SCANNING_MODE_CONTROL                                        0x01
#define CT_AE_MODE_CONTROL                                                      0x02
#define CT_AE_PRIORITY_CONTROL                                          0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL                       0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL                       0x05
#define CT_FOCUS_ABSOLUTE_CONTROL                                       0x06
#define CT_FOCUS_RELATIVE_CONTROL                                       0x07
#define CT_FOCUS_AUTO_CONTROL                                           0x08
#define CT_IRIS_ABSOLUTE_CONTROL                                        0x09
#define CT_IRIS_RELATIVE_CONTROL                                        0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL                                        0x0B
#define CT_ZOOM_RELATIVE_CONTROL                                        0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL                                     0x0D
#define CT_PANTILT_RELATIVE_CONTROL                                     0x0E
#define CT_ROLL_ABSOLUTE_CONTROL                                        0x0F
#define CT_ROLL_RELATIVE_CONTROL                                        0x10
#define CT_PRIVACY_CONTROL                                              0x11

extern UINT32 uvc_ct_controls;
extern BOOL xUvacCT_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen);

