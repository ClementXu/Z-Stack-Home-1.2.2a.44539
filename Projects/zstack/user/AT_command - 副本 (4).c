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
char CORRECT[]="+OK\r\n";
char ERROR[]="ERROR\r\n";
char *AT_GROUP[AT_GROUP_NUM]; 
extern uint8  zclZHAtest_OnOff;
extern uint8  zclZHAtest_Level_to_Level;
extern uint16 zclZHAtest_Alarm_Status;
extern uint16 zclZHAtest_Smoke_Type;
extern uint8 zclZHAtest_Humidity_Value;
extern uint16 zclZHAtest_Light_Color_Status;
extern int16 zclZHAtest_Temperature_Value;
extern uint8 zclZHAtest_WD_Duration;
extern uint8 zclZHAtest_Warning;
extern uint8 zclZHAtest_WD_SQUAWK;
extern uint16 zclZHAtest_HUE_Status;
extern uint16 zclZHAtest_Illumiance_Value;

extern NODE_INFO_t Node_Info[];
NODE_INFO_t AssoList[];
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
    ENUM_AT_ADD,
    ENUM_AT_SETDEVICE,
    ENUM_AT_GETDEVICE,
    ENUM_AT_LIGHT,
    ENUM_AT_PIR,
    ENUM_AT_TEMP,        
    ENUM_AT_COLORTEM,
    ENUM_AT_HUMILITY,
    ENUM_AT_DOORSENSOR,
    ENUM_AT_SOUNDLIGHTSENSOR,
    ENUM_AT_SMOKEDETECTOR,
    ENUM_AT_LEVEL,
    ENUM_AT_LUMIN,
    ENUM_AT_XYY,
    ENUM_AT_GETEXTPID,
    ENUM_AT_SETEXTPID,
    ENUM_AT_DENYJOIN,
    ENUM_AT_ACCEPTJOIN,
    ENUM_AT_CHECKEP,
    ENUM_AT_CHECKSIMPLEDESC,
    ENUM_AT_ZONETYPE,
    ENUM_AT_SETREP,
    ENUM_AT_WATERSENSOR, 




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
    ENUM_AT_GETCH,
    ENUM_AT_SCANCHMASK,
    ENUM_AT_SETPID,
    ENUM_AT_GETPID,
    ENUM_AT_SETPWR,
    //        ENUM_AT_LIST,
    ENUM_AT_GETCFG,
    ENUM_AT_GETINFO,
    ENUM_AT_SETPWRMODE
}at_command;


void Return_Message1(uint8 length)
{
    uint8 pValue[8];
    osal_memset(at_status.DATA_buffer,0,128);
    at_status.DATA_buffer[0]=NWK_command.NWKCB_Header;
    at_status.DATA_buffer[1]=length + 35;
    osal_memcpy(at_status.DATA_buffer+2,NWK_command.NWK_General_Frame.NWKCG_FrameControl,2);  
                                             
    osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
    osal_memcpy(at_status.DATA_buffer+18,pValue,8);
    osal_memcpy(at_status.DATA_buffer+10,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);   
    osal_memcpy(at_status.DATA_buffer+28,NWK_command.NWK_General_Frame.NWKCG_CMDID,2);  
    at_status.DATA_buffer[29]=0x80;
    osal_memcpy(at_status.DATA_buffer+30,NWK_command.NWK_General_Frame.NWKCG_GroupID,2);
    osal_memcpy(at_status.DATA_buffer+32,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Index,2);   
    at_status.DATA_buffer[34]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex;
    at_status.DATA_buffer[35]=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Opt; 
    at_status.DATA_buffer[36]=length; 
    osal_memcpy(at_status.DATA_buffer+37,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,length);
    at_status.DATA_buffer[37+length]=0xFF;
    at_status.DATA_buffer[38+length]=NWK_command.NWKCB_Footer;  
    if(at_status.checkOD==1)
    {
       HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,(39+length));
       at_status.checkOD=0;
    }
    osal_memset(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,0,128);
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
    AT_GROUP[ENUM_AT_ECHO]="ECHO";
    AT_GROUP[ENUM_AT_UART]="UART";
    AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
    AT_GROUP[ENUM_AT_BINDING]="BINDING";
    AT_GROUP[ENUM_AT_VALUE]="VALUE";
    AT_GROUP[ENUM_AT_LIST]="LIST";
    AT_GROUP[ENUM_AT_CHECK]="CHECK";
    AT_GROUP[ENUM_AT_SETDEVICE]="SETDEV";
    AT_GROUP[ENUM_AT_GETDEVICE]="GETDEV";
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
    AT_GROUP[ENUM_AT_XYY]="XYY";
    AT_GROUP[ENUM_AT_GETEXTPID]="GETEXTPID";
    AT_GROUP[ENUM_AT_SETEXTPID]="SETEXTPID";
    AT_GROUP[ENUM_AT_DENYJOIN]="DENYJOIN";
    AT_GROUP[ENUM_AT_CHECKEP]="CHECKEP";
    AT_GROUP[ENUM_AT_CHECKSIMPLEDESC]="CHECKDESC";
    AT_GROUP[ENUM_AT_ZONETYPE]="ZONETYPE";
    AT_GROUP[ENUM_AT_SETREP]="SETREP";
    AT_GROUP[ENUM_AT_WATERSENSOR]="WATERSEN";
    AT_GROUP[ENUM_AT_ADD]="ADD";
    AT_GROUP[ENUM_AT_ACCEPTJOIN] = "ACCEPTJOIN";


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
    AT_GROUP[ENUM_AT_GETCH]="GETCH";
    AT_GROUP[ENUM_AT_SCANCHMASK]="SCANCHMASK";
    AT_GROUP[ENUM_AT_SETPID]="SETPID";
    AT_GROUP[ENUM_AT_GETPID]="GETPID";
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
    DEVICE_NAME[watersensor]="water";
    DEVICE_NAME[onoffswitch]="onoffswitch";
};
void SendDatatoComputer(char *p)
{
    HalUARTWrite(HAL_UART_PORT_0,p,strlen(p));
}

