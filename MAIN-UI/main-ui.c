#include "include.h"
#define FillColor  GRAY  //右边区域的底色
//#include <locale.h>
//AQI计算表
const float AQITab[7][4]={

	{0,15.4,0,50},
	{15.5,40.4,51,100},
	{40.5,65.4,101,150},
	{60.5,150.4,151,200},
	{150.5,250.4,201,300},
	{250.5,350.4,301,400},
	{350.5,500.4,401,500}	
};

extern const unsigned char WifiLogo[32];
extern const unsigned char NoWifiLogo[32];
uint8_t orLoadUI[2] = {TRUE,FALSE};
uint8_t const Item_Id[7] = {HCHO_ID,PM2_5_ID,DangerGas_ID,Temp_ID,Dump_ID,UPres_ID,Illumn_ID};
unsigned char const *ItemName[] = {"温度","湿度","挥发","气压","有害","光强","PM2.5"};
uint8_t const *QualityGrade[]={"优","良","轻","中","重","爆","AQI"};
uint16_t const ColorTab[] = {GREEN,LIGHTGREEN,RED,BLUE,BROWN,LIGHTBLUE,YELLOW};//颜色值
uint16_t const ValueTab[] = {0,35,75,115,150,250,500};//色条上的数值
volatile uint8_t CurrentuiID =1;	//当前UI的ID标号
uint8_t ToDataSDCard_WriteFlag = 0;  //向SD卡写入数据标志
uint8_t Wave_Id = 0;
uint8_t PreWave_Id = 0XFF;

uint16_t R_init,G_init,B_init;   //数据显示的时候的颜色状态

