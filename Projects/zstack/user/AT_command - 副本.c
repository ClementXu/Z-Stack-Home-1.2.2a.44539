#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "AT_command.h"
#include "onboard.h"
#include "sapi.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ss.h"
#include "zcl_ha.h"
#include "zcl_ms.h"
#include "SerialApp.h"
#include "zcl_ZHAtest.h"
#include "hal_uart.h"
#include "hal_led.h"
//zAddrType_t dstAddr;



#define BigtoLittle16(A)  ((((uint16)(A) & 0xff00) >> 8) | \
                                                          (((uint16)(A) & 0x00ff) << 8))
#define BigtoLittle32(A)  ((((uint32)(A) & 0xff000000) >> 24) | \
                                                          (((uint32)(A) & 0x00ff0000) >> 8) | \
                                                          (((uint32)(A) & 0x0000ff00) << 8) | \
                                                          (((uint32)(A) & 0x000000ff) << 24))
char *AT_GROUP[AT_GROUP_NUM]; 
extern uint8  zclZHAtest_OnOff;
extern uint8  zclZHAtest_Level_to_Level;
extern uint8 zclZHAtest_PIR_Status;
extern uint8 zclZHAtest_Smoke_Type;
extern uint16 zclZHAtest_Light_Color_Status;


char *DEVICE_NAME[DEVICE_NUM];
extern afAddrType_t zclZHAtest_DstAddr;
extern devStates_t zclZHAtest_NwkState;
extern uint8 appState;
extern zAddrType_t dstAddr;
const uint8 zclZHAtest_HWRevision1[4]="V1.0";
extern const uint8 zclZHAtest_ZCLVersion;
extern const uint8 zclZHAtest_ManufacturerName[];
extern const uint8 zclZHAtest_ModelId[];
extern const uint8 zclZHAtest_DateCode[];
unsigned char devicetp;
Node_Status_t Node_Status;
AT_STATUS at_status;
NWK_COMMAND_BASIC_t NWK_command;
Network_Parameter_t Network_Parameter;
Trigger_Parameter_t Trigger_Parameter;

enum
{
	ENUM_AT_MAC=1,
	ENUM_AT_FMVER,
	ENUM_AT_SAVE,
	ENUM_AT_FACTORY,
	ENUM_AT_REBOOT,
	ENUM_AT_ECHO,
        ENUM_AT_UART,
        ENUM_AT_DEVICE,
        ENUM_AT_BINDING,
        ENUM_AT_VALUE,
        ENUM_AT_LIST,
        ENUM_AT_CHECK,
        ENUM_AT_SETDEVICE,
        
        ENUM_AT_VER,
        ENUM_AT_RESET,
        ENUM_AT_RTOKEN,
        ENUM_AT_FORM,
        ENUM_AT_LEAVE,
        ENUM_AT_PERMITJOIN,
        ENUM_AT_MTOGAP,
        ENUM_AT_BOOST,
        ENUM_AT_EXTPA,
        ENUM_AT_USEUFL,
        ENUM_AT_SETUART,
        ENUM_AT_SETCH,
        ENUM_AT_SCANCHMASK,
        ENUM_AT_SETPID,
        ENUM_AT_SETPWR,
//        ENUM_AT_LIST,
        ENUM_AT_GETCFG,
        ENUM_AT_GETINFO,
        ENUM_AT_SETPWRMODE
}at_command;
enum
{
    gateway=0,
    router,
    light,
    colorlight,
    dimmer,
    thermometer,
    thermohygrograph,
    alterleak,
    altermove,
    alterlight,
    outlet,
    doorbell,
    onoffswitch
}devicetype_t;

