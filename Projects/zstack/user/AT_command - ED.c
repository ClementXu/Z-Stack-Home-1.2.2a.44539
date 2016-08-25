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




typedef enum
{
    ENUM_AT_MAC=1,
    ENUM_AT_FMVER,
    ENUM_AT_SAVE,
    ENUM_AT_FACTORY,
    ENUM_AT_REBOOT,
    ENUM_AT_DEVICE,
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
    ENUM_AT_WATERSENSOR, 
    ENUM_AT_COSENSOR,
    ENUM_AT_GASSENSOR,
    ENUM_AT_GLASSSENSOR,
    ENUM_AT_ZONECONTROL,
    ENUM_AT_OUTLET,
    ENUM_AT_LIGHTSWITCH,
    ENUM_AT_CHANNEL,
    ENUM_AT_PANID,

}at_command;





void AT_Init()
{
    AT_GROUP[ENUM_AT_MAC]="MAC";
    AT_GROUP[ENUM_AT_FMVER]="FMVER";
    AT_GROUP[ENUM_AT_SAVE]="SAVE";
    AT_GROUP[ENUM_AT_FACTORY]="FACTORY";
    AT_GROUP[ENUM_AT_REBOOT]="REBOOT";
    AT_GROUP[ENUM_AT_DEVICE]="DEVICE";
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
    AT_GROUP[ENUM_AT_XYY]="XYY";
    AT_GROUP[ENUM_AT_GETEXTPID]="GETEXTPID";
    AT_GROUP[ENUM_AT_WATERSENSOR]="WATERSEN";
    AT_GROUP[ENUM_AT_CHANNEL]="CHANNEL";
    AT_GROUP[ENUM_AT_PANID]="PANID";


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
    DEVICE_NAME[glasssensor]="glasssensor";
    DEVICE_NAME[zonecontroller]="zonecontroller";
    DEVICE_NAME[outlet]="outlet";
};
void SendDatatoComputer(char *p)
{
    HalUARTWrite(HAL_UART_PORT_0,p,strlen(p));
}



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
    //if()
    //VALUE_MIX
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


void _SetTemp(getDeviceData Setting)
{
    char AT_Format[]="%d.%d";
    char O_Format[]="%d";
    uint8 buffer[16];
    uint8 i,len=0,sign=0;
    int8 data[2],data1[2];
    osal_memset(buffer,0,16);
    osal_memset(data,0,2);
    osal_memset(data1,0,2);
    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == '.')
            {
                sign = 1;
                len = strlen(&Setting.RecData[i+1]);
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(len>=2)
        {
            strncpy(buffer,Setting.RecData,strlen(Setting.RecData)-len+2);
        }else if(len<2 && sign==1)
        {
            strcpy(buffer,Setting.RecData);
            strcat(buffer,"0");
        }else
        {
            strcpy(buffer,Setting.RecData);
        }
        if(sign == 1)
        {
            sscanf(buffer,AT_Format,data,data1);
        }else if(sign == 2)
        {
            sscanf(buffer,O_Format,data);
        }
        if(data[0]<0)
        {
            zclZHAtest_Temperature_Value = ((data[0]<<8))/2.56 - data1[0];
        }
        else
        {
            zclZHAtest_Temperature_Value = ((data[0]<<8))/2.56 + data1[0];
        }

        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
    }

}