DispCirDef const AllDispCirInfo[7]=
{      //这里长和宽用外圆和内圆代替
   //x0  y0 r外 r内 XoffYoff C1clr外 C2clr内 Valuecolor Textcolor TextBackcolor ValueBackcolor Text_Name Text_UnitMeas
	{0,18,88,110,   22,  85,  {RED,DARKBLUE,   RED,  WHITE,   DARKBLUE,FillColor},	 "甲醛","mg/m3",36},	
	{92,18,92,110,  27,  85,  {RED,GRAYBLUE,   RED,  WHITE,   DARKBLUE,FillColor},     "PM2.5","ug/m3",36},
	{189,18, 90,110,20,  85,  {RED,GRAYBLUE,   RED,  WHITE,   DARKBLUE,FillColor},    "易燃","mg/m3",36},	
	
	{28,168, 25,22,22,  28,  {WHITE,GRAYBLUE, WHITE,WHITE, BLACK,GRAYBLUE},  "温度(℃)",    "deg",16},
	{95,168, 25,22,21,  28,  {WHITE,GRAYBLUE, WHITE,WHITE, BLACK,GRAYBLUE},  "湿度(%)",     "  %",16},	
	{160,168, 25,22,27, 28,  {WHITE,GRAYBLUE, WHITE,WHITE, BLACK,GRAYBLUE},  "气压(KPa)",   "KPa",16},	
	{232,168,25,22,25,  28,  {WHITE,GRAYBLUE, WHITE,WHITE, BLACK,GRAYBLUE},  "光照(Lux)",   "Lux",16},	
};
//返回两个数中的最大值
uint16_t MAX(uint16_t val1,uint16_t val2)
{
	return (val1>val2)?val1:val2;
}
//返回两个数中的最小值
uint16_t MIN(uint16_t val1,uint16_t val2)
{
	return (val1>val2)?val2:val1;	
}
//数值范围判定
uint16_t DeadScale(uint16_t val,uint16_t min,uint16_t max)
{
	if(val>min&&val<max)
		return val;
	else if(val<min)
		return min;
	else if(val>max)
		return max;
}
//DispCirDef *pCirInfo[7]={&AllDispCirInfo[Temp_ID],&AllDispCirInfo[Dump_ID],
//					  &AllDispCirInfo[HCHO_ID],&AllDispCirInfo[UPres_ID],
//					  &AllDispCirInfo[DangerGas_ID],&AllDispCirInfo[Illumn_ID],
//					  &AllDispCirInfo[PM2_5_ID]};
void TestPm2_5(void)
{
	PM2_5Ctrl = 1;	
	delay_ms(1);
	SesorData[0].AnoSenor.PM2_5 =  Pm2_5Read(ChalTab[2],2);
    PM2_5Ctrl = 0;
	delay_ms(7);
	LCD_ShowNum(0,100,SesorData[0].AnoSenor.PM2_5,5,24,BLACK,LIGHTBLUE); 
}
//加载传感器数据
void LoadSensorData(Sensor *sensordata)
{
	memset((void *)&SesorData[0],0,sizeof(SesorData[0]));
	//传感器数据转换
	bmp180Convert(&sensordata->Result_UPres);        //气压传感器读取(无需要滤波)
    SendJQCmd(JQCmd);  //发送甲醛读取命令数据
    sensordata->AnoSenor.FireGas = Get_Adc_Average(ChalTab[0],10);  	//模拟量传感器数据计算(已经滤波后)
	sensordata->AnoSenor.LightStren = Get_Adc_Average(ChalTab[1],10)/2;
	PM2_5Ctrl = 1;	
	//delay_ms(1);
	delay_us(280);
	SesorData[0].AnoSenor.PM2_5 = Get_Adc(ChalTab[2]);  // Pm2_5Read(ChalTab[2],2);
	delay_us(40);
    PM2_5Ctrl = 0;
	delay_ms(7);
	DHT11_Read_Data(&sensordata->Temp,&sensordata->Damp); //温度传感器读取(滤波后的数据)	  
	sensordata->AnoSenor.FireGas = sensordata->AnoSenor.FireGas/41-5;  //单位换算减30为误差值
	sensordata->AnoSenor.LightStren = sensordata->AnoSenor.LightStren/58;
	sensordata->AnoSenor.PM2_5 = sensordata->AnoSenor.PM2_5*50*10/(4095*3.3);      //0.5V/0.1mg/m3 乘10为扩大
}
//
//加载一个色谱
//
void LoadColorPat(void)
{ 
	uint16_t i=0;
	uint16_t color;
	uint16_t Ri=0,Gi=0,Bi=0;
	uint16_t ystep = (lcddev.height-13)/6;  	//改变色谱的长度改这里
	Show_Str(CURSOR-21,20,30,16,(uint8_t*)QualityGrade[6],16,0,BLACK,FillColor);
	Show_Str(CURSOR-21,225,18,16,(uint8_t*)QualityGrade[0],16,0,DARKBLUE,FillColor);
	Ri = 0;Gi = 0;Bi = 0;
	for(i=240;i>240-ystep;i--)  //黒到蓝
	{
		color  = (unsigned long) Bi*1000/4047<<5;
		color |= (unsigned long) Gi*1000/8225;
		color |= (unsigned long) Ri*1000/8225<<11;
		LCD_DrawLine(310,i,320,i,color);
		Gi = Gi+6;
	}
	Show_Str(CURSOR-21,225-ystep,18,16,(uint8_t*)QualityGrade[1],16,0,color,FillColor);
	Ri = 0;Gi = 0;Bi = 240;
	for(i=240-ystep;i>240-ystep*2;i--)	//蓝到浅蓝
	{
		color = (unsigned long) Gi*1000/8225<<11;
		color |= (unsigned long) Ri*1000/4047<<5;	  
		color |= (unsigned long) Bi*1000/8225;	
		LCD_DrawLine(310,i,320,i,color);
		Ri = Ri+6;
	}
	Show_Str(CURSOR-21,225-ystep*2,18,16,(uint8_t*)QualityGrade[2],16,0,color,FillColor);
	Ri = 0;Gi = 240;Bi = 240; 
	for(i=240-ystep*2;i>240-ystep*3;i--)   //浅蓝到绿
	{
		color = (unsigned long) Ri*1000/8225<<11;			   	
		color |= (unsigned long) Bi*1000/4047<<5;	  
		color |= (unsigned long) Gi*1000/8225;
		LCD_DrawLine(310,i,320,i,color);
		Gi = Gi-6;
	}
	Show_Str(CURSOR-21,225-ystep*3,18,16,(uint8_t*)QualityGrade[3],16,0,color,FillColor);
	Ri = 0;Gi = 240;Bi = 0;
	for(i=240-ystep*3;i>240-ystep*4;i--)//绿到黄
	{
		color  = (unsigned long) Gi*1000/4047<<5;
		color |= (unsigned long) Ri*1000/8225;		
		color |= (unsigned long) Bi*1000/8225<<11;
		LCD_DrawLine(310,i,320,i,color);		
		Bi = Bi+6;
	}
	Show_Str(CURSOR-21,225-ystep*4,18,16,(uint8_t*)QualityGrade[4],16,0,color,FillColor);
	Ri = 240;Gi = 0;Bi = 240;
	for(i=240-ystep*4;i>240-ystep*5;i--)//黄到红
	{
		color = (unsigned long) Ri*1000/8225<<11;
		color |= (unsigned long) Bi*1000/4047<<5;	  
		color |= (unsigned long) Gi*1000/8225;
		LCD_DrawLine(310,i,320,i,color);
		Bi = Bi-6;
	}
	Show_Str(CURSOR-21,225-ystep*5,18,16,(uint8_t*)QualityGrade[5],16,0,color,FillColor);
	Ri = 240;Gi = 0;Bi = 0;
	for(i=240-ystep*5;i>240-ystep*6;i--)//红到紫
	{
		color  = (unsigned long) Bi*1000/8225;
		color |= (unsigned long) Ri*1000/8225<<11;
		color |= (unsigned long) Gi*1000/4047<<5;
		LCD_DrawLine(310,i,320,i,color);
		Bi = Bi+6;
	}
	POINT_COLOR = RED;
}
//绘制动态的红色指示点
void DrawDynamicCir(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{
	uint16_t static Prex0,Prey0;
	LCDDrawCir(Prex0,Prey0,r,FillColor,0,1);//清掉上一个圆
	LCDDrawCir(x0,y0,r,color,0,1);//重绘新圆
	Prex0 = x0;
	Prey0 = y0;
}
//计算AQI的值
uint16_t Cal_AQI_Value(uint16_t Cvalue)
{
	float AQIValue;
	float Ihigh,Ilow,Chigh,Clow;
	AQIValue = Cvalue/1000;
	if(Cvalue>AQITab[0][0]&&Cvalue<AQITab[0][1]){
		Clow = AQITab[0][0];Chigh = AQITab[0][1];Ilow =  AQITab[0][2];Ihigh = AQITab[0][3];}
	
	else if(Cvalue>AQITab[1][0]&&Cvalue<AQITab[1][1]){
		Clow = AQITab[1][0];Chigh = AQITab[1][1];Ilow =  AQITab[1][2];Ihigh = AQITab[1][3];}

	else if(Cvalue>AQITab[2][0]&&Cvalue<AQITab[2][1]){
		Clow = AQITab[2][0];Chigh = AQITab[2][1];Ilow =  AQITab[2][2];Ihigh = AQITab[2][3];}
		
	else if(Cvalue>AQITab[3][0]&&Cvalue<AQITab[3][1]){
		Clow = AQITab[3][0];Chigh = AQITab[3][1];Ilow =  AQITab[3][2];Ihigh = AQITab[3][3];}
		
	else if(Cvalue>AQITab[4][0]&&Cvalue<AQITab[4][1]){
		Clow = AQITab[4][0];Chigh = AQITab[4][1];Ilow =  AQITab[4][2];Ihigh = AQITab[4][3];}	
		
	else if(Cvalue>AQITab[5][0]&&Cvalue<AQITab[5][1]){
		Clow = AQITab[5][0];Chigh = AQITab[5][1];Ilow =  AQITab[5][2];Ihigh = AQITab[5][3];}
		
	else if(Cvalue>AQITab[6][0]&&Cvalue<AQITab[6][1]){
		Clow = AQITab[6][0];Chigh = AQITab[6][1];Ilow =  AQITab[6][2];Ihigh = AQITab[6][3];}	
		
		
	AQIValue = (uint16_t)((Ihigh - Ilow)*(Cvalue - Clow)/(Chigh - Clow) + Ilow);
	return AQIValue;
}
//质量检测判断，在三个界面上都有显示
void EnvirQuaty(uint16_t x,uint16_t y)
{
	uint16_t y0;
	uint16_t AQIValue;
	AQIValue = (uint16_t)Cal_AQI_Value(SesorData[0].AnoSenor.PM2_5);
	Show_Str(0,215,300,24,"AQI:",24,0,BLUE,BLACK);
    LCD_ShowNum(45,y,AQIValue,3,24,RED,DARKBLUE);
	y0 = AQIValue/1.3;
	if(y0>219)
		y0 = 219;
	if(AQIValue<100)
	{	
		DrawDynamicCir(CURSOR,240-y0,5,RED);
		SetDriverPWM(0,0);
		if(SesorData[0].Jiaquanmg<5)
		{
			if(SesorData[0].AnoSenor.FireGas<50)
				Show_Str(x,y,180,24,"      空气良好",24,0,GREEN,DARKBLUE);
			else if(SesorData[0].AnoSenor.FireGas>70)
				Show_Str(x,y,180,24,"      燃气泄漏",24,0,GREEN,DARKBLUE);
		}
		else if(SesorData[0].Jiaquanmg>8)
			Show_Str(x,y,180,24,"      甲醛超标",24,0,GREEN,DARKBLUE);
	}
	else if(AQIValue>=100)
	{
		if(AQIValue>100&&AQIValue<200)
		{
			DrawDynamicCir(CURSOR,DeadScale(240-y0,17,120),5,RED);			
			SetDriverPWM(500,500);
			if(SesorData[0].Jiaquanmg<5)
			{
				if(SesorData[0].AnoSenor.FireGas<50)
					Show_Str(x,y,180,24,"      中度污染",24,0,BRRED,DARKBLUE);
				else if(SesorData[0].AnoSenor.FireGas>70)
					Show_Str(x,y,180,24,"      燃气泄漏",24,0,GREEN,DARKBLUE);
			}
			else if(SesorData[0].Jiaquanmg>8)
				Show_Str(x,y,180,24,"中霾,甲醛超标",24,0,GREEN,DARKBLUE);				
		}
		else 
		{
			DrawDynamicCir(CURSOR,DeadScale(240-y0,17,120),5,RED);
			SetDriverPWM(800,800);
			if(SesorData[0].Jiaquanmg<5)
			{
				if(SesorData[0].AnoSenor.FireGas<50)
					Show_Str(x,y,180,24,"      污染严重",24,0,RED,DARKBLUE);
				else if(SesorData[0].AnoSenor.FireGas>70)
					Show_Str(x,y,180,24,"      燃气泄漏",24,0,GREEN,DARKBLUE);
			}
			else if(SesorData[0].Jiaquanmg>8)
				Show_Str(x,y,180,24,"重霾,甲醛超标",24,0,GREEN,DARKBLUE);					
		}
	}
}
void LoadShowUI(DispCirDef const *DispCirInfo,uint8_t Item)
{	
	switch(Item)
	{
		case Temp_ID:
	    case Dump_ID:
		case UPres_ID:
		case Illumn_ID:
			LCDDrawCir(DispCirInfo->x0,DispCirInfo->y0,DispCirInfo->r1,DispCirInfo->ColorInfo.Cir1color,0,1);  	 //外圆
			LCDDrawCir(DispCirInfo->x0,DispCirInfo->y0,DispCirInfo->r2,DispCirInfo->ColorInfo.Cir2color,0,1);      //内圆
			Show_Str(DispCirInfo->x0-DispCirInfo->Xoff,DispCirInfo->y0+DispCirInfo->Yoff,100,12,DispCirInfo->Text_Name,12,0,DispCirInfo->ColorInfo.Textcolor,DispCirInfo->ColorInfo.TextBackcolor);
			break;
		case HCHO_ID:
		case PM2_5_ID:
		case DangerGas_ID:
			LCD_Fill(DispCirInfo->x0,DispCirInfo->y0,DispCirInfo->x0+DispCirInfo->r1,DispCirInfo->y0+DispCirInfo->r2,DispCirInfo->ColorInfo.ValueBackcolor);
			Show_Str(DispCirInfo->x0+DispCirInfo->Xoff,DispCirInfo->y0+8,100,16,DispCirInfo->Text_Name,16,0,DispCirInfo->ColorInfo.Textcolor,DispCirInfo->ColorInfo.ValueBackcolor);
			Show_Str(DispCirInfo->x0+DispCirInfo->Xoff,DispCirInfo->y0+DispCirInfo->Yoff,100,16,DispCirInfo->Text_UnitMeas,16,0,DispCirInfo->ColorInfo.Textcolor,DispCirInfo->ColorInfo.ValueBackcolor);
		break;
	}
}
//计算颜色
uint16_t CalColorValue(uint16_t Value,uint8_t IdItem)
{
	uint16_t R,G,B;
	uint16_t RGB = 0x0000;
	switch(IdItem)
	{
		case HCHO_ID:
		{
			if(Value<5){
				R_init = 0;G_init = 0;B_init = 13;}
			else if(Value>=5&&Value<10){
				R_init = 0;G_init = 13;B_init = 0;}
			else{
				R_init = 13;G_init = 0;B_init = 0;}
				
		}break;
		case PM2_5_ID:
		{
			if(Value<16){
				R_init = 0;G_init = 0;B_init = 50;}
			else if(Value>=16&&Value<32){
				R_init = 0;G_init = 50;B_init = 0;}
			else{
				R_init = 50;G_init = 0;B_init = 0;}
		}break;
		case DangerGas_ID:
		{
			if(Value<30){
				R_init = 0;G_init = 0;B_init = 100;}
			else if(Value>=30&&Value<60){
				R_init = 0;G_init = 100;B_init = 0;}
			else{
				R_init = 100;G_init = 0;B_init = 0;}
		}break;
	}
	R=(uint16_t) Value*240/R_init;if(R>240)R=240;
	G=(uint16_t) Value*240/G_init;if(G>240)G=240;
	B=(uint16_t) Value*240/B_init;if(B>240)B=240;		
	RGB |=(unsigned long) R*1000/8225<<11;			   
	RGB |=(unsigned long) G*1000/4047<<5;	  //转换成屏幕的4W色显示
	RGB |=(unsigned long) B*1000/8225;
	return RGB;
}
//绘制显示框的圆形
void DispCirInfo(DispCirDef const *DispCirInfo,uint8_t Item)
{
	char buf[20];			
	uint16_t RGB;
	switch(Item)
	{
		case Temp_ID:
		{
			sprintf(buf,"%d",SesorData[0].Temp);
			Show_Str(DispCirInfo->x0-5,DispCirInfo->y0-10,35,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,DispCirInfo->ColorInfo.Valuecolor,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case Dump_ID:
		{
			sprintf(buf,"%d%",SesorData[0].Damp);
			Show_Str(DispCirInfo->x0-5,DispCirInfo->y0-10,35,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,DispCirInfo->ColorInfo.Valuecolor,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case UPres_ID:
		{ 
			sprintf(buf,"%d.%d%d",SesorData[0].Result_UPres/1000,SesorData[0].Result_UPres%1000/100,SesorData[0].Result_UPres%100/10);
			Show_Str(DispCirInfo->x0-20,DispCirInfo->y0-10,100,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,DispCirInfo->ColorInfo.Valuecolor,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case HCHO_ID:
		{
			RGB = CalColorValue(SesorData[0].Jiaquanmg,HCHO_ID);
			sprintf(buf,"%d.%d%d",SesorData[0].Jiaquanmg/100,SesorData[0].Jiaquanmg%100/10,SesorData[0].Jiaquanmg%10);
			Show_Str(DispCirInfo->x0+5,DispCirInfo->y0+36,105,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,RGB,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case DangerGas_ID:
		{
			RGB = CalColorValue(SesorData[0].AnoSenor.FireGas,DangerGas_ID);
			sprintf(buf,"%.2d",SesorData[0].AnoSenor.FireGas);
			Show_Str(DispCirInfo->x0+25,DispCirInfo->y0+36,100,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,RGB,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case Illumn_ID:
		{
			sprintf(buf,"%d",SesorData[0].AnoSenor.LightStren);
			Show_Str(DispCirInfo->x0-5,DispCirInfo->y0-10,35,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,DispCirInfo->ColorInfo.Valuecolor,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		case PM2_5_ID:
		{			
			RGB = CalColorValue(SesorData[0].AnoSenor.PM2_5,PM2_5_ID);
			sprintf(buf,"%.3d",SesorData[0].AnoSenor.PM2_5*10);
			Show_Str(DispCirInfo->x0+21,DispCirInfo->y0+36,100,DispCirInfo->FontSize,buf,DispCirInfo->FontSize,0,RGB,DispCirInfo->ColorInfo.ValueBackcolor);
		}break;
		default:break;
	}
}
//主显示数据的界面1
void LoadUI_1(void)
{
	uint8_t i;
	if(orLoadUI[1] != TRUE) //上一次的
	{		
		//LCD_Clear(BLACK);//清屏
		LCD_Fill(0,0,CURSOR-12,240,BLACK);  //参数显示界面的底色
		LCD_Fill(CURSOR-21,19,320,240,FillColor);  //右边的长条底色
		LoadColorPat();   //加载色谱和标度
		for(i=0;i<7;i++)
		{
			LoadShowUI(&AllDispCirInfo[Item_Id[i]],Item_Id[i]);
		}
		LCD_DrawLine(0,210,CURSOR-22,210,BLUE);
		orLoadUI[1] = orLoadUI[0];
		orLoadUI[0] = FALSE;
	}
	else
	{
		for(i=0;i<7;i++)
		{
			DispCirInfo(&AllDispCirInfo[Item_Id[i]],Item_Id[i]);
		}
		EnvirQuaty(113,215);  //质量检测判断
	}
}
//主显示数据的界面2
//显示纯文本数据
void LoadUI_2(uint16_t x0,uint16_t y0,uint8_t ystep,uint8_t fontsize)
{
	uint8_t i = 0;
	char buf[20];
	if(orLoadUI[1] != TRUE) //上一次的
	{		
		//LCD_Clear(BLACK);//清屏	
		LCD_Fill(0,0,CURSOR-13,209,BLACK);
		LoadColorPat();  //加载色谱
		LCD_DrawLine(0,40,CURSOR-13,40,BLUE);
		LCD_DrawLine(0,145,CURSOR-13,145,BLUE);
		LCD_DrawLine(0,210,CURSOR-13,210,BLUE);
		orLoadUI[1] = orLoadUI[0];
		orLoadUI[0] = FALSE;
		//POINT_COLOR = RED;
		//BACK_COLOR = BLACK;
		//TestSDCard();
	}
	sprintf(buf,"温度: %d℃",SesorData[0].Temp);
	Show_Str(x0,y0+13,80,16,buf,16,0,AllDispCirInfo[Temp_ID].ColorInfo.Textcolor,AllDispCirInfo[Temp_ID].ColorInfo.TextBackcolor);
	i++;
	sprintf(buf,"湿度: %d%%",SesorData[0].Damp);
	Show_Str(x0+135,y0+13,80,16,buf,16,0,AllDispCirInfo[Dump_ID].ColorInfo.Textcolor,AllDispCirInfo[Dump_ID].ColorInfo.TextBackcolor);
	i++;
	
	sprintf(buf,"HCHO:%d.%d%d",SesorData[0].Jiaquanmg/100,SesorData[0].Jiaquanmg%100/10,SesorData[0].Jiaquanmg%10);
	Show_Str(x0,y0+(i++)*ystep,280,AllDispCirInfo[HCHO_ID].FontSize,buf,AllDispCirInfo[HCHO_ID].FontSize,0,RED,BLACK);
	Show_Str(x0+188,y0+(--i)*ystep+16,80,16,"mg/m3",16,0,WHITE,BLACK);
	i++;	
	sprintf(buf,"Gas:%.3d",SesorData[0].AnoSenor.FireGas);
	Show_Str(x0,y0+(i++)*ystep+17,280,36,buf,36,0,RED,BLACK);
	Show_Str(x0+170,y0+(--i)*ystep+33,280,16,"mg/m3",16,0,WHITE,BLACK);
	i++;
	sprintf(buf,"PM2.5:%.3d",SesorData[0].AnoSenor.PM2_5*10);
	Show_Str(x0,y0+(i++)*ystep+32,280,36,buf,36,0,RED,BLACK);
	Show_Str(x0+188,y0+(--i)*ystep+48,280,16,"ug/m3",16,0,WHITE,BLACK);
	i++;
	
	sprintf(buf,"气压:%d.%d%d KPa",SesorData[0].Result_UPres/1000,SesorData[0].Result_UPres%1000/100,SesorData[0].Result_UPres%100/10);
	Show_Str(x0,y0+(i++)*ystep+65,300,16,buf,16,0,AllDispCirInfo[UPres_ID].ColorInfo.Textcolor,AllDispCirInfo[UPres_ID].ColorInfo.TextBackcolor);	
	sprintf(buf,"光强:%3d Lux",SesorData[0].AnoSenor.LightStren);
	Show_Str(x0+135,y0+(--i)*ystep+65,250,16,buf,16,0,AllDispCirInfo[Illumn_ID].ColorInfo.Textcolor,AllDispCirInfo[Illumn_ID].ColorInfo.TextBackcolor);
	
	EnvirQuaty(113,215);
}
//加载UI3界面,波形显示
//水平分割，垂直分割，项目名称
//x0: 20  y0:190
void LoadUI_3(uint16_t x0,uint16_t y0,uint16_t HScalUnit,uint16_t VScalUnit,uint8_t Item_Id)
{
	uint16_t i;
	uint8_t HCell,VCell;
	uint8_t buf[5];
	HCell = H_LEN/HScalUnit;  //水平格子
	VCell = V_LEN/VScalUnit;  //垂直格子
	if(orLoadUI[1]!=TRUE)
	{
		Wave_Id = 1;
		LCD_Clear(BLACK);		//清屏
		BACK_COLOR = BLACK;
		Show_Str(86,0,130,24,"数据统计",24,0,GREEN,BLACK);
		Show_Str(285,y0+5,50,16,"时间",16,0,BLUE,BLACK);	
		LCD_DrawLine(x0,y0,320,y0,BLUE);		//画水平线坐标(50,180) (300,180)
		LCD_DrawLine(x0,y0,x0,y0-170,BLUE);		//画垂直线坐标(50,180) (50,20)
		//加载水平标度：时间格子
		for(i=0;i<HScalUnit;i++)
		{
			LCD_DrawLine(x0+HCell*(i+1),y0-1,x0+HCell*(i+1),y0-170,GRAY);
			if(i%2==0)
			{
				LCD_ShowNum(x0+HCell*i-6,y0+5,i,2,12,BLUE,BLACK);//显示一个数字
			}
		}
		//加载垂直标度：项目单位格子
		for(i=0;i<VScalUnit;i++)
			LCD_DrawLine(x0+1,y0-(i+1)*VCell,320,y0-(i+1)*VCell,GRAY);
		for(i=0;i<7;i++)
		{
			Show_Str(40*i,y0+20,40,20,(uint8_t *)ItemName[i],16,0,(uint16_t)ColorTab[i],BLACK);
		}
		orLoadUI[1] = orLoadUI[0];
		orLoadUI[0] = FALSE;
	}		
	switch(Item_Id)
	{
		case Temp_ID:
		{
			Show_Str(0,y0-188,100,20,"温度(℃)   ",16,0,(uint16_t)ColorTab[Temp_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{
				sprintf(buf,"  %2d",i*5);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[Temp_ID],BLACK);//显示一个字符串,12/16字体
			}
		}break;
		case Dump_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i*5,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"湿度(%)   ",16,0,(uint16_t)ColorTab[Dump_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{				
				sprintf(buf,"  %2d",i*5);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[Dump_ID],BLACK);
			}
		}break;
		case HCHO_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"甲醛(ug/m3)",16,0,(uint16_t)ColorTab[HCHO_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{
				sprintf(buf,"0.%d%d",i/10,i%10);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[HCHO_ID],BLACK);
			}
		}break;
		case UPres_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"气压(KPa)  ",16,0,(uint16_t)ColorTab[UPres_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{			
				sprintf(buf,"  %2d",i*10);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[UPres_ID],BLACK);
			}
		}break;
		case DangerGas_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"毒气(%)   ",16,0,(uint16_t)ColorTab[DangerGas_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{				
				sprintf(buf,"  %2d",i*10);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[DangerGas_ID],BLACK);
			}
		}break;
		case Illumn_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"光强(Lux)  ",16,0,(uint16_t)ColorTab[Illumn_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{
				sprintf(buf,"  %2d",i*100);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[Illumn_ID],BLACK);
			}
		}break;
		case PM2_5_ID:
		{
			//LCD_ShowNum(7,190-(i+1)*VCell+8,i,2,12);//显示一个数字
			Show_Str(0,y0-188,100,20,"PM2.5(mg/m3)",16,0,(uint16_t)ColorTab[PM2_5_ID],BLACK);
			for(i=0;i<VScalUnit;i++)
			{
				sprintf(buf,"  %2d",i*10);
				LCD_ShowString(0,y0-(i+1)*VCell+8,20,12,12,buf,(uint16_t)ColorTab[PM2_5_ID],BLACK);
			}
		}break;
		default :break;
	}
	LoadDataWaveFromSD("0:/Sensordata",Item_Id);  //读取SD卡的传感器数据，返回开始记录时刻的时刻值
}
//保存记录的数据到SD卡
//返回值：0正确 1
uint8_t SaveDataToSDCard(uint8_t *filename)
{
	UINT bw;
	FIL *f_Savesenordata;
	uint8_t res = 0;
	uint16_t *p=0;   //数据写入指针
	static uint32_t offset = 0;   //要偏移的结构数据长度
	DataRemTypedef HourData;
	uint16_t Writelen = 0;
	Writelen = sizeof(DataRemTypedef);
	f_Savesenordata  = (FIL *)mymalloc(sizeof(FIL));
	if(f_Savesenordata == NULL)
	{
		return 1;
	}
	//创建文件
	res = f_open(f_Savesenordata,(const TCHAR*)filename,FA_READ|FA_WRITE);  //创建新的文件在SD卡中
	if(res==0X04)
		res = f_open(f_Savesenordata,(const TCHAR*)filename,FA_WRITE|FA_CREATE_NEW);
	if(res==FR_OK)  //创建成功的话执行
	{
		HourData.Time = calendar.hour;
		HourData.Item_Id[Temp_ID] = SesorData[0].Temp;
		HourData.Item_Id[Dump_ID] = SesorData[0].Damp;
		HourData.Item_Id[HCHO_ID] = SesorData[0].Jiaquanmg;
		HourData.Item_Id[UPres_ID] = SesorData[0].Jiaquanppm;
		HourData.Item_Id[DangerGas_ID] = SesorData[0].AnoSenor.FireGas;
		HourData.Item_Id[Illumn_ID] = SesorData[0].AnoSenor.LightStren;
		HourData.Item_Id[PM2_5_ID] = SesorData[0].AnoSenor.PM2_5;
		p = (uint16_t*)&HourData.Time;   		//指针指向数据结构区
		res = f_lseek (f_Savesenordata,offset);
		res = f_write(f_Savesenordata,p,Writelen,&bw);  //写入新建的文件区
		offset += Writelen;
		f_close(f_Savesenordata); //关闭文件
	}
	myfree(f_Savesenordata);
	return FR_OK;
}
//显示时间到主界面同时保存传感器采集到的数据
void DispTimeToUI(void)
{
	uint16_t BatADCVal = 0;
	uint8_t BatVal = 0;
	char buf[10];
	sprintf(buf,"已运行%2d:%02d", calendar.hour, calendar.min);
	Show_Str(lcddev.width-128,0,100,16,buf,16,0,GREEN,BLACK);//显示已经运行的时间
	if(Esp8266Config.WifiLinkSuccFlag==1)
	{
		Show_DrawBMP(lcddev.width-235,0,16,16,(uint8_t*)WifiLogo,WHITE);
		Show_Str(lcddev.width-213,6,180,12,Esp8266Config.WireNet[0],12,0,GREEN,BLACK);		
	}
	else if(Esp8266Config.WifiLinkSuccFlag==0)
	{
		Show_DrawBMP(lcddev.width-235,0,16,16,(uint8_t*)NoWifiLogo,WHITE);
		Show_Str(lcddev.width-213,6,180,12,"当前WIFI未连接",12,0,RED,BLACK);
	}
	
	if(calendar.hour==23)  //每到一天清理SD卡的数据
	{
		f_unlink((const TCHAR*)"0:/Sensordata");  //删除指定的文件或目录
	}
	if((calendar.min%5==0) && calendar.sec<2 && ToDataSDCard_WriteFlag==0)  //每一个小时往SD卡里面写入指定的数据
	{
		SaveDataToSDCard("0:/Sensordata.txt");
		ToDataSDCard_WriteFlag = 1;
	}
	if(calendar.min%5 != 0)
		ToDataSDCard_WriteFlag = 0;
	BatADCVal = Get_Adc_Average(ChalTab[3],5);  //得到电池电压值的模拟量
	BatVal = (uint8_t)(BatADCVal*4096*(BAT_RES1+BAT_RES2))/(3.3*BAT_RES1);  //采样电阻为BAT_RES1
	LCD_DrawRectangle(lcddev.width-30,3,lcddev.width-3,13,GRAY);
	LCD_Fill(lcddev.width-3,5,lcddev.width,10,GRAY);
	LCD_Fill(lcddev.width-29,4,lcddev.width-29+BatVal,12,LGRAY);  //横坐标不能大于35，框的横向像素范围
}
//从SD卡里面读取保存的传感器数据
//文件路径 ID号 数据保存区
void LoadDataWaveFromSD(uint8_t *filename,uint8_t Item)
{
	UINT br;
	FIL *f_Readsenordata;
	uint8_t res = 0;
	uint16_t i;
	uint16_t ReadLen = 0;
	uint16_t TempData;  //280为像素点
	uint16_t *Databuf;  //数据指针区
	uint8_t *Senorbuf = 0;
	DataRemTypedef *tempdaydata;
	ReadLen = sizeof(DataRemTypedef);     //15*24*3;  
	Databuf = (uint16_t *)mymalloc(ReadLen);
	if(Databuf==NULL)
		return;
	f_Readsenordata = (FIL *)mymalloc(sizeof(FIL));
	if(f_Readsenordata==NULL)
	{
		myfree(Databuf);  //释放掉内存
		return;
	}
	res=f_open(f_Readsenordata,(const TCHAR*)filename,FA_READ);
	if(res==0)  		  //读取成功的话
	{
		f_read(f_Readsenordata,Databuf,ReadLen,(UINT*)&br);
	}
	tempdaydata = (DataRemTypedef*)Databuf;  //指针转换为结构体可用的类型
	f_lseek(f_Readsenordata,tempdaydata->Time);  //文件指针偏移到数据的起始处
	for(i=0;i<280;i++)  //如果是从一天的某个时刻开机的则读取此时的到23时就行了
	{
		switch(Item)
		{
			case Temp_ID:
			{
				 TempData = DeadScale(tempdaydata->Item_Id[Temp_ID],0,50);
				 LCD_Fast_DrawPoint(X0+i,Y0-TempData*3.4,(uint16_t)ColorTab[Temp_ID]);
			}break;
			case Dump_ID:
			{
				 TempData = DeadScale(tempdaydata->Item_Id[Dump_ID],0,50);
				 LCD_Fast_DrawPoint(X0+i,Y0-TempData*3.4,(uint16_t)ColorTab[Dump_ID]);
			}break;
			case HCHO_ID:
			{
				 TempData = DeadScale(tempdaydata->Item_Id[HCHO_ID],0,20);
				 LCD_Fast_DrawPoint(X0+i,Y0-TempData*17,(uint16_t)ColorTab[HCHO_ID]);
			}break;
			case UPres_ID:
			{
				 TempData = DeadScale(tempdaydata->Item_Id[UPres_ID],0,10000);
				LCD_Fast_DrawPoint(X0+i,Y0-TempData*1.7/100,(uint16_t)ColorTab[UPres_ID]);
			}break;
			case DangerGas_ID:
			{
				 TempData = DeadScale(tempdaydata->Item_Id[DangerGas_ID],0,100);
				 LCD_Fast_DrawPoint(X0+i,Y0-TempData*1.7,(uint16_t)ColorTab[DangerGas_ID]);
			}break;
			case PM2_5_ID:
			{
				TempData = DeadScale(tempdaydata->Item_Id[PM2_5_ID],0,20);
				LCD_Fast_DrawPoint(X0+i,Y0-TempData*1.7,(uint16_t)ColorTab[PM2_5_ID]);
			}break;
			case Illumn_ID:
			{
				TempData = DeadScale(tempdaydata->Item_Id[Illumn_ID],0,150);
				LCD_Fast_DrawPoint(X0+i,Y0-TempData*1.7,(uint16_t)ColorTab[Illumn_ID]);
			}break;			
		    default:break;
		}
		f_read(f_Readsenordata,Databuf,sizeof(DataRemTypedef),(UINT*)&br);
		tempdaydata = (DataRemTypedef*)Databuf;
		f_lseek(f_Readsenordata,sizeof(DataRemTypedef));  //文件指针偏移
	}
	f_close(f_Readsenordata);  //关闭文件,释放内存
	myfree(Databuf);
	myfree(f_Readsenordata);
}
//切换界面信息
//界面标号
void ChangeUI(uint8_t UInum)
{
	LoadSensorData(&SesorData[0]);  //加载传感器数据
	switch(UInum)
	{
		case 1:
		{
			LoadUI_1();
		}break;
		case 2:
		{
			LoadUI_2(0,5,18,16);			
		}break;
		case 3:
		{
			if(PreWave_Id!=Wave_Id)
			{
				LoadUI_3(X0,Y0,HSCAL_UNIT,VSCAL_UNIT,(uint8_t)Item_Id[Wave_Id]);				
				PreWave_Id = Wave_Id;
				if(Wave_Id==0)
				{
					//LCD_Fill(CURSOR-22,16,320,240,FillColor);
					PreWave_Id = 0XFF;
				}
			}
//			LoadUI_3(X0,Y0,HSCAL_UNIT,VSCAL_UNIT,Item_Id[UPres_ID]);
		}break;
		default :break;
	}
	DispTimeToUI();  //更新时间
}

uint16_t Testbuf[10]={12345,56789,23546,65519,58754,25345,61257,25643,23453,45332};
//SD卡读写测试.单,双字节下已经测试成功
void SDWiriteDataTest(uint8_t *filename,uint8_t mode)
{
	UINT bw;
	FIL *F_testdata;
	uint16_t *databuf;
	uint8_t res;
	databuf = (uint16_t *)mymalloc(sizeof(Testbuf));  //申请50个字节的空间
	if(databuf==NULL)
	{
		Show_Str(0,100,300,16,"内存申请失败",16,0,GREEN,BLACK);
		return;
	}
	databuf = Testbuf;
	F_testdata = (FIL *)mymalloc(sizeof(FIL));
	if(F_testdata==NULL)
	{
		myfree(databuf);
		return;
	}
	if(mode==1)
		res = f_open(F_testdata,(const TCHAR*)filename,FA_READ|FA_WRITE);  	//打开之前有的文件
	if(mode==0||res==0x04)  //如果打开失败则创建新文件
		res = f_open(F_testdata,(const TCHAR*)filename,FA_WRITE|FA_CREATE_NEW);
	//f_sync(F_testdata);
	if(res==FR_OK)
	{
		res = f_write(F_testdata,databuf,sizeof(Testbuf),(UINT *)&bw); 
		f_close(F_testdata);
	}
	myfree(databuf);
	myfree(F_testdata);
}
void SDReadDataTest(uint8_t *filename)
{
	uint8_t i;
	UINT br;
	FIL *F_testdata;
	uint16_t *databuf;
	uint8_t res;
	databuf = (uint16_t *)mymalloc(sizeof(Testbuf));  //申请50个字节的空间
	if(databuf==NULL)
	{
		Show_Str(0,100,300,16,"内存申请失败",16,0,GREEN,BLACK);
		return;
	}
	F_testdata = (FIL *)mymalloc(sizeof(FIL));
	if(F_testdata==NULL)
	{
		myfree(databuf);
		return;
	}
	res=f_open(F_testdata,(const TCHAR*)filename,FA_READ);
	if(res==0)
	{
		f_read(F_testdata,databuf,sizeof(Testbuf),(UINT *)&br);
		for(i=0;i<10;i++)
		{
			LCD_ShowNum(0,i*18,databuf[i],6,16,GREEN,BLACK);//LCD_ShowNum(uint16_t x,uint16_t y,u32 num,uint8_t len,uint8_t size);
		}
		f_close(F_testdata);
	}
	myfree(databuf);
	myfree(F_testdata);	
}
void TestSDCard(void)
{
	SDWiriteDataTest("0:/guchenglong.txt",1);
	SDReadDataTest("0:/guchenglong.txt");
	Show_Str(0,200,300,16,"完成",16,0,GREEN,BLACK);
}



