#ifndef _UARTSCREEN_C
#define _UARTSCREEN_C	
//串口屏驱动部分源文件
#include "include.h"
//////////////////串口屏驱动部分////////////////////////////////////////////////////////////////////////
//显示传感器数据帧部分初始化
DataDeal TrnasfData;    //传感器数据转换
SendPkg SendSensorData;   //发送传感器数据
ScreStat SetOraWake;      //设置串口屏是否为显示模式
//代码语法段测试
/*************************************
struct Test
{
	uint8_t i;
	uint8_t j;
	uint16_t k;
};
void TestCode(struct Test *test)
{
	test->i = 1;
	test->j = 2;
	test->k = 3;
}************************************/
//串口屏相关初始化
void UartScreenInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 	
       
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_6); 
}
//发送数据到串口屏
//画面Id ,控件Id
//要发送的数据
void SendDataToScreen(uint16_t FrameId,uint16_t WidgetId,uint8_t *DataBuf)
{
  uint8_t i;
  uint8_t len;
  uint8_t *buf;
//  Data&=0XFFFF;
  SendSensorData.head = 0XEE;   //帧头
  SendSensorData.FrameId[0] = 0XB1;
  SendSensorData.FrameId[1] = 0X10; 
  SendSensorData.FrameId[2] = (FrameId>>8)&0XFF;   //得到高八位
  SendSensorData.FrameId[3] = FrameId&0XFF;        //得到低八位 
  SendSensorData.WidgetId[0] = (WidgetId>>8)&0XFF;  //得到控件的Id号
  SendSensorData.WidgetId[1] = WidgetId&0XFF;
  ///////////////////////////////////////////
  for(i=0;i<4;i++)
    SendSensorData.Data[i] = *(DataBuf+i);
  /////////////// ////////////////////////////
  SendSensorData.Tail[0] = 0XFF;
  SendSensorData.Tail[1] = 0XFC;
  SendSensorData.Tail[2] = 0XFF;
  SendSensorData.Tail[3] = 0XFF;
  len = sizeof(SendSensorData);
  buf = (uint8_t*)(&SendSensorData.head);
  for(i=0;i<len;i++)
     UsartSendDex(3,*(buf+i));
}
//数据转换
void DataTransf(void)
{
    uint8_t i;
    uint8_t ThousBit,HundreBit,TenBit,IndiBit;  //千 百 十 个 位
    //传感器数据转换
    SendJQCmd(JQCmd);  //发送甲醛读取命令数据
    delay_ms(50);
    PM2_5Ctrl = 1;
    SesorData[0].AnoSenor.FireGas = Get_Adc_Average(ChalTab[0],5);  	//模拟量传感器数据计算(已经滤波后)
	SesorData[0].AnoSenor.LightStren = Get_Adc_Average(ChalTab[1],5);
	SesorData[0].AnoSenor.PM2_5 = Get_Adc_Average(ChalTab[2],5);
    PM2_5Ctrl = 0;
    DHT11_Read_Data(&SesorData[0].Temp,&SesorData[0].Damp); //温度传感器读取(滤波后的数据)	
//    bmp180Convert(&SesorData[0].Result_UPres);        //气压传感器读取(无需要滤波)

    SesorData[0].Temp = SesorData[0].Temp;//温度
    SesorData[0].Damp = SesorData[0].Damp;//湿度
    SesorData[0].Result_UPres = SesorData[0].Result_UPres;     //气压
    SesorData[0].Jiaquanmg = SesorData[0].Jiaquanmg;           //甲醛mg
    SesorData[0].Jiaquanppm = SesorData[0].Jiaquanppm;         //甲醛ppm
    SesorData[0].AnoSenor.FireGas = SesorData[0].AnoSenor.FireGas/20;     //通道0数据解算（可燃性气体）0-4095，除以41表示转换为0-100
    SesorData[0].AnoSenor.LightStren = SesorData[0].AnoSenor.LightStren;     //通道1数据解算（光照传感器）0-4095
    SesorData[0].AnoSenor.PM2_5= SesorData[0].AnoSenor.PM2_5/60;     //通道2数据解算（PM2.5）     0-4095
    SesorData[0].AnoSenor.BatVol = SesorData[0].AnoSenor.BatVol;    //通道3数据解算（备用-PH水质监测）0-4095，转换为0-15范围之内
//	  printf("温度为 :%d\r\n",SensorData.Temp);
//		printf("湿度为 :%d\r\n",SensorData.Damp);
//		printf("气压为 :%d\r\n",SensorData.Result_UPres);
//		printf("甲醛m为 :%d\r\n",SensorData.Jiaquanmg);
//		printf("甲醛ppm为 :%d\r\n",SensorData.Jiaquanppm);
//		printf("模拟量1为 :%d\r\n",SensorData.ADC_ChangeValue[0]);
//		printf("模拟量2为 :%d\r\n",SensorData.ADC_ChangeValue[1]);	
//		printf("模拟量3为 :%d\r\n",SensorData.ADC_ChangeValue[2]);
//		printf("模拟量4为 :%d\r\n",SensorData.ADC_ChangeValue[3]);
		//    //温度
    TenBit = SesorData[0].Temp/10%10%16;
    IndiBit = SesorData[0].Temp%10%16;
    TrnasfData.Temp[0] = 0X30|TenBit;
    TrnasfData.Temp[1] = 0X30|IndiBit; 
    //湿度
    TenBit = SesorData[0].Damp/10%10%16;
    IndiBit = SesorData[0].Damp%10%16;
    TrnasfData.Damp[0] = 0X30|TenBit;
    TrnasfData.Damp[1] = 0X30|IndiBit;
    //甲醛mg   
    TenBit = SesorData[0].Jiaquanmg/10%10%16;
    IndiBit = SesorData[0].Jiaquanmg%10%16;
    TrnasfData.JiaQuan_mg[0] = 0X30|TenBit;
    TrnasfData.JiaQuan_mg[1] = 0X30|IndiBit;
    //甲醛PPM   
    TenBit = SesorData[0].Jiaquanppm/10%10%16;
    IndiBit = SesorData[0].Jiaquanppm%10%16;
    TrnasfData.JiaQuan_ppm[0] = 0X30|TenBit;
    TrnasfData.JiaQuan_ppm[1] = 0X30|IndiBit;
    //气压
    SesorData[0].Result_UPres = SesorData[0].Result_UPres/1000;  //转换为KPa
    HundreBit = SesorData[0].Result_UPres/100%10%16;
    TenBit = SesorData[0].Result_UPres/10%10%16;
    IndiBit = SesorData[0].Result_UPres%10%16;
    TrnasfData.Presure[0] = 0X30|HundreBit;
    TrnasfData.Presure[1] = 0X30|TenBit;
    TrnasfData.Presure[2] = 0X30|IndiBit;
   
    for(i=0;i<4;i++)  //循环转存传感器读到的数据
    {
        SesorData[0].AnoSenor.FireGas = SesorData[0].AnoSenor.FireGas%10000;      //分割出有用的数据
        ThousBit = SesorData[0].AnoSenor.FireGas/1000%10%16;   //数据位分离转化为十六进制的值
        HundreBit = SesorData[0].AnoSenor.LightStren/100%10%16;
        TenBit = SesorData[0].AnoSenor.LightStren/10%10%16;
        IndiBit = SesorData[0].AnoSenor.PM2_5%10%16;
        TrnasfData.AnoSensor[i][0] = 0x30|ThousBit;        //数据融合起来存入数据结构
        TrnasfData.AnoSensor[i][1] = 0x30|HundreBit;
        TrnasfData.AnoSensor[i][2] = 0x30|TenBit;
        TrnasfData.AnoSensor[i][3] = 0x30|IndiBit;
   }
}
//往串口屏幕上显示读取后的数据
void ScreenDispData(void)
{
   DataTransf();   //传感器数据转换
   SendDataToScreen(0,37,TrnasfData.Temp);  //发送窗口ID，控件ID数据 温度
   SendDataToScreen(0,38,TrnasfData.Damp);  //湿度
   SendDataToScreen(0,40,TrnasfData.JiaQuan_mg);//甲醛mg
   SendDataToScreen(0,41,TrnasfData.JiaQuan_ppm);//甲醛ppm
   SendDataToScreen(0,43,TrnasfData.Presure);      //气压
   SendDataToScreen(0,44,TrnasfData.AnoSensor[0]); //可燃气体
   SendDataToScreen(0,39,TrnasfData.AnoSensor[1]); //光照                       
   SendDataToScreen(0,42,TrnasfData.AnoSensor[2]); //PM2.5
   SendDataToScreen(0,45,TrnasfData.AnoSensor[3]); //备用.水质
}
//自动屏保模式
//输入参数：切换画面的Id号  唤醒串口屏
//{0XEE, 0XB1, 0X00, 0X00, 0X00, 0XFF, 0XFC, 0XFF, 0XFF}  画面切换
//EE [77 Enable BL_ON BL_OFF BL_ON_Time] FF FC FF FF  自动屏保模式
void aWake(uint16_t FrameId,uint8_t Enable,uint8_t BL_ON,uint8_t BL_OFF,uint8_t BL_ON_Time)
{
   uint8_t i;
   SendSensorData.head = 0XEE;  //数据帧头
   UsartSendDex(3,SendSensorData.head);
   SetOraWake.aWake[0] = 0X77;
   SetOraWake.aWake[1] = Enable;
   SetOraWake.aWake[2] = BL_ON;
   SetOraWake.aWake[3] = BL_OFF;
   SetOraWake.aWake[4] = BL_ON_Time;
   for(i=0;i<4;i++)
      UsartSendDex(3,SetOraWake.aWake[i]);     //发送控制数据内容
   SendSensorData.Tail[0] = 0XFF;
   SendSensorData.Tail[1] = 0XFC;
   SendSensorData.Tail[2] = 0XFF;
   SendSensorData.Tail[3] = 0XFF;
   for(i=0;i<4;i++)
      UsartSendDex(3,SendSensorData.Tail[i]);  //发送帧尾
   UsartSendDex(3,SendSensorData.head);        
   SetOraWake.Frame[0] =  0XB1;                //控制内容
   SetOraWake.Frame[1] =  0X00;
   SetOraWake.Frame[2] =  (FrameId>>8)&0XFF;   //取得高八位
   SetOraWake.Frame[3] =  FrameId&0XFF;        //取得低八位
   for(i=0;i<4;i++)     //通过串口发送数据
      UsartSendDex(3,SetOraWake.Frame[i]);  //发送切换画面指令 
   for(i=0;i<4;i++)
      UsartSendDex(3,SendSensorData.Tail[i]);  //发送帧尾  
}
//时间换算
/*void TimeCalcu(void)
{
	 uint8_t i;
	 uint8_t shiwei,gewei;
	 RTC_Get();  //得到当前的时间
   //if(Rem1Time)  //如果使能了时间计数标志
	 {
			 if(MainFacePar.Pre1Sec != calendar.sec)
			 {
				 MainFacePar.Pre1Sec = calendar.sec;
				 MainFacePar.Rem1Sec++;
				 if(MainFacePar.Rem1Sec==60)
				 {
					 MainFacePar.Rem1Sec=0;
					 MainFacePar.Rem1Min++;
					 if(MainFacePar.Rem1Min==60)
					 {
							MainFacePar.Rem1Min = 0;
							MainFacePar.Rem1Hour++;
							if(MainFacePar.Rem1Hour==60)
								MainFacePar.Rem1Hour=0;
					 }
				 }
			 }
   }
	 shiwei = MainFacePar.Rem1Min/10%16;
	 gewei = MainFacePar.Rem1Min%10%16;
	 Fen[7] = 0x30|shiwei;
	 Fen[8] = 0x30|gewei;
	 //秒换算
	 shiwei = MainFacePar.Rem1Sec/10%16;
	 gewei = MainFacePar.Rem1Sec%10%16;
	 Miao[7] = 0x30|shiwei;
	 Miao[8] = 0x30|gewei;
	 for(i=0;i<13;i++)
	 {
			UsartSendDex(3,Fen[i]);  //发送分钟数据
		  //delay_ms(2);
	 }
	 for(i=0;i<13;i++)
	 {
			UsartSendDex(3,Miao[i]); //发送秒数据
		  //delay_ms(2);
	 }
}  */





#endif




