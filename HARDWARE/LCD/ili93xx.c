#include "include.h"
#include "font.h"
//ili9488液晶屏驱动
//实现了画点函数和液晶屏的初始化等
			 
//LCD的画笔颜色和背景色	   
uint16_t POINT_COLOR = GREEN;//0xF800;	//画笔颜色 默认为红色 黑色为0X0000
uint16_t BACK_COLOR = BLACK;//SYSGBLUE;	//默认系统背景颜色
//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;
	 
//写寄存器函数
//regval:寄存器值         0XE0
void LCD_WR_REG(uint8_t regval)
{ 	 
	LCD_CS_CLR; 
	LCD_RS_CLR;//写地址	
		 
	GPIOB->ODR = (regval<<8);
	
	LCD_WR_CLR; 
	LCD_WR_SET;
	
 	LCD_CS_SET;
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(uint8_t data)
{				
	LCD_CS_CLR;	
	LCD_RS_SET;

	GPIOB->ODR = (data<<8);
	
	LCD_WR_CLR;
	LCD_WR_SET;
	
	LCD_CS_SET;  
}
//读LCD数据
//返回值:读到的值
uint8_t LCD_RD_DATA(void)
{										    	   
	uint8_t Temp;
 	GPIOB->CRH=0X88888888; //PB8-15  上拉输入
	GPIOB->ODR &= 0x0000;     //全部输出0

	LCD_RS_SET;
	LCD_CS_CLR;
	LCD_RD_CLR;
	Temp = DATAIN;  
	LCD_RD_SET;
	LCD_CS_SET; 

	GPIOB->CRH = 0X33333333; //PB0-7  上拉输出
	GPIOB->ODR |= 0XFF00;    //全部输出高
	return (uint8_t)(Temp>>8);   
}					   
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{					
	uint16_t Temp;
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	delay_ms(5);	
	Temp = LCD_RD_DATA()|(LCD_RD_DATA()>>8);
//	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
//	delay_ms(5);
//	Temp |= (LCD_RD_DATA()>>8);
	return Temp;		//返回读到的值
}   
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
  LCD_WR_REG(lcddev.wramcmd);
}	 
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code>>8);//写十六位GRAM
	LCD_WR_DATA(RGB_Code&0xff);	
}
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 
//当mdk -O1时间优化时需要设置
//延时i
void opt_delay(uint8_t i)
{
	while(i--);
}
//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r,g,b;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		   
	GPIOB->CRH=0X88888888; //PB8-15 上拉输入
	GPIOB->ODR=0XFFFF;     //全部输出高

	LCD_RS_SET;
	LCD_CS_CLR;	    
	//读取数据(读GRAM时,第一次为假读)	
	LCD_RD_CLR;	
  	delay_us(1);//延时1us					   
	LCD_RD_SET;
 	//dummy READ
	LCD_RD_CLR;					   
	delay_us(1);//延时1us					   
 	r=DATAIN;  	//实际坐标颜色
	LCD_RD_SET;
 
	LCD_RD_CLR;					   
	b=DATAIN;//读取蓝色值  	  
	LCD_RD_SET;
	g=r&0XFF;//对于9341,第一次读取的是RG的值,R在前,G在后,各占8位
	g<<=8;
 
	LCD_CS_SET;
	GPIOB->CRH=0X33333333; 		//PB8-15 上拉输出
	//GPIOB->ODR=0XFFFF;    		//全部输出高  
  return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
}			 
//LCD开启显示
void LCD_DisplayOn(void)
{
	LCD_WriteReg(0X07,0x0173);		
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg(0X07,0x00);	
}    		 
//LCD是否进入睡眠模式
void LCD_EnterSleep(uint8_t orSleep)
{
	uint16_t i=0;
 	uint16_t j=0;
	
	if(orSleep) 
	{
		LCD_WR_REG(0x28);
		delay_ms(10); 
		LCD_WR_REG(0x10);     
		delay_ms(120); 
	}
	else
	{
		LCD_WR_REG(0x11);     
		delay_ms(120); 
		LCD_WR_REG(0x29);
	}
}
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	    
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA((Xpos>>8)&0xff);
	LCD_WR_DATA(Xpos&0xff); 			 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA((Ypos>>8)&0xff);
	LCD_WR_DATA(Ypos&0xff); 		
} 
//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	uint16_t temp;  
	if(lcddev.dir==1&&lcddev.id!=0X6804)//横屏时，对6804不改变扫描方向！
	{
		switch(dir)//方向转换
		{
			case 0:dir=6;break;
			case 1:dir=7;break;
			case 2:dir=4;break;
			case 3:dir=5;break;
			case 4:dir=1;break;
			case 5:dir=0;break;
			case 6:dir=3;break;
			case 7:dir=2;break;	     
		}
	}
	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(1<<7)|(1<<6)|(1<<5); 
			break;	 
	}
	dirreg=0X36;
	regval|=0X08;//5310/5510不需要BGR      
	LCD_WriteReg(dirreg,regval);
	if((regval&0X20)||lcddev.dir==1)
	{
		if(lcddev.width<lcddev.height)//交换X,Y
		{
			temp=lcddev.width;
			lcddev.width=lcddev.height;
			lcddev.height=temp;
		}
	}else  
	{
		if(lcddev.width>lcddev.height)//交换X,Y
		{
			temp=lcddev.width;
			lcddev.width=lcddev.height;
			lcddev.height=temp;
		}
	}  
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.width-1)>>8);
	LCD_WR_DATA((lcddev.width-1)&0XFF);
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.height-1)>>8);
	LCD_WR_DATA((lcddev.height-1)&0XFF);  
}   
//读取液晶屏的ID号
uint32_t Read_LCD_ID(void)
{
	LCD_WR_REG(0xd3);

}
//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	   
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(x>>8); 	
	LCD_WR_DATA(x&0XFF); 
	LCD_WR_DATA(x>>8); 	
	LCD_WR_DATA(x&0XFF); 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(y>>8); 	
	LCD_WR_DATA(y&0XFF);
	LCD_WR_DATA(y>>8); 	
	LCD_WR_DATA(y&0XFF); 	
	LCD_WR_REG(lcddev.wramcmd); 
	LCD_WR_DATA((color>>8)&0xff); 	
	LCD_WR_DATA(color&0xff);
}	 


