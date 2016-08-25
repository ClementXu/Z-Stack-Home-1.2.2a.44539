 #include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "AT_command.h"
#include "onboard.h"
#include "sapi.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "ZDApp.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_lighting.h"
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
#define SOURCE_DATA_LENGTH 256
char *AT_GROUP[AT_GROUP_NUM]; 
extern uint8  zclZHAtest_OnOff;
extern uint8  zclZHAtest_Level_to_Level;
extern uint16 zclZHAtest_Alarm_Status;
extern uint16 zclZHAtest_Alarm_Type;
extern uint16 zclZHAtest_Humidity_Value;
extern uint16 zclZHAtest_Light_Color_Status;
extern int16 zclZHAtest_Temperature_Value;
extern uint8 zclZHAtest_WD_Duration;
extern uint8 zclZHAtest_Warning;
extern uint8 zclZHAtest_WD_SQUAWK;
extern uint16 zclZHAtest_HUE_Status;
extern uint16 zclZHAtest_Illumiance_Value;

DEVICE_STATUS_t DeviceStatus[5];
NODE_INFO_t AssoList[5];
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
uint8 TempDevice[170];



typedef enum
{
    ENUM_AT_MAC=1,
    ENUM_AT_FMVER,
    ENUM_AT_SAVE,
    ENUM_AT_FACTORY,
    ENUM_AT_REBOOT,
    ENUM_AT_DEVICE,
    ENUM_AT_ADDINFO,
    ENUM_AT_ADDSTATUS,
    ENUM_AT_DENYJOIN,
    ENUM_AT_ACCEPTJOIN,
    ENUM_AT_ONLINE,
    ENUM_AT_VER,
    ENUM_AT_RESET,
    ENUM_AT_FORM,
    ENUM_AT_LEAVE,
    ENUM_AT_PERMITJOIN,
    ENUM_AT_CHANNEL,
    ENUM_AT_PANID,
    ENUM_AT_GETEXTPID,
    ENUM_AT_REMOVEDEV,
}at_command;


void __CleanTempDevice()
{
    osal_memset(TempDevice,0,sizeof(TempDevice));
}

void Return_Message(uint8 length)
{
    uint8 pValue[8];
    if(at_status.checkOD==1)
    {
            __CleanTempDevice();
        //osal_memset(TempDevice,0,256);
        TempDevice[0]=NWK_command.NWKCB_Header;
        TempDevice[1]=length + 35;
        osal_memcpy(TempDevice+2,NWK_command.NWK_General_Frame.NWKCG_FrameControl,2);  
                                                 
        osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
        osal_memcpy(TempDevice+18,pValue,8);
        osal_memcpy(TempDevice+10,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);   
        osal_memcpy(TempDevice+28,NWK_command.NWK_General_Frame.NWKCG_CMDID,2);  
        TempDevice[29]=0x80;
        osal_memcpy(TempDevice+30,NWK_command.NWK_General_Frame.NWKCG_GroupID,2);
        osal_memcpy(TempDevice+32,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index,2);   
        TempDevice[34]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex;
        TempDevice[35]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Opt; 
        TempDevice[36]=length; 
        osal_memcpy(TempDevice+37,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,length);
        TempDevice[37+length]=0xFF;
        TempDevice[38+length]=NWK_command.NWKCB_Footer;  
        if(at_status.checkOD==1)
        {
           HalUARTWrite(HAL_UART_PORT_0,TempDevice,(39+length));
           at_status.checkOD=0;
        }
        osal_memset(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,0,128);
        __CleanTempDevice();
    }
   //DelayMS(10000);
//   osal_msg_deallocate(NWK_command);
   
}

void AT_Init()
{
    AT_GROUP[ENUM_AT_MAC]="MAC";
    AT_GROUP[ENUM_AT_FMVER]="FMVER";
    AT_GROUP[ENUM_AT_SAVE]="SAVE";
    AT_GROUP[ENUM_AT_FACTORY]="FACTORY";
    AT_GROUP[ENUM_AT_REBOOT]="REBOOT";
    AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
    AT_GROUP[ENUM_AT_DENYJOIN]="DENYJOIN";
    AT_GROUP[ENUM_AT_ADDINFO]="ADDINFO";
    AT_GROUP[ENUM_AT_ADDSTATUS]="ADDSTATUS";
    AT_GROUP[ENUM_AT_ACCEPTJOIN] = "ACCEPTJOIN";
    AT_GROUP[ENUM_AT_VER]="VER";
    AT_GROUP[ENUM_AT_RESET]="RESET";
    AT_GROUP[ENUM_AT_FORM]="FORM";
    AT_GROUP[ENUM_AT_LEAVE]="LEAVE";
    AT_GROUP[ENUM_AT_PERMITJOIN]="PERMITJOIN";
    AT_GROUP[ENUM_AT_CHANNEL]="CHANNEL";
    AT_GROUP[ENUM_AT_PANID]="PANID";
    AT_GROUP[ENUM_AT_ONLINE]="ONLINE";  
    AT_GROUP[ENUM_AT_GETEXTPID]="GETEXTPID";
    AT_GROUP[ENUM_AT_REMOVEDEV]="REMOVEDEV";
};

void bitProcess(uint32 source,uint32 *target)
{
   uint8 i;
   for(i=0;i<sizeof(target);i++)
      target[i]=(source>>i) &0x01;

}

unsigned char IndexofAT(char *data)
{
    unsigned char i=1;
    for(i=1;i<=AT_GROUP_NUM;i++)
    {
        if(strcmp(data,AT_GROUP[i])==0)
            return i;
    }
    return 0;
};

void SendDatatoComputer(char *p)
{
    HalUARTWrite(HAL_UART_PORT_0,p,strlen(p));
}

#if 0

void cleanDeviceList()
{
     osal_memset(&AssoList,0,5*sizeof(NODE_INFO_t)); 
     zb_WriteConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
     
}


void writeDeviceList(NODE_INFO_t *p)
{
    uint8 i,flag=0;
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(AssoList[i].uiNwk_Addr == 0 && flag==0)
        {
            osal_memcpy(&AssoList[i],p,sizeof(NODE_INFO_t));
            flag=1;
        }
    }
    zb_WriteConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );

}

void testDeviceList()
{
    NODE_INFO_t test;
    osal_memset(&test,0,sizeof(NODE_INFO_t));
    cleanDeviceList();
    
    test.ep=13;
    test.batteryValue=2;
    test.sensorType=3;
    test.uiNwk_Addr=1234;
    
    writeDeviceList(&test);
    writeDeviceList(&test);
    writeDeviceList(&test);


}

#endif

void sendDatatoComputer(getDeviceData *Setting)
{
    uint8 i=0,j=0;
    char format[200];
    osal_memset(format,0,sizeof(format));
    char valueSign[3];
    if(Setting->length==0)
    {
        sprintf(format,"+OK\r\n");
        HalUARTWrite(HAL_UART_PORT_0,format,strlen(format));
        return;
    }
    j = sprintf(format,"+OK=");
    
    if(Setting->type == VALUE_D)
    {
        strcpy(valueSign,"%d");
        for(i=0;i<Setting->length;i++)
        {
            if(i!=0)
            {
                strcat(format,",");
            }
            j += sprintf(format+j+i,valueSign,*(Setting->data[i]));
        }
    }
    else
    {
        strcpy(valueSign,"%s");
        for(i=0;i<Setting->length;i++)
        {
            if(i!=0)
            {
                strcat(format,",");
            }
            j += sprintf(format+j+i,valueSign,(char *)(Setting->data[i]));
        }        

    }
    strcat(format,"\r\n");     
    HalUARTWrite(HAL_UART_PORT_0,format,strlen(format));
    osal_memset(&Setting,0,sizeof(Setting));
}

