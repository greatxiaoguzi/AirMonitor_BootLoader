//DHT11温湿度传感器驱动程序
#include "include.h"
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在    	 
uint8_t DHT11_Init(void)
{	 
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //PC11端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);				 //初始化IO口
 	GPIO_SetBits(GPIOC,GPIO_Pin_5);						 //PC11 输出高		    
	DHT11_Rst();  //复位DHT11
	return DHT11_Check();//等待DHT11的回应
} 

//复位DHT11
void DHT11_Rst(void)	   
{                 
    DHT11_IO_OUT(); 	//SET OUTPUT
    DHT11_DQ_OUT=0; 	//拉低DQ
    delay_ms(30);    	//拉低至少18ms
    DHT11_DQ_OUT=1; 	//DQ=1 
    delay_us(40);     	//主机拉高20~40us
}
//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void) 	   
{   
	uint8_t retry=0;
	DHT11_IO_IN();       //SET INPUT
    while(DHT11_DQ_IN&&retry<100)//DHT11会拉低40~80us
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=100)
        return 1;
	else 
        retry=0;
    while(!DHT11_DQ_IN&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)
        return 1;	    
	return 0;
}
//从DHT11读取一个位
//返回值：1/0
uint8_t DHT11_Read_Bit(void)
{
 	uint8_t retry=0;
	while(DHT11_DQ_IN&&retry<100)//等待变为低电平
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!DHT11_DQ_IN&&retry<100)//等待变高电平
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//等待40us
	if(DHT11_DQ_IN)return 1;
	else return 0;		   
}
//从DHT11读取一个字节
//返回值：读到的数据
uint8_t DHT11_Read_Byte(void)    
{
    uint8_t i,dat;
    dat=0;
    for (i=0;i<8;i++) 
    {
        dat<<=1; 
        dat|=DHT11_Read_Bit();
    }
    return dat;
}
#define ErrorAllow  10   //误差范围判5°
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)if((((TempData[1]<=TempData[0])&&(TempData[0]<TempData[1]+ErrorAllow))||((TempData[0]<=TempData[1])&&(TempData[1]<TempData[0]+ErrorAllow)))
	//		&&(((HumiData[1]<=HumiData[0])&&(HumiData[0]<HumiData[1]+ErrorAllow))||((HumiData[0]<=HumiData[1])&&(HumiData[1]<HumiData[0]+ErrorAllow))))
//返回值：0,正常;1,读取失败
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)    
{        
 	uint8_t buf[5];
	uint8_t j;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{		
		for(j=0;j<5;j++)//读取4位数据
		{
			buf[j]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3]) == buf[4])
		{											
			*temp = buf[2];
			*humi = buf[0];	
		}
	}
	else 
		return 1;
	return 0;	    
}
//传感器数据滤波处理舍去两端的数据，中间的取平均
//转换5次，升序排列，
void DHT11_Filted(uint8_t *temp,uint8_t *humi)
{
  uint8_t i,j;
  uint8_t Link;
  uint8_t Val[2][5];  //数据暂存
  uint16_t Res[2]={0,0};  
  for(i=0;i<5;i++)
  {
      DHT11_Read_Data(&Val[0][i],&Val[1][i]);    
  }	 		    
	for(i=0;i<4; i++)//排序
	{
		for(j=i+1;j<5;j++)
		{
			if(Val[0][i]>Val[0][j])//升序排列
			{
				Link = Val[0][i];
				Val[0][i] = Val[0][j];
				Val[0][j] = Link;
			}
            if(Val[1][i]>Val[1][j])//升序排列
			{
				Link = Val[1][i];
				Val[1][i] = Val[1][j];
				Val[1][j] = Link;
			}
		}
	}
	for(i=1;i<4;i++)
    {
        Res[0] += Val[0][i];
        Res[1] += Val[1][i];
    }
    *temp = (uint8_t)(Res[0]/3)&0XFF;
    *humi = (uint8_t)(Res[1]/3)&0XFF;
}