//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
	//横屏  				
	lcddev.dir=1;	//横屏
	lcddev.width=480;
	lcddev.height=320;
	
	lcddev.wramcmd=0X2C;
	lcddev.setxcmd=0X2A;
	lcddev.setycmd=0X2B;  	 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}
//液晶屏背光亮度调节初始化
void TFT_BK_Light(void)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;       //输出比较
    TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;     //定时时基
    GPIO_InitTypeDef GPIO_InitStructure;
  
	uint16_t PrescalerValue = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE);
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;  //3-1=2;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 999;		//定时器预装载值
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;	//pwm时钟分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		//向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	/* PWM1 Mode configuration: Channel */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;//初始占空比为0
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
//	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM2,ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	//初始化PWM输出控制引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	TIM2->CCR2 =1;  //开始先不点亮屏,占空比越小，屏幕越亮
}
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void TFTLCD_Init(void)
{
	uint16_t i,j;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); //使能PORTB,C时钟和AFIO时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//开启SWD，失能JTAG
	TFT_BK_Light();
	//LCD_RD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//LCDBKCtrl
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//LCD_RS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//LCD_CS  LCD_WR
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//LCD_DB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|
								GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	//除过PB11那个脚
	GPIO_Init(GPIOB, &GPIO_InitStructure); //GPIOB
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|
						GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
 	delay_ms(50);  
	
	LCD_RS_SET;
	delay_ms(1);
	LCD_RS_CLR;
	delay_ms(10);
	LCD_RS_SET;
	delay_ms(120);
	
	delay_ms(50); // delay 50 ms 
 	LCD_WriteReg(0x0000,0x0001);
	delay_ms(50); // delay 50 ms 
	lcddev.id = LCD_ReadReg(0XD3);
	
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x0D); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x3c); 
	LCD_WR_DATA(0x78); 
	LCD_WR_DATA(0x4A); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x09); 
	LCD_WR_DATA(0x1B); 
	LCD_WR_DATA(0x1e); 
	LCD_WR_DATA(0x0f);  
	
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x24); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x12); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x36); 
	LCD_WR_DATA(0x47); 
	LCD_WR_DATA(0x47); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x0a); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x37); 
	LCD_WR_DATA(0x0f); 
	
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x10); 
	
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA(0x41); 
	
	LCD_WR_REG(0xC5); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x80); 
	
	LCD_WR_REG(0x36); 
	LCD_WR_DATA(0x48); 
	
	LCD_WR_REG(0x3A); //Interface Mode Control
	LCD_WR_DATA(0x55);
		
	LCD_WR_REG(0XB0);  //Interface Mode Control  
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0xB1);   //Frame rate 70HZ  
	LCD_WR_DATA(0xB0); 
	LCD_WR_DATA(0x11); 
	LCD_WR_REG(0xB4); 
	LCD_WR_DATA(0x02);   
	LCD_WR_REG(0xB6); //RGB/MCU Interface Control
	LCD_WR_DATA(0x02); 
	LCD_WR_DATA(0x02); 
	
	LCD_WR_REG(0xB7); 
	LCD_WR_DATA(0xC6); ;
	
	LCD_WR_REG(0xE9); 
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0XF7);    
	LCD_WR_DATA(0xA9); 
	LCD_WR_DATA(0x51); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x82);
	
	LCD_WR_REG(0x11); 
	delay_ms(120); 
	LCD_WR_REG(0x29); 
	
	//LCD_WR_REG(0xd3);
	LCD_Display_Dir(1);
	LCD_LED = 0;					//点亮背光
	//LCD_Clear(BLACK);
}
//清屏函数
//color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 			//得到总点数 	 			
	LCD_SetCursor(0,0);		//设置光标位置    	 
	LCD_WriteRAM_Prepare();     		//开始写入GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD_WR_DATA((color>>8)&0XFF);	
		LCD_WR_DATA(color&0XFF);
	}
}  
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
	uint8_t hsareg,heareg,vsareg,veareg;
	uint16_t hsaval,heaval,vsaval,veaval;

	hsaval=sy;				
	heaval=height;
	vsaval=lcddev.width-width-1;
	veaval=lcddev.width-sx-1;
	hsareg=0X50;heareg=0X51;//水平方向窗口寄存器
	vsareg=0X52;veareg=0X53;//垂直方向窗口寄存器	

	//设置寄存器值
	LCD_WriteReg(hsareg,hsaval);
	LCD_WriteReg(heareg,heaval);
	LCD_WriteReg(vsareg,vsaval);
	LCD_WriteReg(veareg,veaval);		
	LCD_SetCursor(sx,sy);	//设置光标位置
}
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色     0,150,200,250
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j;
	uint16_t xlen=0;
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(xsta>>8);
	LCD_WR_DATA(xsta&0xff);
	LCD_WR_DATA(xend>>8);
	LCD_WR_DATA(xend&0xff);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(ysta>>8);
	LCD_WR_DATA(ysta&0xff);
	LCD_WR_DATA(yend>>8);
	LCD_WR_DATA(yend&0xff);
	LCD_WriteRAM_Prepare();
	xlen=xend-xsta+1;	
	for(i=ysta;i<=yend;i++) 
	{
		for(j=0;j<xlen;j++)
		{
			LCD_WR_DATA((color>>8)&0XFF);	//设置光标位置 
			LCD_WR_DATA(color&0XFF);
		}
	}
}
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  ,
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_Fast_DrawPoint(uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

//画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}
//在指定位置画一个指定角度的圆弧
//(x,y):中心点
//r    :半径
//Angle:角度
void LCDDrawCir(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color,uint8_t xiangxian,uint8_t orFill)
{
	int i,j;
	int a;//对象限的分割线
	int b;//
	int di;
	a=0;b=r;
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		if(orFill)
		{
			for(i=x0;i<x0+a;i++)
			{
				LCD_Fast_DrawPoint(i,y0-a,color);          //1   
				LCD_Fast_DrawPoint(i,y0-b,color);
			}
			for(i=x0+a;i<x0+b;i++)
			{
				LCD_Fast_DrawPoint(i,y0-a,color);       //2         
			}
			for(i=x0+a;i<x0+b;i++)
			{
				LCD_Fast_DrawPoint(i,y0+a,color);         //3
			}
			for(i=x0;i<x0+a;i++)
			{
				LCD_Fast_DrawPoint(i,y0+a,color);         //4
				LCD_Fast_DrawPoint(i,y0+b,color);
			}
			for(i=x0-a;i<x0;i++)
			{
				LCD_Fast_DrawPoint(i,y0+a,color);         //5
				LCD_Fast_DrawPoint(i,y0+b,color);
			}
			for(i=x0-b;i<=x0-a;i++)
			{
				LCD_Fast_DrawPoint(i,y0+a,color);         //6
			}
			for(i=x0-b;i<x0-a;i++)
			{
				LCD_Fast_DrawPoint(i,y0-a,color);         //7
			}
			for(i=x0-a;i<x0;i++)
			{
				LCD_Fast_DrawPoint(i,y0-a,color);      //8
				LCD_Fast_DrawPoint(i,y0-b,color);
			}
		}
		else
		{
			switch(xiangxian)
			{
				case 1:   //第一象限
				LCD_Fast_DrawPoint(x0+a,y0-b,color);             //5
				LCD_Fast_DrawPoint(x0+b,y0-a,color);             //0 
				break;
				case 4:  //第四象限
				LCD_Fast_DrawPoint(x0+b,y0+a,color);             //4               
				LCD_Fast_DrawPoint(x0+a,y0+b,color);             //6 
				break;
				case 3:  //第三象限
				LCD_Fast_DrawPoint(x0-a,y0+b,color);             //1       
				LCD_Fast_DrawPoint(x0-b,y0+a,color);             //3
				break;
				case 2:  //第二象限
				LCD_Fast_DrawPoint(x0-a,y0-b,color);             //2             
				LCD_Fast_DrawPoint(x0-b,y0-a,color);             //7
				break;
				case 0:
				LCD_Fast_DrawPoint(x0+a,y0-b,color);             //5
				LCD_Fast_DrawPoint(x0+b,y0-a,color);
				LCD_Fast_DrawPoint(x0+b,y0+a,color);             //4               
				LCD_Fast_DrawPoint(x0+a,y0+b,color);
				LCD_Fast_DrawPoint(x0-a,y0+b,color);             //1       
				LCD_Fast_DrawPoint(x0-b,y0+a,color);
				LCD_Fast_DrawPoint(x0-a,y0-b,color);             //2             
				LCD_Fast_DrawPoint(x0-b,y0-a,color);  
				break;
				default :break;
			}
		}
		a++;
		//使用Bresenham算法画圆
		if(di<0)
			di += 4*a+6;
		else
		{
			di+=10+4*(a-b);
			b--;
		} 						    
	}
}
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode,uint16_t point_color,uint16_t backcolor)
{  							  
  uint8_t temp,t1,t;
	uint16_t y0=y;
	uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数	
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	for(t=0;t<csize;t++)
	{   
		if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
		else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
		else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
		else if(size==36)temp=asc2_3636[num][t];
		else return;								//没有的字库
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)
				LCD_Fast_DrawPoint(x,y,point_color);
			else if(mode==0)
				LCD_Fast_DrawPoint(x,y,backcolor);
			temp<<=1;
			y++;
			if(x>=lcddev.width)return;		//超区域了
			if((y-y0)==size)
			{
				y=y0;
				x++;
				if(x>=lcddev.width)return;	//超区域了
				break;
			}
		}  	 
	}  	
}

