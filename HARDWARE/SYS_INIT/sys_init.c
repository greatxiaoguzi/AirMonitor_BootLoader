#ifndef __SYS_INIT_C
#define __SYS_INIT_C
#include "include.h"
//系统时钟配置，外设时钟配置
void sys_clock_init(uint8_t PLL)
{
	unsigned char temp=0;
	//MYRCC_DeInit();		  //复位并配置向量表
 	RCC->CR|=0x00010000;  //外部高速时钟使能HSEON
	while(!(RCC->CR>>17));//等待外部时钟就绪
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//抵消2个单位
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON
	FLASH->ACR|=0x32;	  //FLASH 2个延时周期

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟
	while(temp!=0x02)     //等待PLL作为系统时钟设置成功
	{
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}
	//打开外设时钟，APB2总线上
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //打开要使用的外设时钟,GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //打开要使用的外设时钟,GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  //打开要使用的外设时钟,GPIOC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);   //打开要使用的外设时钟,ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PA,PD端口时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	 //使能PB,PF端口时钟

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	RCC_ClockSecuritySystemCmd(ENABLE);                    //系统时钟安全设置，
}
#endif