void Return_Message(uint8 length)
{
   osal_memset(at_status.DATA_buffer,0,128);
   at_status.DATA_buffer[0]=NWK_command.NWKCB_Header;
   at_status.DATA_buffer[1]=NWK_command.NWKCB_Length;
   osal_memcpy(at_status.DATA_buffer+2,NWK_command.NWK_General_Frame.NWKCG_FrameControl,2);  
   osal_memcpy(at_status.DATA_buffer+10,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
   osal_memcpy(at_status.DATA_buffer+18,NWK_command.NWK_General_Frame.NWKCG_SourceAddress,8);   
   osal_memcpy(at_status.DATA_buffer+28,NWK_command.NWK_General_Frame.NWKCG_CMDID,2);  
   osal_memcpy(at_status.DATA_buffer+30,NWK_command.NWK_General_Frame.NWKCG_GroupID,2);
   osal_memcpy(at_status.DATA_buffer+32,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index,2);   
   at_status.DATA_buffer[34]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex;
   at_status.DATA_buffer[35]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Opt; 
   at_status.DATA_buffer[36]=length; 
   osal_memcpy(at_status.DATA_buffer+37,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,length);
   at_status.DATA_buffer[37+length]=0xFF;
   at_status.DATA_buffer[38+length]=NWK_command.NWKCB_Footer;  
   HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
//   osal_msg_deallocate(NWK_command);
   
}


void AT_Init()
{
	AT_GROUP[ENUM_AT_MAC]="MAC";
	AT_GROUP[ENUM_AT_FMVER]="FMVER";
	AT_GROUP[ENUM_AT_SAVE]="SAVE";
	AT_GROUP[ENUM_AT_FACTORY]="FACTORY";
	AT_GROUP[ENUM_AT_REBOOT]="REBOOT";
        AT_GROUP[ENUM_AT_ECHO]="ECHO";
	AT_GROUP[ENUM_AT_UART]="UART";
        AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
        AT_GROUP[ENUM_AT_BINDING]="BINDING";
        AT_GROUP[ENUM_AT_VALUE]="VALUE";
        AT_GROUP[ENUM_AT_LIST]="LIST";
        AT_GROUP[ENUM_AT_CHECK]="CHECK";
        AT_GROUP[ENUM_AT_SETDEVICE]="SETDEV";
        
        AT_GROUP[ENUM_AT_VER]="VER";
        AT_GROUP[ENUM_AT_RESET]="RESET";
        AT_GROUP[ENUM_AT_RTOKEN]="RTOKEN";
        AT_GROUP[ENUM_AT_FORM]="FORM";
        AT_GROUP[ENUM_AT_LEAVE]="LEAVE";
        AT_GROUP[ENUM_AT_PERMITJOIN]="PERMITJOIN";
        AT_GROUP[ENUM_AT_MTOGAP]="MTOGAP";
        AT_GROUP[ENUM_AT_BOOST]="BOOST";
        AT_GROUP[ENUM_AT_EXTPA]="EXTPA";
        AT_GROUP[ENUM_AT_USEUFL]="USEUFL";
        AT_GROUP[ENUM_AT_SETUART]="SETUART";
        AT_GROUP[ENUM_AT_SETCH]="SETCH";
        AT_GROUP[ENUM_AT_SCANCHMASK]="SCANCHMASK";
        AT_GROUP[ENUM_AT_SETPID]="SETPID";
        AT_GROUP[ENUM_AT_SETPWR]="SETPWR";
        AT_GROUP[ENUM_AT_GETCFG]="GETCFG";
        AT_GROUP[ENUM_AT_GETINFO]="GETINFO";
        AT_GROUP[ENUM_AT_SETPWRMODE]="SETPWRMODE";
        
};



unsigned char IndexofAT(char *data)
{
	unsigned char i=1;
	for(i=1;i<=AT_GROUP_NUM;i++)
	{
            if(strcmp(data,AT_GROUP[i])==0)
               return i;
        }
};

unsigned char Indexofdevice(char *data)
{
	unsigned char i=20;
	for(i=20;i<=DEVICE_NUM;i++)
	{
            if(strcmp(data,DEVICE_NAME[i])==0)
               return i;
        }
};

void Device_type_Init()
{
	DEVICE_NAME[gateway]="gateway";
        DEVICE_NAME[router]="router";
	DEVICE_NAME[light]="light";
	DEVICE_NAME[colorlight]="colorlight";
	DEVICE_NAME[dimmer]="dimmer";
	DEVICE_NAME[thermometer]="thermometer";
        DEVICE_NAME[thermohygrograph]="thermohygrograph";
	DEVICE_NAME[alterleak]="alterleak";
        DEVICE_NAME[altermove]="altermove";
        DEVICE_NAME[alterlight]="alterlight";
        DEVICE_NAME[outlet]="outlet";
        DEVICE_NAME[doorbell]="doorbell";
        DEVICE_NAME[onoffswitch]="onoffswitch";
};

void AT_process_event()
{
  
          const char *pFormat = "AT+%[^=]=%[^,],%[^,],%[^,],%[^,],%[^,] ";
          char cmd[8] = {0} ;
          char data1[16] = {0} ;
          char data2[8] = {0}  ;
          char data3[8] = {0} ;
          char data4[8] = {0} ;
          char data5[8] = {0} ;
          sscanf(at_status.DATA_buffer,pFormat ,cmd,data1,data2,data3,data4,data5 ) ;
          osal_memset(at_status.DATA_buffer,0,128);
          static uint8 allowBind=FALSE;
          static uint8 allowJoin=TRUE;
          uint8 logicalType;
          uint8 devicetype;
          uint8 channellist;
          uint16 panid;
          char b[]="V1.0" ; 
          char c[]="+OK\r\n";
          char sendbuffer[64];
          unsigned char index = IndexofAT(cmd);
	switch(index)
	{
        
		case ENUM_AT_MAC:
                  {
//                      zmain_dev_info();
                      //uint8 pValue[16];
                     // osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
                      uint8 i;
                      uint8 *xad;
                      uint8 buf[Z_EXTADDR_LEN*2+1];

                      // Display the extended address.
                      xad = aExtendedAddress + Z_EXTADDR_LEN - 1;

                      for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
                      {
                        uint8 ch;
                        ch = (*xad >> 4) & 0x0F;
                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                        ch = *xad & 0x0F;
                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                      }
 //                     zb_ReadConfiguration(ZCD_NV_EXTADDR, sizeof(uint8), &logicalType);

//                      HalFlashRead(PAGE_OF_MAC_ADDR,OSET_OF_MAC_ADDR,devmacaddr,LEN_OF_MAC_ADDR);
                      //sprintf(sendbuffer,"+OK=%x%x%x%x%x%x%x%x %x\r\n",pValue[0],pValue[1],pValue[2],pValue[3],pValue[4],pValue[5],pValue[6],pValue[7],_NIB.nwkDevAddress);
                      HalUARTWrite(HAL_UART_PORT_0,buf,sizeof(buf)); 
//                zb_GetDeviceInfo(ZB_INFO_PARENT_SHORT_ADDR,&short_addr);
                  }
		break;
		case ENUM_AT_FMVER:
                      sprintf(at_status.DATA_buffer,"+OK=%s\r\n",b);
                      HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(b)+6);  
                      //osal_memset(sendbuffer,0,16);
		break;
	
                case ENUM_AT_SAVE:
                      zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &at_status.AT_logicalType);
                      osal_nv_item_init(ZCD_NV_DEVICE_TYPE,sizeof(uint8),NULL);
                      zb_WriteConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &at_status.AT_devicetype);
                      HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
		break;
                
                case ENUM_AT_FACTORY:
                      HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                      DelayMS(1000);
                      uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
                      zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                      Onboard_soft_reset();
                break;
                                
                                
                 case ENUM_AT_REBOOT:
                      HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                      DelayMS(1000);
                      zb_SystemReset();
                 break;
                
                 case ENUM_AT_ECHO:
                      if(strcmp(data1,"")!=0)
                      {
                          HalUARTWrite(HAL_UART_PORT_0,"ECHO",sizeof("ECHO")); 
                      }
                      else
                      {
                          sprintf(at_status.DATA_buffer,"+OK=%d,%d,%d,%d,%d\r\n",1,2 ,3 ,4,8);
                          HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer)); 
                          osal_memset(sendbuffer,0,sizeof(at_status.DATA_buffer));
                      }
                break;
                
                case ENUM_AT_UART:
                      if(strcmp(data1,"")!=0)
                      {
                          HalUARTWrite(HAL_UART_PORT_0,"UART",sizeof("UART")); 
                      }
                      else
                      {
                          sprintf(at_status.DATA_buffer,"+OK=%s,%s,%s,%s,%s\r\n","38400","8" ,"NONE" ,"1","NONE");  
                          //sprintf(sendbuffer,"+OK=%d,%d,%d,%d,%d\r\n",SBP_UART_BR,HAL_UART_8_BITS_PER_CHAR ,HAL_UART_NO_PARITY ,HAL_UART_ONE_STOP_BIT,SBP_UART_FC);
                          HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer)); 
                          osal_memset(sendbuffer,0,sizeof(at_status.DATA_buffer));
                      }  

                break;       

                case ENUM_AT_DEVICE:

                  
