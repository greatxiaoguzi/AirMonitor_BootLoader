#include "key.h"
#include "delay.h"	 
//按键初始化函数 
volatile uint8_t Encoder_TYPE;   //编码器键值
void EncoderInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟		
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTC时钟
	RCC_LSEConfig(RCC_LSE_OFF);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_14|GPIO_Pin_15;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
 	GPIO_Init(GPIOC, &GPIO_InitStructure);			
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource15);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;           
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);	
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line14;           
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   //上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);	
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line15;           
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   //上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级11
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级11
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
}
//开机键初始化
void StartMachineInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 //LED0-->PD2 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}
//编码器键值判断
void EXTI15_10_IRQHandler()
{
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();
#endif
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
		delay_ms(10);
		EXTI->PR = EXTI_Line14;
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14) == 0)
		{
			Encoder_TYPE = Encoder_Forword;
		}
		
	}
	else if(EXTI_GetITStatus(EXTI_Line15) != RESET)
	{
		delay_ms(10);
		EXTI->PR = EXTI_Line15;
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15) == 0)
		{
			Encoder_TYPE = Encoder_Backword;
		}
	}
#ifdef OS_TICKS_PER_SEC	 	
	OSIntExit();  					//退出中断保护				 		 
#endif
}
//按键中断
void EXTI3_IRQHandler(void)
{
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
OSIntEnter();
#endif 	
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
		delay_ms(10);
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)==0)
		{
			Encoder_TYPE = Encoder_Pressed;
			//REncoderHandle(Encoder_TYPE);
		}
		EXTI_ClearITPendingBit(EXTI_Line3); 
	}
#ifdef OS_TICKS_PER_SEC	 	
OSIntExit();  					//退出中断保护				 		 
#endif
}
//按键扫面函数
//支持短按长按功能
//返回值：1长按值，0短按值
uint8_t KeyScan(void)
{
	uint16_t Cnt = 0;
	if(KEY0==0)
	{
		delay_ms(15);	//去抖动 
		if(KEY0==0)
		{
			while(KEY0&&Cnt<=300)
			{
				Cnt++;
				delay_ms(10);
			}
			if(Cnt>300)
				return 0;
			else
				return 0;
		}
	}
	return 1;
}










