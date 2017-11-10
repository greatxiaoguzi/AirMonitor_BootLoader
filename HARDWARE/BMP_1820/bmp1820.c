#ifndef BMP1820_C   
#define BMP1820_C 
//气压传感器驱动程序部分
#include "include.h"
//参量定义初始化
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;
//bmp1820初始化
uint8_t BMP1820Init(void)
{
    IIC_Init();//初始化IIC总线协议
    //这几个寄存器是只读的寄存器,在数据读取之前要先读取这11个字的数据
    ac1 = ReadBmp1820Data(BMP180_REG_AC1);
    ac2 = ReadBmp1820Data(BMP180_REG_AC2);
    ac3 = ReadBmp1820Data(BMP180_REG_AC3);
    ac4 = ReadBmp1820Data(BMP180_REG_AC4);
    ac5 = ReadBmp1820Data(BMP180_REG_AC5);
    ac6 = ReadBmp1820Data(BMP180_REG_AC6);
    b1 =  ReadBmp1820Data(BMP180_REG_B1);
    b2 =  ReadBmp1820Data(BMP180_REG_B2);
    mb =  ReadBmp1820Data(BMP180_REG_MB);
    mc =  ReadBmp1820Data(BMP180_REG_MC);
    md =  ReadBmp1820Data(BMP180_REG_MD);
	return 0;
}
//读取BMP820数据
uint16_t ReadBmp1820Data(uint8_t ST_Add)
{
    uint8_t msb, lsb;
    uint16_t data;
    IIC_Start_bit();                      //发送起始信号
    IIC_Send_Byte(BMP180_SlaveAddress);    //发送设备地址+写信号
    while(IIC_Wait_Ack());
    IIC_Send_Byte(ST_Add);             //发送存储单元地址
    while(IIC_Wait_Ack());                                 
    IIC_Start_bit();                          //起始信号
    IIC_Send_Byte(BMP180_SlaveAddress+1);         //发送设备地址+读信号
    while(IIC_Wait_Ack());                                        

    msb = IIC_Read_Byte(1);                 //BUF[0]存储                
    lsb = IIC_Read_Byte(0);                //最后一个数据需要回NOACK

    IIC_Stop();                           //停止信号
    delay_ms(5);
    data = msb << 8;
    data |= lsb;	
    return data;
}
//读取气压传感器的压力
uint16_t ReadBmp1820Pressure(void)
{
    IIC_Start_bit();                 //起始信号
    IIC_Send_Byte(BMP180_SlaveAddress);   //发送设备地址+写信号
    while(IIC_Wait_Ack());
    IIC_Send_Byte(0xF4);	          // write register address
    while(IIC_Wait_Ack());
    IIC_Send_Byte(0x34);       	  // write register data for pressure
    while(IIC_Wait_Ack());
    IIC_Stop();                    //发送停止信号
    delay_ms(20);    	                  // max time is 4.5ms

    //pressure = Multiple_read(0xF6);
    //pressure &= 0x0FFFF;

    return ReadBmp1820Data(BMP180_REG_OUT_MSB);    //pressure;	
}
//读取温度数据
uint16_t bmp180ReadTemp(void)
{

    IIC_Start_bit();                  //起始信号
    IIC_Send_Byte(BMP180_SlaveAddress);   //发送设备地址+写信号   BMP180_SlaveAddress  0xee	气压传感器器件地址
    while(IIC_Wait_Ack());
    IIC_Send_Byte(BMP180_REG_CTRL_MEAS);	          // write register address
    while(IIC_Wait_Ack());
    IIC_Send_Byte(0x2E);       	// write register data for temp
    while(IIC_Wait_Ack());
    IIC_Stop();                   //发送停止信号
    delay_ms(10);	// max time is 4.5ms
    return ReadBmp1820Data(BMP180_REG_OUT_MSB);
}
//气压数据转换
//最终的气压值
void bmp180Convert(uint32_t *BMP_UP)
{
    //u32 *BMP_UT;
    unsigned int ut;
    unsigned long up;
    long x1, x2, b5, b6, x3, b3, p;
    unsigned long b4, b7;

    ut = bmp180ReadTemp();	   // 读取温度
    up = ReadBmp1820Pressure();  // 读取压强    return pressure;	
    //*************
    x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
    x2 = ((long) mc << 11) / (x1 + md);
    //b5 = x1 + x2;
    //*BMP_UT = ((b5 + 8) >> 4);
    //*************
    b6 = b5 - 4000;
                             // Calculate B3
    x1 = (b2 * (b6 * b6)>>12)>>11;
    x2 = (ac2 * b6)>>11;
    x3 = x1 + x2;
    b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;	
                             // Calculate B4
    x1 = (ac3 * b6)>>13;
    x2 = (b1 * ((b6 * b6)>>12))>>16;
    x3 = ((x1 + x2) + 2)>>2;
    b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

    b7 = ((unsigned long)(up - b3) * (50000>>OSS));
    if (b7 < 0x80000000)
    p = (b7<<1)/b4;
    else
    p = (b7/b4)<<1;

    x1 = (p>>8) * (p>>8);
    x1 = (x1 * 3038)>>16;
    x2 = (-7357 * p)>>16;
    *BMP_UP = (p+((x1 + x2 + 3791)>>4));
}




































#endif



