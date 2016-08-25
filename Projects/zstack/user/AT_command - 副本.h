#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "onboard.h"
#define AT_GROUP_NUM 40
#define DEVICE_NUM 40








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
void AT_process_event();
void NWK_CPMMAND_precess_event();