void sendDatatoComputer(char *buffer, uint32 *data,uint8 length,uint8 type)
{
    uint8 i=0;
    char format[32]={0};
    if(length==0)
    {
        sprintf(format,"+OK");        
    }
    else
    {
        sprintf(format,"+OK=");
    }
    
    for(i;i<length;i++)
    {
        if(i==0)
        {
            if(type == 0)
                strcat(format,"%d"); 
            else
                strcat(format,"%s");           
        }
        else
        {
            if(type == 0)
                strcat(format,",%d"); 
            else
                strcat(format,",%s"); 
        }
        sprintf(buffer,format,*(data+i));  
    }
    strcat(buffer,"/r/n");     
    //printf("%s",buffer);
    HalUARTWrite(HAL_UART_PORT_0,buffer,strlen(buffer));
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

uint8 StringToHEX(uint8 *data)
{
      uint8 i;
      for(i=0;i<8;i++)
      {
        data[7-i]=CovertACSIITWOB(data);
            data=data+2;
      }
      for(i=8;i<16;i++)
      {
        data[i]=0x00;
      
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

void anaFunction(uint32 *source,uint32 *target,uint8 length)
{
    uint8 i ;
    //lenght==0在函数外层处理
    for(i=0;i<length;i++)
    {
        target[i] = source[i];
    }
    SendDatatoComputer(CORRECT);
}



void AT_process_event()
{
	const char *pFormat = "AT+%[^=]=%[^,],%[^,],%[^,],%[^,],%[^,] ";
	char cmd[8] = {0} ;
	char data1[17] = {0} ;
	char data2[16] = {0}  ;
	char data3[16] = {0} ;
	char data4[16] = {0} ;
	char data5[16] = {0} ;
        uint32 *anaData[5];
	sscanf(at_status.DATA_buffer,pFormat ,cmd,data1,data2,data3,data4,data5 ) ;
	uint8 logicalType;
	uint8 devicetype;
	uint32 channellist;
	uint8 jointime;
	uint16 panid;
	char c[]="+OK\r\n";
	uint8 i,j;
        afAddrType_t dstAddr;
	zclReadCmd_t BasicAttrsList;
        NLME_LeaveReq_t req;
        zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
	unsigned char index = IndexofAT(cmd);
        osal_memset(at_status.DATA_buffer,0,sizeof(at_status.DATA_buffer));

	switch(index)
	{
         //获取MAC地址
            case ENUM_AT_MAC:
            {
                    uint8 i;
                    uint8 *xad;
                    uint8 buf[Z_EXTADDR_LEN*2+1]=0;
                    xad = aExtendedAddress + Z_EXTADDR_LEN - 1;

                    for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
                    {
                            uint8 ch;
                            ch = (*xad >> 4) & 0x0F;
                            buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                            ch = *xad & 0x0F;
                            buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                    }
                    sprintf(at_status.DATA_buffer,"+OK=%s/r/n",buf);
            break;
            }

            
            case ENUM_AT_FMVER:
            {
                if(logicalType!=0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=TESTDEVICE V1.7\r\n");
                    SendDatatoComputer(at_status.DATA_buffer);  
                    //osal_memset(sendbuffer,0,16);
                }
                else if(logicalType==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=GATEWAY V1.7\r\n");
                    SendDatatoComputer(at_status.DATA_buffer);
                }
            break;
            }
            //保存配置
            case ENUM_AT_SAVE:
            {
                zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &at_status.AT_logicalType);
                osal_nv_item_init(ZCD_NV_DEVICE_TYPE,sizeof(uint8),NULL);
                zb_WriteConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &at_status.AT_devicetype);
                SendDatatoComputer(c);
                    //HalUARTWrite(HAL_UART_PORT_0,c,sizeof(c)); 
            break;
            }
            //恢复出厂设置
            case ENUM_AT_FACTORY:
            {
                SendDatatoComputer(c);
                uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
                zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                at_status.FactoryFlag=1;					
            break;
            }
            //重启设备（设备信息不丢失）              
            case ENUM_AT_REBOOT:
            {
                SendDatatoComputer(c);
                at_status.FactoryFlag=2;
            break;
            }

            //LED灯本地控制命令         
            case ENUM_AT_LIGHT:
            {
                if(data1[0]==0)
                {   //读取指令
                    sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",zclZHAtest_OnOff);
                    SendDatatoComputer(at_status.DATA_buffer);                    
                }
                else
                {   //设置指令
                    if(data1[1]>0x29&&data1[1]<0x32)
                    {
                        zclZHAtest_OnOff=data1[1]-0x30;
                        SendDatatoComputer(c);
                    }
                    else
                    {
                        SendDatatoComputer(ERROR);
                    }
                    //anaFunction(data1[1],&zclZHAtest_OnOff,1);
                }
                if(data1[1]==0x30 )
                {
                    zclZHAtest_OnOff=0;
                    SendDatatoComputer(c);
                }
                else if(data1[1]==0x31 )
                {
                    zclZHAtest_OnOff=1;
                    SendDatatoComputer(c);
                }else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",zclZHAtest_OnOff);
                    SendDatatoComputer(at_status.DATA_buffer);
                }       
                if ( zclZHAtest_OnOff == LIGHT_ON )
                {
                    HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
                }
                else
                {
                    HalLedSet( HAL_LED_4, HAL_LED_MODE_OFF );
                }
            break;
            }
            //调色灯本地控制命令
            case ENUM_AT_LEVEL:
            {
                if(data1[1]==0x30 )
                {
                    zclZHAtest_OnOff=0;
                    zclZHAtest_Level_to_Level = atoi(data2);
                    zclZHAtest_Level_to_Level = zclZHAtest_Level_to_Level*25.4;
                    SendDatatoComputer(c);
                }
                else if(data1[1]==0x31 )
                {
                    zclZHAtest_OnOff=1;
                    zclZHAtest_Level_to_Level = atoi(data2);                           
                    SendDatatoComputer(c);
                }
                else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=0%x,%d\r\n",zclZHAtest_OnOff,zclZHAtest_Level_to_Level);
                    SendDatatoComputer(at_status.DATA_buffer);         
                }             
            break;
            }
            //本地人体热功能控制
            case ENUM_AT_PIR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;                  
                if(data1[0]!=0)
                {
                        zclZHAtest_Alarm_Status=CovertACSIITWO(data1);
                        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                        SendDatatoComputer(c);           
                }
                else if(data1[0]==0)
                {
                        
                        sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",zclZHAtest_Alarm_Status);
                        SendDatatoComputer(at_status.DATA_buffer);         
                }
            break;    
            }
             //本地温度测量控制
            case ENUM_AT_TEMP:
            {
                if(data1[0]!=0)
                {
                    zclZHAtest_Temperature_Value = atol(data1);
                    SendDatatoComputer(c);  
                }
                else if(data1[0]==0)
                {    
                    sprintf(at_status.DATA_buffer,"+OK=%d\r\n",zclZHAtest_Temperature_Value);
                    SendDatatoComputer(at_status.DATA_buffer);          
                }
            break;   
            }
            //色温控制
            case ENUM_AT_COLORTEM:
            {
                if(data1[1]==0x30 )
                {
                    zclZHAtest_OnOff=0;
                    zclZHAtest_Level_to_Level = atoi(data2);
                    zclZHAtest_Light_Color_Status = atol(data3);
                    SendDatatoComputer(c);  
                }
                else if(data1[1]==0x31 )
                {
                    zclZHAtest_OnOff=1;
                    zclZHAtest_Level_to_Level = atoi(data2);
                    zclZHAtest_Light_Color_Status = atol(data3);
                    SendDatatoComputer(c);  
                }                  
                else if(data1[0]==0)
                {
                    if(zclZHAtest_Level_to_Level<=0x0F)
                    {
                        sprintf(at_status.DATA_buffer,"+OK=0%x,%d,%d\r\n",zclZHAtest_OnOff,zclZHAtest_Level_to_Level,zclZHAtest_Light_Color_Status);
                    }
                    else
                    {
                        sprintf(at_status.DATA_buffer,"+OK=0%x,%d,%d\r\n",zclZHAtest_OnOff,zclZHAtest_Level_to_Level,zclZHAtest_Light_Color_Status);
                    }                                        	
                    SendDatatoComputer(at_status.DATA_buffer);
                }
            break;
            }
            //湿度传感器
            case ENUM_AT_HUMILITY:
            {
                if(data1[0]!=0)
                {
                    zclZHAtest_Humidity_Value = atoi(data2);
                    SendDatatoComputer(c);
                }
                else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=%d\r\n",zclZHAtest_Humidity_Value);
                    SendDatatoComputer(at_status.DATA_buffer);  
                }
            break;
            }
            //门磁
            case ENUM_AT_DOORSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;                  
                if(data1[0]!=0)
                {
                    zclZHAtest_Alarm_Status=CovertACSIITWO(data1);
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                    SendDatatoComputer(c);           
                }
                else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",zclZHAtest_Alarm_Status);
                    SendDatatoComputer(at_status.DATA_buffer);           
                }
            break;
            }
            //声光报警器
            case ENUM_AT_SOUNDLIGHTSENSOR:
            {
                if(data2[0]!=0)
                {
                    if(data1[1]==0x30)
                    {
                        zclZHAtest_Warning=CovertACSIITWO(data2);
                    }
                    else if(data1[1]==0x31)
                    {
                        zclZHAtest_WD_SQUAWK=CovertACSIITWO(data2);
                    }
                    else if(data1[1]==0x32)
                    {
                        zclZHAtest_WD_Duration=CovertACSIITWO(data2);
                    }
                    SendDatatoComputer(c); 
                }
                else if(data2[0]==0)
                {				
                       
                    if(data1[1]==0x30)
                    {
                        sprintf(at_status.DATA_buffer,"+OK=00,0%x\r\n",zclZHAtest_Warning);
                        SendDatatoComputer(at_status.DATA_buffer);  
                    }
                    else if(data1[1]==0x31)
                    {

                        sprintf(at_status.DATA_buffer,"+OK=01,0%x\r\n",zclZHAtest_WD_SQUAWK);
                        SendDatatoComputer(at_status.DATA_buffer); 
                    }
                    else if(data1[1]==0x32)
                    {
                        if(zclZHAtest_WD_Duration<=0x0F)
                            sprintf(at_status.DATA_buffer,"+OK=02,0%x\r\n",zclZHAtest_WD_Duration);
                        else
                            sprintf(at_status.DATA_buffer,"+OK=02,%x\r\n",zclZHAtest_WD_Duration);
                        SendDatatoComputer(at_status.DATA_buffer);   
                    }          
                }
              
            break;
            }
            //烟雾报警器
            case ENUM_AT_SMOKEDETECTOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;
                zclZHAtest_Smoke_Type = SS_IAS_ZONE_TYPE_FIRE_SENSOR;
                if(data2[0]!=0)
                {
                    if(data1[1]==0x30)
                    {
                        zclZHAtest_Alarm_Status=CovertACSIITWO(data2);
                        zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                    }
                    else if(data1[1]==0x31)
                    {
                        zclZHAtest_Smoke_Type=CovertACSIITWO(data2);
                    }
                    SendDatatoComputer(c);               
                }
                else if(data2[0]==0)
                {
                   if(data1[1]==0x30)
                    {
                        sprintf(at_status.DATA_buffer,"+OK=00,0%x\r\n",zclZHAtest_Alarm_Status);
                        SendDatatoComputer(at_status.DATA_buffer);  
                    }
                    else if(data1[1]==0x31)
                    {
                        sprintf(at_status.DATA_buffer,"+OK=01,0%x\r\n",zclZHAtest_Smoke_Type);
                        SendDatatoComputer(at_status.DATA_buffer);  
                    }
                }
            break;
            }
            //光照强度检测
            case ENUM_AT_LUMIN:
            {
                if(data1[0]!=0)
                {
                    zclZHAtest_Illumiance_Value = atol(data2);
                    SendDatatoComputer(c);          
                }
                else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=%d\r\n",zclZHAtest_Illumiance_Value);
                    SendDatatoComputer(at_status.DATA_buffer);
                }
            break;
            }
            case ENUM_AT_XYY:
              
              
            break;
            //水报警器
            case ENUM_AT_WATERSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;                  
                zclZHAtest_Smoke_Type = SS_IAS_ZONE_TYPE_WATER_SENSOR;
                if(data1[0]!=0)
                {              
                    zclZHAtest_Alarm_Status=CovertACSIITWO(data1);
                    SendDatatoComputer(c);
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1, &dstAddr,data1[0],10,  false, 0);
                }
                else if(data1[0]==0)
                {
                    sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",zclZHAtest_Alarm_Status);
                    SendDatatoComputer(at_status.DATA_buffer);         
                }          
            break;
            }
            //建立网络     
            case ENUM_AT_FORM:
            {
                if(logicalType==0)
                {
                    appState == APP_START;
                    uint8 logicalType = ZG_DEVICETYPE_COORDINATOR;
                    zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                    ZDOInitDevice(0);
                    ZDO_StartDevice(ZG_DEVICETYPE_COORDINATOR,MODE_HARD,15, 15);
                    SendDatatoComputer(c);
                }
            break;
            }
            //离网
            case ENUM_AT_LEAVE:
            {
                if(logicalType==0)
                {                  
                    NLME_LeaveReq_t req;
                    req.removeChildren = FALSE;
                    req.rejoin         = FALSE;
                    req.silent         = FALSE;
                    if(NLME_LeaveReq(&req)==ZSuccess)
                    {
                        SendDatatoComputer(c);
                    }
                }
             break;
            }
             //允许设备入网
            case ENUM_AT_PERMITJOIN:
            {
                if(logicalType==0)
                {       
       
                    for(i=0;i<=at_status.DEVNUM;i++)
                    {
                        AssocRemove(Node_Info[i].aucMAC);
                        osal_memset(&Node_Info[i],0,sizeof(Node_Info[i]));
                    }
                    DelayMS(100);
                    jointime=CovertACSIITWO(data1);
                    NLME_PermitJoiningRequest(jointime);
                    at_status.DEVNUM=0;
                    at_status.deviceCount=0;
                    SendDatatoComputer(c);
                }
             break;
             }
             //设置频道
            case ENUM_AT_SETCH:
            {
                uint32 channel;
              
                for(i=0;i<2;i++)
                {
                    if(data1[i]<=57)
                    {
                        data1[i]=(data1[i]-48);
                    }
                    else if(data1[i]<=103 && data1[i]>=97)
                    {
                        data1[i]=(data1[i]-87);
                    }
                }
                switch(((data1[0]&0x0F)<<4)+(data1[1]&0x0F))
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
                SendDatatoComputer(c);
            break;
            }
            //设置PID
            case ENUM_AT_SETPID:
            {
                if(strcmp(data1,"AUTO")==0)
                {
                    SendDatatoComputer(c);
                    at_status.PIDType=0;
                }
                else
                {
                    panid=CovertACSIIFOUR(data1);
                    zb_WriteConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
                    SendDatatoComputer(c);
                    at_status.PIDType=1;
                }
            break;
            }
            //获取Extended PID
            case ENUM_AT_GETEXTPID:
            {
                uint8 zgExtendedPANID[8];
                osal_cpyExtAddr( zgExtendedPANID, _NIB.extendedPANID );
                //zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, 8,  &zgExtendedPANID);
                            //osal_nv_read(ZCD_NV_EXTADDR ,0, Z_EXTADDR_LEN, pValue);
                uint8 i;
                uint8 *xad;
                uint8 buf[Z_EXTADDR_LEN*2]=0;
                // Display the extended address.
                xad = zgExtendedPANID + Z_EXTADDR_LEN - 1;

                for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
                {
                        uint8 ch;
                        ch = (*xad >> 4) & 0x0F;
                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                        ch = *xad & 0x0F;
                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                }          
                        sprintf(at_status.DATA_buffer,"+OK=");
                //char e[Z_EXTADDR_LEN*2+7]="+OK=";
                strcat( at_status.DATA_buffer,buf);
                strcat(at_status.DATA_buffer,"\r\n");
                SendDatatoComputer(at_status.DATA_buffer);
             break;
             }
            //查看待加网设备的信息和状态
            case ENUM_AT_ADD:
            {
                at_status.acceptFlag=0;
                if(strcmp(data1,"INFO")==0)
                {
                    osal_memset(at_status.DATA_buffer,0,sizeof(at_status.DATA_buffer));
                    sprintf(at_status.DATA_buffer,"+OK=");
                    at_status.deviceCount=0;
                    uint8 j,k,x;
                    
                    for(j=0;j<5;j++)
                    {
                        uint8 i;
                        uint8 *xad;
                        uint8 buf[Z_EXTADDR_LEN*2+1]=0;//
                        osal_memset(buf,0,Z_EXTADDR_LEN*2+1);
                        // Display the extended address.
                        if(AssociatedDevList[j].shortAddr!=0 &&AssociatedDevList[j].shortAddr!=0xFFFF)
                        {
                            for(x=0;x<5;x++)
                            {
                                if(Node_Info[x].uiNwk_Addr==AssociatedDevList[j].shortAddr)
                                {
                                    xad = Node_Info[x].aucMAC+ Z_EXTADDR_LEN - 1; 

                                    for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
                                    {
                                        uint8 ch;
                                        ch = (*xad >> 4) & 0x0F;
                                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                                        ch = *xad & 0x0F;
                                        buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
                                    }          
                                    if(k==1)
                                    {
                                        strcat(at_status.DATA_buffer,";");
                                        k=0;
                                    }
                                    //char e[Z_EXTADDR_LEN*2+7]="+OK=";
                                    osal_memcpy(Node_Info[x].showMAC,buf,16);
                                    strcat( at_status.DATA_buffer,buf);
                                    strcat( at_status.DATA_buffer,",");
                                    osal_memcpy(data1,zclZHAtest_ManufacturerName+1,16);
                                    data1[16]=0x00;
                                    strcat( at_status.DATA_buffer,data1);
                                    strcat( at_status.DATA_buffer,",");
                                    strcat( at_status.DATA_buffer,"00");
                                    strcat( at_status.DATA_buffer,buf);
                                    k++;
                                    at_status.deviceCount++;
                                    at_status.DEVNUM=x;
                                }                   
                            }
                        }
                    }
                    strcat( at_status.DATA_buffer,"\r\n");
                    SendDatatoComputer(at_status.DATA_buffer);
                }
                else if(strcmp(data1,"STATUS")==0)
                { 
                    uint8 j,k;
                    osal_memset(at_status.DATA_buffer,0,sizeof(at_status.DATA_buffer));
                    sprintf(at_status.DATA_buffer,"+OK=");                   
                    for(j=0;j<at_status.deviceCount;j++)
                    {
                        if(k==1)
                        {
                            strcat(at_status.DATA_buffer,";");
                            k=0;
                        }
                        strcat( at_status.DATA_buffer,"00");
                        osal_memcpy(data1,Node_Info[j].showMAC,16);
                        data1[16]=0x00;
                        strcat( at_status.DATA_buffer,data1);
                        strcat(at_status.DATA_buffer,",");
                        //strcat( at_status.DATA_buffer,Node_Info[j].deviceName);
                        strcat( at_status.DATA_buffer,"TestDevice");
                        strcat(at_status.DATA_buffer,",");
                        //at_status.DATA_buffer[strlen(at_status.DATA_buffer)+1] = Node_Info[j].batteryValue;
                        at_status.DATA_buffer[strlen(at_status.DATA_buffer)] = 0x33;
                        //strcat( at_status.DATA_buffer,Node_Info[j].batteryValue);
                        strcat(at_status.DATA_buffer,",");
                        strcat( at_status.DATA_buffer,"1.5");
                        //strcat( at_status.DATA_buffer,Node_Info[j].version);
                        strcat(at_status.DATA_buffer,",");
                        at_status.DATA_buffer[strlen(at_status.DATA_buffer)] = Node_Info[j].sensorType;
                        //strcat( at_status.DATA_buffer,Node_Info[j].sensorType);
                        strcat(at_status.DATA_buffer,",");
                        at_status.DATA_buffer[strlen(at_status.DATA_buffer)] = 0x31;
                        k++;

                    }
                        //at_status.deviceCount=0;
                    strcat(at_status.DATA_buffer,"\r\n");
                    SendDatatoComputer(at_status.DATA_buffer);
                }
            break;
            }
            case ENUM_AT_SETEXTPID:
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
#if 0                  
                //uint16 panid;
                zb_ReadConfiguration(ZCD_NV_PANID, sizeof(uint16),  &panid);
                if(panid<0x000F)
                   sprintf(at_status.DATA_buffer,"+OK=000%x\r\n",panid);
                else if(panid<0x00FF && panid>0x000F)
                   sprintf(at_status.DATA_buffer,"+OK=00%x\r\n",panid);
                else if(panid<0x0FFF && panid>0x00FF)
                   sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",panid);
                else
                  sprintf(at_status.DATA_buffer,"+OK=%x\r\n",panid);
                SendDatatoComputer(at_status.DATA_buffer);
                //获得Extended panID
                //zb_ReadConfiguration(ZCD_NV_EXTENDED_PAN_ID, 8,  pValue);
                //sprintf(at_status.DATA_buffer,"EXT PANID:%x\r\n",pValue);
                
                //获取network KEY
                //nwkActiveKeyItems keyItems;
                //zb_ReadConfiguration(ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), &keyItems);
                //osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+19,keyItems.active.key,16);
                //获得Channel List
                uint32 channe;
                zb_ReadConfiguration(ZCD_NV_CHANLIST, sizeof(uint32), &channellist);
                sprintf(at_status.DATA_buffer,"+OK=Channel:%x\r\n",channellist);
                for(i=0;i<32;i++)
                {
                    channellist=channellist>>1;
                    if(channellist&&0x01==1)
                       if((i+1)<=0x0F)
                           sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",i+1);
                       else
                           sprintf(at_status.DATA_buffer,"+OK=%x\r\n",i+1);
                }
                SendDatatoComputer(at_status.DATA_buffer);
                //HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
    //                               MT_UtilSetPanID();  
     //  