//                   zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
 //                  sprintf(sendbuffer,"+OK=%s\r\n",logicalType);  
//                    HalUARTWrite(HAL_UART_PORT_0,sendbuffer,sizeof(sendbuffer)); 

                      if(strcmp(data1,"gateway")==0)
                      {                  
                              at_status.AT_logicalType = ZG_DEVICETYPE_COORDINATOR;
//                              zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &at_status.AT_logicalType);
//                              zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                              //                           uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
                              //                           zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                              at_status.AT_devicetype = ZG_DEVICE_TYPE_GATEWAY;
          //                    zb_WriteConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                              HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c));      
                              // Reset the device with new configuration
                              //zb_SystemReset();
                      }
                      else if(strcmp(data1,"light")==0)
                      {
                          at_status.AT_logicalType = ZG_DEVICETYPE_ENDDEVICE;
                          at_status.AT_devicetype=ZG_DEVICE_TYPE_LIGHT;
//                          zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                          HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                      }
                      else if(strcmp(data1,"router")==0)
                      {   
                              at_status.AT_logicalType = ZG_DEVICETYPE_ROUTER;
                              at_status.AT_devicetype=ZG_DEVICE_TYPE_ROUTER; 
//                              zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
//                              zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                              //    zb_SystemReset();
//                              zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
//                              uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
//                              zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                              HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c));   
                              // Reset the device with new configuration
                              // zb_SystemReset();
                      }
                      else if(strcmp(data1,"onoffswitch")==0)
                      {   
 
                              at_status.AT_logicalType = ZG_DEVICETYPE_ENDDEVICE;
                              at_status.AT_devicetype = ZG_DEVICE_TYPE_ONOFFSWITCH; 
                              HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c));   
                      }                      
                      else 
                      {
                          zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
 //                         if ( osal_nv_item_init(ZCD_NV_DEVICE_TYPE,sizeof(uint8),NULL) == ZSUCCESS )
                          zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                          if(logicalType==0)
                          {
                              sprintf(at_status.DATA_buffer,"+OK=%s\r\n","Gateway");
                              HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer)); 
                          }else if(logicalType==1&&devicetype==1)
                          {
                              sprintf(at_status.DATA_buffer,"+OK=%s\r\n","Router");
                              HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer)); 
                          }else if(logicalType==2)
                          {
                              sprintf(at_status.DATA_buffer,"+OK=%s\r\n",DEVICE_NAME[devicetype]);
                              HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer)); 