//数据处理函数
void AnaDataProcess(getDeviceData *Setting)
{
    uint8 i;
    char AT_Format[32];
    osal_memset(AT_Format,0,32);
    if(Setting->type == VALUE_MIX)
        strcat(AT_Format,"%[^,],%[^,],%d,%d,%d");
    for(i=0;i<Setting->length;i++)
    {
        if(i==0)
        {
            if(Setting->type == VALUE_D)
                    strcat(AT_Format,"%d"); 
            else if(Setting->type == CHAR)
                    strcat(AT_Format,"%[^,]"); 
            else
                break;
        }
        else
        {
            if(Setting->type == VALUE_D)
                    strcat(AT_Format,",%d"); 
            else if(Setting->type == CHAR)
                    strcat(AT_Format,",%[^,]"); 
            else
                break;
        }
    }
    //清零发送缓存
    if(Setting->RecSign == 0)//strlen(Setting->RecData)==0
    {	//读取指令
        sendDatatoComputer(Setting);	
        //Setting->RecSign = 0; 
    }
    else
    {	//写入指令
        sscanf(Setting->RecData,AT_Format,Setting->data[0],Setting->data[1],Setting->data[2],Setting->data[3],Setting->data[4]) ;
        Setting->length = 0;
        //Setting->RecSign = 1;
        sendDatatoComputer(Setting);
    }
        
}

void _CheckOnOff(uint16 shortAddr, uint16 seq)
{
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=shortAddr;
    dstAddr.endPoint=1;
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
}

void _CheckTemp(uint16 shortAddr, uint16 seq)
{
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=shortAddr;
    dstAddr.endPoint=1;
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_MS_TEMPERATURE_MEASURED_VALUE;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
}

void _CheckLevel(uint16 shortAddr , uint16 seq)
{
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=shortAddr;
    dstAddr.endPoint=1;
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
    //DelayMS(50);
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
}

void _CheckColorTemp(uint16 shortAddr , uint16 seq)
{
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=shortAddr;
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
    //DelayMS(50);
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
    //DelayMS(50);
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
    
    
}

void _CheckZoneStatus(uint16 shortAddr , uint16 seq)
{
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList;
    dstAddr.addrMode=afAddr16Bit;
    dstAddr.addr.shortAddr=shortAddr;
    BasicAttrsList.numAttr = 1;
    BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
    zcl_SendRead( 1, &dstAddr,
                    ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, seq);
    
    
}

void CheckDeviceStatus()
{
    //1.遍历硬盘表中的设备类型，将硬盘里有的设备的短地址和设备OD放入到设备状态表中
    //2.根据设备类型和短地址发送相应的查询信息
    uint8 i,k=0;
    afAddrType_t dstAddr;
    zclReadCmd_t BasicAttrsList[3];
    uint16 clusterID[3];
    osal_memset(&BasicAttrsList,0,3*sizeof(zclReadCmd_t));
    osal_memset(&clusterID,0,3*sizeof(uint16));
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(AssoList[i].uiNwk_Addr!=0 && AssoList[i].uiNwk_Addr !=0xffff)
        {
            DeviceStatus[k].uiNwk_Addr = AssoList[i].uiNwk_Addr;
            DeviceStatus[k].supportOD = AssoList[i].supportOD;
            k++;
        }
    }
    for(k=0;k<5;k++)
    {
        if(DeviceStatus[k].uiNwk_Addr!=0 && DeviceStatus[k].uiNwk_Addr !=0xffff)
        {
            dstAddr.addrMode=afAddr16Bit;
            dstAddr.addr.shortAddr=DeviceStatus[k].uiNwk_Addr;
            dstAddr.endPoint=1;
            switch(DeviceStatus[k].supportOD)
            {
                case 0x2968:
                    //_CheckTemp(DeviceStatus[k].uiNwk_Addr , DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_MS_TEMPERATURE_MEASURED_VALUE;
                    clusterID[0] = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;
//                    BasicAttrsList[1].numAttr = 1;
//                    BasicAttrsList[1].attrID[0] = ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE;
//                    clusterID[1] = ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY;
                    break;  
                case 0x2BC1:
                    //_CheckOnOff(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_ON_OFF;
                    clusterID[0] = ZCL_CLUSTER_ID_GEN_ON_OFF;
                    break; 
                case 0x2BC2:
                    //_CheckLevel(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_ON_OFF;
                    clusterID[0] = ZCL_CLUSTER_ID_GEN_ON_OFF;
                    BasicAttrsList[1].numAttr = 1;
                    BasicAttrsList[1].attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
                    clusterID[1] = ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL;
                    break; 
                case 0x2BD7: 
                    //_CheckColorTemp(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_ON_OFF;
                    clusterID[0] = ZCL_CLUSTER_ID_GEN_ON_OFF;
                    BasicAttrsList[1].numAttr = 1;
                    BasicAttrsList[1].attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
                    clusterID[1] = ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL;
                    BasicAttrsList[2].numAttr = 1;
                    BasicAttrsList[2].attrID[0] = ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE;
                    clusterID[2] = ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL;             
                    break; 
                case 0x2904:
                  //_CheckZoneStatus(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                    clusterID[0] = ZCL_CLUSTER_ID_SS_IAS_ZONE;
                    break; 
                case 0x29CC:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE;
                    clusterID[0] = ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY;
                    break;  
                case 0x2A30:
                  //_CheckZoneStatus(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                    clusterID[0] = ZCL_CLUSTER_ID_SS_IAS_ZONE;
                    break;    
                case 0x2A94:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_MS_ILLUMINANCE_MEASURED_VALUE;
                    clusterID[0] = ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT;
                    break; 
                case 0x2AF8:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 2;
                    BasicAttrsList[0].attrID[0] = COMMAND_SS_IAS_WD_START_WARNING;
                    BasicAttrsList[0].attrID[1] = COMMAND_SS_IAS_WD_SQUAWK;
                    clusterID[0] = ZCL_CLUSTER_ID_SS_IAS_WD;            
                    BasicAttrsList[1].numAttr = 1;
                    BasicAttrsList[1].attrID[0] = ATTRID_SS_IAS_WD_MAXIMUM_DURATION;
                    clusterID[1] = ZCL_CLUSTER_ID_SS_IAS_WD;                    
                    break; 
                case 0x2B5C:
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 2;
                    BasicAttrsList[0].attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                    BasicAttrsList[0].attrID[1] = ATTRID_SS_IAS_ZONE_TYPE;
                    clusterID[0] = ZCL_CLUSTER_ID_SS_IAS_ZONE;
                    break; 
                case 0x2BE1:
                    break;   
                case 0x2E00:
                  //_CheckZoneStatus(DeviceStatus[k].uiNwk_Addr, DeviceStatus[k].seq);
                    DeviceStatus[k].seq++;
                    BasicAttrsList[0].numAttr = 1;
                    BasicAttrsList[0].attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                    clusterID[0] = ZCL_CLUSTER_ID_SS_IAS_ZONE;                
                    break; 
                default:
                    break;
            }
            for(i=0;i<3;i++)
            {
                if(BasicAttrsList[i].numAttr !=0)
                {
                    zcl_SendRead( 1, &dstAddr,
                                clusterID[i], &BasicAttrsList[i],
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, DeviceStatus[k].seq);
                    //DelayMS(500);
                }
                
            }
        }   
    }
    //osal_memset(&BasicAttrsList,0,3*sizeof(zclReadCmd_t));
    //osal_start_timerEx(zclZHAtest_TaskID,DEVICE_STATUS_EVT,10000);
}