#endif    
            break;
            //获得PID
            case ENUM_AT_GETPID:
            {
                panid=_NIB.nwkPanId;
                NLME_UpdateNV( NWK_NV_NIB_ENABLE);
                if(at_status.PIDType==0)
                {
                    if(panid<=0x000F)
                        sprintf(at_status.DATA_buffer,"+OK=AUTO:000%x\r\n",panid);
                    else if(panid<=0x00FF && panid>0x000F)
                        sprintf(at_status.DATA_buffer,"+OK=AUTO:00%x\r\n",panid);
                    else if(panid<=0x0FFF && panid>0x00FF)
                        sprintf(at_status.DATA_buffer,"+OK=AUTO:0%x\r\n",panid);
                    else
                        sprintf(at_status.DATA_buffer,"+OK=AUTO:%x\r\n",panid);
                    SendDatatoComputer(at_status.DATA_buffer);
                }
                else if(at_status.PIDType==1)
                {
                    if(panid<=0x000F)
                        sprintf(at_status.DATA_buffer,"+OK=SET:000%x\r\n",panid);
                    else if(panid<=0x00FF && panid>0x000F)
                        sprintf(at_status.DATA_buffer,"+OK=SET:00%x\r\n",panid);
                    else if(panid<=0x0FFF && panid>0x00FF)
                        sprintf(at_status.DATA_buffer,"+OK=SET:0%x\r\n",panid);
                    else
                        sprintf(at_status.DATA_buffer,"+OK=SET:%x\r\n",panid);
                    SendDatatoComputer(at_status.DATA_buffer);                    
                }
            break;
            }
            //获取Channel
            case ENUM_AT_GETCH:
            {
                uint32 channe;
                zb_ReadConfiguration(ZCD_NV_CHANLIST, sizeof(uint32), &channellist);
                //sprintf(at_status.DATA_buffer,"+OK=Channel:%x\r\n",channellist);
                for(i=0;i<32;i++)
                {
                    channellist=channellist>>1;
                    if(channellist&&0x01==1)
                        if((i+1)<=0x0F)
                            sprintf(at_status.DATA_buffer,"+OK=0%x\r\n",i+1);
                        else
                            sprintf(at_status.DATA_buffer,"+OK=%x\r\n",i+1);
                }
                SendDatatoComputer(at_status.DATA_buffer);
            break;  
            }
            case ENUM_AT_SETDEVICE:
#if 0
                     //afAddrType_t dstAddr;
                      zAddrType_t destAddr;
                      destAddr.addrMode = Addr16Bit;
                      destAddr.addr.shortAddr =  AssociatedDevList[0].shortAddr;
                      dstAddr.addrMode=afAddr16Bit;
                      //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                      //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                      dstAddr.addr.shortAddr=AssociatedDevList[0].shortAddr;
                      dstAddr.endPoint=1;
                      switch(data1[0])
                      {
                        case 0:
                           zclGeneral_SendLevelControlMove(1, &dstAddr,data1[1],10,  false, 0);
                           ZDP_ActiveEPReq( &destAddr,  AssociatedDevList[0].shortAddr, 0);
                         break;
                        case 1:
                           // zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 13, &zclZHAtest_DstAddr,10, 10,   false, 0 );
                            zclLighting_ColorControl_Send_MoveToColorTemperatureCmd( 1, &dstAddr,data1[1], 10,   false, 0 );
                            ZDP_SimpleDescReq( &destAddr,  AssociatedDevList[0].shortAddr,1, 0);
                         break;
                        case 2:
                            zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1, &dstAddr,data1[1],10,  false, 0);
                              zclReadCmd_t BasicAttrsList;
                              BasicAttrsList.numAttr = 6;
                              BasicAttrsList.attrID[0] = ATTRID_BASIC_ZCL_VERSION;
                              BasicAttrsList.attrID[1] = ATTRID_BASIC_HW_VERSION;
                              BasicAttrsList.attrID[2] = ATTRID_BASIC_MODEL_ID;
                              BasicAttrsList.attrID[3] = ATTRID_BASIC_ZCL_VERSION;
                              BasicAttrsList.attrID[4] = ATTRID_BASIC_DATE_CODE;
                              BasicAttrsList.attrID[5] = ATTRID_BASIC_POWER_SOURCE;
                                zcl_SendRead( 1, &dstAddr,
                                        ZCL_CLUSTER_ID_GEN_BASIC, &BasicAttrsList,
                                        ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                        break;
                      case 3:
                              NLME_LeaveReq_t req;
                              APSME_LookupExtAddr(AssociatedDevList[0].shortAddr,req.extAddr);

                              req.removeChildren = FALSE;
                              req.rejoin         = FALSE;
                              req.silent         = FALSE;
                              NLME_LeaveReq(&req);
                      //  zcl_SendCommand( 1, &dstAddr, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                      //        ATTRID_MS_TEMPERATURE_MEASURED_VALUE, TRUE, 
                     //         ZCL_FRAME_SERVER_CLIENT_DIR, false, 0, 0, 3, data1 );
                        break;
                      case 4:
                        zcl_SendCommand( 1, &dstAddr, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
                              ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE, TRUE, 
                              ZCL_FRAME_SERVER_CLIENT_DIR, false, 0, 0, 3, data1 );
                        break;
                      case 5:
                                      //zclReadCmd_t BasicAttrsList;
                              BasicAttrsList.numAttr = 1;
                              BasicAttrsList.attrID[0] = ATTRID_ON_OFF;
                              //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                            zcl_SendRead( 0, &dstAddr,
                                            ZCL_CLUSTER_ID_GEN_ON_OFF, &BasicAttrsList,
                                        ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                        break;
                      case 6:
                        zclGeneral_SendOnOff_CmdToggle(7, &dstAddr, false, 0 );
                        break;
                        
                      }
