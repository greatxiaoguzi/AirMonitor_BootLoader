#ifndef __ADC_H
#define __ADC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////
#define THRESHOLD   5  //传感器变化阈值
#define PM2_5Ctrl  PCout(3)
extern uint8_t const ChalTab[4];  //要转换的ADC的通道
extern uint8_t const JQCmd[5];
extern uint8_t const JQOFFSET[5];
//传感器数据结构体
typedef struct  SENSOR
{		
	uint32_t Result_UPres;//气压
	uint16_t Jiaquanmg;    //甲醛数据
	uint16_t Jiaquanppm;
	struct ANOSENOR   //ADC模拟传感器数据结构
	{
		uint16_t LightStren;  //光强
		uint16_t PM2_5;			//PM2.5
		uint16_t FireGas;		//可燃气体
		uint16_t BatVol;		//电池电压
	}AnoSenor;		
	uint8_t Temp;//温度数据
	uint8_t Damp;//湿度  
}Sensor;
extern Sensor SesorData[2];
extern uint8_t const ThreVal[7];
void Adc_Init(void);
void ADC_DMAInit(DMA_Channel_TypeDef* DMA_CHx,u32 PeripAdd,u32 MemAdd,uint16_t BufLen);

uint16_t Get_Adc(uint8_t ch);
uint16_t Get_Adc_Average(uint8_t ch,uint8_t times);
uint16_t Pm2_5Read(uint8_t ch,uint8_t times);
uint8_t SensorDtaChageCheck(Sensor *Sensor);
#endif 


