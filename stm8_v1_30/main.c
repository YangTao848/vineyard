/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    18-November-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include"SHT10.h"
#include<stdio.h>
#include<SHT10_01.h>
// #include"Usart.h"
/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#define LED_GPIO_PORT  (GPIOD)
#define LED_GPIO_PINS  (GPIO_PIN_4)

float TempResult;   // ��� �¶�����
float HumResult;   // ��� ʪ������
float TempSend[2];
float HumpSedn[2];
uint8_t addressUsart;  // �ڵ��ַ ��� ����
uint8_t ReceFLag;     // 
uint8_t GetUart1Data[4];
uint8_t sendBuff[10];  // �������ݻ��� 
uint16_t TempValue;
uint16_t HumValue;
static void init_usart1(void);
void Send(uint8_t dat);
void SendSting(char * str);
void recerve_usart_intdeal(void);
void Delay(uint16_t nCount)
{
  /* Decrement nCount value */
  while (nCount != 0 )
  {   
    if(ReceFLag && GetUart1Data[0]==addressUsart)
      return;  //��� ���յ� ��ַ��Ϣ  ������ʱ �жϵȴ������Ϣ
    nCount--;
  }
}
void addressInit()
{
   GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);   // �������� 
   GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_5, GPIO_MODE_IN_PU_NO_IT); 
   GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT); 
   GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT); 
   // addressUsart=GPIO_ReadInputPin(GPIOC,GPIO_PIN_4);
   addressUsart=GPIO_ReadInputData(GPIOC);
   addressUsart=addressUsart>>4;
   addressUsart&=0x0F;
}
void addressRead(void)
{
   addressUsart=GPIO_ReadInputData(GPIOC);
   addressUsart=addressUsart>>4;
   addressUsart&=0x0F;
}
void main(void)
{ 

  uint8_t *sendflot;
  uint8_t CheckValue=0x00;
  uint8_t error=0x00;
  uint8_t i;
  void *pt;
  
  /* Initialize I/Os in Output Mode */
  GPIO_Init(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS, GPIO_MODE_OUT_PP_HIGH_SLOW);
  GPIO_Init(GPIOA,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_WriteLow(GPIOA,GPIO_PIN_3);
  SHT10_Config();
  SHT10_Config_1();
  addressInit();
  addressRead();
  // addressUsart=0x02;
  init_usart1();   // �����ж�
  ReceFLag=0;
  enableInterrupts(); 
  while (1)
  {
    /* Toggles LEDs */
    // addressRead();  
   //  if(ReceFLag==1)
   //  disableInterrupts();
      SHT10_ConReset_1(); 
      error += SHT10_Measure_1(&TempValue,&CheckValue,TEMP);
      Delay(1);
      error += SHT10_Measure_1(&HumValue,&CheckValue,HUMI);	
      // Delay(20);
      // SHT10_Cal_1(TempValue ,HumValue,&TempResult,&HumResult);       
      /* ��ֵ*/
      pt=(void *)(&TempValue);
      sendflot=(uint8_t *)pt;
      if(ReceFLag==0)
      {
        for(i=0;i<2;i++)
        {
           sendBuff[i+4]=sendflot[i];
        }
        pt=(void*)(&HumValue);
        sendflot=(uint8_t *)pt;  
        for(i=0;i<2;i++)
        {
            sendBuff[i+6]=sendflot[i];
        }  
        

      }      
      /* ȡ�� ������ �¶�ʪ�� */
      SHT10_ConReset(); 
      error += SHT10_Measure(&TempValue,&CheckValue,TEMP);  
      Delay(1);
      error += SHT10_Measure(&HumValue,&CheckValue,HUMI);	 
      
      pt=(void *)(&TempValue);
      sendflot=(uint8_t *)pt;
      if(ReceFLag==0)
      {
        for(i=0;i<2;i++)
        {
           sendBuff[i]=sendflot[i];
        }
        pt=(void*)(&HumValue);
        sendflot=(uint8_t *)pt;  
        for(i=0;i<2;i++)
        {
            sendBuff[i+2]=sendflot[i];
        } 
      }
      ReceFLag=0;
      GPIO_WriteReverse(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS);
      Delay(200);
    }
  
}

static void init_usart1(void)
{
  UART1_DeInit();
  /* ������ 9600 9λ����  1��ֹͣλ ��  ����żУ��  */
  UART1_Init((u32)115200, UART1_WORDLENGTH_9D, UART1_STOPBITS_1, UART1_PARITY_NO, 
           UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
  UART1_SetAddress(addressUsart);   // set add as 0x01 
  UART1_ReceiverWakeUpCmd(ENABLE);  // RWU set as 1 
  UART1_WakeUpConfig(UART1_WAKEUP_ADDRESSMARK);//  Address wakup   ��ַ���� 
  UART1_ClearITPendingBit(UART1_IT_RXNE); 
  UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
}
void recerve_usart_intdeal(void)
{
  uint8_t i;
  if(GetUart1Data[0]==addressUsart)  //��ַ�жϣ����������Լ�����Ϣ��� ��������ô�˳�����
  { 
    GPIO_WriteReverse(LED_GPIO_PORT, (GPIO_Pin_TypeDef)LED_GPIO_PINS);
    GPIO_WriteHigh(GPIOA,GPIO_PIN_3);
    Send(addressUsart);
    Send('A');
    for(i=0;i<9/*4*sizeof(uint16_t)*/;i++)
    {
       Send(sendBuff[i]);
    }
    GPIO_WriteLow(GPIOA,GPIO_PIN_3);
  }
  ReceFLag=0;
}
void Send(uint8_t dat)
{
  while(( UART1_GetFlagStatus(UART1_FLAG_TXE)==RESET));	
    UART1_SendData8(dat);	
}
void SendSting(char * str)
{
  while(*str++)
  {
    Send(*str);
  }
}
///*
// *   fputc
// */
//int fputc(int ch, FILE *f)
//{
//	/* ??Printf?����Y���騪����??�� */
//    UART1_SendData8((uint8_t) ch);
//    while(( UART1_GetFlagStatus(UART1_FLAG_TXE)!=SET));	
//    return (ch);
//}
#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