#endif
            {
                  zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
                  //zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                  if(strcmp(data1,"light")==0)
                  {
                      //at_status.AT_logicalType = ZG_DEVICETYPE_ENDDEVICE;
                      devicetype=ZG_DEVICE_TYPE_LIGHT;
                  }
                  else if(strcmp(data1,"level")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_COLORLIGHT;  
                  }
                  else if(strcmp(data1,"colortem")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_DIMMER;  
                  }
                  else if(strcmp(data1,"temperature")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_THERMOMETER;  
                  }
                  else if(strcmp(data1,"pir")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_PIR;  
                  }
                  else if(strcmp(data1,"humility")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_THERMOHYGROGRAPH;  
                  }
                  else if(strcmp(data1,"doorsen")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_DOORSENSOR;  
                  }
                  else if(strcmp(data1,"lumin")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_ILLUMINANCE;  
                  }
                  else if(strcmp(data1,"slsensor")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_SLSENSOR;  
                  }
                  else if(strcmp(data1,"smoke")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_SMOKESENSOR;  
                  }
                  else if(strcmp(data1,"water")==0)
                  {   
                      devicetype = ZG_DEVICE_TYPE_WATERSENSOR;  
                  }                      
                  zb_WriteConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                  SendDatatoComputer(c);
              break;
              
            }
            case ENUM_AT_GETDEVICE:
            {
                zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                switch(devicetype)
                {
                    case 0:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","light");
                    break;
                    case 1:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","level");
                    break;
                    case 2:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","colortem");
                    break;                      
                    case 3:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","temperature");
                    break;                       
                    case 4:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","pir");
                    break;                       
                    case 5:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","humility");
                    break;                       
                    case 6:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","doorsen");
                    break;                       
                    case 7:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","lumin");
                    break;                       
                    case 8:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","slsensor");
                    break;                      
                    case 9:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","smoke");
                    break;                      
                    case 10:
                        sprintf(at_status.DATA_buffer,"+OK=%s\r\n","water");
                    break;                      
                    default:
                    break;
                }
                SendDatatoComputer(at_status.DATA_buffer);                                 
            break;
            }
            case ENUM_AT_DENYJOIN:
            {
                NLME_LeaveReq_t req;
                //APSME_LookupExtAddr(AssociatedDevList[0].shortAddr,req.extAddr);
                osal_memcpy(data5,data1,8);
                req.extAddr=data5;
                //  req.extAddr = ;
                req.removeChildren = FALSE;
                req.rejoin         = FALSE;
                req.silent         = FALSE;
                if(NLME_LeaveReq(&req)==ZSUCCESS)
                SendDatatoComputer(c);
            break;
            }

            case ENUM_AT_ACCEPTJOIN:
            {
                uint8 k=0;
                uint8 *p1,*p2,*p3,*p4,*p5;
                uint8 mac1[8];
                uint8 mac2[8];
                uint8 mac3[8];
                uint8 mac4[8];
                uint8 mac5[8];
                p1=data1;
                p2=data2;
                p3=data3;
                p4=data4;
                p5=data5;
                for(i=0;i<8;i++)
                {
                    mac1[7-i]=CovertACSIITWOB(p1);
                    mac2[7-i]=CovertACSIITWOB(p2);
                    mac3[7-i]=CovertACSIITWOB(p3);
                    mac4[7-i]=CovertACSIITWOB(p4);
                    mac5[7-i]=CovertACSIITWOB(p5);
                    p1=p1+2;
                    p2=p2+2;
                    p3=p3+2;
                    p4=p4+2;
                    p5=p5+2;
                }
                //StringToHEX(data1);
                for(i=0;i<at_status.deviceCount;i++)
                {
                    if(strcmp(mac1,Node_Info[i].aucMAC)==0 & mac1!=0)
                    {
                        osal_memcpy(&AssoList[k],&Node_Info[i],sizeof(Node_Info[i]));
                        k++;
                    }else if(strcmp(mac2,Node_Info[i].aucMAC)==0 & mac2!=0)
                    {
                        osal_memcpy(&AssoList[k],&Node_Info[i],sizeof(Node_Info[i]));
                        k++;
                    } 
                    else if(strcmp(mac3,Node_Info[i].aucMAC)==0 & mac3!=0)
                    {
                        osal_memcpy(&AssoList[k],&Node_Info[i],sizeof(Node_Info[i]));
                        k++;
                    } 
                    else if(strcmp(mac4,Node_Info[i].aucMAC)==0 & mac4!=0 )
                    {
                        osal_memcpy(&AssoList[k],&Node_Info[i],sizeof(Node_Info[i]));
                        k++;
                    } 
                    else if(strcmp(mac5,Node_Info[i].aucMAC)==0 & mac5!=0 )
                    {
                        osal_memcpy(&AssoList[k],&Node_Info[i],sizeof(Node_Info[i]));
                        //osal_mem_free(&Node_Info[i]);
                        k++;
                    }   
                    else if(strcmp(Node_Info[i].aucMAC,mac1)!=0 | strcmp(Node_Info[i].aucMAC,mac2)!=0 |strcmp(Node_Info[i].aucMAC,mac3)!=0| strcmp(Node_Info[i].aucMAC,mac4)!=0 |strcmp(Node_Info[i].aucMAC,mac5)!=0)
                    {

                        req.extAddr=Node_Info[i].aucMAC;
                        //  req.extAddr = ;
                        req.removeChildren = FALSE;
                        req.rejoin         = FALSE;
                        req.silent         = FALSE;
                        NLME_LeaveReq(&req);
                        //osal_mem_free(&Node_Info[i]);
                    }
                    osal_memset(&Node_Info[i],0,sizeof(Node_Info[i]));
                }
                at_status.deviceCount=0;
                k=0;
                at_status.acceptFlag=1;
                SendDatatoComputer(c);
            
            break;
            }  
                      
            case ENUM_AT_CHECKEP:
            {

                for(j=0;j<21;j++)
                {
                    uint8 x=0;
                    //判断地址是否在列表
                    for(i=0;i<8;i++)
                    {
                        if(Node_Info[j].aucMAC[i] == data1[i])
                            x++;
                        else
                            break;
                    }
                    if(x==8 )
                    { 
                        zAddrType_t simpleDescReqAddr;
                        simpleDescReqAddr.addrMode=afAddr16Bit;
                        //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                        simpleDescReqAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                        ZDP_ActiveEPReq( &simpleDescReqAddr, simpleDescReqAddr.addr.shortAddr, 0);
                    }
                }
              break;
              }         
                
            case ENUM_AT_CHECKSIMPLEDESC:
            {
                  for(j=0;j<21;j++)
                {
                      uint8 x=0;
                         //判断地址是否在列表
                      for(i=0;i<8;i++)
                      {
                              if(Node_Info[j].aucMAC[i] == data1[i])
                                x++;
                              else
                                break;
                      }
                      if(x==8 )
                      { 
                              zAddrType_t simpleDescReqAddr;
                              simpleDescReqAddr.addrMode=afAddr16Bit;
                              //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                              //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                              simpleDescReqAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                              ZDP_SimpleDescReq( &simpleDescReqAddr, simpleDescReqAddr.addr.shortAddr,data1[8], 0);  
                      }
                }
           
              
              break;
              }
            case ENUM_AT_ZONETYPE:
            {
            for(j=0;j<21;j++)
            {
                uint8 x=0;
                //判断地址是否在列表
                for(i=0;i<8;i++)
                {
                    if(Node_Info[j].aucMAC[i] == data1[i])
                        x++;
                    else
                        break;
                }
                if(x==8 )
                { 
                    dstAddr.addrMode=afAddr16Bit;
                    dstAddr.addr.shortAddr=AssociatedDevList[j].shortAddr;
                    BasicAttrsList.numAttr = 1;
                    BasicAttrsList.attrID[0] = ATTRID_SS_IAS_ZONE_TYPE;
                    //BasicAttrsList.attrID[1] = ATTRID_LEVEL_CURRENT_LEVEL;
                    zcl_SendRead( data1[8], &dstAddr,
                                    ZCL_CLUSTER_ID_SS_IAS_ZONE, &BasicAttrsList,
                                    ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0);
                }
            }
            break;
            }
            case ENUM_AT_SETREP:
            {
                zclCfgReportCmd_t cfgRptCmd;
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=AssociatedDevList[0].shortAddr;
                //osal_memcpy(dstAddr.addr.extAddr,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                //dstAddr.addr.shortAddr=Node_Info.uiNwk_Addr;
                cfgRptCmd.numAttr = 1;
                cfgRptCmd.attrList[0].direction = 0;
                cfgRptCmd.attrList[0].attrID = ATTRID_SS_IAS_ZONE_STATUS;
                cfgRptCmd.attrList[0].dataType = ZCL_DATATYPE_BITMAP16;
                cfgRptCmd.attrList[0].minReportInt = 5;
                cfgRptCmd.attrList[0].maxReportInt = 10;
                cfgRptCmd.attrList[0].timeoutPeriod = 0;
                cfgRptCmd.attrList[0].reportableChange = NULL;

                zcl_SendConfigReportCmd(data1[0],
                                        &dstAddr,
                                        ZCL_CLUSTER_ID_SS_IAS_ZONE,
                                        &cfgRptCmd,
                                        ZCL_FRAME_CLIENT_SERVER_DIR,
                                        0, 1);          
            break;
            }
                
            default:
            {
                sprintf(at_status.DATA_buffer,"ERROR:=%s\r\n","COMMAND ERROR");
                SendDatatoComputer(at_status.DATA_buffer);
            }
            break;
        }

        
};

                                    