void UpdateDeviceStatus1(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[0]=data[0];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

void UpdateDeviceStatus2(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[1]=data[1];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

void UpdateDeviceStatus3(uint16 shortAddr, uint16 *data)
{
    uint8 i=0;
    for(i=0;i<5;i++)
    {
        if(DeviceStatus[i].uiNwk_Addr == shortAddr)
        {
            DeviceStatus[i].status[2]=data[2];
            //osal_memcpy(&DeviceStatus[i].status,&data,sizeof(DeviceStatus[i].status));
            return;
        }
              
    }
    
}

uint8 CovertACSIITWO(uint8 *buf)
{
    uint8 i;
    for(i=0;i<2;i++)
    {
        if(buf[i]<=57)
        {
            buf[i]=(buf[i]-48);
        }
        else if(buf[i]<=103 && buf[i]>=97)
        {
            buf[i]=(buf[i]-87);
        }
    }
    return ((buf[0]&0x0F)<<4)+(buf[1]&0x0F);


}

uint8 CovertACSIITWOB(uint8 *buf)
{
    uint8 i;
    for(i=0;i<2;i++)
    {
        if(buf[i]<=57)
        {
            buf[i]=(buf[i]-48);
        }
        else if(buf[i]<=71 && buf[i]>=65)
        {
            buf[i]=(buf[i]-55);
        }
    }
    return ((buf[0]&0x0F)<<4)+(buf[1]&0x0F);


}

void StringToHEX(uint8 *data ,uint8 *target,uint8 length)
{
      uint8 i;
      for(i=0;i<(length/2);i++)
      {
        target[length/2-i-1]=CovertACSIITWOB(data);
            data=data+2;
      }

}

void HEXtoString(uint8 *data , uint8 *targetBuf)
{
    uint8 i;
    uint8 *xad;
    //uint8 targetBuf[Z_EXTADDR_LEN*2+1]=0;
    // Display the extended address.
    xad = data + Z_EXTADDR_LEN - 1;

    for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
    {
            uint8 ch;
            ch = (*xad >> 4) & 0x0F;
            targetBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
            ch = *xad & 0x0F;
            targetBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    }      
    
}

uint16 CovertACSIIFOUR(uint8 *buf)
{
        uint8 i;
        for(i=0;i<4;i++)
        {
            if(buf[i]<=57)
            {
                buf[i]=(buf[i]-48);
            }
            else if(buf[i]<=103 && buf[i]>=97)
            {
                buf[i]=(buf[i]-87);
            }
        }
    return ((buf[0]&0x0F)<<12)+((buf[1]&0x0F)<<8)+((buf[2]&0x0F)<<4)+(buf[3]&0x0F);

}

void _BuildNet()
{
    uint8 logicalType;
    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        ZDOInitDevice(0);
        ZDO_StartDevice(ZG_DEVICETYPE_COORDINATOR,MODE_HARD,15, 15);
    }
}

uint8 _LeaveNet(uint8 *p)
{
    uint8 logicalType;
    NLME_LeaveReq_t req;
    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        //#NLME_LeaveReq_t req;
        req.extAddr = p;
        req.removeChildren = FALSE;
        req.rejoin         = FALSE;
        req.silent         = FALSE;
        if(NLME_LeaveReq(&req)==ZSuccess)
        {
            return 1;
        }
    }
    return 0;
}

void SetTempDeviceSA(uint16 data,uint8 *mac)
{   //保存短地址
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {   //审查身份
        if(p->device[i].uiNwk_Addr==data)
            return;
    }
    
    for(i=0;i<=5;i++)
    {   //找坑
        if(p->device[i].uiNwk_Addr==0)
        {
            p->device[i].uiNwk_Addr=data;
            osal_memcpy(p->device[i].aucMAC,mac,8);
            return;
        }
    }    
}

void SetTempDeviceHW(uint16 shortAddr,uint8 version)
{   //保存短地址
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].version = version;
            return;
        }
    }  
}

void SetTempDeviceEP(uint16 shortAddr,uint8 ep)
{   //保存短地址
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].ep = ep;
            return;
        }
    }  
}

void SetTempDeviceManuName(uint16 shortAddr,uint8 *buffer)
{   //保存短地址
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].factoryName,0,17);
            osal_memcpy(p->device[i].factoryName,buffer,16);
            return;
        }
    }  
}

void SetTempDeviceBAT(uint16 shortAddr,uint8 battery)
{   //保存短地址
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].batteryValue = battery;
            return;
        }
    }  
}

void _RemoveJoin()
{
    uint8 i=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<5;i++)
    {
        if(p->device[i].uiNwk_Addr !=0 && p->device[i].supportOD !=0)
            _LeaveNet(p->device[i].aucMAC);
    }
}

uint8 __CompareMac(uint8 *p, uint8 *mac)
{
    uint8 i=0;
    
    for(i=0;i<5;i++)
    {
        if(p[i] != mac[i])
            return 0;
    }
    return 1;
}

void _AcceptJoin(uint8 *data)
{
    uint8 i,k,flag=0,flag1=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    //NODE_INFO_Group *h=(NODE_INFO_Group *)buffer;
    uint8 buf[8];
    osal_memset(buf,0,8);
    //NODE_INFO_t buffer[5];
    //osal_memset(buffer,1,sizeof( NODE_INFO_t )*5);
    //HEXtoString(data,buf);
    //step.1 字符串转数值
    //step.2 往硬盘上写
        // 循环5次 找相等 ->覆盖
        //没找到 ->在第一个为0
    //step.3 在找到的位置写入数据
    StringToHEX(data,buf,16);
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(__CompareMac(buf,p->device[i].aucMAC)==1)
        {
            k=i;
            flag=1;
            break;
        }
    }
    if(flag==0)//内存无数据，上位机发送有误
        return;
    flag=0;
    for(i=0;i<5;i++)
    {
        if(__CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            osal_memcpy(&AssoList[i],&p->device[k],sizeof(NODE_INFO_t));
            osal_memset(&p->device[k],0,sizeof(NODE_INFO_t));
            zb_WriteConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
            return;
        }
        if(AssoList[i].uiNwk_Addr == 0 && flag==0)
        {
            flag1=i;
            flag=1;
        }
    }
    if(flag == 0)//硬盘无空间存储数据
        return;
    osal_memcpy(&AssoList[flag1],&p->device[k],sizeof(NODE_INFO_t));
    osal_memset(&p->device[k],0,sizeof(NODE_INFO_t)); 
    zb_WriteConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    //osal_start_timerEx(zclZHAtest_TaskID,DEVICE_STATUS_EVT,10000);

}

void _RemoveDevice(uint8 *data)
{
    uint8 i,k=0;
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    uint8 buf[8];
    osal_memset(buf,0,8);
    StringToHEX(data,buf,16);
    //NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<5;i++)
    {
        if(__CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            _LeaveNet(AssoList[i].aucMAC);
            osal_memset(&AssoList[i],0,sizeof(NODE_INFO_t));
        }  
    }
    zb_WriteConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
}

