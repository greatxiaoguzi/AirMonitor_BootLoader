#ifndef __ADC_C
#define __ADC_C	
#include "include.h"	
uint8_t const ThreVal[7] = {0,0,0,0,0,0,0};//传感器数据改变判断阈值
uint8_t const ChalTab[4] = {ADC_Channel_0,ADC_Channel_1,ADC_Channel_2,ADC_Channel_3};  //要转换的ADC的通道
uint8_t const JQCmd[5] = {0XA5,0X5A,0X02,0X80,0XAA};  //读取数据帧
uint8_t const JQOFFSET[5] = {0XA5,0X5A,0X02,0X85,0XAA};  //传感器校零
Sensor SesorData[2];   		//所有传感器数据
void  Adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

    //PA0---PA3作为模拟通道输入引脚
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);				
	GPIO_ResetBits(GPIOC,GPIO_Pin_3);						 
    
    ADC_DeInit(ADC1);       //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在多通道扫描模式,有许多通道
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

    ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

    ADC_ResetCalibration(ADC1);	//使能复位校准  

    while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

    ADC_StartCalibration(ADC1);	 //开启AD校准

    while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

//    ADC_Cmd(ADC1, ENABLE);		 
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
}				
 	    
//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//外设模式->从存储器/16位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//PeripAdd:外设地址
//MemAdd:存储器地址
//BufLen:数据传输量 
void ADC_DMAInit(DMA_Channel_TypeDef* DMA_CHx,u32 PeripAdd,u32 MemAdd,uint16_t BufLen)
{
	DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输

    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = PeripAdd;  //DMA外设ADC基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = MemAdd;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC ;  //数据传输方向，从外设读取发送到内存
    DMA_InitStructure.DMA_BufferSize = BufLen;  //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular ;  //工作在正常缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输	
    DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	
    DMA_Cmd(DMA_CHx, ENABLE);
} 

///获得ADC值
//ch:通道值3 0--3
uint16_t Get_Adc(uint8_t ch)
{
//    uint16_t ADCVal;
    //ADC_Cmd(ADC1, ENABLE);
    ADC1->CR2 |= 0x00000001;//打开ADC1
    //设置指定ADC的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	

    //while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
    while(!(ADC1->SR & 0x02));                       //等待转换结束

    return  (uint16_t)ADC1->DR;                //ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
//得到ADC转换后的数值12位数据0--4095
//转换次数为5次
uint16_t Get_Adc_Average(uint8_t ch,uint8_t times)
{
    uint8_t i;
//    uint16_t temp_val[5] = {0,0,0,0,0};
//    uint16_t Temp = 0;
    uint32_t Sum = 0;
    for(i=0;i<times;i++)
    {
        Sum += Get_Adc(ch);
//        temp_val[i] = Get_Adc(ch);
        delay_ms(5);
    }
//    for(i=0;i<times-1; i++)//排序
//    {
//        for(j=i+1;j<times;j++)
//        {
//            if(temp_val[i]>temp_val[j])//升序排列
//            {
//                 Temp = temp_val[i];
//                 temp_val[i] = temp_val[j];
//                 temp_val[j] =  Temp;
//            }      
//        }
//    }
//    for(i=1;i<times-1;i++)  //舍去两端的极值
//    {
//        Sum += temp_val[i];
//    }
    //ADC_Cmd(ADC1, DISABLE);
    ADC1->CR2 &= 0xFFFFFFFE;     //关闭ADC1
//    return Sum/(times-2); //返回平均值,转换为16位的数据
    return Sum/times;
} 	 
uint16_t Pm2_5Read(uint8_t ch,uint8_t times)
{
	uint8_t i;
	uint32_t Sum = 0;
	for(i=0;i<times;i++)
    {
        Sum += Get_Adc(ch);
        delay_ms(1);
    }
	ADC1->CR2 &= 0xFFFFFFFE;     //关闭ADC1
    return Sum/times;
}

/*
功能：传感器数据变化检测，若有变动在一定范围内则不发送数据，否则发送传感器数据
输入参数;传感器的数据
返回值：真值状态发送或不发送;
*/
uint8_t SensorDtaChageCheck(Sensor *Sensordata)
{
	if(abs(Sensordata->AnoSenor.FireGas - SesorData[1].AnoSenor.FireGas>ThreVal[0]))
        return 1;
    if(abs(Sensordata->AnoSenor.LightStren - SesorData[1].AnoSenor.LightStren)>ThreVal[1])
        return 1;
	if(abs(Sensordata->AnoSenor.PM2_5 - SesorData[1].AnoSenor.PM2_5)>ThreVal[2])
        return 1;
	if(abs(Sensordata->Temp - SesorData[1].Temp)>ThreVal[3])
        return 1;
	if(abs(Sensordata->Damp - SesorData[1].Damp)>ThreVal[4])
        return 1;
	if(abs(Sensordata->Result_UPres - SesorData[1].Result_UPres)>ThreVal[5])
        return 1;
	if(abs(Sensordata->Jiaquanmg - SesorData[1].Jiaquanmg)>ThreVal[6])
        return 1;
	SesorData[1].AnoSenor.FireGas = Sensordata->AnoSenor.FireGas;
	SesorData[1].AnoSenor.LightStren = Sensordata->AnoSenor.LightStren;
	SesorData[1].AnoSenor.PM2_5 = Sensordata->AnoSenor.PM2_5;
	SesorData[1].Temp = Sensordata->Temp;
	SesorData[1].Damp = Sensordata->Damp;
	SesorData[1].Result_UPres = Sensordata->Result_UPres;
	SesorData[1].Jiaquanmg = Sensordata->Jiaquanmg;
	SesorData[1].Jiaquanppm = Sensordata->Jiaquanppm;
   return 0;
}





#endif 

























