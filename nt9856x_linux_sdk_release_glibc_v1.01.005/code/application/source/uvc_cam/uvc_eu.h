
#include "hd_type.h"
#ifndef ENABLE
#define ENABLE 1
#define DISABLE 0
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

//UVC Extension Unit Control Selectors
#define EU_CONTROL_ID_01							0x01
#define EU_CONTROL_ID_02							0x02
#define EU_CONTROL_ID_03							0x03
#define EU_CONTROL_ID_04							0x04
#define EU_CONTROL_ID_05							0x05
#define EU_CONTROL_ID_06							0x06
#define EU_CONTROL_ID_07							0x07
#define EU_CONTROL_ID_08							0x08
#define EU_CONTROL_ID_09							0x09
#define EU_CONTROL_ID_10							0x0A
#define EU_CONTROL_ID_11							0x0B
#define EU_CONTROL_ID_12							0x0C
#define EU_CONTROL_ID_13							0x0D
#define EU_CONTROL_ID_14							0x0E
#define EU_CONTROL_ID_15							0x0F
#define EU_CONTROL_ID_16							0x10

extern BOOL xUvacEU_CB(UINT32 CS, UINT8 request, UINT8 *pData, UINT32 *pDataLen);

#define NUM_PACK 152	//(1920*1080*1.5)/20KB=151.875
#define SIZE_PACK 20480	//max 20KB
#define SIZE_RAW 0x002FAEC0//(1920*1080*1.5)