//                              osal_memset(sendbuffer,0,128);
                          }
                      }
               
                 break;
                
                case ENUM_AT_BINDING:
                    //zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
#if 0
                    allowBind ^= 1;
                    if (allowBind) 
                    {
                      // Turn ON Allow Bind mode infinitly
                      zb_AllowBind( 0xFF );
//                              HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
                      //This node is the gateway node
                      isGateWay = TRUE;

                     HalUARTWrite(HAL_UART_PORT_0,"GateWay",sizeof("sendbuffer")); 

                    }
                    else
                    {
                      // Turn OFF Allow Bind mode infinitly
                      zb_AllowBind( 0x00 );
//                             HalLedSet( HAL_LED_2, HAL_LED_MODE_OFF );
                      isGateWay = FALSE;
                      
                      // Update the display
                      #if defined ( LCD_SUPPORTED )
                      HalUARTWrite(HAL_UART_PORT_0,"router",sizeof("router")); 
//                              HalLcdWriteString( "Collector", HAL_LCD_LINE_2 );
                      #endif
                      }
                 

                    dstAddr.addr.shortAddr = NLME_GetCoordShortAddr();  //获取父节点地址
                    dstAddr.addrMode = Addr16Bit;                       //设置地址模式
                    if(bindAddEntry(ZHAtest_ENDPOINT,               //添加绑定表
                                    &dstAddr,
                                    ZHAtest_ENDPOINT,
                                    ZCLZHAtest_MAX_CLUSTERS,         
                                   (cId_t *)zclZHAtest_ClusterList))
                    {
                          HalUARTWrite(HAL_UART_PORT_0,a,sizeof(a));
                    }

                    if(strcmp(DEVICE_NAME[devicetype],"onoffswitch")==0){
                            dstAddr.addrMode = afAddr16Bit;
                            dstAddr.addr.shortAddr = 0;   // Coordinator makes the match
                            ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                           ZHAtest_ENDPOINT,
                           ZCL_HA_PROFILE_ID,
                           0, NULL,   // No incoming clusters to bind
                           ZCLZHAtest_BINDINGLIST, bindingClusters,
                           TRUE );
                    }
                    else if(strcmp(DEVICE_NAME[devicetype],"light")==0){
                          dstAddr.addrMode = afAddr16Bit;
                          dstAddr.addr.shortAddr = 0;   // Coordinator makes the match
                          ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                           ZHAtest_ENDPOINT,
                           ZCL_HA_PROFILE_ID,
                           ZCLZHAtest_BINDINGLIST, bindingClusters,
                           0, NULL,   // No Outgoing clusters to bind
                           TRUE );
                    
                    }
                      HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c));
                   // binding();

                    
                    
                        dstAddr.addrMode = AddrBroadcast;
                        dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
                        ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                           ZCL_HA_PROFILE_ID,
                           ZCLZHAtest_BINDINGLIST, bindingClusters,
                           0, NULL,   // No Outgoing clusters to bind
                           TRUE );
 #endif                  
 
                        dstAddr.addrMode = afAddr16Bit;
                        dstAddr.addr.shortAddr = 0;   // Coordinator makes the match
                        ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                                               data1[0],
                                               ZCL_HA_PROFILE_ID,
                                               ZCLZHAtest_BINDINGLIST, bindingClusters,
                                               ZCLZHAtest_BINDINGLIST, bindingClusters,
                                               TRUE );
                        
                     
                break;
         
                case ENUM_AT_VALUE:
        {
                    switch(data1[0])
                    {
                    case 0x31:
                    zclGeneral_SendOnOff_CmdToggle(1, &zclZHAtest_DstAddr, false, 0 );
                          break;
                    case 0x43:
                    zclGeneral_SendOnOff_CmdToggle(13, &zclZHAtest_DstAddr, false, 0 );
                      break;
                    }
              
        }
                  break;
