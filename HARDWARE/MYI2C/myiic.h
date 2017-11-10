#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"
#include "include.h" 	  

//GPIO操作说明:GPIOB->CRL寄存器主管引脚0-7，GPIOB->CRH主管引脚8-15，寄存器每四位负责一个引脚的设置，只需配置CNFy和MODEy即可
//IO方向设置 已完全理解
#define SDA_IN()  {GPIOC->CRL&=0XFFFFFF0F;GPIOC->CRL|=8<<4;}  //8左移4位：0XFFFFFF0F | 0000 0000 1000 0000  相当于把寄存器CRL第7位置一，为输入
#define SDA_OUT() {GPIOC->CRL&=0XFFFFFF0F;GPIOC->CRL|=3<<4;}  //3左移4位：0XFFFFFF0F | 0000 0000 0011 0000  相当于把寄存器的第4和5位置一，为输出

//IO操作函数	
#define IIC_SCL    PCout(0) //SCL
#define IIC_SDA    PCout(1) //SDA
#define READ_SDA   PCin(1)  //输入SDA

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void  IIC_Start_bit(void);				        //发送IIC开始信号
void IIC_Stop(void);	  			      //发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			    //IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号 
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  




#endif
















