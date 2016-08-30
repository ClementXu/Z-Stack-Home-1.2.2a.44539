#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "at_command.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "ZDApp.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zha_project.h"

extern uint8 netState;
NODE_INFO_t AssoList[5];
uint8 TempDevice[256];
DEVICE_STATUS_t DeviceStatus[5];
const char *AT_GROUP[AT_GROUP_NUM];
const char *DEVICE_NAME[DEVICE_NUM];
uint8 (*commandProcess[AT_GROUP_NUM])(RawData Setting);

/*
数据发送函数
*/
void SendDatatoComputer(char *p)
{
    HalUARTWrite(HAL_UART_PORT_0,p,strlen(p));
}

/*
清空Buffer
*/
void __CleanTempDevice()
{
    osal_memset(TempDevice,0,sizeof(TempDevice));
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

/*
保存MAC地址和短地址
*/
void SetTempDeviceSA(uint16 data,uint8 *mac)
{   
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {   //审查身份
        if(p->device[i].uiNwk_Addr==data)
            return;
    }
    
    for(i=0;i<=5;i++)
    {   
        if(p->device[i].uiNwk_Addr==0)
        {
            p->device[i].uiNwk_Addr=data;
            osal_memcpy(p->device[i].aucMAC,mac,8);
            return;
        }
    }    
}

/*
保存版本号
*/
void SetTempDeviceHW(uint16 shortAddr,uint8 version)
{   
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

/*
保存Endpoint
*/
void SetTempDeviceEP(uint16 shortAddr,uint8 *buffer )
{
    uint8 i;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].ep,0,5);
            osal_memcpy(p->device[i].ep,buffer,5);
            //p->device[i].ep = ep;
            return;
        }
    }  
}

/*
保存厂商名
*/
void SetTempDeviceManuName(uint16 shortAddr,uint8 *buffer)
{
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

/*
保存电池电量
*/
void SetTempDeviceBAT(uint16 shortAddr,uint8 battery)
{  
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

/*
保存设备类型
*/
void SetTempDeviceType(uint16 shortAddr,uint16 change)
{   
    uint8 i;
    uint16 supportOD=0;
    uint8 deviceType[16];
    osal_memset(deviceType,0,sizeof(deviceType));
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    switch(change)
    {
        case 0x000d:
            osal_memcpy(deviceType,"pir",sizeof("pir"));
            break;
        case 0x0015:
            osal_memcpy(deviceType,"doorsen",sizeof("doorsen")); 
            break;
        case 0x0028:
            osal_memcpy(deviceType,"smoke",sizeof("smoke")); 
            break;                                          
        case 0x002a:
            osal_memcpy(deviceType,"watersen",sizeof("watersen")); 
            break;   
        case 0x0100: 
            osal_memcpy(deviceType,"light",sizeof("light")); 
            break;
        case 0x0115:
            osal_memcpy(deviceType,"zonectrl",sizeof("zonectrl"));
            break;   
        case 0x0202:
            osal_memcpy(deviceType,"outlet",sizeof("outlet"));
            break;
        case 0x0226:
            osal_memcpy(deviceType,"glasssen",sizeof("glasssen"));
            break;   
        case 0x0227:
            osal_memcpy(deviceType,"cosensor",sizeof("cosensor"));
            break;   
        case 0x0225:
            osal_memcpy(deviceType,"slsensor",sizeof("slsensor"));
            break;             
        case 0x0302:
            osal_memcpy(deviceType,"temp",sizeof("temp"));
            break;
        case 0x0101:
            osal_memcpy(deviceType,"level",sizeof("level"));
            break;                            
        case 0x0102:
            osal_memcpy(deviceType,"colortem",sizeof("colortem"));
            break;                            
        case 0x0307:
            osal_memcpy(deviceType,"humility",sizeof("humility"));
            break;   
        case 0x0308:
            osal_memcpy(deviceType,"lumin",sizeof("lumin"));
            break;  
        case 0x0403:
            osal_memcpy(deviceType,"slsensor",sizeof("slsensor"));
            break;   
        default:

            return;
    }
    for(i=0;i<=5;i++)
    {  
        if(p->device[i].uiNwk_Addr==shortAddr)
        {
            osal_memset(&p->device[i].deviceType,0,16);
            osal_memcpy(p->device[i].deviceType,deviceType,strlen(deviceType));
            return;
        }
    }
}

/*
比较MAC地址
*/
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

/*
将字符串转换为不可见字符
*/
void StringToHEX(uint8 *data ,uint8 *target,uint8 length)
{
      uint8 i;
      for(i=0;i<(length/2);i++)
      {
        target[length/2-i-1]=CovertACSIITWOB(data);
            data=data+2;
      }

}

/*
将不可见字符转换为字符串
*/
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

/*
将要发送的数据打包发送
*/
void sendDatatoComputer(RawData *Setting)
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

/*
数据处理函数
*/
void AnaDataProcess(RawData *Setting)
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

/*
AT测试函数
*/
uint8 testat()
{
    uint8 buffer[256];
    //uint8 buf[256];
    osal_memset(buffer,2,sizeof(buffer));
    //osal_memcpy(buf,buffer,255);
    HalUARTWrite(0, buffer,255);
    osal_memset(buffer,1,sizeof(buffer));
    HalUARTWrite(0, buffer,255);
}

/*
获取MAC地址
*/
uint8 MAC(RawData Setting)
{
    uint8 buf[Z_EXTADDR_LEN*2+1];
    osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
    HEXtoString(aExtendedAddress,buf);
    Setting.data[0] =(uint32 *)&buf;
    Setting.length = 1;
    Setting.type = CHAR;
    Setting.RecSign = 0;  
    AnaDataProcess(&Setting);
    
}

/*
恢复出厂设置
*/
uint8 Factory(RawData Setting)
{
    uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
    osal_nv_write( ZCD_NV_STARTUP_OPTION,0, sizeof(uint8), &startOptions );
    HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",strlen("+OK\r\n"));
    //osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
    osal_set_event( zha_project_TaskID,RESET_EVT);
}

/*
重启设备，信息不丢失
*/
uint8 Reboot(RawData Setting)
{
    HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",strlen("+OK\r\n"));
    osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
    //osal_set_event( zha_project_TaskID,RESET_EVT);
}

/*
建立网络
*/
uint8 FormNet(RawData Setting)
{
    uint8 logicalType;
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        if(netState ==0 )
        {
            ZDOInitDevice(0);
            ZDO_StartDevice(ZG_DEVICETYPE_COORDINATOR,MODE_HARD,15, 15);
            Setting.length = 0;
            AnaDataProcess(&Setting);
        }
        else if(netState == 1)
        {
            HalUARTWrite(HAL_UART_PORT_0,"Network already established.\r\n",strlen("Network already established.\r\n"));
        }
    }

}

/*
离开网络
*/
uint8 _LeaveNet(uint8 *p)
{
    uint8 logicalType;
    NLME_LeaveReq_t req;
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        //#NLME_LeaveReq_t req;
        req.extAddr = p;
        req.removeChildren = FALSE;
        req.rejoin         = TRUE;
        req.silent         = FALSE;
        if(NLME_LeaveReq(&req)==ZSuccess)
        {
            return 1;
        }
    }
    return 0;
}