#if 0 
                    if (devicetype!=0&&devicetype!=1)
                    {
//                        devicetp=Indexofdevice(devicetype);
                        switch(devicetype)

                            case light:

                                if(strcmp(data1,"1")==0)
                                {
                                    zclGeneral_SendOnOff_CmdOn( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr, false, 0 );
                                    zclZHAtest_OnOff=LIGHT_ON;
                                    HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                                }
                                else if(strcmp(data1,"0")==0)
                                {
                                    zclGeneral_SendOnOff_CmdOff(ZHAtest_ENDPOINT, &zclZHAtest_DstAddr, false, 0);
                                    zclZHAtest_OnOff=LIGHT_OFF;
                                    HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                                }
                                else 
                                {
                                    if(zclZHAtest_OnOff==LIGHT_ON)
                                    {
                                        sprintf(sendbuffer,"+OK=%s\r\n", "1"); 
                                        HalUARTWrite(HAL_UART_PORT_0,sendbuffer,7); 
                                    }else if(zclZHAtest_OnOff==LIGHT_OFF)
                                    {
                                        sprintf(sendbuffer,"+OK=%s\r\n", "0"); 
                                        HalUARTWrite(HAL_UART_PORT_0,sendbuffer,7); 
                                    }

                                }
                                if ( zclZHAtest_OnOff == LIGHT_ON )
                                    HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
                                else
                                    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
                                break;
                                
                                
                                
                                
                    }
                    else if(devicetype==0)
                    {
                        if(strcmp(data1,"1")==0||strcmp(data1,"0")==0)
                        {
                            sprintf(sendbuffer,"ERROR:=%s\r\n", "The device is Gateway"); 
                            HalUARTWrite(HAL_UART_PORT_0,sendbuffer,28); 
                        }
                        else 
                        {
                            if(zclZHAtest_OnOff==LIGHT_ON)
                            {
                                sprintf(sendbuffer,"+OK=%s\r\n", "1"); 
                                HalUARTWrite(HAL_UART_PORT_0,sendbuffer,7); 
                            }else if(zclZHAtest_OnOff==LIGHT_OFF)
                            {
                                sprintf(sendbuffer,"+OK=%s\r\n", "0"); 
                                HalUARTWrite(HAL_UART_PORT_0,sendbuffer,7); 
                            }

                        }
                        if ( zclZHAtest_OnOff == LIGHT_ON )
                            HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
                        else
                            HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );

                    }else if(devicetype==1)
                    {
                        if(strcmp(data1,"1")==0||strcmp(data1,"0")==0)
                        {
                            sprintf(sendbuffer,"ERROR:=%s\r\n", "The device is Router"); 
                            HalUARTWrite(HAL_UART_PORT_0,sendbuffer,27); 
                        }
                    }
                      
                    
                break;   
