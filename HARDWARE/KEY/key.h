#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	
//配置PC3端口的状态
#define  SET_IN   {GPIOC->CRH&=0XFFFFFF0F;GPIOC->CRH|=8<<4;}   //PC9端口设置为上拉输入模式
#define  SET_OUT  {GPIOC->CRH&=0XFFFFFF0F;GPIOC->CRH|=3<<4;}  //PC9的端口设置为推挽输出模式

#define KEY0  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3)//读取按键0
#define POWER_DETECC GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define POWER_CTRL    PCout(6)
extern volatile uint8_t Encoder_TYPE;   //编码器键值

//按键值
typedef enum
{
	Encoder_Void = 0,
	Encoder_Forword =1,
	Encoder_Backword = 2,
	Encoder_Pressed = 3
}EncoderVal;
void EncoderInit(void);		//编码器初始化
uint8_t KeyScan(void);
void StartMachineInit(void);

	
	
	
	
#endif




