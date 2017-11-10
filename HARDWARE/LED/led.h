#ifndef __LED_H
#define __LED_H	 
#include "include.h"								  
////////////////////////////////////////////////////////////////////////////////// 
typedef struct
{
   uint8_t BeepEn;
	 uint8_t BeepTime;
}BEEP;
extern BEEP Bee;  //蜂鸣器状态标志

#define LED0    PCout(4)	//     指示灯

void LED_Init(void);//初始化
void BeepAlarm(uint8_t Enable,uint16_t times);  //蜂鸣器驱动
void Beep_init(void);  //蜂鸣器初始化
void LEDAlarm(uint8_t Times);
void JDQInit(void);
#endif