#endif            
                case ENUM_AT_LIST:
#if 0
                   if(strcmp(data1,"DEL")==0)
                    {
                           zb_BindDevice(FALSE,ZCL_CLUSTER_ID_GEN_ON_OFF, (uint8 *)data2);
                           HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                    }else if(strcmp(data1,"ADD")==0)
                    {
                           zb_BindDevice(TRUE,ZCL_CLUSTER_ID_GEN_ON_OFF, (uint8 *)data2);
                           HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                    
                    }else
                    {
                      
//                          deviceinfo();
                    
                    }
#endif
 //                       deviceinfo();
//                      strcat( a,AssociatedDevList[NWK_MAX_DEVICES].shortAddr);  
                        sprintf(sendbuffer,"+OK=%x,%x,%x\r\n",AssociatedDevList[0].shortAddr,AssociatedDevList[0].nodeRelation,
                        AssociatedDevList[0].devStatus );
//                        sprintf(sendbuffer,"+OK=%x,%x,%x\r\n",AssociatedDevList[1].shortAddr,AssociatedDevList[1].nodeRelation,
//                        AssociatedDevList[1].devStatus );
 //                     sprintf(sendbuffer,"+OK=%x\r\n",bindAddrIndexGet(dstAddr));
                        HalUARTWrite(HAL_UART_PORT_0,sendbuffer,32); //sizeof(AssociatedDevList[0].shortAddr)+sizeof(AssociatedDevList[0].nodeRelation)+sizeof(AssociatedDevList[0].devStatus)
                        osal_memset(sendbuffer,0,32);
//                        DelayMS(200);
                      

                break;    
                
                case ENUM_AT_CHECK:
          
                break; 
                
                case ENUM_AT_SETDEVICE:
                  switch(data1[0])
                  {
                    case 0:
                       zclGeneral_SendLevelControlMoveToLevel(13, &zclZHAtest_DstAddr,data1[1],10,  false, 0);
                     break;
                    case 1:
                       // zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 13, &zclZHAtest_DstAddr,10, 10,   false, 0 );
                        zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 13, &zclZHAtest_DstAddr,data1[1], 10,   false, 0 );
                     break;
                    case 2:
                        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(13, &zclZHAtest_DstAddr,data1[1],10,  false, 0);
                    break;
                  case 3:
                    zcl_SendCommand( 13, &zclZHAtest_DstAddr, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                          ATTRID_MS_TEMPERATURE_MEASURED_VALUE, TRUE, 
                          ZCL_FRAME_SERVER_CLIENT_DIR, false, 0, 0, 3, data1 );
                    break;
                  case 4:
                    zcl_SendCommand( 13, &zclZHAtest_DstAddr, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
                          ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE, TRUE, 
                          ZCL_FRAME_SERVER_CLIENT_DIR, false, 0, 0, 3, data1 );
                    break;
                  case 5:
                                  zclReadCmd_t BasicAttrsList;
                          BasicAttrsList.numAttr = 1;
                          BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
                          //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                        zcl_SendRead( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr,
                                        ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                    break;
                  case 6:
                    zclGeneral_SendOnOff_CmdToggle(1, &zclZHAtest_DstAddr, false, 0 );
                    break;
                    
                  }
                  HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
