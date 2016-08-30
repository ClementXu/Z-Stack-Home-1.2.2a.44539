/**************************************************************************************************
  Filename:       zha_project.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef zha_project_H
#define zha_project_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define SAMPLELIGHT_ENDPOINT            1

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Application Events
#define ZHA_ATTRIBUTE_REQ_EVT            0x0004
#define ZHA_ACTIVE_EP_EVT                0x0008 
#define SIMPLE_DESC_QUERY_EVT            0x0010 
#define ZONE_TYPE_EVT                    0x0020    
#define SIMPLE_DESC_EVT                  0x0040   
#define DEVICE_STATUS_EVT                0x0100
#define ZHA_ATTRIBUTE_POWER_EVT          0x0080
#define RESET_EVT                        0x0200  

// Application Display Modes
#define LIGHT_MAINMODE      0x00
#define LIGHT_HELPMODE      0x01

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint8		aucMAC[8];
    uint16		uiNwk_Addr;
    uint8		ucAge;
    uint8       batteryValue;
    uint8       deviceType[16];
    uint8       factoryName[17];
    uint8       version;
    uint8       ep[5];
}NODE_INFO_t;
typedef struct
{
    NODE_INFO_t device[5];
}NODE_INFO_Group;

typedef struct
{
    uint16 uiNwk_Addr;
    uint16 supportOD;
    uint8  deviceType[16];
    uint16 status[3];
    uint16 seq;
}DEVICE_STATUS_t;
   
/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zha_project_SimpleDesc;

extern byte zha_project_TaskID;

extern CONST zclCommandRec_t zha_project_Cmds[];

extern CONST uint8 zclCmdsArraySize;

// attribute list
extern CONST zclAttrRec_t zha_project_Attrs[];
extern CONST uint8 zha_project_NumAttributes;

// Identify attributes
extern uint16 zha_project_IdentifyTime;
extern uint8  zha_project_IdentifyCommissionState;

// OnOff attributes
extern uint8  zha_project_OnOff;

// Level Control Attributes
#ifdef ZCL_LEVEL_CTRL
extern uint8  zha_project_LevelCurrentLevel;
extern uint16 zha_project_LevelRemainingTime;
extern uint16 zha_project_LevelOnOffTransitionTime;
extern uint8  zha_project_LevelOnLevel;
extern uint16 zha_project_LevelOnTransitionTime;
extern uint16 zha_project_LevelOffTransitionTime;
extern uint8  zha_project_LevelDefaultMoveRate;
#endif

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zha_project_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zha_project_event_loop( byte task_id, UINT16 events );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* zha_project_H */