void _SetOnlineDeveice(uint8 *data)
{
    //1.查询mac地址是否在硬盘表中
    //2.如果存在，则将短地址设为目的地址
    //3.根据设备的OD来判断发送的指令
    uint8 i;
    uint8 buf[8];
    osal_memset(buf,0,8);
    afAddrType_t dstAddr;
    StringToHEX(&data[0],buf,16);
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(__CompareMac(buf,AssoList[i].aucMAC)==1)
        {
            dstAddr.addrMode=afAddr16Bit;
            dstAddr.addr.shortAddr=AssoList[i].uiNwk_Addr;
            dstAddr.endPoint=1;
            switch(AssoList[i].supportOD)
            {

                case 0x2BC1:
                    if(data[17]==0)
                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, false, 0 );
                    else if(data[17]==1)
                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, false, 0 );
                    break; 
                case 0x2BC2:
                    if(data[17]==0)
                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, false, 0 );
                    else if(data[17]==1)
                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, false, 0 );
                    zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,(data[18]*2.56),10,  false, 0);
                    break; 
                case 0x2BD7: 
                    if(data[17]==0)
                        zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, false, 0 );
                    else if(data[17]==1)
                        zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, false, 0 );
                    zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,(data[18]*2.56),10,  false, 0);
                    zclZHAtest_Light_Color_Status=((data[19] &0x00FF)<<8)+data[20];
                    zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 1, &dstAddr,zclZHAtest_Light_Color_Status, 10,   false, 0 );                  
                    break; 
                case 0x2AF8:
                    zclWriteCmd_t AttriList;
                    if(data[17]<=3)
                    {
                        zclZHAtest_Warning = data[17];
                        zclSS_Send_IAS_WD_StartWarningCmd(1, &dstAddr, zclZHAtest_Warning,10,  false, 0);
                    }
                    else if(data[17]>=4 & data[17]<6)
                    {
                        zclZHAtest_WD_SQUAWK = data[17]-4;
                        zclSS_Send_IAS_WD_SquawkCmd(1, &dstAddr, zclZHAtest_WD_SQUAWK,10,  false, 0);
                    }
                    zclZHAtest_WD_Duration = data[18];
                    AttriList.numAttr=1;
                    AttriList.attrList[0].attrID=ATTRID_SS_IAS_WD_MAXIMUM_DURATION;
                    AttriList.attrList[0].dataType=ZCL_DATATYPE_UINT8;
                    AttriList.attrList[0].attrData = &zclZHAtest_WD_Duration;
                    zcl_SendWrite(1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_WD, &AttriList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                    break;
              
                default:
                    break;                
            
            }
        }

    }
}

void _GetOnlineDeveice()
{
    //1.查询在线设备
    //2.根据设备的序列号来索引相应的设备状态
    uint8 i,j=0,k=0;
    uint8 level;
    int num=0;
    uint8 sendBuffer[180];
    uint8 buf[17];
    osal_memset(sendBuffer,0,180);
    osal_memset(buf,0,17);
    afAddrType_t dstAddr;
    j = sprintf(sendBuffer,"+OK=");
    zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE,(sizeof( NODE_INFO_t )*5), &AssoList );
    for(i=0;i<5;i++)
    {
        if(AssoList[i].uiNwk_Addr!=0)
        {
            HEXtoString(AssoList[i].aucMAC,buf);
            if(k!=0)
                strcat(sendBuffer,";");
            switch(AssoList[i].supportOD)
            {
                case 0x2968:
                    zclZHAtest_Temperature_Value = (int)DeviceStatus[i].status[0];
                    num = (zclZHAtest_Temperature_Value/100);
                    if(abs(zclZHAtest_Temperature_Value- num*100) < 10)
                        j += sprintf(sendBuffer+j+k,"%s,%x,%d.0%d",buf,0x2968,num,abs(zclZHAtest_Temperature_Value- num*100));
                    else
                        j += sprintf(sendBuffer+j+k,"%s,%x,%d.%d",buf,0x2968,num,abs(zclZHAtest_Temperature_Value- num*100));
                    break;  
                case 0x2BC1:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d",buf,0x2BC1,DeviceStatus[i].status[0]);
                    break; 
                case 0x2BC2:
                    level = DeviceStatus[i].status[1]/2.55;
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,0x2BC2,DeviceStatus[i].status[0],level);
                    break; 
                case 0x2BD7: 
                    level = DeviceStatus[i].status[1]/2.55;
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d,%d",buf,0x2BD7,DeviceStatus[i].status[0],level,DeviceStatus[i].status[2]);
                    break; 
                case 0x2904:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,0x2904,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break; 
                case 0x29CC:
                    zclZHAtest_Humidity_Value = DeviceStatus[i].status[0];
                    num = (zclZHAtest_Humidity_Value/100);
                    if((zclZHAtest_Humidity_Value- num*100) < 10)
                        j += sprintf(sendBuffer+j+k,"%s,%x,%d.0%d",buf,0x29CC,num,zclZHAtest_Humidity_Value- num*100);
                    else
                        j += sprintf(sendBuffer+j+k,"%s,%x,%d.%d",buf,0x29CC,num,zclZHAtest_Humidity_Value- num*100);
                    break;  
                case 0x2A30:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,0x2A30,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break;    
                case 0x2A94:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d",buf,0x2A94,DeviceStatus[i].status[0]);
                    break; 
                case 0x2AF8:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,0x2AF8,(DeviceStatus[i].status[0]  + DeviceStatus[i].status[1]),DeviceStatus[i].status[2]); 
                    break; 
                case 0x2B5C:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d,%d",buf,0x2B5C,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1),DeviceStatus[i].status[1]);
                    break; 
                case 0x2BE1:
                    break;   
                case 0x2E00:
                    j += sprintf(sendBuffer+j+k,"%s,%x,%d,%d",buf,0x2E00,(DeviceStatus[i].status[0] & 0x01),((DeviceStatus[i].status[0] & 0x02)>>1)); 
                    break; 
                default:
                    break;
            }
            k++;
        }

    }
    strcat(sendBuffer,"\r\n");
    HalUARTWrite(HAL_UART_PORT_0,sendBuffer,strlen(sendBuffer));
}

void _OnlineDeveice(getDeviceData Setting)
{
    char AT_Format[]="%[^,],%d,%d,%d";
    uint8 buffer[36];
    uint8 i,sign=0;
    osal_memset(buffer,0,36);
    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == ',')
            {
                sign = 1;
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(sign == 1)
        {
            sscanf(Setting.RecData,AT_Format,&buffer[0],&buffer[17],&buffer[18],&buffer[19]);
            _SetOnlineDeveice(buffer);
            HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        }
    }else
    {
        _GetOnlineDeveice();
    }


}