#if 0
                    if(zclZHAtest_NwkState == DEV_ZB_COORD)
                    {
                        unsigned char indexDE = Indexofdevice(data2);
                        switch(indexDE)

                          case light:
                  
                           if(strcmp(data3,"1")==0)
                          {
                                 zclGeneral_SendOnOff_CmdOn( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr,false, 0 );
                                 zclZHAtest_OnOff=LIGHT_ON;
                                 HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                          }
                          else if(strcmp(data3,"0")==0)
                          {
                                   zclGeneral_SendOnOff_CmdOff(ZHAtest_ENDPOINT, &zclZHAtest_DstAddr, false, 0);
                                   zclZHAtest_OnOff=LIGHT_OFF;
                                   HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
                          }
                          
                    }else
                    {
                      
                      sprintf(sendbuffer,"ERROR:%s\r\n", "Not a gateway");    
                      HalUARTWrite(HAL_UART_PORT_0,sendbuffer,19);                    
                    }
#endif
                break; 
                
                default:
                   sprintf(at_status.DATA_buffer,"ERROR:=%s\r\n","COMMAND ERROR");
                   HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
                break;
//zb_WriteConfiguration
                
                
                
        case ENUM_AT_VER:
          
          HalUARTWrite(HAL_UART_PORT_0,"RY COO V1.0",sizeof("RY COO V1.0"));
                   
          break;
        case ENUM_AT_RESET:
          HalUARTWrite(HAL_UART_PORT_0,"Reset in 6 seconds...",sizeof("Reset in 6 seconds..."));
//          Delay(6000);
          
          break;
        case ENUM_AT_RTOKEN:
                    break;
        case ENUM_AT_FORM:
                    break;
        case ENUM_AT_LEAVE:
//          ZDP_MgmtLeaveReq();
                    break;
        case ENUM_AT_PERMITJOIN:
                    break;
        case ENUM_AT_MTOGAP:
                    break;
        case ENUM_AT_BOOST:
                    break;
        case ENUM_AT_EXTPA:
                    break;
        case ENUM_AT_USEUFL:
                    break;
        case ENUM_AT_SETUART:
          HalUARTWrite(0,"UART configured",sizeof("UART configured"));
          break;
        case ENUM_AT_SETCH:
          channellist=data1[0];
          zb_WriteConfiguration(ZCD_NV_CHANLIST, 4, &channellist);
          HalUARTWrite(0,"Channel (frequency) configured",sizeof("Channel (frequency) configured"));
          break;
        case ENUM_AT_SCANCHMASK:
          break;
        case ENUM_AT_SETPID:
            panid=(uint16)data1;
//            _NIB.nwkPanId = panid;
//            NLME_UpdateNV(0x01);

          zb_WriteConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
          HalUARTWrite(0,"PAN ID configured",sizeof("PAN ID configured"));
//          zb_SystemReset();
          break;
        case ENUM_AT_SETPWR:
          HalUARTWrite(0,"TX power configured",sizeof("TX power configured"));
          break;
        case ENUM_AT_SETPWRMODE:
          HalUARTWrite(0,"Power mode configured",sizeof("Power mode configured"));
          break;          

        case ENUM_AT_GETCFG:
          
                    break;
        case ENUM_AT_GETINFO:
          zb_ReadConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
          sprintf(at_status.DATA_buffer,"%x",panid);
          HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
//                               MT_UtilSetPanID();  
 //       
//
          break;
	}
};

                                    

