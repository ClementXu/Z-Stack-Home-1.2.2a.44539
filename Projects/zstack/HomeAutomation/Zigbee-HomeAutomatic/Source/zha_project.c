/**************************************************************************************************
  Filename:       zha_project.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample device application.


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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/*********************************************************************
  This application implements a ZigBee HA 1.2 Light. It can be configured as an
  On/Off light, or as a dimmable light. The following flags must be defined in
  the compiler's pre-defined symbols.

  ZCL_ON_OFF
  ZCL_LEVEL_CTRL    (only if dimming functionality desired)
  HOLD_AUTO_START
  ZCL_EZMODE

  This device supports all mandatory and optional commands/attributes for the
  OnOff (0x0006) and LevelControl (0x0008) clusters.

  SCREEN MODES
  ----------------------------------------
  Main:
    - SW1: Toggle local light
    - SW2: Invoke EZMode
    - SW4: Enable/Disable local permit join
    - SW5: Go to Help screen
  ----------------------------------------
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT_SYS.h"

#include "nwk_util.h"

#include "ZDObject.h"
#include "ZDProfile.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ss.h"
#include "zcl_ezmode.h"
#include "zcl_diagnostic.h"

#include "zha_project.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#if ( defined (ZGP_DEVICE_TARGET) || defined (ZGP_DEVICE_TARGETPLUS) \
      || defined (ZGP_DEVICE_COMBO) || defined (ZGP_DEVICE_COMBO_MIN) )
#include "zgp_translationtable.h"
  #if (SUPPORTED_S_FEATURE(SUPP_ZGP_FEATURE_TRANSLATION_TABLE))
    #define ZGP_AUTO_TT
  #endif
#endif

#if (defined HAL_BOARD_ZLIGHT) || (defined HAL_PWM)
#include "math.h"
#include "hal_timer.h"
#endif

#include "NLMEDE.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#if (defined HAL_BOARD_ZLIGHT)
#define LEVEL_MAX                 0xFE
#define LEVEL_MIN                 0x0
#define GAMMA_VALUE               2
#define PWM_FULL_DUTY_CYCLE       1000
#elif (defined HAL_PWM)
#define LEVEL_MAX                 0xFE
#define LEVEL_MIN                 0x0
#define GAMMA_VALUE               2
#define PWM_FULL_DUTY_CYCLE       100
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zha_project_TaskID;
uint8 zha_projectSeqNum;
ZDO_ActiveEndpointRsp_t   *zclZHAtest_ActiveEP;
endPointDesc_t zclZHAtest_epDesc;
afAddrType_t zclZHAtest_DstAddr;
static zAddrType_t simpleDescReqAddr;
uint8 ep[5];

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
afAddrType_t zha_project_DstAddr;

#ifdef ZCL_EZMODE
static void zha_project_ProcessZDOMsgs( zdoIncomingMsg_t *pMsg );
static void zha_project_EZModeCB( zlcEZMode_State_t state, zclEZMode_CBData_t *pData );


// register EZ-Mode with task information (timeout events, callback, etc...)
//static const zclEZMode_RegisterData_t zha_project_RegisterEZModeData =
//{
//  &zha_project_TaskID,
//  SAMPLELIGHT_EZMODE_NEXTSTATE_EVT,
//  SAMPLELIGHT_EZMODE_TIMEOUT_EVT,
//  &zha_projectSeqNum,
//  zha_project_EZModeCB
//};

#else
uint16 bindingInClusters[] =
{
  ZCL_CLUSTER_ID_GEN_ON_OFF
#ifdef ZCL_LEVEL_CTRL
  , ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL
#endif
};
#define ZCLSAMPLELIGHT_BINDINGLIST (sizeof(bindingInClusters) / sizeof(bindingInClusters[0]))

#endif  // ZCL_EZMODE

// Test Endpoint to allow SYS_APP_MSGs
static endPointDesc_t sampleLight_TestEp =
{
  SAMPLELIGHT_ENDPOINT,
  &zha_project_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

uint8 giLightScreenMode = LIGHT_MAINMODE;   // display the main screen mode first

uint8 gPermitDuration = 0;    // permit joining default to disabled

devStates_t zha_project_NwkState = DEV_INIT;

#if ZCL_LEVEL_CTRL
uint8 zha_project_WithOnOff;       // set to TRUE if state machine should set light on/off
uint8 zha_project_NewLevel;        // new level when done moving
bool  zha_project_NewLevelUp;      // is direction to new level up or down?
int32 zha_project_CurrentLevel32;  // current level, fixed point (e.g. 192.456)
int32 zha_project_Rate32;          // rate in units, fixed point (e.g. 16.123)
uint8 zha_project_LevelLastLevel;  // to save the Current Level before the light was turned OFF
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */



static void zha_project_HandleKeys( byte shift, byte keys );
static void zha_project_BasicResetCB( void );
static void zha_project_IdentifyCB( zclIdentify_t *pCmd );
static void zha_project_IdentifyQueryRspCB( zclIdentifyQueryRsp_t *pRsp );
static void zha_project_OnOffCB( uint8 cmd );
#ifdef ZCL_LEVEL_CTRL
static void zha_project_LevelControlMoveToLevelCB( zclLCMoveToLevel_t *pCmd );
static void zha_project_LevelControlMoveCB( zclLCMove_t *pCmd );
static void zha_project_LevelControlStepCB( zclLCStep_t *pCmd );
static void zha_project_LevelControlStopCB( void );
static void zha_project_DefaultMove( void );
static uint32 zha_project_TimeRateHelper( uint8 newLevel );
static uint16 zha_project_GetTime ( uint8 level, uint16 time );
static void zha_project_MoveBasedOnRate( uint8 newLevel, uint32 rate );
static void zha_project_MoveBasedOnTime( uint8 newLevel, uint16 time );
static void zha_project_AdjustLightLevel( void );
#endif