uint8 LeaveNet(RawData Setting)
{
    uint8 buffer[8];
    osal_memset(buffer,0,8);
    if(netState ==0)
    {
        HalUARTWrite(HAL_UART_PORT_0,"Please establish network first.\r\n",strlen("Please establish network first.\r\n"));
        return 0;
    }
    if(_LeaveNet(buffer))
    {
        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
        osal_start_timerEx( zha_project_TaskID, RESET_EVT,100);
        return 1;
    }else
    {
        HalUARTWrite(HAL_UART_PORT_0,"Failed to leave.\r\n",strlen("Failed to leave.\r\n"));
        return 2;
    }

}

/*
获取和设置PANID
*/
uint8 PermitJoin(RawData Setting)
{
    uint8 jointime;
    uint8 i,logicalType;
    //NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    Setting.data[0] =(uint32 *)&jointime;
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    osal_nv_read(ZCD_NV_LOGICAL_TYPE,0, sizeof(uint8), &logicalType); 
    if(logicalType==0)
    {       
//        for(i=0;i<=5;i++)
//        {
//            if(p->device[i].!=0)
//                AssocRemove(p->device[i].aucMAC);
//        }
        __CleanTempDevice();
        //DelayMS(100);
        NLME_PermitJoiningRequest(jointime);
    }  
}

/*
获取和设置CHANNEL
*/
uint32 _GetChannel()
{
    uint8 i;
    uint32 channel=0;
    uint32 channellist;
    osal_nv_read(ZCD_NV_CHANLIST,0, sizeof(uint32), &channellist);
    for(i=0;i<32;i++)
    {
        channellist=channellist>>1;
        if(channellist&&0x01==1)
              channel ++;
    }
    return channel;
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
    osal_nv_write(ZCD_NV_CHANLIST, 0, 4, &channel);
}

uint8 Channel(RawData Setting)
{
    uint32 channel=_GetChannel();
    Setting.data[0] =&channel;
    Setting.length = 1;
    Setting.type = VALUE_D;
    AnaDataProcess(&Setting);
    _SetChannel(channel);
}

