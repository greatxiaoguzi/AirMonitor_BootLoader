#ifndef BMP1820_H
#define BMP1820_H
#include "sys.h"
#define	BMP180_SlaveAddress     0xee	//气压传感器器件地址
//器件寄存器地址
#define BMP180_REG_AC1 0XAA  //以下11个寄存器的地址,是代表着11个字的地址。
#define BMP180_REG_AC2 0XAC  //这几个寄存器是只读的寄存器,在数据读取之前要先读取这11个字的数据                      
#define BMP180_REG_AC3 0XAE
#define BMP180_REG_AC4 0XB0
#define BMP180_REG_AC5 0XB2
#define BMP180_REG_AC6 0XB4
#define BMP180_REG_B1  0XB6
#define BMP180_REG_B2  0XB8
#define BMP180_REG_MB  0XBA
#define BMP180_REG_MC  0XBC
#define BMP180_REG_MD  0XBE
#define BMP180_REG_OUT_XLSB   0XF8
#define BMP180_REG_OUT_LSB    0XF7
#define BMP180_REG_OUT_MSB    0XF6
#define BMP180_REG_CTRL_MEAS  0XF4
#define BMP180_REG_SOFT_RESET 0XE0
#define BMP180_REG_ID         0XD0

#define Standard_Pressure 101325   //单位pa
#define   OSS 0	 //	BMP085使用
#define   REG_Pressure 0x34 // oss=0 4.5
extern uint32_t  result_UT;
extern uint32_t  result_UP;
uint8_t BMP1820Init(void);
uint16_t ReadBmp1820Data(uint8_t ST_Add);
uint16_t ReadBmp1820Pressure(void);
uint16_t bmp180ReadTemp(void);
void bmp180Convert(uint32_t *BMP_UP);
































#endif



