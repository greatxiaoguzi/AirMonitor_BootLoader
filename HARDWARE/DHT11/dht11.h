#ifndef __DHT11_H
#define __DHT11_H 
#include "sys.h"   
//IO方向设置  C5
#define DHT11_IO_IN()  {GPIOC->CRL&=0XFF0FFFFF;GPIOC->CRL|=8<<20;}    //配置为输入
#define DHT11_IO_OUT() {GPIOC->CRL&=0XFF0FFFFF;GPIOC->CRL|=3<<20;}    //配置为输出
////IO操作函数											   
#define	DHT11_DQ_OUT PCout(5) //数据端口	PC5
#define	DHT11_DQ_IN  PCin(5)  //数据端口	PC5


uint8_t DHT11_Init(void);//初始化DHT11
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi);//读取温湿度
uint8_t DHT11_Read_Byte(void);//读出一个字节
uint8_t DHT11_Read_Bit(void);//读出一个位
uint8_t DHT11_Check(void);//检测是否存在DHT11
void DHT11_Rst(void);//复位DHT11   
void DHT11_Filted(uint8_t *temp,uint8_t *humi);
#endif


