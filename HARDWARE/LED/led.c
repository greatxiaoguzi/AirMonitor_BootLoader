#include "include.h" 	   
BEEP Bee;
//初始化D2为输出口.并使能这个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;			
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PD端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LED0-->PD2 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD2
	GPIO_SetBits(GPIOC,GPIO_Pin_4);						 //PD2 输出高
}
void LEDAlarm(uint8_t Times)
{
    uint8_t i;
    for(i=0;i<Times;i++)
    {
        LED0 = !LED0;
        delay_ms(300);
    }
}
//蜂鸣器端口初始化
void Beep_init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
	
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);	
   GPIO_ResetBits(GPIOA,GPIO_Pin_3);//开始时关闭，蜂鸣器不响
	 
   Bee.BeepEn = 0;  //使能
   Bee.BeepTime = 0;  //时间
}
//state:响或者不响
//times:鸣叫时间,取值范围 0---2000
void BeepAlarm(uint8_t Enable,uint16_t times)
{
	Bee.BeepTime = times;
	if(Enable==1)
	{
		 Bee.BeepEn = 1;   //使能标志
	   TIM3->CR1 |= TIM_CR1_CEN;  //打开定时器3，为蜂鸣器鸣叫定时		 
	}
	else
	{
		 Bee.BeepEn = 0;                                       //取消使能标志
	}
}



