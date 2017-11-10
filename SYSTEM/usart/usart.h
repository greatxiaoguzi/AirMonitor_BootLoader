#ifndef __USART_H
#define __USART_H
#include "sys.h" 
#define UartMode  1
#define TFTMode   0
//串口1初始化		   
//V1.3修改说明
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//////////////////////////////////////////////////////////////////////////////////
//帧处理
#define Frme_Head        0              //帧头开始
#define Frme_CmdIndex    2              //命令索引
#define Frme_ID          4              //设备ID
#define Frme_DtaType     5              //数据类型从7开始
#define Frme_DtaLen      6              //数据长度
#define Frme_DtaCoten    7              //数据内容
#define MAX_DTA_TYPE     0X07           //7种数据类型 
#define MAX_PKG_LEN       53
///////////////////////////////////////////////
#define USART_REC_LEN  			150  	//定义最大接收字节数 10
//#define USART_TEC_LEN       7     //定义最大发送字节数为7
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收 	
extern uint8_t WIFIRecDataFlag; //串口接收到数据
extern uint8_t UartRecBuf[USART_REC_LEN];
//////////////////////////////////////////
//extern uint8_t JQRecFlag;
extern uint8_t JQDataDealRes;
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound1,u32 bound3,u32 bound4);
void UsartSendDex(uint8_t num,uint8_t data);
void ClearBuf(void);

uint8_t Sensor_Data_To_cJson(void);
void SendJQCmd(uint8_t const *buf);         //发送甲醛读取函数
uint8_t JQSensorTest(void);
void UartSendText(char *text,uint8_t uartNum);
void PB11ConfigUart(uint8_t Mode);

void FreecJSONSpace(char *p);
#endif