//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(uint8_t m,uint8_t n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}
uint8_t GetValueBit(uint32_t Value)
{
	uint8_t Bit;
	if((Value/10000)!=0)
		Bit = 5;
	else if((Value/1000)!=0)
		Bit = 4;
	else if((Value/100)!=0)
		Bit = 3;
	else if((Value/10)!=0)
		Bit = 2;
	else 
		Bit = 1;
	return Bit;
}
//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(uint16_t x,uint16_t y,u32 num,uint8_t len,uint8_t size,uint16_t Point_Color,uint16_t Back_Color)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;		
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0,Point_Color,Back_Color);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0,Point_Color,Back_Color); 
	}
} 
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(uint16_t x,uint16_t y,u32 num,uint8_t len,uint8_t size,uint8_t mode)
{
	uint8_t t,temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)
					LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01,POINT_COLOR,BACK_COLOR);  
				else
					LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01,POINT_COLOR,BACK_COLOR);  
 				continue;
			}
			else
				enshow=1; 		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01,POINT_COLOR,BACK_COLOR); 
	}
}
//显示字符串
//x,y:起点坐标
//width,height:区域大小
//size:字体大小
//*p:字符串起始地址
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p,uint16_t point_color,uint16_t backcolor)
{
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width)
		{
			x=x0;
			y+=size;
		}
        if(y>=height)
			break;//退出
        LCD_ShowChar(x,y,*p,size,0,point_color,backcolor);
        x+=size/2;
        p++;
    }
}
//显示一个系统初始化进度,一共二十个点
void LCDDispStart(uint16_t x0,uint16_t y0,uint8_t Numpoint)
{
	uint8_t i;
	for(i=0;i<Numpoint;i++)
	{
		LCDDrawCir(x0+i*10,y0,2,LGRAY,0,1);
		delay_ms(150);
	}
}

//OLED显示一个简图在指定位置
//输入参数：坐标，显示区域大小，位图存贮指针，显示模式(是否填充)
void Show_DrawBMP(uint16_t x,uint16_t y,uint16_t BmpXSize,uint16_t BmpYSize,uint8_t *buf,uint16_t Colour)
{
	uint16_t temp,t,t1;
	uint16_t y0=y;		
	uint16_t Area_Size = BmpXSize*BmpYSize/8;  //总的字节大小
    for(t=0;t<Area_Size;t++)
    {
		temp=buf[t];  //得到图片的一个字节数据	                          
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)
				LCD_Fast_DrawPoint(x,y,Colour);	
			else 
				LCD_Fast_DrawPoint(x,y,BLACK);	   
			temp<<=1;
			y++;
			if((y-y0)==BmpYSize)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }  
}


























