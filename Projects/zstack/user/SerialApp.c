
#include <stdio.h>

#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "hal_uart.h"





#include "SerialApp.h"

//static uint8 sendMsgTo_TaskID;
static void serialAppInitTransport(void);

/*�ú����������������ĳ�ʼ�������е���*/
void SerialApp_Init(void)
{
  //����uart��ʼ������
  serialAppInitTransport();
  //��¼��������taskID������
//  sendMsgTo_TaskID = taskID;
}

/*uart��ʼ�����룬���ô��ڵĲ����ʡ������Ƶ�*/
void serialAppInitTransport( )
{
  halUARTCfg_t uartConfig;

  // configure UART
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = SBP_UART_BR;//������
  uartConfig.flowControl          = SBP_UART_FC;//������
  uartConfig.flowControlThreshold = SBP_UART_FC_THRESHOLD;//��������ֵ��������flowControlʱ����������Ч
  uartConfig.rx.maxBufSize        = SBP_UART_RX_BUF_SIZE;//uart���ջ�������С
  uartConfig.tx.maxBufSize        = SBP_UART_TX_BUF_SIZE;//uart���ͻ�������С
  uartConfig.idleTimeout          = SBP_UART_IDLE_TIMEOUT;
  uartConfig.intEnable            = SBP_UART_INT_ENABLE;//�Ƿ����ж�
  uartConfig.callBackFunc         = sbpSerialAppCallback;//uart���ջص��������ڸú����ж�ȡ����uart����

  // start UART
  // Note: Assumes no issue opening UART port.
  (void)HalUARTOpen( SBP_UART_PORT, &uartConfig );

  return;
}
  uint16 numBytes;
/*uart���ջص�����*/
void sbpSerialAppCallback(uint8 port, uint8 event)
{
  uint8  pktBuffer[SBP_UART_RX_BUF_SIZE];
  // unused input parameter; PC-Lint error 715.
  (void)event;
  int i=0;
  for(i=6000;i>0;i--){
	asm("nop");
  }
  //HalLcdWriteString("Data form my UART:", HAL_LCD_LINE_4 );
  //���ؿɶ����ֽ�
  if ( (numBytes = Hal_UART_RxBufLen(port)) > 0 ){
  	//��ȡȫ����Ч�����ݣ��������һ��һ����ȡ���Խ����ض�������
	(void)HalUARTRead (port, pktBuffer, numBytes);
        //���յ����ݺ������LCD��ʾ��
        HalLcdWriteString((char *)pktBuffer,HAL_LCD_LINE_6);
  }
  
}
void sbpSerialAppWrite(uint8 *pBuffer, uint16 length)
{
	HalUARTWrite (SBP_UART_PORT, pBuffer, length);
}
/*
��ӡһ���ַ���
str�����԰���0x00�����ǽ�β
*/
void SerialPrintString(uint8 str[])
{
  HalUARTWrite (SBP_UART_PORT, str, osal_strlen((char*)str));
}
/******************************************************************************
*��    �ƣ�putchar
*��    �ܣ�����ַ���
*��ڲ������ַ���ASCII
*���ڲ�������
*����ֵ  ��������ַ���
******************************************************************************/
__near_func int  putchar(int c)
{
  if(c >= 0)
  {
    if(SBP_UART_PORT == HAL_UART_PORT_0 )
    {
      U0DBUF = (uint8)c;
      while(UTX0IF == 0);
      UTX0IF = 0;
    }
    else
    {
      U1DBUF = (uint8)c;
      while(UTX1IF == 0);
      UTX1IF = 0;
    }
    return c;
  }
  else
  {
    return -1; 
  }
}
/*
��ӡָ���ĸ�ʽ����ֵ
����
title,ǰ׺�ַ���
value,��Ҫ��ʾ����ֵ
format,��Ҫ��ʾ�Ľ��ƣ�ʮ����Ϊ10,ʮ������Ϊ16
*/
void SerialPrintValue(char *title, uint16 value, uint8 format)
{
  uint8 tmpLen;
  uint8 buf[256];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  osal_memcpy( buf, title, tmpLen );
  buf[tmpLen] = ' ';
  err = (uint32)(value);
  _ltoa( err, &buf[tmpLen+1], format );
  SerialPrintString(buf);		
}

