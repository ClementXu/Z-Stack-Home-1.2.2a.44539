#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "onboard.h"
#define AT_GROUP_NUM 60
#define DEVICE_NUM 40




typedef enum
{
    light=0,
    colorlight,
    dimmer,
    thermometer,
    pir,
    thermohygrograph,
    doorsensor,
    illuminance,
    slsensor,
    smokesensor,
    watersensor,
    cosensor,
    gassensor,
    glasssensor,
    zonecontroller,
    alertleak,
    altermove,
    alterlight,
    outlet,
    doorbell,
    lightswitch,
}devicetype_t;


typedef enum
{
	CHAR=0,
	VALUE_D,
        VALUE_MIX 
	
}DATA_TYPE;
typedef struct __getDeviceData
{
	char *RecData;
	uint32 *data[10];//等待发送的数据（是个数组的首地址）
	uint8 length;//需要发送数据的个数
	DATA_TYPE type;//需要发送数据的类型
    uint8 RecSign;
}getDeviceData;


  #if ( ZG_DEVICE_TYPE_GATEWAY )
    // If capable, default to coordinator
    #define DEVICE_TYPE ZG_DEVICE_TYPE_GATEWAY
  #elif ( ZG_DEVICE_TYPE_ROUTER )
    #define DEVICE_TYPE ZG_DEVICE_TYPE_ROUTER
  #elif ( ZG_DEVICE_TYPE_LIGHT )
    // Must be an end device
    #define DEVICE_TYPE ZG_DEVICE_TYPE_LIGHT

  #endif


void AT_Init();
unsigned char IndexofAT(char *data);
void AT_process_event(char *p,uint16 length);
void NWK_CPMMAND_precess_event();