void SetTempDeviceOD(uint16 shortAddr,uint16 change,uint8 ep)
{   //保存短地址
    uint8 i;
    uint16 supportOD=0;
    uint8 sensorType=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    switch(change)
    {
        case 0x000d:
            sensorType=4;
            supportOD=PIR_Status_Idx;
        break;
        case 0x0015:
            sensorType=1;
            supportOD=Door_Sensor_Idx;
        break;
        case 0x0028:
            sensorType=14;
            supportOD=Smoke_Detector_Idx;
        break;                                          
        case 0x002a:
            sensorType=3;
            supportOD=Over_Walter_Idx;
        break; 
        case 0x002b:
            sensorType=0;
            supportOD=Gas_Sensor_Idx;
        break;   
        case 0x0100: 
            sensorType=0;
            supportOD=Light_status_Idx;
            break;
        case 0x0101:
            sensorType=5;
            supportOD=0x2BC2;
            break;                            
        case 0x0102:
            sensorType=6;
            supportOD=Light_Temperature_Idx;
            break;  
        case 0x0226:
            sensorType=0;
            supportOD=Glass_Broken_Idx;      
            break; 
        case 0x0225:
            sensorType=9;
            supportOD=Sound_Light_Idx;      
            break; 
        case 0x0227:
            sensorType=0;
            supportOD=CO_Sensor_Idx;      
            break;             
        case 0x0302:
            supportOD=TEMPERATURE_Idx;
            sensorType=2;
            break;    
        case 0x0307:
            sensorType=15;
            supportOD=Humility_Idx;
            break;   
        case 0x0308:
            sensorType=13;
            supportOD=Illuminance_Idx;
            break;  
        case 0x0403:
            sensorType=9;
            supportOD=Sound_Light_Idx;
            break;   
        default:

            return;
    }
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            p->device[i].supportOD = supportOD;
            p->device[i].sensorType = sensorType;
            p->device[i].ep=ep;
            return;
        }
    }
    //while(1);
}

void _AllowJoin(uint8 time)
{
    uint8 i,logicalType;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType); 
    if(logicalType==0)
    {       
        for(i=0;i<=5;i++)
        {
            if(p->device[i].supportOD!=0)
                AssocRemove(p->device[i].aucMAC);
        }
        __CleanTempDevice();
        //DelayMS(100);
        NLME_PermitJoiningRequest(time);
    }    
}

void _SetChannel(uint32 channel)
{
    switch(channel)
    {
        case 0x0B:
                channel=0x800;
        break;
        case 0x0C:
                channel=0x00001000;
        break;
        case 0x0D:
                channel=0x00002000;
        break;
        case 0x0E:
                channel=0x00004000;
        break;
        case 0x0F:
                channel=0x00008000;
        break;
        case 0x10:
                channel=0x00010000;
        break;
        case 0x11:
                channel=0x00020000;
        break;
        case 0x12:
                channel=0x00040000;
        break;
        case 0x13:
                channel=0x00080000;
        break;
        case 0x14:
                channel=0x00100000;
        break;         
        case 0x15:
                channel=0x00200000;
        break;  
        case 0x16:
                channel=0x00400000;
        break;  
        case 0x17:
                channel=0x00800000;
        break;  
        case 0x18:
                channel=0x01000000;
        break;  
        case 0x19:
                channel=0x02000000;
        break;  
        case 0x1A:
                channel=0x04000000;
        break;  
        default:
                channel=0x800;
        break;
    }
    zb_WriteConfiguration(ZCD_NV_CHANLIST, 4, &channel);
}

uint32 _GetChannel()
{
    uint8 i;
    uint32 channel=0;
    uint32 channellist;
    zb_ReadConfiguration(ZCD_NV_CHANLIST, sizeof(uint32), &channellist);
    for(i=0;i<32;i++)
    {
        channellist=channellist>>1;
        if(channellist&&0x01==1)
              channel ++;
    }
    return channel;
}

typedef struct __addInfo
{
    char mac[17];
    char name[17];
}addInfo;
void _AddInfo()
{
    uint8 i=0,j=0,k=0;
    char buffer[256];
    osal_memset(buffer,0,256);
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    addInfo    *s=(addInfo *)&buffer[4];
    strcat(buffer,"+OK=");
    for(j=0;j<5;j++)
    {
        for(i=0;i<5;i++)
        {
            if(p->device[i].uiNwk_Addr==AssociatedDevList[j].shortAddr && p->device[i].uiNwk_Addr !=0 && p->device[i].uiNwk_Addr !=0xFFFF)
            {
                if(k!=0)
                    strcat(buffer,";");
                s=(addInfo *)&buffer[strlen(buffer)];
                HEXtoString(p->device[i].aucMAC,(uint8 *)(s)->mac);
                strcat( buffer,",");
                osal_memcpy(&s->name,&p->device[i].factoryName[1],16);
                //strcat( buffer,",00");
                //osal_memcpy(s->sel,s->mac,16);
                k++;
            }                   
        }
    }
    strcat(buffer,"\r\n"); 
    SendDatatoComputer(buffer);
}

typedef struct __addStatus
{
    char mac[17];
    char deviceName[17];
    uint8 batteryValue[2];
    uint8 version[4];
    uint8 sensorType[2];
    uint8 time[2];
    
    
}addStatus;

void _AddStatus()
{
    /*
    example:+OK=sel,deviceName,batteryValue,version,sensorType,0x31
    */
    uint8 i=0,j=0,k=0;
    uint8 buffer[256];
    osal_memset(buffer,0,256);
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    addStatus    *s=(addStatus *)&buffer[4];
    strcat(buffer,"+OK=");
    uint8 buf[Z_EXTADDR_LEN*2+1];
    osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
 
    for(j=0;j<5;j++)
    {
        for(i=0;i<5;i++)
        {
            if(p->device[i].uiNwk_Addr==AssociatedDevList[j].shortAddr && p->device[i].uiNwk_Addr !=0 && p->device[i].uiNwk_Addr !=0xFFFF)
            {
                if(k!=0)
                    strcat(buffer,";");
                s=(addStatus *)&buffer[strlen(buffer)];
                HEXtoString(p->device[i].aucMAC,(uint8 *)(s)->mac);
                strcat( buffer,",");
                strcat( buffer,"TestDevice");
                strcat(buffer,",");
                //strcat(buffer,"3");
                sprintf(&buffer[strlen(buffer)],"%d",p->device[i].batteryValue);
                strcat(buffer,",");
                strcat( buffer,"1.5");
                strcat(buffer,",");
                sprintf(&buffer[strlen(buffer)],"%d",p->device[i].sensorType);
                strcat(buffer,",");
                strcat(buffer,"1");
                k++;
            }                   
        }
    }
//    for(j=0;j<5;j++)
//    {
//        for(i=0;i<5;i++)
//        {
//            if(p->device[i].uiNwk_Addr==AssociatedDevList[j].shortAddr && p->device[i].uiNwk_Addr !=0 && p->device[i].uiNwk_Addr !=0xFFFF)
//            {
//                if(i!=0)
//                {
//                    strcat(buffer,";");
//                }
//                strcat( buffer,"00");
//                //osal_memcpy(data1,p->device[i].showMAC,16);
//                //data1[16]=0x00;
//                HEXtoString(p->device[i].aucMAC,buf); 
//                strcat( buffer,buf);
//                strcat(buffer,",");
//                //strcat( at_status.DATA_buffer,Node_Info[j].deviceName);
//                strcat( buffer,"TestDevice");
//                strcat(buffer,",");
//                //at_status.DATA_buffer[strlen(at_status.DATA_buffer)+1] = Node_Info[j].batteryValue;
//                buffer[strlen(buffer)] = 0x33;
//                //strcat( at_status.DATA_buffer,Node_Info[j].batteryValue);
//                strcat(buffer,",");
//                strcat( buffer,"1.5");
//                //strcat( at_status.DATA_buffer,Node_Info[j].version);
//                strcat(buffer,",");
//                sprintf(&buffer[strlen(buffer)],"%d",p->device[i].sensorType);
//                //at_status.DATA_buffer[strlen(at_status.DATA_buffer)] = Node_Info[j].sensorType;
//                //strcat( at_status.DATA_buffer,Node_Info[j].sensorType);
//                strcat(buffer,",");
//                buffer[strlen(buffer)] = 0x31;
//            }                   
//        }
//    }
    strcat(buffer,"\r\n"); 
    SendDatatoComputer(buffer);
}