/*
获取和设置PANID
*/
uint8 PANID(RawData Setting)
{
    uint32 AutoSign=0;//0 auto
    uint16 panid;
    osal_nv_read(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);//硬盘上的数据
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
        return 0;
    
    if(AutoSign==1)
    {
        osal_nv_write(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);
        _NIB.nwkPanId = panid;
        NLME_UpdateNV(0x01);
    }
    else
    {
        panid = 0xffff;
        osal_nv_write(ZCD_NV_PANID, 0, sizeof(uint16),  &panid);
    }

}

/*
获取Extended PANID
*/
uint8 ExtendedPANID(RawData Setting)
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
}

/*
移除设备
*/
void _RemoveProcess(uint8 *data)
{
    uint8 i,k=0;
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
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
    osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
}
uint8 RemoveDevice(RawData Setting)
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
        _RemoveProcess(mac[i]);
    }

}

/*
允许鉴权通过的设备入网
*/
void _AcceptProcess(uint8 *data)
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
    osal_nv_read( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
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
            osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );
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
    osal_nv_write( ZCD_NV_DEVICE_TABLE,0,(sizeof( NODE_INFO_t )*5), &AssoList );

}
void _RemoveJoin()
{
    uint8 i=0;
    NODE_INFO_Group *p=(NODE_INFO_Group *)TempDevice;
    for(i=0;i<5;i++)
    {
        if(p->device[i].uiNwk_Addr !=0 )
            _LeaveNet(p->device[i].aucMAC);
    }
}
uint8 AcceptJoin(RawData Setting)
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
        _AcceptProcess(mac[i]);
    }
    _RemoveJoin();    
}

/*
AddInfo
*/
uint8 AddInfo(RawData Setting)
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

#if ZG_BUILD_ENDDEVICE_TYPE
/*
设备类型初始化
*/
void Device_type_Init()
{
    DEVICE_NAME[light]="light";
    DEVICE_NAME[colorlight]="level";
    DEVICE_NAME[dimmer]="colortem";
    DEVICE_NAME[thermometer]="temperature";
    DEVICE_NAME[pir]="pir";
    DEVICE_NAME[thermohygrograph]="humility";
    DEVICE_NAME[doorsensor]="doorsen";
    DEVICE_NAME[illuminance]="lumin";
    DEVICE_NAME[slsensor]="slsensor";
    DEVICE_NAME[smokesensor]="smoke";
    DEVICE_NAME[watersensor]="watersen";
    DEVICE_NAME[lightswitch]="lightswitch";
    DEVICE_NAME[cosensor]="cosensor";
    DEVICE_NAME[gassensor]="gassensor";
    DEVICE_NAME[glasssensor]="glasssen";
    DEVICE_NAME[zonecontroller]="zonecontroller";
    DEVICE_NAME[outlet]="outlet";
};

unsigned char Indexofdevice(char *data)
{
    unsigned char i=0;
    for(i=0;i<=DEVICE_NUM;i++)
    {
        if(strcmp(data,DEVICE_NAME[i])==0)
            return i;
    }
    return 0;
};
#endif

/*
设备类型
*/
uint8 Device(RawData Setting)
{
	uint8 devicetype;
    char buffer[16];
    osal_memset(buffer,0,16);
#if ZG_BUILD_COORDINATOR_TYPE 
        strcpy(buffer,"gateway");  
        Setting.data[0] =(uint32 *)&buffer[0];
        Setting.length = 1;
        Setting.type = CHAR;
        AnaDataProcess(&Setting);
#endif
#if ZG_BUILD_ENDDEVICE_TYPE
        osal_nv_read(ZCD_NV_DEVICE_TYPE,0, sizeof(uint8), &devicetype);
        osal_memcpy(buffer,DEVICE_NAME[devicetype],strlen(DEVICE_NAME[devicetype]));
        Setting.data[0] =(uint32 *)&buffer[0];
        Setting.length = 1;
        Setting.type = CHAR;
        AnaDataProcess(&Setting);
        devicetype = Indexofdevice(buffer);
        osal_nv_write(ZCD_NV_DEVICE_TYPE, 0,sizeof(uint8), &devicetype);
#endif
}

/*
AT指令索引，返回对应序号
*/
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

