#ifndef __Time3_pwm_init
#define __Time3_pwm_init
#include <stm32f10x.h>
#include "include.h"
uint16_t Timecnt2=0;
extern uint8_t CntFlag;
void Timer2_Init(uint32_t period)
{
    TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;     //定时时基
    NVIC_InitTypeDef NVIC_InitStructure;                 //中断配置
	/* Compute the prescaler value */
	/* Time base configuration */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   //时钟使能
	TIM_TimeBaseStructure.TIM_Period = period;//1ms产生一次中断		       //计数上线	500,50ms产生一次中断
	TIM_TimeBaseStructure.TIM_Prescaler = 72000-1;	           //产生1KHz的频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);        //初始化结构体		
	
	TIM_ARRPreloadConfig(TIM2, ENABLE);   //使能预装载寄存器
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); 						//TIM2_DIER[2]=1  允许更新中断
	TIM_Cmd(TIM2, DISABLE);                      //使能定时器3计时
	NVIC_InitStructure.NVIC_IRQChannel = 	TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器  
}
//定时器3任务：判断输出端口的状态以及为蜂鸣器提供定时时间
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)     //确认是定时器2产生中断
    {
        TIM2->SR = (uint16_t)~TIM_IT_Update;		       //清除中断标志位
		CntFlag = 0;										//置零标志
	}
}
//定时器PWM初始化
void Tim3_init(void)
{
    TIM_OCInitTypeDef TIM_OCInitStructure;       //输出比较
    TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;     //定时时基
    GPIO_InitTypeDef GPIO_InitStructure;
  
	uint16_t PrescalerValue = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE);
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;  //3-1=2;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 999;		//定时器预装载值
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;	//pwm时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	/* PWM1 Mode configuration: Channel */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;//初始占空比为0
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	//初始化PWM输出控制引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; //PBO PB1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	//TIM3->CCR4=250;
}
//设置PWM的值，调节灯光的亮度
void SetDriverPWM(uint16_t Drive1,uint16_t Drive2)
{	
  if(Drive1>1000)
	  Drive1 = 1000;
	else if(Drive1<=0)
		Drive1 = 0;
	if(Drive2>1000)
	  Drive2 = 1000;
	else if(Drive2 <= 0)
		Drive2 = 0;
	TIM3->CCR1 = Drive1;
	TIM3->CCR2 = Drive2;
}




#endif


