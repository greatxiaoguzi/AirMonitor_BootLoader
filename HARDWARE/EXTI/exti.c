#ifndef __EXTI__C
#define __EXTI__C
//按键中断部分
#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "stm32f10x_exti.h"
#include "include.h"
//外部IO接口GPIOC3和GPIOC5
//外部中断初始化函数
/*void EXTIX_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟
    //初始化中断PC3和PC5IO口的时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPD;		//上拉输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //GPIOA.10	  中断线以及中断初始化配置
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource2);

    EXTI_InitStructure.EXTI_Line=EXTI_Line2;            
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			//使能按键所在的外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级2， 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;					//子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure); 
    //EXTI_InitStructure.EXTI_LineCmd = DISABLE;
}*/
/*void EXTI2_IRQHandler(void)
{
    //uint8_t status;
//    uint8_t sta;
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();     //保存全局中断标志
    OSIntNesting++;
    OS_EXIT_CRITICAL();      //恢复全局中断标志		  
    //SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
    //status=NRF24L01_Read_Buf(STATUS,Che_StaReg,1);
    if(EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
       if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)==0)
        {

        }
        EXTI_ClearITPendingBit(EXTI_Line2);  //清除EXTI0线路挂起位
    }
    OSIntExit();
}
*/

#endif


