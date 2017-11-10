#ifndef _UARTSCREEN_H
#define _UARTSCREEN_H
#include "include.h"
#pragma pack(1)
/*数据帧结构*/
typedef struct SENDPKG
{
   uint8_t  head;        //EE  帧头
   uint8_t  FrameId[4];  //B1 10 00 00  画面切换指令 
   uint8_t  WidgetId[2];   //00 00 控件Id
   uint8_t  Data[4];     //数据内容
   uint8_t  Tail[4];     //帧尾
}SendPkg;
typedef struct DATADEAL
{
   uint8_t Temp[4];  //温度
   uint8_t Damp[4];  //湿度
   uint8_t JiaQuan_mg[4];  //甲醛
   uint8_t JiaQuan_ppm[4]; //甲醛
   uint8_t Presure[4];    //气压
   uint8_t AnoSensor[4][4];  //四路模拟传感器
}DataDeal;

//系统状态指令数据结构
typedef struct SCRESTAT
{
   uint8_t Frame[4];  //屏幕画面
   uint8_t aWake[5];  //唤醒事件
}ScreStat;
extern SendPkg SendSensorData;  //发送传感器数据
extern DataDeal TrnasfData;   //传感器数据数据
void UartScreenInit(void);
void aWake(uint16_t FrameId,uint8_t Enable,uint8_t BL_ON,uint8_t BL_OFF,uint8_t BL_ON_Time);

void SendDataToScreen(uint16_t FrameId,uint16_t WidgetId,uint8_t *DataBuf);
void DataTransf(void);
void ScreenDispData(void);
#endif


