// app display functions
static void zha_project_DisplayLight( void );

#if (defined HAL_BOARD_ZLIGHT) || (defined HAL_PWM)
void zha_project_UpdateLampLevel( uint8 level );
#endif

// Functions to process ZCL Foundation incoming Command/Response messages
static void zha_project_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zha_project_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zha_project_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zha_project_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zha_project_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zha_project_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zha_project_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif

/*********************************************************************
 * STATUS STRINGS
 */


/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zha_project_CmdCallbacks =
{
  zha_project_BasicResetCB,            // Basic Cluster Reset command
  zha_project_IdentifyCB,              // Identify command
#ifdef ZCL_EZMODE
  NULL,                                   // Identify EZ-Mode Invoke command
  NULL,                                   // Identify Update Commission State command
#endif
  NULL,                                   // Identify Trigger Effect command
  zha_project_IdentifyQueryRspCB,      // Identify Query Response command
  zha_project_OnOffCB,                 // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  zha_project_LevelControlMoveToLevelCB, // Level Control Move to Level command
  zha_project_LevelControlMoveCB,        // Level Control Move command
  zha_project_LevelControlStepCB,        // Level Control Step command
  zha_project_LevelControlStopCB,        // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};

/*********************************************************************
 * @fn          zha_project_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zha_project_Init( byte task_id )
{
  zha_project_TaskID = task_id;

  // Set destination address to indirect
  zha_project_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zha_project_DstAddr.endPoint = 0;
  zha_project_DstAddr.addr.shortAddr = 0;

  // This app is part of the Home Automation Profile
  zclHA_Init( &zha_project_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLELIGHT_ENDPOINT, &zha_project_CmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( SAMPLELIGHT_ENDPOINT, zha_project_NumAttributes, zha_project_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zha_project_TaskID );

#ifdef ZCL_DISCOVER
  // Register the application's command list
  zcl_registerCmdList( SAMPLELIGHT_ENDPOINT, zclCmdsArraySize, zha_project_Cmds );
#endif

  // Register for all key events - This app will handle all key events
  RegisterForKeys( zha_project_TaskID );

  // Register for a test endpoint
  afRegister( &sampleLight_TestEp );

#ifdef ZCL_EZMODE
  // Register EZ-Mode
  //zcl_RegisterEZMode( &zha_project_RegisterEZModeData );

  // Register with the ZDO to receive Match Descriptor Responses
    ZDO_RegisterForZDOMsg(task_id, Match_Desc_rsp);
    ZDO_RegisterForZDOMsg( task_id, End_Device_Bind_rsp );
    ZDO_RegisterForZDOMsg( task_id, Match_Desc_rsp );
    ZDO_RegisterForZDOMsg( task_id, Active_EP_rsp );
    ZDO_RegisterForZDOMsg( task_id, Simple_Desc_rsp );
    ZDO_RegisterForZDOMsg( task_id, Device_annce );
#endif


#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( SAMPLELIGHT_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif

#ifdef LCD_SUPPORTED
  HalLcdWriteString ( (char *)sDeviceName, HAL_LCD_LINE_3 );
#endif  // LCD_SUPPORTED

#ifdef ZGP_AUTO_TT
  zgpTranslationTable_RegisterEP ( &zha_project_SimpleDesc );
#endif
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zha_project_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

    if ( events & SYS_EVENT_MSG )
    {
        while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zha_project_TaskID )) )
        {
            switch ( MSGpkt->hdr.event )
            {
                #ifdef ZCL_EZMODE
                case ZDO_CB_MSG:
                    zha_project_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
                    break;
                #endif
                case ZCL_INCOMING_MSG:
                // Incoming ZCL Foundation command/response messages
                    zha_project_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
                    break;

                case KEY_CHANGE:
                    zha_project_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
                    break;
                case AF_INCOMING_MSG_CMD:
                    zcl_ProcessMessageMSG(MSGpkt);

                    //ZHAtest_MessageMSGCB( MSGpkt );
                    break;    
                

                case ZDO_STATE_CHANGE:
                    zha_project_NwkState = (devStates_t)(MSGpkt->hdr.status);

                    // now on the network
                    if ( (zha_project_NwkState == DEV_ZB_COORD) ||
                    (zha_project_NwkState == DEV_ROUTER)   ||
                    (zha_project_NwkState == DEV_END_DEVICE) )
                    {
                    giLightScreenMode = LIGHT_MAINMODE;
                    //zha_project_LcdDisplayUpdate();
                    #ifdef ZCL_EZMODE
                    zcl_EZModeAction( EZMODE_ACTION_NETWORK_STARTED, NULL );
                    #endif // ZCL_EZMODE
                    }
                break;

                default:
                    break;
            }

          // Release the memory
          osal_msg_deallocate( (uint8 *)MSGpkt );
        }
        return (events ^ SYS_EVENT_MSG);
    }
    if ( events & ZHA_ATTRIBUTE_REQ_EVT )
    {
        uint8 i=0;
        afAddrType_t  dscReqAddr;
        dscReqAddr.addrMode=afAddr16Bit;
        dscReqAddr.addr.shortAddr=simpleDescReqAddr.addr.shortAddr;
        dscReqAddr.endPoint=1;
        zclReadCmd_t BasicAttrsList;
        BasicAttrsList.numAttr = 5;
        BasicAttrsList.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
        BasicAttrsList.attrID[1] = ATTRID_BASIC_HW_VERSION;
        BasicAttrsList.attrID[2] = ATTRID_BASIC_MODEL_ID;
        BasicAttrsList.attrID[3] = ATTRID_BASIC_MANUFACTURER_NAME;
        //BasicAttrsList.attrID[5] = ATTRID_BASIC_DATE_CODE;
        BasicAttrsList.attrID[4] = ATTRID_BASIC_POWER_SOURCE;
        zcl_SendRead( 1, &dscReqAddr,ZCL_CLUSTER_ID_GEN_BASIC, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
        
        //return ( events ^ ZHA_ATTRIBUTE_REQ_EVT );
    }  
  
    if(events & ZHA_ATTRIBUTE_POWER_EVT)
    {
        afAddrType_t  dscReqAddr;
        dscReqAddr.addrMode=afAddr16Bit;
        dscReqAddr.addr.shortAddr=simpleDescReqAddr.addr.shortAddr;
        dscReqAddr.endPoint=1;
        zclReadCmd_t BasicAttrsList;
        BasicAttrsList.numAttr = 2;
        BasicAttrsList.attrID[0] = ATTRID_POWER_CFG_BATTERY_VOLTAGE;
        BasicAttrsList.attrID[1] = 0x0021;
        zcl_SendRead( 1, &dscReqAddr,
                    ZCL_CLUSTER_ID_GEN_POWER_CFG, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
        //return ( events ^ ZHA_ATTRIBUTE_POWER_EVT );    
    }
    if ( events & ZONE_TYPE_EVT )
    {
        afAddrType_t  dscReqAddr;
        dscReqAddr.addrMode=afAddr16Bit;
        dscReqAddr.addr.shortAddr=simpleDescReqAddr.addr.shortAddr;
        dscReqAddr.endPoint=1;
        zclReadCmd_t BasicAttrsList;
        BasicAttrsList.numAttr = 1;
        BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_TYPE;
        zcl_SendRead( 1, &dscReqAddr,
                    ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0); 
        return ( events ^ ZONE_TYPE_EVT );
    } 
    if ( events & ZHA_ACTIVE_EP_EVT )
    {
        ZDP_ActiveEPReq( &simpleDescReqAddr, simpleDescReqAddr.addr.shortAddr, 0);
        //return ( events ^ ZHA_ACTIVE_EP_EVT );
    }  
  
  // event to get simple descriptor of the newly joined device
    if ( events & SIMPLE_DESC_QUERY_EVT )
    {
        uint8 i;
        for(i=0;i<=zclZHAtest_ActiveEP->cnt;i++)
        {
            if(ep[i]!=0)
            {
                  ZDP_SimpleDescReq( &simpleDescReqAddr, simpleDescReqAddr.addr.shortAddr,
                            ep[i], 0);
            }
        }
        osal_memset(ep,0,sizeof(ep));
        //return ( events ^ SIMPLE_DESC_QUERY_EVT );
    }

  // handle processing of timeout event triggered by request fast polling command

    if ( events & SIMPLE_DESC_EVT )
    {
        ZDP_SimpleDescReq( &simpleDescReqAddr, simpleDescReqAddr.addr.shortAddr,1, 0);
        //return ( events ^ SIMPLE_DESC_QUERY_EVT );
    }
    if ( events & RESET_EVT )
    {
        Onboard_soft_reset();
        return ( events ^ RESET_EVT );
    }  
 
  // Discard unknown events
  return 0;
}


/*********************************************************************
 * @fn      zha_project_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zha_project_HandleKeys( byte shift, byte keys )
{
  if ( keys & HAL_KEY_SW_1 )
  {
    giLightScreenMode = LIGHT_MAINMODE;

    // toggle local light immediately
    zha_project_OnOff = zha_project_OnOff ? LIGHT_OFF : LIGHT_ON;
#ifdef ZCL_LEVEL_CTRL
    zha_project_LevelCurrentLevel = zha_project_OnOff ? zha_project_LevelOnLevel : ATTR_LEVEL_MIN_LEVEL;
#endif
  }

  if ( keys & HAL_KEY_SW_2 )
  {
#if (defined HAL_BOARD_ZLIGHT)

    zha_project_BasicResetCB();

#else

    giLightScreenMode = LIGHT_MAINMODE;

#ifdef ZCL_EZMODE
    {
      // Invoke EZ-Mode
      zclEZMode_InvokeData_t ezModeData;

      // Invoke EZ-Mode
      ezModeData.endpoint = SAMPLELIGHT_ENDPOINT; // endpoint on which to invoke EZ-Mode
      if ( (zha_project_NwkState == DEV_ZB_COORD) ||
          (zha_project_NwkState == DEV_ROUTER)   ||
            (zha_project_NwkState == DEV_END_DEVICE) )
      {
        ezModeData.onNetwork = TRUE;      // node is already on the network
      }
      else
      {
        ezModeData.onNetwork = FALSE;     // node is not yet on the network
      }
      ezModeData.initiator = FALSE;          // OnOffLight is a target
      ezModeData.numActiveOutClusters = 0;
      ezModeData.pActiveOutClusterIDs = NULL;
      ezModeData.numActiveInClusters = 0;
      ezModeData.pActiveOutClusterIDs = NULL;
      zcl_InvokeEZMode( &ezModeData );
    }

#else // NOT EZ-Mode
    {
      zAddrType_t dstAddr;
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request, this bind request will
      // only use a cluster list that is important to binding.
      dstAddr.addrMode = afAddr16Bit;
      dstAddr.addr.shortAddr = 0;   // Coordinator makes the match
      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                           SAMPLELIGHT_ENDPOINT,
                           ZCL_HA_PROFILE_ID,
                           ZCLSAMPLELIGHT_BINDINGLIST, bindingInClusters,
                           0, NULL,   // No Outgoing clusters to bind
                           TRUE );
    }
#endif // ZCL_EZMODE
#endif // HAL_BOARD_ZLIGHT
  }

  if ( keys & HAL_KEY_SW_3 )
  {
    NLME_SendNetworkStatus( zha_project_DstAddr.addr.shortAddr,
                       NLME_GetShortAddr(), NWKSTAT_NONTREE_LINK_FAILURE, FALSE );
  }

  if ( keys & HAL_KEY_SW_4 )
  {
    giLightScreenMode = LIGHT_MAINMODE;

    if ( ( zha_project_NwkState == DEV_ZB_COORD ) ||
          ( zha_project_NwkState == DEV_ROUTER ) )
    {
      zAddrType_t tmpAddr;

      tmpAddr.addrMode = Addr16Bit;
      tmpAddr.addr.shortAddr = NLME_GetShortAddr();

      // toggle permit join
      gPermitDuration = gPermitDuration ? 0 : 0xff;

      // Trust Center significance is always true
      ZDP_MgmtPermitJoinReq( &tmpAddr, gPermitDuration, TRUE, FALSE );
    }
  }

  // Shift F5 does a Basic Reset (factory defaults)
  if ( shift && ( keys & HAL_KEY_SW_5 ) )
  {
    zha_project_BasicResetCB();
  }
  else if ( keys & HAL_KEY_SW_5 )
  {
    giLightScreenMode = giLightScreenMode ? LIGHT_MAINMODE : LIGHT_HELPMODE;
  }

  // update the display, including the light
  //zha_project_LcdDisplayUpdate();
}


/*********************************************************************
 * @fn      zha_project_DisplayLight
 *
 * @brief   Displays current state of light on LED and also on main display if supported.
 *
 * @param   none
 *
 * @return  none
 */
static void zha_project_DisplayLight( void )
{
  // set the LED1 based on light (on or off)
  if ( zha_project_OnOff == LIGHT_ON )
  {
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
  }
  else
  {
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
  }

#ifdef LCD_SUPPORTED
  if (giLightScreenMode == LIGHT_MAINMODE)
  {
#ifdef ZCL_LEVEL_CTRL
    // display current light level
    if ( ( zha_project_LevelCurrentLevel == ATTR_LEVEL_MIN_LEVEL ) &&
         ( zha_project_OnOff == LIGHT_OFF ) )
    {
      HalLcdWriteString( (char *)sLightOff, HAL_LCD_LINE_2 );
    }
    else if ( ( zha_project_LevelCurrentLevel >= ATTR_LEVEL_MAX_LEVEL ) ||
              ( zha_project_LevelCurrentLevel == zha_project_LevelOnLevel ) ||
               ( ( zha_project_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT ) &&
                 ( zha_project_LevelCurrentLevel == zha_project_LevelLastLevel ) ) )
    {
      HalLcdWriteString( (char *)sLightOn, HAL_LCD_LINE_2 );
    }
    else    // "    LEVEL ###"
    {
      zclHA_uint8toa( zha_project_LevelCurrentLevel, &sLightLevel[10] );
      HalLcdWriteString( (char *)sLightLevel, HAL_LCD_LINE_2 );
    }
#else
    if ( zha_project_OnOff )
    {
      HalLcdWriteString( (char *)sLightOn, HAL_LCD_LINE_2 );
    }
    else
    {
      HalLcdWriteString( (char *)sLightOff, HAL_LCD_LINE_2 );
    }
#endif // ZCL_LEVEL_CTRL
  }
#endif // LCD_SUPPORTED
}

/*********************************************************************
 * @fn      zha_project_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zha_project_BasicResetCB( void )
{
  NLME_LeaveReq_t leaveReq;
  // Set every field to 0
  osal_memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );

  // This will enable the device to rejoin the network after reset.
  leaveReq.rejoin = TRUE;

  // Set the NV startup option to force a "new" join.
  zgWriteStartupOptions( ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );

  // Leave the network, and reset afterwards
  if ( NLME_LeaveReq( &leaveReq ) != ZSuccess )
  {
    // Couldn't send out leave; prepare to reset anyway
    ZDApp_LeaveReset( FALSE );
  }
}

/*********************************************************************
 * @fn      zha_project_IdentifyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 *
 * @param   srcAddr - source address and endpoint of the response message
 * @param   identifyTime - the number of seconds to identify yourself
 *
 * @return  none
 */
static void zha_project_IdentifyCB( zclIdentify_t *pCmd )
{
  zha_project_IdentifyTime = pCmd->identifyTime;
//zha_project_ProcessIdentifyTimeChange();
}

/*********************************************************************
 * @fn      zha_project_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this application.
 *
 * @param   srcAddr - requestor's address
 * @param   timeout - number of seconds to identify yourself (valid for query response)
 *
 * @return  none
 */
static void zha_project_IdentifyQueryRspCB(  zclIdentifyQueryRsp_t *pRsp )
{
  (void)pRsp;
#ifdef ZCL_EZMODE
  {
    zclEZMode_ActionData_t data;
    data.pIdentifyQueryRsp = pRsp;
    zcl_EZModeAction ( EZMODE_ACTION_IDENTIFY_QUERY_RSP, &data );
  }
#endif
}

/*********************************************************************
 * @fn      zha_project_OnOffCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   cmd - COMMAND_ON, COMMAND_OFF or COMMAND_TOGGLE
 *
 * @return  none
 */
static void zha_project_OnOffCB( uint8 cmd )
{
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();

  zha_project_DstAddr.addr.shortAddr = pPtr->srcAddr.addr.shortAddr;


  // Turn on the light
  if ( cmd == COMMAND_ON )
  {
    zha_project_OnOff = LIGHT_ON;
  }
  // Turn off the light
  else if ( cmd == COMMAND_OFF )
  {
    zha_project_OnOff = LIGHT_OFF;
  }
  // Toggle the light
  else if ( cmd == COMMAND_TOGGLE )
  {
    if ( zha_project_OnOff == LIGHT_OFF )
    {
      zha_project_OnOff = LIGHT_ON;
    }
    else
    {
      zha_project_OnOff = LIGHT_OFF;
    }
  }

#if ZCL_LEVEL_CTRL
  zha_project_DefaultMove( );
#endif

  // update the display
  //zha_project_LcdDisplayUpdate( );
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zha_project_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zha_project_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zha_project_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zha_project_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT
    // Attribute Reporting implementation should be added here
    case ZCL_CMD_CONFIG_REPORT:
      // zha_project_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      // zha_project_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      // zha_project_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      // zha_project_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      // zha_project_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zha_project_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zha_project_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zha_project_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zha_project_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zha_project_ProcessInDiscAttrsExtRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zha_project_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
    uint8 i,k;
    uint8 *j;
    uint16 *p;
    readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd; 
    switch(pInMsg->clusterId)
    {
      case ZCL_CLUSTER_ID_GEN_BASIC:
        {
          for (i = 0; i < readRspCmd->numAttr; i++)
          {
                zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
                j=statusRec->data;
                switch(statusRec->attrID)
                {
                    case ATTRID_BASIC_POWER_SOURCE:
                        uint8 zclZHAtest_PowerSource=*j;
                        break;
//                    case ATTRID_POWER_CFG_BATTERY_VOLTAGE:
//                        zclZHAtest_BatteryVoltage=*j;
//                        SetTempDeviceBAT(pInMsg->srcAddr.addr.shortAddr,zclZHAtest_BatteryVoltage);
//                        break;
                    case ATTRID_BASIC_ZCL_VERSION:
                        break;
                    case ATTRID_BASIC_MODEL_ID:
                        break;
                    case ATTRID_BASIC_MANUFACTURER_NAME:
                        //SetTempDeviceManuName(pInMsg->srcAddr.addr.shortAddr,j);
                        break;
                    case ATTRID_BASIC_HW_VERSION:
                        uint8 zclZHAtest_HWRevision = *j;
                        //SetTempDeviceHW(pInMsg->srcAddr.addr.shortAddr,zclZHAtest_HWRevision);
                        break;                        
                    default:
                      break;
                
                }

          }
        }
        //osal_set_event( zclZHAtest_TaskID,ZHAtest_ATTRIBUTE_POWER_EVT);
        
        break;
        
      case ZCL_CLUSTER_ID_GEN_POWER_CFG:
        {
          for (i = 0; i < readRspCmd->numAttr; i++)
          {
                zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
                j=statusRec->data;
                switch(statusRec->attrID)
                {
                    case ATTRID_POWER_CFG_BATTERY_VOLTAGE:
                        uint8 zclZHAtest_BatteryVoltage=*j;
                        //SetTempDeviceBAT(pInMsg->srcAddr.addr.shortAddr,zclZHAtest_BatteryVoltage);
                        break;
                    default:
                      break;
                
                }

          }
      
        }        
        
        
        //osal_set_event( zclZHAtest_TaskID, ZHAtest_ACTIVE_EP_EVT );
        break;
//      case ZCL_CLUSTER_ID_GEN_ON_OFF:
//        {
//          for (i = 0; i < readRspCmd->numAttr; i++)
//          {
//            zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//            j=statusRec->data;
//            switch(statusRec->attrID)
//            {
//                case ATTRID_ON_OFF:
//                    uint16 buf[3];
//                    osal_memset(buf,0,sizeof(buf));
//                    zclZHAtest_OnOff=*j;
//                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_OnOff;
//                    Return_Message(1);
//                    buf[0] = zclZHAtest_OnOff;
//                    UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                  break;
//                default:
//                  break;
//            
//            }
//
//          }
//      
//        }
//        break;
//        case ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL:
//            {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    p=(uint16 *)statusRec->data;
//                    //j = zclSerializeData( statusRec->dataType, statusRec->data, j );
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE:
//                            zclZHAtest_Light_Color_Status = *p;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            buf[2] = zclZHAtest_Light_Color_Status;
//                            UpdateDeviceStatus3(pInMsg->srcAddr.addr.shortAddr,buf);
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_Light_Color_Status&0x00FF;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_Light_Color_Status&0xFF00)>>8;
//                            Return_Message(2);
//                        break;
//                        case ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_HUE:
//                            zclZHAtest_HUE_Status = *p;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_HUE_Status&0x00FF;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_HUE_Status&0xFF00)>>8;
//                            Return_Message(2); 
//                          break;
//                        case ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION:
//                            zclZHAtest_Saturation = *p;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_Saturation;
//                            Return_Message(1);                           
//                          break;
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }
//    
//        break;
//        case ZCL_CLUSTER_ID_SS_IAS_ZONE:
//       {        uint16 supportOD = 0;
//                uint8 sensorType = 0;
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    p=(uint16 *)statusRec->data;
//                    //j = zclSerializeData( statusRec->dataType, statusRec->data, j );
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_SS_IAS_ZONE_STATUS:
//                            zclZHAtest_Alarm_Status = *p;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            buf[0] = zclZHAtest_Alarm_Status;
//                            UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_Alarm_Status&0x00FF;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_Alarm_Status&0xFF00)>>8;
//                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_Alarm_Status;
//                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_Light_Color_Status&0xFF00)>>8;
//                            Return_Message(2);
//                        break;
//                        case ATTRID_SS_IAS_ZONE_TYPE:
//                            zAddrType_t addr;
//                            uint8 address[8];
//                            uint8 pValue[Z_EXTADDR_LEN];
//                            addr.addrMode = Addr64Bit;
//                            osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
//                            //osal_memcpy(pValue,addr.addr.extAddr,8);
//                            osal_memcpy(addr.addr.extAddr,pValue,8);
//                            //addr.addr.shortAddr=pSimpleDescRsp->nwkAddr;
//                            APSME_LookupExtAddr(pInMsg->srcAddr.addr.shortAddr,address);
//                            ZDP_BindUnbindReq(Bind_req, &dstAddr, address,
//                                                   1,
//                                                   0x0020,
//                                                   &addr,  pInMsg->endPoint,
//                                                    FALSE );
//                            zclZHAtest_Smoke_Type = *p;
//                            //uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            buf[1] = zclZHAtest_Smoke_Type;
//                            UpdateDeviceStatus2(pInMsg->srcAddr.addr.shortAddr,buf);
//                            //zclSS_IAS_Send_ZoneStatusEnrollRequestCmd(0x01,&destAddr,zclZHAtest_Smoke_Type,0,false,0);
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=zclZHAtest_Smoke_Type&0x00FF;
//                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_Smoke_Type&0xFF00)>>8;                                
//                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0+i]=zclZHAtest_Smoke_Type;
//                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=(zclZHAtest_Light_Color_Status&0xFF00)>>8;
//                            //Return_Message(2+i);  
//                            SetTempDeviceOD(pInMsg->srcAddr.addr.shortAddr,zclZHAtest_Smoke_Type);
//                         break;
//                      
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }
//      
//      break;
//      case ZCL_CLUSTER_ID_SS_IAS_WD:
//            {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    j=statusRec->data;
//                    switch(statusRec->attrID)
//                    {
//                        case COMMAND_SS_IAS_WD_START_WARNING:
//                            //zclZHAtest_Warning = *j;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            //buf[0] = zclZHAtest_Warning;
//                            //UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                            break;
//                        case COMMAND_SS_IAS_WD_SQUAWK:
//                            //zclZHAtest_WD_SQUAWK = *j;
//                            //uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                           // buf[1] = zclZHAtest_WD_SQUAWK;
//                            //UpdateDeviceStatus2(pInMsg->srcAddr.addr.shortAddr,buf);
//                            break;
//
//                        default:
//                            break;
//
//                    }
//
//                }
//
//             }        
//        
//        break;
//      
//      case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
//       {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    p=(uint16 *)statusRec->data;
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_MS_TEMPERATURE_MEASURED_VALUE:
//                            //zclZHAtest_Temperature_Value = *p;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            //buf[0] = zclZHAtest_Temperature_Value;
//                            //UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                            
//                        break;
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }
//         break;
//        case ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL:
//           {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    j=statusRec->data;
//                    //j = zclSerializeData( statusRec->dataType, statusRec->data, j );
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_MS_TEMPERATURE_MEASURED_VALUE:
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            //zclZHAtest_Level_to_Level = *j;
//                            buf[1] = zclZHAtest_Level_to_Level;
//                           // UpdateDeviceStatus2(pInMsg->srcAddr.addr.shortAddr,buf); 
//                        break;
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }
//    
//        break;
//      case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
//           {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    p=(uint16 *)statusRec->data;
//                    //j = zclSerializeData( statusRec->dataType, statusRec->data, j );
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE:
//                            //zclZHAtest_Humidity_Value = *p;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                           // buf[0] = zclZHAtest_Humidity_Value;
//                            //UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                            
//                        break;
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }     
//        
//        
//        break;   
//      case ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT:
//           {
//                for (i = 0; i < readRspCmd->numAttr; i++)
//                {
//                    zclReadRspStatus_t *statusRec = &(readRspCmd->attrList[i]);
//                    p=(uint16 *)statusRec->data;
//                    //j = zclSerializeData( statusRec->dataType, statusRec->data, j );
//                    switch(statusRec->attrID)
//                    {
//                        case ATTRID_MS_ILLUMINANCE_MEASURED_VALUE:
//                            //zclZHAtest_Illumiance_Value = *p;
//                            uint16 buf[3];
//                            osal_memset(buf,0,sizeof(buf));
//                            //buf[0] = zclZHAtest_Illumiance_Value;
//                            //UpdateDeviceStatus1(pInMsg->srcAddr.addr.shortAddr,buf);
//                            
//                        break;
//                        default:
//                        break;
//
//                    }
//
//                }
//
//             }          
//        
//        
//        break;
      default:
        break;
    //ReadRspStatus.attrID = readRspCmd->attrList;
    // Notify the originator of the results of the original read attributes 
    // attempt and, for each successfull request, the value of the requested 
    // attribute
  }

  
  
  return TRUE; 
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zha_project_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zha_project_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zha_project_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zha_project_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zha_project_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zha_project_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}
#endif // ZCL_DISCOVER