void NWK_CPMMAND_precess_event()
{
    NWK_COMMAND_BASIC_t *p;
    uint16 IDX_CMD;
    uint8 k=0;
    uint8 i;
    uint8 j;
    zclReadCmd_t BasicAttrsList;
    uint8 logicalType;
    zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
    if(logicalType==0)
    {
        at_status.checkOD=1;
        for(i=0;i<128;i++)
        {
            if(at_status.DATA_buffer[128-i]==0x00 && at_status.DATA_buffer[127-i]==0x23&&at_status.DATA_buffer[0]==0x2A)
            {
                    
                k=1;
               
            }
            else
            {
                k=0;
            }
        }
        if(k=1)
        {
            //
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
            osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,at_status.DATA_buffer+37,NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Length);
              
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
                                Return_Message1(4);
                            break;                                     
                            case 2:
                                osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.aucHardware_Version,4);
                                Return_Message1(4);
                            break;
                            case 3:
                                osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.aucDev_Type,6);
                                Return_Message1(6);
                            break;
                            case 4:
                                osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,Node_Status.auiSurport_Func,8);
                                Return_Message1(8);
                            break;
                        }
                    }
                
                break;
                }
                case DATE_TIME_Idx:
                    HalUARTWrite(HAL_UART_PORT_0,at_status.DATA_buffer,sizeof(at_status.DATA_buffer));
                break;
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
                        Return_Message1(38);
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
                        Return_Message1(3);
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
                                        Return_Message1(1);
                                    }else if(appState == APP_INIT)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                        Return_Message1(1);
                                        appState == APP_START;
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
                                        NLME_LeaveReq_t req;

                                        req.removeChildren = FALSE;
                                        req.rejoin         = FALSE;
                                        req.silent         = FALSE;

                                        if(NLME_LeaveReq(&req)==ZSuccess)
                                        {
                                            //DelayMS(3000);
                                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                            Return_Message1(1);
                                        }
                                        else
                                        {
                                            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                            Return_Message1(1);                                         
                                        }
                                        // appState = APP_INIT;
                                        //DelayMS(1000);
                                        // uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
                                        //zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
                                        //Onboard_soft_reset();

                                    }else if(appState == APP_INIT)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                        Return_Message1(1);
                                    }
                                }
                                //以默认配置建立网络
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==0x02)
                                {
                                    if(appState == APP_START)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                        Return_Message1(1);
                                    }else if(appState == APP_INIT)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                        Return_Message1(1);
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
                                    for(i=0;i<at_status.deviceCount;i++)
                                    {
                                        AssocRemove(Node_Info[i].aucMAC);
                                        osal_memset(&Node_Info[i],0,sizeof(Node_Info[i]));
                                    }
                                    NLME_PermitJoiningRequest(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]);

                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x00;
                                    Return_Message1(1);
                                }
                            break;
                            default:
                                //返回错误值
                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
                                Return_Message1(1);
                            break;
                        }
                    }
                    else
                    {
                        //Target/source地址错误
                        osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,NWK_command.NWK_General_Frame.NWKCG_SourceAddress,8);
                        osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+8,NWK_command.NWK_General_Frame.NWKCG_TargetAddress,8);
                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[16]=0x70;
                        Return_Message1(17);
                    }
                break;	
                }

                case FORWARD_PARAMETER_Idx:
                break;
                case NODE_INFO_Idx:
                {
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
                                    if(at_status.acceptFlag==1)
                                    {
                                        if(AssociatedDevList[j].shortAddr!=0x0000 && AssociatedDevList[j].shortAddr!=0xffff)
                                        {
                                            if( APSME_LookupExtAddr(AssociatedDevList[j].shortAddr,Node_Info[j].aucMAC ))
                                            {
                                                //osal_memset(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data,0,90);
                                                osal_memcpy(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data+16*j,Node_Info[j].aucMAC,8 );                             
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[8+16*j]=AssociatedDevList[j].shortAddr&0x00FF;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[9+16*j]=(AssociatedDevList[j].shortAddr&0xFF00)>>8;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[11+16*j]=0x01;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[12+16*j]=Node_Info[j].supportOD&0x00FF;;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[13+16*j]=(Node_Info[j].supportOD&0xFF00)>>8;;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[14+16*j]=0x00;
                                                NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[15+16*j]=0x00;
                                                k++;
                                            }
                                        }
                                    }
                                }
                                Return_Message1(80);
                            }
                        }
                    }
                break;
                }
                case NEIGHBOR_INFO_Idx:
                break;
                case TEMPERATURE_Idx:
                {
                    //温度状态获取
                    for(j=0;j<21;j++)
                    {
                        uint8 x=0;
                        //判断地址是否在列表
                        for(i=0;i<8;i++)
                        {
                            if(Node_Info[j].aucMAC[i] == NWK_command.NWK_General_Frame.NWKCG_TargetAddress[i])
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
                    //灯控制及状态获取
                    for(j=0;j<21;j++)
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
                                    zcl_SendRead( 2, &dstAddr,
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
                                    Return_Message1(1);
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                                {
                                    dstAddr.endPoint=1;
                                    zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                    zclGeneral_SendLevelControlMoveToLevel(1, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0);
                                    NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                    Return_Message1(1);
                                }
                            }
                        }
                    }
                break; 
                }

                case Light_Temperature_Idx:
                {
                    for(j=0;j<21;j++)
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
                                        Return_Message1(1);
                                    }
                                }
                            }
                        }
                    }
                
                break;
                }
                case PIR_Status_Idx:
                {
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                                        Return_Message1(1);                                   
                                    }
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]==1)
                                {
                                    if(zclSS_Send_IAS_WD_SquawkCmd(1, &dstAddr, NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[1],10,  false, 0)==ZSUCCESS)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                        Return_Message1(1);                                   
                                    } 
                                }
                            }
                        }
                    }
                break;
                }                          

                case Smoke_Detector_Idx:
                {
                    for(j=0;j<21;j++)
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
                    for(j=0;j<21;j++)
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
                                    Return_Message1(1);
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==3)
                                {
                                    zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                    if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                        Return_Message1(1);
                                    }
                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==4)
                                {
                                    zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                    if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                        Return_Message1(1);
                                    }

                                }
                                else if(NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_SUBIndex==5)
                                {
                                    zclZHAtest_Level_to_Level=NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0];
                                    if(zclGeneral_SendLevelControlMoveToLevel(2, &dstAddr,zclZHAtest_Level_to_Level,10,  false, 0)==ZSUCCESS)
                                    {
                                        NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=1;
                                        Return_Message1(1);
                                    }
                                }
                            }
                        }
                    }
                break;
                }            
                case Over_Walter_Idx:
                {

                    for(j=0;j<21;j++)
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
                    Return_Message1(1);
                break;
                }
            }

        }
        else
        {
            NWK_command.NWK_General_Frame.NWK_AppDev_Frame.NWKCA_Data[0]=0x70;
            Return_Message1(1);
        
        }
    }
}