void _SetHumility(getDeviceData Setting)
{
    char AT_Format[]="%d.%d";
    char O_Format[]="%d.%d";
    uint8 buffer[16];
    uint8 i,len=0,sign=0;
    uint8 data[2],data1[2];
    osal_memset(buffer,0,16);
    osal_memset(data,0,2);
    osal_memset(data1,0,2);
    if(Setting.RecSign == 1)
    {
        for(i=0;i<strlen(Setting.RecData);i++)
        {
            if(Setting.RecData[i] == '.')
            {
                sign = 1;
                len = strlen(&Setting.RecData[i+1]);
                break;
            }
            else
            {
                sign = 2;
            }
        }
        if(len>=2)
        {
            strncpy(buffer,Setting.RecData,strlen(Setting.RecData)-len+2);
        }else if(len<2 && sign==1)
        {
            strcpy(buffer,Setting.RecData);
            strcat(buffer,"0");
        }else
        {
            strcpy(buffer,Setting.RecData);
        }
        if(sign == 1)
        {
            sscanf(buffer,AT_Format,data,data1);
        }else if(sign == 2)
        {
            sscanf(buffer,O_Format,data);
        }
        zclZHAtest_Humidity_Value = ((data[0]<<8))/2.56 + data1[0];
        HalUARTWrite(HAL_UART_PORT_0,"+OK\r\n",sizeof("+OK\r\n"));
    }


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
                sprintf(buffer,"+OK=TESTDEVICE V1.9\r\n");
                SendDatatoComputer(buffer);  
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
            //LED灯本地控制命令         
            case ENUM_AT_LIGHT:
            {
                sendData[0] = zclZHAtest_OnOff;
                Setting.data[0] =&sendData[0];
                Setting.length = 1;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_OnOff = sendData[0];
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
                sendData[0] = zclZHAtest_OnOff;
                sendData[1] = zclZHAtest_Level_to_Level/2.56;
                Setting.data[0] =&sendData[0];
                Setting.data[1] =&sendData[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_OnOff = sendData[0];
                zclZHAtest_Level_to_Level = sendData[1]*2.56;
            break;
            }
            //本地人体热功能控制
            case ENUM_AT_PIR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[4];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;

            break;    
            }
             //本地温度测量控制
            case ENUM_AT_TEMP:
            {
                _SetTemp(Setting);

            break;   
            }
            //色温控制
            case ENUM_AT_COLORTEM:
            {
                sendData[0] = zclZHAtest_OnOff;
                sendData[1] = zclZHAtest_Level_to_Level/2.56;
                Setting.data[0] =&sendData[0];
                Setting.data[1] =&sendData[1];
                Setting.data[2] =(uint32 *)&zclZHAtest_Light_Color_Status;
                Setting.length = 3;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_OnOff = sendData[0];
                zclZHAtest_Level_to_Level = sendData[1]*2.56;
            break;
            }
            //湿度传感器
            case ENUM_AT_HUMILITY:
            {
                _SetHumility(Setting);
            break;
            }
            //门磁
            case ENUM_AT_DOORSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[4];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;
            break;
            }
            //声光报警器
            case ENUM_AT_SOUNDLIGHTSENSOR:
            {        
                uint32 data[4];
                data[0] = zclZHAtest_Warning | zclZHAtest_WD_SQUAWK;
                data[1] = zclZHAtest_WD_Duration;
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                if(data[0]<=3)
                    zclZHAtest_Warning = data[0];
                else if(data[0]>=4 & data[0]<6)
                    zclZHAtest_WD_SQUAWK = data[0]-4;
                zclZHAtest_WD_Duration = data[1];
              
            break;
            }
            //烟雾报警器
            case ENUM_AT_SMOKEDETECTOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1; 
                uint32 data[4];
                bitProcess(zclZHAtest_Alarm_Status,data);
                data[2] = zclZHAtest_Alarm_Type ;
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.data[2] =&data[2];
                Setting.length = 3;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                zclZHAtest_Smoke_Type = data[2];
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;
            break;
            }
            //光照强度检测
            case ENUM_AT_LUMIN:
            {
                Setting.data[0] =(uint32 *)&zclZHAtest_Illumiance_Value;
                Setting.length = 1;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
            break;
            }
            //水报警器
            case ENUM_AT_WATERSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[2];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;    
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
             case ENUM_AT_DEVICE:
            {
                zb_ReadConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint16),  &logicalType);
                char buffer[16];
                osal_memset(buffer,0,16);
                if(logicalType == 0)
                {
                    strcpy(buffer,"gateway");
                    Setting.data[0] =(uint32 *)&buffer[0];
                    Setting.length = 1;
                    Setting.type = CHAR;
                    AnaDataProcess(&Setting);
                }
                else
                {
                    zb_ReadConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                    osal_memcpy(buffer,DEVICE_NAME[devicetype],strlen(DEVICE_NAME[devicetype]));
                    Setting.data[0] =(uint32 *)&buffer[0];
                    Setting.length = 1;
                    Setting.type = CHAR;
                    AnaDataProcess(&Setting);
                    devicetype = Indexofdevice(buffer);
                    zb_WriteConfiguration(ZCD_NV_DEVICE_TYPE, sizeof(uint8), &devicetype);
                    //SendDatatoComputer(at_status.DATA_buffer); 
                }       
                
            break;
            }
            case ENUM_AT_COSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[2];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;                  
                break;
            }
            case ENUM_AT_GASSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[2];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;                  
                break;
            }
            case ENUM_AT_GLASSSENSOR:
            {
                dstAddr.addrMode=afAddr16Bit;
                dstAddr.addr.shortAddr=0;
                dstAddr.endPoint=1;          
                uint32 data[2];
                bitProcess(zclZHAtest_Alarm_Status,data);
                Setting.data[0] =&data[0];
                Setting.data[1] =&data[1];
                Setting.length = 2;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_Alarm_Status = data[0] + (data[1]<<1);
                if(Setting.RecSign == 1)
                    zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(1,&dstAddr,zclZHAtest_Alarm_Status,0,false, 0);
                Setting.RecSign = 0;                  
                break;
            }
            case ENUM_AT_OUTLET:
            {
                sendData[0] = zclZHAtest_OnOff;
                Setting.data[0] =&sendData[0];
                Setting.length = 1;
                Setting.type = VALUE_D;
                AnaDataProcess(&Setting);
                zclZHAtest_OnOff = sendData[0];

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