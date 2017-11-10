#ifndef _ESP8266_H
#define _ESP8266_H
#include "include.h"
#define WaitTime  250  //等待超时时间
#define FLASH_SAVE_ADDR  0X08020000 				//设置FLASH 保存地址(必须为偶数)
__packed typedef struct
{
	uint8_t  WireNet[2][20];
	uint8_t  ComunitPro[4];
	uint8_t  ServerIP[15];
	uint16_t ServerPort;
//	uint8_t  ConectFlag;
	uint8_t  SlrID;
	uint8_t  WifiStartFlag;
	uint8_t  WifiLinkSuccFlag;  //wifi连接成功标志
	uint8_t  ServerLinkStat;       //服务器状态
}Esp8266;
//网络接受到的数据结构体
__packed typedef struct
{
	uint8_t  Switch[4];
	uint8_t  PWM[2];     //存储接收到的PWM的值
}NET;

//服务器密钥
extern uint8_t UserKey[60];
#define MAC  "MACDeviceAdd123456#"  //设备硬件地址
extern NET NetData;   //外部调用
extern Esp8266 Esp8266Config;
extern uint8_t WifiSetFlag;
void Esp8266_Config(void);
uint8_t Esp8266_ReStart(void);
uint8_t  Esp8266_SetMode(uint8_t Mode);
uint8_t Esp8266_CheckNET(void);
void Esp8266_BackIPAdd(void);
uint8_t Esp8266_ConnectWire(void);
uint8_t Esp8266_OpenMoreConect(void);
uint8_t Esp8266_ConectServer(void);
void TryReConectServer(void);
uint8_t Esp8266_DisConectServer(void);
uint8_t Esp8266_CIPSendData(char *Data,uint16_t Len);
void ConfigServerMode(void);
uint8_t GetEsp8266CurrStat(void);
void TryReConectWifi(void);
uint8_t ReadServerPar(void);



uint8_t ListenRec(void);
void ClearBuffer(uint8_t *data);
uint8_t Hand(char *a,uint16_t Time);
uint8_t SpecStrAdj(uint8_t *string);
uint8_t WifiNETSet(void);
void NETDataDeal(void);
uint8_t GetIPAddr(void);

#endif