void AT_process_event(char *recData,uint16 length)
{
	uint8 logicalType;
	uint8 devicetype;
    afAddrType_t dstAddr;
    

	char AT_Format[]="AT+%[^=]";
	char AT_Name[10];
	at_command Index;
    uint32 sendData[5];
	getDeviceData Setting;//解析器设置

	//第一步，读取AT指令名称
	if(sscanf(recData,AT_Format ,AT_Name)!=-1)
	{	//解析成功
		Index = IndexofAT(AT_Name);//给赋值
		if(recData[strlen(AT_Name)+3] == '=')
		{//设置指令
            Setting.RecData = 0;
            Setting.RecData = &recData[strlen(AT_Name)+4];//
            Setting.RecSign = 1;
		}
		else
		{	//读取指令
            osal_memset(recData,0,SBP_UART_RX_BUF_SIZE);
            Setting.RecData = &recData[0];
            Setting.RecSign = 0;  
		}
	}
	else
	{	//解析失败
		Index = 0;//返回错误
	}
	
	//第二步具体解析指令	
	switch(Index)
	{
        //获取MAC地址
        case ENUM_AT_MAC:
        {
            uint8 buf[Z_EXTADDR_LEN*2+1];
            osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
            HEXtoString(aExtendedAddress,buf);
            Setting.data[0] =(uint32 *)&buf;
            Setting.length = 1;
            Setting.type = CHAR;
            AnaDataProcess(&Setting);
        break;
        }
        case ENUM_AT_FMVER:
        {
            uint8 buffer[16];
            osal_memset(buffer,0,16);
            zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint16),  &logicalType);
            if(logicalType!=0)
            {
                sprintf(buffer,"+OK=TESTDEVICE V1.8\r\n");
                SendDatatoComputer(buffer);  
            }
            else if(logicalType==0)
            {
                sprintf(buffer,"+OK=GATEWAY V1.8\r\n");
                SendDatatoComputer(buffer);
            }
        break;
        }
        //恢复出厂设置
        case ENUM_AT_FACTORY:
        {
            uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
            zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
            Onboard_soft_reset();				
        break;
        }
        //重启设备（设备信息不丢失）              
        case ENUM_AT_REBOOT:
        {
            Onboard_soft_reset();
        break;
        }
        //建立网络     
        case ENUM_AT_FORM:
        {
            _BuildNet();
            Setting.length = 0;
            AnaDataProcess(&Setting);
        break;
        }
        //离网
        case ENUM_AT_LEAVE:
        {
            uint8 buffer[8];
            osal_memset(buffer,0,8);
            if(_LeaveNet(buffer))
            {
                HalUARTWrite(HAL_UART_PORT_0,"+OK",3);
                break;
            }
            HalUARTWrite(HAL_UART_PORT_0,"+ERROR",strlen("+ERROR"));
            break;
        }
        //允许设备入网
        case ENUM_AT_PERMITJOIN:
        {
            uint8 jointime;
            Setting.data[0] =(uint32 *)&jointime;
            Setting.length = 1;
            Setting.type = VALUE_D;
            AnaDataProcess(&Setting);
            _AllowJoin(jointime);
            break;
         }
         //设置频道
        case ENUM_AT_CHANNEL:
        {
            uint32 channel=_GetChannel();
            Setting.data[0] =&channel;
            Setting.length = 1;
            Setting.type = VALUE_D;
            AnaDataProcess(&Setting);
            _SetChannel(channel);
        break;
        }
        //设置PID
        case ENUM_AT_PANID:
        {
            //读取当前状态
            uint32 AutoSign=0;//0 auto
            uint16 panid;
            zb_ReadConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);//硬盘上的数据
            if(panid==_NIB.nwkPanId)
            {
                AutoSign=1;
            }
            panid = _NIB.nwkPanId;
            Setting.data[0] = &AutoSign;
            Setting.data[1] =(uint32 *)&panid;
            Setting.length = 2;
            Setting.type = VALUE_D;
            AnaDataProcess(&Setting);
            if(panid==0||panid==0xffff)
                break;
            
            if(AutoSign==1)
            {
                zb_WriteConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
                _NIB.nwkPanId = panid;
                NLME_UpdateNV(0x01);
            }
            else
            {
                panid = 0xffff;
                zb_WriteConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
            }
        break;
        }
        //获取Extended PID
        case ENUM_AT_GETEXTPID:
        {
            uint8 zgExtendedPANID[8];
            osal_cpyExtAddr( zgExtendedPANID, _NIB.extendedPANID );
            uint8 buf[Z_EXTADDR_LEN*2+1];
            osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
            // Display the extended address.
            HEXtoString(zgExtendedPANID,buf);
     
            Setting.data[0] =(uint32 *)&buf[0];
            Setting.length = 1;
            Setting.type = CHAR;
            AnaDataProcess(&Setting);
         break;
         }
        //查看待加网设备的信息和状态
        case ENUM_AT_ADDINFO:
        {
            _AddInfo();
            break;
        }            
        //查看待加网设备的信息和状态
        case ENUM_AT_ADDSTATUS:
        {
            _AddStatus();
            break;
        }
        case ENUM_AT_ONLINE:
        {  
            _OnlineDeveice(Setting);
            break;
        }
        case ENUM_AT_DEVICE:
        {
            zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint16),  &logicalType);
            char buffer[16];
            osal_memset(buffer,0,16);
            strcpy(buffer,"gateway");
            Setting.data[0] =(uint32 *)&buffer[0];
            Setting.length = 1;
            Setting.type = CHAR;
            AnaDataProcess(&Setting);
        break;
        }
        case ENUM_AT_ACCEPTJOIN:
        {
            uint8 i;
            uint8 mac[5][17];
            osal_memset(mac,0,sizeof(mac));
            Setting.data[0] =(uint32 *)&mac[0][0];
            Setting.data[1] =(uint32 *)&mac[1][0];
            Setting.data[2] =(uint32 *)&mac[2][0];
            Setting.data[3] =(uint32 *)&mac[3][0];
            Setting.data[4] =(uint32 *)&mac[4][0];
            Setting.length = 5;
            Setting.type = CHAR;
            AnaDataProcess(&Setting);
            for(i=0;i<5;i++)
            {
                if(mac[i][0]==0x00)
                    break;
                _AcceptJoin(mac[i]);
            }
            _RemoveJoin();
        break;
        }             
        case ENUM_AT_REMOVEDEV:
        {
            uint8 i;
            uint8 mac[5][17];
            osal_memset(mac,0,sizeof(mac));
            Setting.data[0] =(uint32 *)&mac[0][0];
            Setting.data[1] =(uint32 *)&mac[1][0];
            Setting.data[2] =(uint32 *)&mac[2][0];
            Setting.data[3] =(uint32 *)&mac[3][0];
            Setting.data[4] =(uint32 *)&mac[4][0];
            Setting.length = 5;
            Setting.type = CHAR;
            AnaDataProcess(&Setting);
            for(i=0;i<5;i++)
            {
                if(mac[i][0]==0x00)
                    break;
                _RemoveDevice(mac[i]);
            }
        break;   
        } 
        default:
        {   
            uint8 buffer[16];
            osal_memset(buffer,0,16);
            sprintf(buffer,"ERROR:=%s\r\n","COMMAND ERROR");
            SendDatatoComputer(buffer);
        }
        break;
        }

        
};
void NWK_CPMMAND_precess_event(char *recData)
{
    uint16 IDX_CMD;
    uint8 i;
    uint8 j;
    zclReadCmd_t BasicAttrsList;
    uint8 logicalType;
    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        at_status.checkOD=1;
        NWK_command.NWKCB_Header=recData[0];
        NWK_command.NWKCB_Length=recData[1];
        NWK_command.NWKCB_Footer=0x23;
        osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_FrameControl,recData+2,2);
        osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_SourceAddress,recData+10,8);
        osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,recData+18,8);
        osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_CMDID,recData+28,2);
        osal_memcpy(NWK_command.NWK_General_Frame.NWKCG_GroupID,recData+30,2);
        osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index,recData+32,2);
        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex=recData[34];
        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Opt=recData[35];
        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Length=recData[36];
        osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,recData+37,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Length);
          
        IDX_CMD=((NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index[1]&0xFF) <<8)+NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index[0];

        switch(IDX_CMD)
        {
            case NODE_STATUS_Idx:
            {
                //版本信息
                if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                {
                    osal_memcpy(Node_Status.aucSoftware_Version,zclZHAtest_DateCode,4);
                    osal_memcpy(Node_Status.aucHardware_Version,zclZHAtest_HWRevision1,4);
                    osal_memcpy(Node_Status.aucDev_Type,zclZHAtest_ModelId,6);
                    osal_memcpy(Node_Status.auiSurport_Func,zclZHAtest_ManufacturerName,8);
                    switch(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex)
                    {
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
                }
            
            break;
            }
            case NETWORK_PARAMETER_Cfg_Idx:
            break;
            case NETWORK_PARAMETER_Idx:
            {
                if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                {
                    //MAC地址
                    uint8 pValue[8];
                    osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,pValue,8);
                    //获得panID
                    uint16 panid;
                    zb_ReadConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[10]= panid & 0x00FF;
                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[11]= (panid & 0xFF00)>>8;
                    //获得Extended panID
                    zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, 8,  pValue);
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+12,pValue,8);
                    //获取network KEY
                    nwkActiveKeyItems keyItems;
                    zb_ReadConfiguration(ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), &keyItems);
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+21,keyItems.active.key,16);
                    //获得Channel List
                    uint32 channellist;
                    zb_ReadConfiguration(ZCD_NV_CHANLIST, sizeof(uint32), &channellist);
                    for(int i=0;i<27;i++)
                    {
                        if(((channellist>>i)&0x0F)!=0)
                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[37]=i;

                    }
                    //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[35]=0x0B;
                    Return_Message(38);
                    //获得Profile ID
                    uint8 profile;
                    zb_ReadConfiguration(ZCD_NV_STACK_PROFILE, sizeof(uint8), &profile);
                    //ZCD_NV_EXTENDED_PAN_ID
                    //zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, sizeof(uint16),  &panid);
                    //获得短地址
                    uint16 short_addr;
                    zb_GetDeviceInfo(ZB_INFO_PARENT_SHORT_ADDR,&short_addr);
                }else
                {
                    //返回错误
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,NWK_command.NWK_General_Frame.NWKCG_CMDID,2);
                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[2]=0x70;
                    Return_Message(3);
                }
            break;
            }
            case TRIGGER_PARAMETER_Idx:
            {

                if(strcmp(NWK_command.NWK_General_Frame.NWKCG_SourceAddress,0x00)==0 &&strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)==0)
                {
                    switch(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex)
                    {
                        case ucForm:
                        {
                        //建立网络
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x01)
                            {
                                if(appState == APP_START)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                    Return_Message(1);
                                }else if(appState == APP_INIT)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                    Return_Message(1);
                                    //appState == APP_START;
                                    uint8 logicalType = ZG_DEVICETYPE_COORDINATOR;
                                    zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                                    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                                    ZDOInitDevice(0);
                                    ZDO_StartDevice(ZG_DEVICETYPE_COORDINATOR,MODE_HARD,15, 15);
                                    //zb_StartRequest();                 
                                    //zb_SystemReset();
                                }
                            }
                            //离开网络
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x00)
                            {
                                if(appState == APP_START)
                                {
                                    uint8 buffer[8];
                                    osal_memset(buffer,0,8);
                                    if(_LeaveNet(buffer))
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                        Return_Message(1);
                                    }
                                    else
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                        Return_Message(1);                                         
                                    }

                                }else if(appState == APP_INIT)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                    Return_Message(1);
                                }
                            }
                            //以默认配置建立网络
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x02)
                            {
                                if(appState == APP_START)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                    Return_Message(1);
                                }else if(appState == APP_INIT)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                    Return_Message(1);
                                    uint8 logicalType = ZG_DEVICETYPE_COORDINATOR;
                                    zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                                    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                                    ZDOInitDevice(0);
                                    zb_StartRequest();                 
                                    zb_SystemReset();
                                }
                            }
                        }
                        break;
                        case ucPJoin:
                            //允许设备加入网络
                            if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
                            {
                                _AllowJoin(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]);
                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                Return_Message(1);
                            }
                        break;
                        default:
                            //返回错误值
                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                            Return_Message(1);
                        break;
                    }
                }
                else
                {
                    //Target/source地址错误
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,NWK_command.NWK_General_Frame.NWKCG_SourceAddress,8);
                    osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+8,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[16]=0x70;
                    Return_Message(17);
                }
            break;	
            }

            case FORWARD_PARAMETER_Idx:
            break;
            case NODE_INFO_Idx:
            {
                //osal_start_timerEx(zclZHAtest_TaskID,DEVICE_STATUS_EVT,10000);
                //获取节点信息
                if(strcmp(NWK_command.NWK_General_Frame.NWKCG_SourceAddress,0x00)==0 &&strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)==0)
                {
                    if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                    {
                        if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==0)
                        {
                            osal_memset(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,0,80);
                            uint8 k=0;
                            for(j=0;j<6;j++)
                            {       
                                //if(at_status.acceptFlag==1)
                                //{
                                    if(AssociatedDevList[j].shortAddr!=0x0000 && AssociatedDevList[j].shortAddr!=0xffff)
                                    {
                                        zb_ReadConfiguration( ZCD_NV_DEVICE_TABLE, sizeof(NODE_INFO_t)*5, &AssoList );
                                        //if( APSME_LookupExtAddr(AssociatedDevList[j].shortAddr,AssoList[j].aucMAC ))
                                        for(i=0;i<5;i++)
                                        {
                                            if( AssociatedDevList[j].shortAddr == AssoList[i].uiNwk_Addr )
                                            {
                                                //osal_memset(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,0,90);
                                                osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+16*k,AssoList[i].aucMAC,8 );                             
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[8+16*k]=AssoList[i].uiNwk_Addr&0x00FF;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[9+16*k]=(AssoList[i].uiNwk_Addr&0xFF00)>>8;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[11+16*k]=0x01;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[12+16*k]=AssoList[i].supportOD&0x00FF;;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[13+16*k]=(AssoList[i].supportOD&0xFF00)>>8;;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[14+16*k]=0x00;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[15+16*k]=0x00;
                                                k++;
                                            }
                                        }
                                    }
                                //}
                            }
                            Return_Message(80);
                        }
                    }
                }
            break;
            }
            case NEIGHBOR_INFO_Idx:
            break;
            case TEMPERATURE_Idx:
            {
                at_status.checkOD=1;
                //温度状态获取
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //获取灯状态
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_MS_TEMPERATURE_MEASURED_VALUE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0;
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                                //Return_Message(2);
                            }
                        }
                    }
                }
            break;
            }         

            case Light_status_Idx:
            {
                at_status.checkOD=1;
                //灯控制及状态获取
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;

                        //获取灯状态
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==2)
                            {
                                dstAddr.addrMode=afAddr16Bit;
                                //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                                //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;

                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                            }else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {

                                dstAddr.addrMode=afAddr16Bit;
                                //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                                //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                                    
                                //ZDP_SimpleDescReq(&dstAddr,dstAddr.addr.shortAddr,1,0);

                            }

                        }
                        //发送控制灯指令
                        else if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
                        {
                            dstAddr.addrMode=afAddr16Bit;
                            //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                            //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                            dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                            dstAddr.endPoint=1;
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==2)
                            {
                                if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0)
                                {
                                    zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, false, 0 );
                                    //zclZHAtest_OnOff=LIGHT_OFF;
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==1)
                                {  
                                    zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, false, 0 );
                                    //zclZHAtest_OnOff=LIGHT_ON;
                                }
                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                                Return_Message(1);
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {
                                dstAddr.endPoint=1;
                                zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0);
                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                Return_Message(1);
                            }
                        }
                    }
                }
            break; 
            }

            case Light_Temperature_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {

                            BasicAttrsList.numAttr = 1;
                            BasicAttrsList.attrID[0] = ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE;
                            //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                            zcl_SendRead( 1, &dstAddr,
                            ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, &BasicAttrsList,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0;
                            //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                            //Return_Message(2);
                        }
                        //发送控制命令
                        else if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {
                                zclZHAtest_Light_Color_Status=((NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0] &0x00FF)<<8)+NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1];
                                if(zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 1, &dstAddr,zclZHAtest_Light_Color_Status, 10,   false, 0 )==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);
                                }
                            }
                        }
                    }
                }
            
            break;
            }
            case PIR_Status_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0;
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                                //Return_Message(2);
                            }
                        }
                    }
                }
            break;
            }

            case DEVICE_POWER_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<5;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            switch(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex)
                            {
                                case 0:


                                break;
                                case 1:

                                    BasicAttrsList.numAttr = 1;
                                    BasicAttrsList.attrID[0] = ATTRID_BASIC_POWER_SOURCE;
                                    //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                    zcl_SendRead( 1, &dstAddr,
                                    ZCL_CLUSTER_ID_GEN_BASIC, &BasicAttrsList,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);

                                break;
                                case 2:
                                    //
                                    BasicAttrsList.numAttr = 1;
                                    BasicAttrsList.attrID[0] = ATTRID_POWER_CFG_BATTERY_VOLTAGE;
                                    //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                    zcl_SendRead( 1, &dstAddr,
                                    ZCL_CLUSTER_ID_GEN_POWER_CFG, &BasicAttrsList,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);


                                break;                    
                                case 3:
                                    //
                                    BasicAttrsList.numAttr = 1;
                                    BasicAttrsList.attrID[0] = ATTRID_POWER_CFG_BATTERY_VOLTAGE;
                                    //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                    zcl_SendRead( 1, &dstAddr,
                                    ZCL_CLUSTER_ID_GEN_POWER_CFG, &BasicAttrsList,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   

                                break;              


                            }
                        }
                    }
                }
            break;
            }
            case Humility_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            }
                        }
                    }
                }
            break;
            }


            case Door_Sensor_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                    afAddrType_t dstAddr;
                    dstAddr.addrMode=afAddr16Bit;
                    //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                    //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                    dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                    dstAddr.endPoint=1;
                    //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            } 
                        }
                    }
                }
            break;
            }            


            case Illuminance_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_MS_ILLUMINANCE_MEASURED_VALUE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            } 
                        }
                    }
                }
            break; 
            }              


            case Sound_Light_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {

                                BasicAttrsList.numAttr = 2;
                                BasicAttrsList.attrID[0] = COMMAND_SS_IAS_WD_START_WARNING;
                                BasicAttrsList.attrID[1] = COMMAND_SS_IAS_WD_SQUAWK;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_WD, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            }
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_MS_ILLUMINANCE_MEASURED_VALUE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_WD, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            } 
                        }
                        else if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
                        {
                            afAddrType_t dstAddr;
                            dstAddr.addrMode=afAddr16Bit;
                            //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                            //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                            dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                            dstAddr.endPoint=1;
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0)
                            {
                                if(zclSS_Send_IAS_WD_StartWarningCmd(1, &dstAddr, NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1],10,  false, 0)==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);                                   
                                }
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==1)
                            {
                                if(zclSS_Send_IAS_WD_SquawkCmd(1, &dstAddr, NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1],10,  false, 0)==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);                                   
                                } 
                            }
                            
                        }
                    }
                }
            break;
            }                          

            case Smoke_Detector_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==0)
                            {
                                BasicAttrsList.numAttr = 2;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                                BasicAttrsList.attrID[1] = ATTRID_SS_IAS_ZONE_TYPE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==2) 
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_TYPE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                   

                            }
                        }
                    }
                }
            break;
            }             


            case HSL_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                dstAddr.addrMode=afAddr16Bit;
                                //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                                //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;

                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                            }else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {

                                dstAddr.addrMode=afAddr16Bit;
                                //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                                //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_HUE;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                                    
                                //ZDP_SimpleDescReq(&dstAddr,dstAddr.addr.shortAddr,1,0);
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==4)
                            {

                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                                    
                                //ZDP_SimpleDescReq(&dstAddr,dstAddr.addr.shortAddr,1,0);
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==5)
                            {

                                dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                                dstAddr.endPoint=1;
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_LEVEL_CURRENT_LEVEL;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);                                    
                                //ZDP_SimpleDescReq(&dstAddr,dstAddr.addr.shortAddr,1,0);
                            }

                        }
                        //发送控制灯指令
                        else if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x25)
                        {
                            dstAddr.addrMode=afAddr16Bit;
                            //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                            //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                            dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                            dstAddr.endPoint=1;
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0)
                                {
                                    zclGeneral_SendOnOff_CmdOff( 1, &dstAddr, false, 0 );
                                    //zclZHAtest_OnOff=LIGHT_OFF;
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==1)
                                {  
                                    zclGeneral_SendOnOff_CmdOn( 1, &dstAddr, false, 0 );
                                    //zclZHAtest_OnOff=LIGHT_ON;
                                }
                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                                Return_Message(1);
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                            {
                                zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);
                                }
                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==4)
                            {
                                zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);
                                }

                            }
                            else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==5)
                            {
                                zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                {
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message(1);
                                }
                            }
                        }
                    }
                }
            break;
            }            
            case Over_Walter_Idx:
            {
                at_status.checkOD=1;
                for(j=0;j<6;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表中
                    for(i=0;i<8;i++)
                    {
                        if(AssoList[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
                            x++;
                        else
                            break;
                    }
                    //地址正确
                    if(x==8 && strcmp(NWK_command.NWK_General_Frame.NWKCG_TargetAddress,0x00)!=0)
                    { 
                        afAddrType_t dstAddr;
                        dstAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        dstAddr.endPoint=1;
                        //读取状态命令
                        if(NWK_command.NWK_General_Frame.NWKCG_CMDID[0]==0x20)
                        {
                            if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==1)
                            {
                                BasicAttrsList.numAttr = 1;
                                BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_STATUS;
                                //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                                zcl_SendRead( 1, &dstAddr,
                                ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0;
                                //NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1]=zclZHAtest_OnOff;
                                //Return_Message(2);
                            }
                        }
                    }
                }
            break;
            }  
            default:
            {
                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                Return_Message(1);
            break;
            }
        }
    }
}
