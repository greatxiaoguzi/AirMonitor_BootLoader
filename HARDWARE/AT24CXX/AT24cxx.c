#include "include.h"
#include "AT24cxx.h" 
#include "delay.h" 
//#include "delay.h"
//24CXX驱动函数(适合24C01~24C16,24C32~256未经过测试!有待验证!)

//初始化IIC接口
//返回值：0成功 1失败
uint8_t AT24CXX_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	IIC_Init();
	//IIC_WP口初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	AT24C08_WP_Low;
	if(AT24CXX_Check()==0)
		return 0;
	else
		return 1;
}
//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址
//返回值  :读到的数据
uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr)
{
	uint8_t temp=0;		  	    																 
    IIC_Start_bit();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//发送高地址	    
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据 	   
	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
	IIC_Wait_Ack();	    
	IIC_Start_bit();  	 	   
	IIC_Send_Byte(0XA1);           //进入接收模式			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//产生一个停止条件	    
	return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(uint16_t WriteAddr,uint8_t DataToWrite)
{				   	  	    																 
    IIC_Start_bit();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//发送高地址	  
	}else IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//产生一个停止条件 
	delay_ms(10);
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(uint16_t WriteAddr,uint32_t DataToWrite,uint8_t Len)
{
	uint8_t t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}
}

//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
uint32_t AT24CXX_ReadLenByte(uint16_t ReadAddr,uint8_t Len)
{  	
	uint8_t t;
	uint32_t temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 
		delay_us(500);		
	}
	return temp;
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
uint8_t AT24CXX_Check(void)
{
	uint8_t temp;
	temp=AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX			   
	if(temp==0XA5)
		return 0;		   
	else//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(255,0XA5);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0XA5)
			return 0;
	}
	return 1;											  
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		delay_ms(1);
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	uint16_t i;
	for(i=0;i<NumToWrite;i++)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}
uint8_t WriteBuf[]= "wojiaoguchenglongsilasiladeganhuo!!!!!!";
uint8_t ReadBuf[50];
//测试是否能够写入和读出
void AT24CXXTest(void)
{
	 AT24CXX_Write(0,WriteBuf,strlen(WriteBuf));
	 AT24CXX_Read(0,ReadBuf,strlen(WriteBuf));
	
//	uint8_t Data[3]={0,0,0};
//	AT24CXX_WriteOneByte(0,111);
//	AT24CXX_WriteOneByte(1,222);
//	AT24CXX_WriteOneByte(2,123);
//	Data[0] = AT24CXX_ReadOneByte(0);
//	delay_us(500);
//	Data[1] = AT24CXX_ReadOneByte(1);
//	delay_us(500);
//	Data[2] = AT24CXX_ReadOneByte(2);
//	WriteBuffer(0,WriteBuf);
//	ReadBuffer(0,ReadBuf,sizeof(WriteBuf));
//	LCD_ShowNum(20,150,Data[0],5,16);
//	LCD_ShowNum(20,170,Data[1],5,16);
//	LCD_ShowNum(20,190,Data[2],5,16);
	LCD_ShowString(0,150,320,16,16,ReadBuf,GREEN,BLACK);
//	LCD_ShowString(20,200,200,16,16,"Finish",GREEN,BLACK);
}