void NWK_CPMMAND_precess_event()
{
   NWK_COMMAND_BASIC_t *p;
   uint16 IDX_CMD;
   NWK_command.NWKCB_Header=at_status.DATA_buffer[0];
   NWK_command.NWKCB_Length=at_status.DATA_buffer[1];
   NWK_command.NWKCB_Footer=0x23;
   osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_FrameControl,at_status.DATA_buffer+2,2);
   osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_SourceAddress,at_status.DATA_buffer+10,8);
   osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,at_status.DATA_buffer+18,8);
   osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_CMDID,at_status.DATA_buffer+28,2);
   osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_GroupID,at_status.DATA_buffer+30,2);
   osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index,at_status.DATA_buffer+32,2);
   NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex=at_status.DATA_buffer[34];
   NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Opt=at_status.DATA_buffer[35];
   NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Length=at_status.DATA_buffer[36];
   osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,at_status.DATA_buffer+36,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Length);
      
   IDX_CMD=((NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index[1]&0xFF) <<8)+NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index[0];

   switch(IDX_CMD)
  {
        case NODE_STATUS_Idx:
          {
            
            if(NWK_command.NWK_General_Frame.NWKCG_CMDID[1]==0x20)
            {
              osal_memcpy(Node_Status.aucSoftware_Version,zclZHAtest_DateCode,4);
              osal_memcpy(Node_Status.aucHardware_Version,zclZHAtest_HWRevision1,4);
              osal_memcpy(Node_Status.aucDev_Type,zclZHAtest_ModelId,6);
              osal_memcpy(Node_Status.auiSurport_Func,zclZHAtest_ManufacturerName,8);
              switch(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex)
                  case 0:
                      osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,&Node_Status,24);
                      Return_Message(24);
                    break;
                  case 1:
                      osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.aucHardware_Version,4);
                      Return_Message(4);
                    break;                                     
                  case 2:
                      osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.aucHardware_Version,4);
                      Return_Message(4);
                    break;
                  case 3:
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.aucDev_Type,6);
                    Return_Message(6);
                    break;
                  case 4:
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.auiSurport_Func,8);
                    Return_Message(8);
                    break;
              
            
            }
 //           HalUARTWrite(HAL_UART_PORT_0,"NODE_STATUS_Idx",sizeof("NODE_STATUS_Idx"));
          }
          break;
        case DATE_TIME_Idx:
          HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
          break;
        case NETWORK_PARAMETER_Cfg_Idx:
          break;
        case NETWORK_PARAMETER_Idx:
          
            uint8 pValue[8];
            osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
            osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,pValue,8);

            uint16 panid;
            zb_ReadConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[10]= panid & 0x00FF;
            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[11]= (panid & 0xFF00)>>8;
            
            zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, 8,  pValue);
            osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+11,pValue,8);
            uint8 pValue1[16];
            zb_ReadConfiguration(ZCD_NV_NWKKEY, 16, pValue);
             osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+19,pValue1,8);
            uint8 channellist;
            zb_ReadConfiguration(ZCD_NV_CHANLIST, sizeof(uint8), &channellist);
           NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[20]=channellist;
            Return_Message(20);
            uint8 profile;
            zb_ReadConfiguration(ZCD_NV_STACK_PROFILE, sizeof(uint8), &profile);
          //ZCD_NV_EXTENDED_PAN_ID
            //zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, sizeof(uint16),  &panid);
            uint16 short_addr;
            zb_GetDeviceInfo(ZB_INFO_PARENT_SHORT_ADDR,&short_addr);

                        

          break;
        case TRIGGER_PARAMETER_Idx:
          {
            switch(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex)
            {
            case ucForm:
              {
                  if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x01)
                  {
                    uint8 logicalType = ZG_DEVICETYPE_COORDINATOR;
                    zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                    ZDOInitDevice(0);
                    zb_StartRequest();                 
                    zb_SystemReset();
                  }else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x00)
                  {
                    DelayMS(1000);
                    uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
                    zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                    Onboard_soft_reset();
                  }else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x02)
                  {
                                
                  }
              }
              break;
            case ucPJoin:
              if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
              {
                NLME_PermitJoiningRequest(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]);
              }
            }
          }

          break;
        case FORWARD_PARAMETER_Idx:
          break;
        case NODE_INFO_Idx:
          
          break;
        case NEIGHBOR_INFO_Idx:
          break;
        case TEMPERATURE_Idx:
          
          break;
        case Light_status_Idx:
        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[1]==0x20)
        {
              zclReadCmd_t BasicAttrsList;
              BasicAttrsList.numAttr = 1;
              BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
              //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
            zcl_SendRead( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr,
                        ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                        ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0;
            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
            Return_Message(2);
        }
        else if(NWK_command.NWK_General_Frame.NWKCG_CMDID[1]==0x25)
        {
            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==2)
            {
                if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0)
                {
                    zclGeneral_SendOnOff_CmdOff( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr, false, 0 );
                    zclZHAtest_OnOff=LIGHT_OFF;
                }
                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==1)
                {  
                  zclGeneral_SendOnOff_CmdOn( ZHAtest_ENDPOINT, &zclZHAtest_DstAddr, false, 0 );
                  zclZHAtest_OnOff=LIGHT_ON;
                }
            }
        }
          break;    
        default:
          break;
  }


}