#if ZCL_EZMODE
/*********************************************************************
 * @fn      zha_project_ProcessZDOMsgs
 *
 * @brief   Called when this node receives a ZDO/ZDP response.
 *
 * @param   none
 *
 * @return  status
 */
static void zha_project_ProcessZDOMsgs( zdoIncomingMsg_t *pMsg )
{
  zclEZMode_ActionData_t data;
  ZDO_MatchDescRsp_t *pMatchDescRsp;

  // Let EZ-Mode know of the Simple Descriptor Response
    ZDO_DeviceAnnce_t devAnnce;
    zAddrType_t addr;
    afAddrType_t destAddr;
    uint8 *pData;
    uint8 address[8];
    uint8 i;
    uint8 pValue[Z_EXTADDR_LEN];
    uint8 pValue1[Z_EXTADDR_LEN];
    switch ( pMsg->clusterID )
    {
        case End_Device_Bind_rsp:
        {
            if ( ZDO_ParseBindRsp( pMsg ) == ZSuccess )
            {

            // Light LED
            //HalUARTWrite(HAL_UART_PORT_0,"Bind established",sizeof("Bind established"));
                HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
            }
            #if defined( BLINK_LEDS )
            else
            {
            // Flash LED to show failure
                HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
            }
            #endif
            break;
        }
        case Match_Desc_rsp:
        {
            ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( pMsg );
            if ( pRsp )
            {
                if ( pRsp->status == ZSuccess && pRsp->cnt )
                {
                    zclZHAtest_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
                    zclZHAtest_DstAddr.addr.shortAddr = pRsp->nwkAddr;
                    // Take the first endpoint, Can be changed to search through endpoints
                    zclZHAtest_DstAddr.endPoint = pRsp->epList[0];

                    // Light LED
                    HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
                }
                osal_mem_free( pRsp );
            }
            break;
        }
        case Device_annce:
        { 
            uint8 k=0;
            ZDO_ParseDeviceAnnce( pMsg, &devAnnce );
            
            
            for(i=0;i<6;i++)
            {

                if(AssociatedDevList[i].shortAddr!=devAnnce.nwkAddr)
                {
                    //SetTempDeviceSA(devAnnce.nwkAddr,devAnnce.extAddr);
                }
            }
            // set simple descriptor query event
            //DelayMS(50);
            simpleDescReqAddr.addrMode = (afAddrMode_t)Addr16Bit;
            simpleDescReqAddr.addr.shortAddr = devAnnce.nwkAddr;
            osal_start_timerEx( zha_project_TaskID, ZHA_ATTRIBUTE_REQ_EVT ,100);
            //osal_start_timerEx( zha_project_TaskID, SIMPLE_DESC_EVT,200);
            osal_start_timerEx( zha_project_TaskID, ZHA_ATTRIBUTE_POWER_EVT,300);
            osal_start_timerEx( zha_project_TaskID, ZHA_ACTIVE_EP_EVT,500);
            osal_start_timerEx( zha_project_TaskID, SIMPLE_DESC_QUERY_EVT,1000);
            //osal_set_event( zclZHAtest_TaskID, SIMPLE_DESC_QUERY_EVT );
            break;
        }

        case Active_EP_rsp:
        {
            ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( pMsg );
            *zclZHAtest_ActiveEP = *pRsp;
            osal_memset(ep,0,sizeof(ep));
            osal_memcpy(ep,pRsp->epList,pRsp->cnt);
            simpleDescReqAddr.addrMode = (afAddrMode_t)Addr16Bit;
            simpleDescReqAddr.addr.shortAddr = pRsp->nwkAddr;
            //HalUARTWrite(HAL_UART_PORT_0,ep,pRsp->cnt);
            //zclZHAtest_ActiveEP.status = pRsp->status;
            //zclZHAtest_ActiveEP.nwkAddr= pRsp->nwkAddr;
            //osal_memcpy(zclZHAtest_ActiveEP.epList,pRsp->epList,sizeof(uint8));
            //zclZHAtest_ActiveEP.epList[] = pRsp->epList[];
            //DelayMS(50);
            //SetTempDeviceEP(pRsp->nwkAddr , ep );
            //osal_set_event( zha_project_TaskID, SIMPLE_DESC_QUERY_EVT );
            osal_mem_free( pRsp ); 
            break;
        }
        case Simple_Desc_rsp:
        {
            uint8 k=0;
            ZDO_SimpleDescRsp_t *pSimpleDescRsp;   // pointer to received simple desc response
            pSimpleDescRsp = (ZDO_SimpleDescRsp_t *)osal_mem_alloc( sizeof( ZDO_SimpleDescRsp_t ) );


            if(pSimpleDescRsp)
            {
                pSimpleDescRsp->simpleDesc.pAppInClusterList = NULL;
                pSimpleDescRsp->simpleDesc.pAppOutClusterList = NULL;

                ZDO_ParseSimpleDescRsp( pMsg, pSimpleDescRsp );
                if(pSimpleDescRsp->simpleDesc.AppDeviceId ==0x0402)
                {
                    afAddrType_t  dscReqAddr;
                    dscReqAddr.addrMode=afAddr16Bit;
                    dscReqAddr.addr.shortAddr=pSimpleDescRsp->nwkAddr;
                    dscReqAddr.endPoint=1;
                    zclReadCmd_t BasicAttrsList;
                    BasicAttrsList.numAttr = 1;
                    BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_TYPE;
                    zcl_SendRead( 1, &dscReqAddr,
                    ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0); 
                    osal_set_event( zha_project_TaskID, ZONE_TYPE_EVT );
                    //DelayMS(100);
                    //zclSampleCIE_WriteIAS_CIE_Address(&destAddr); 
                }
                //else
                    //SetTempDeviceOD(pSimpleDescRsp->nwkAddr,pSimpleDescRsp->simpleDesc.AppDeviceId);
                //osal_mem_free( pSimpleDescRsp );
                // free memory for InClusterList
                if (pSimpleDescRsp->simpleDesc.pAppInClusterList)
                {
                    osal_mem_free(pSimpleDescRsp->simpleDesc.pAppInClusterList);
                }

                // free memory for OutClusterList
                if (pSimpleDescRsp->simpleDesc.pAppOutClusterList)
                {
                    osal_mem_free(pSimpleDescRsp->simpleDesc.pAppOutClusterList);
                }

                osal_mem_free( pSimpleDescRsp );
            }
            break;
        }

        case Bind_rsp:
            ZDO_MgmtBindRsp_t *Bind_pRsp =ZDO_ParseMgmtBindRsp( pMsg ) ;
            afAddrType_t  dscReqAddr;
            dscReqAddr.addrMode=afAddr16Bit;
            if ( Bind_pRsp )
            {
              //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
              //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
              dscReqAddr.addr.shortAddr=AssociatedDevList[0].shortAddr;
              dscReqAddr.endPoint=0x01;
              //zclSampleCIE_WriteIAS_CIE_Address(&dscReqAddr);   
            }
            osal_mem_free( Bind_pRsp );
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      zha_project_EZModeCB
 *
 * @brief   The Application is informed of events. This can be used to show on the UI what is
*           going on during EZ-Mode steering/finding/binding.
 *
 * @param   state - an
 *
 * @return  none
 */
static void zha_project_EZModeCB( zlcEZMode_State_t state, zclEZMode_CBData_t *pData )
{
#ifdef LCD_SUPPORTED
  char *pStr;
  uint8 err;
#endif

  // time to go into identify mode
  if ( state == EZMODE_STATE_IDENTIFYING )
  {
#ifdef LCD_SUPPORTED
    HalLcdWriteString( "EZMode", HAL_LCD_LINE_2 );
#endif

    zha_project_IdentifyTime = ( EZMODE_TIME / 1000 );  // convert to seconds
    //zha_project_ProcessIdentifyTimeChange();
  }

  // autoclosing, show what happened (success, cancelled, etc...)
  if( state == EZMODE_STATE_AUTOCLOSE )
  {
#ifdef LCD_SUPPORTED
    pStr = NULL;
    err = pData->sAutoClose.err;
    if ( err == EZMODE_ERR_SUCCESS )
    {
      pStr = "EZMode: Success";
    }
    else if ( err == EZMODE_ERR_NOMATCH )
    {
      pStr = "EZMode: NoMatch"; // not a match made in heaven
    }
    if ( pStr )
    {
      if ( giLightScreenMode == LIGHT_MAINMODE )
      {
        HalLcdWriteString ( pStr, HAL_LCD_LINE_2 );
      }
    }
#endif
  }

  // finished, either show DstAddr/EP, or nothing (depending on success or not)
  if( state == EZMODE_STATE_FINISH )
  {
    // turn off identify mode
    zha_project_IdentifyTime = 0;
    //zha_project_ProcessIdentifyTimeChange();

#ifdef LCD_SUPPORTED
    // if successful, inform user which nwkaddr/ep we bound to
    pStr = NULL;
    err = pData->sFinish.err;
    if( err == EZMODE_ERR_SUCCESS )
    {
      // already stated on autoclose
    }
    else if ( err == EZMODE_ERR_CANCELLED )
    {
      pStr = "EZMode: Cancel";
    }
    else if ( err == EZMODE_ERR_BAD_PARAMETER )
    {
      pStr = "EZMode: BadParm";
    }
    else if ( err == EZMODE_ERR_TIMEDOUT )
    {
      pStr = "EZMode: TimeOut";
    }
    if ( pStr )
    {
      if ( giLightScreenMode == LIGHT_MAINMODE )
      {
        HalLcdWriteString ( pStr, HAL_LCD_LINE_2 );
      }
    }
#endif
    // show main UI screen 3 seconds after binding
    //osal_start_timerEx( zha_project_TaskID, SAMPLELIGHT_MAIN_SCREEN_EVT, 3000 );
  }
}
#endif // ZCL_EZMODE

/****************************************************************************
****************************************************************************/