/*
AT指令初始化
*/
void AT_Init()
{
    //通用控制指令
    AT_GROUP[ENUM_AT_MAC]="MAC";
    AT_GROUP[ENUM_AT_FMVER]="FMVER";
    AT_GROUP[ENUM_AT_FACTORY]="FACTORY";
    AT_GROUP[ENUM_AT_REBOOT]="REBOOT";
    AT_GROUP[ENUM_AT_CHANNEL]="CHANNEL";
    AT_GROUP[ENUM_AT_PANID]="PANID";
    AT_GROUP[ENUM_AT_GETEXTPID]="GETEXTPID";
    AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
    commandProcess[ENUM_AT_MAC] = MAC;
    commandProcess[ENUM_AT_PANID] = PANID;
    commandProcess[ENUM_AT_FACTORY] = Factory;
    commandProcess[ENUM_AT_REBOOT] = Reboot;
    commandProcess[ENUM_AT_CHANNEL] = Channel;
    commandProcess[ENUM_AT_GETEXTPID] = ExtendedPANID;
    commandProcess[ENUM_AT_DEVICE] = Device;

#if ZG_BUILD_COORDINATOR_TYPE   
    AT_GROUP[ENUM_AT_FORM]="FORM";
    AT_GROUP[ENUM_AT_LEAVE]="LEAVE";
    AT_GROUP[ENUM_AT_PERMITJOIN]="PERMITJOIN";
    AT_GROUP[ENUM_AT_DENYJOIN]="DENYJOIN";
    AT_GROUP[ENUM_AT_ADDINFO]="ADDINFO";
    AT_GROUP[ENUM_AT_ADDSTATUS]="ADDSTATUS";
    AT_GROUP[ENUM_AT_ACCEPTJOIN] = "ACCEPTJOIN";
    AT_GROUP[ENUM_AT_ONLINE]="ONLINE"; 
    AT_GROUP[ENUM_AT_REMOVEDEV]="REMOVEDEV";
    
    commandProcess[ENUM_AT_FORM] =FormNet;
    commandProcess[ENUM_AT_LEAVE] =LeaveNet;
    commandProcess[ENUM_AT_PERMITJOIN] = PermitJoin;
    commandProcess[ENUM_AT_ADDINFO] = AddInfo;
    commandProcess[ENUM_AT_REMOVEDEV] = RemoveDevice;
    commandProcess[ENUM_AT_ACCEPTJOIN] = AcceptJoin;
    
#endif   
#if ZG_BUILD_ENDDEVICE_TYPE
    AT_GROUP[ENUM_AT_LIGHT]="LIGHT";
    AT_GROUP[ENUM_AT_PIR]="PIR";
    AT_GROUP[ENUM_AT_TEMP]="TEMP";
    AT_GROUP[ENUM_AT_COLORTEM]="COLORTEM";
    AT_GROUP[ENUM_AT_HUMILITY]="HUMILITY";
    AT_GROUP[ENUM_AT_DOORSENSOR]="DOORSEN";
    AT_GROUP[ENUM_AT_SOUNDLIGHTSENSOR]="SLSENSOR";
    AT_GROUP[ENUM_AT_SMOKEDETECTOR]="SMOKE";
    AT_GROUP[ENUM_AT_LEVEL]="LEVEL";
    AT_GROUP[ENUM_AT_LUMIN]="LUMIN";
    AT_GROUP[ENUM_AT_COSENSOR]="COSENSOR";
    AT_GROUP[ENUM_AT_GASSENSOR]="GASSENSOR";
    AT_GROUP[ENUM_AT_GLASSSENSOR]="GLASSSENSOR";
    AT_GROUP[ENUM_AT_ZONECONTROL]="ZONECONTROL";
    AT_GROUP[ENUM_AT_LIGHTSWITCH]="LIGHTSWITCH";
    AT_GROUP[ENUM_AT_OUTLET]="OUTLET";    
#endif
    
};

/*
AT指令集主函数
*/
void At_Command(char *recData)
{
    RawData Setting;
    char AT_Name[15];
    
    uint8 Index;
    
    char AT_Format[]="AT+%[^=]";
    osal_memset(AT_Name,0,sizeof(AT_Name));
    osal_memset(&Setting,0,sizeof(RawData));
	//第一步，读取AT指令名称
	if(sscanf(recData,AT_Format ,AT_Name)!=0)
	{	//解析成功
		Index = IndexofAT(AT_Name);//给赋值
		if(recData[strlen(AT_Name)+3] == '=')
		{//设置指令
            Setting.RecData = 0;
            Setting.RecData = &recData[strlen(AT_Name)+4];//
            Setting.RecSign = 1;
            commandProcess[Index](Setting);
		}
		else
		{	//读取指令
            commandProcess[Index](Setting);
            Setting.RecSign = 0;  
		}
	}
	else
	{	//解析失败
		uint8 buffer[16];
        osal_memset(buffer,0,16);
        sprintf(buffer,"ERROR:=%s\r\n","COMMAND ERROR");
        SendDatatoComputer(buffer);
	}    
}
