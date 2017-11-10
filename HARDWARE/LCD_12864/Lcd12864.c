#ifndef _LCD12864__C
#define _LCD12864__C
#include "include.h"
#define   D    3               //延时宏定义
u8 const table_num[]="0123456789abcdefg";    //显示数字用
//功能：12864I\O口初始化
//参数：无
void lcd12864_IO_init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;	
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PC端口时钟
	 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_All;               //数据控制端口
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 				//输出频率
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 					//设置为推挽输出
	 GPIO_Init(GPIOD, &GPIO_InitStructure);										//结构体初始化	
	 GPIO_SetBits(GPIOD,GPIO_Pin_All);
	
	 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;;//命令控制端口
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 				//输出频率
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 					//设置为推挽输出
	 GPIO_Init(GPIOC, &GPIO_InitStructure);										//结构体初始化	
	 GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);	
}
//功能：检测12864是否处于忙的状态
//参数：无
/*void Lcd12864CheckBusy( void )      
{
    unsigned int nTimeOut = 0 ;
    SET_INC;
    SET_READ;
    CLR_EN;
    SET_EN;
	  GPIOB->CRH=0X00;
    while( ( GPIOB->IDR & 0x80 ) && ( ++nTimeOut != 0 ) ) ;
	  GPIOB->CRH=0XFF00;
    CLR_EN;
    SET_INC;
    SET_READ;
}*/
//功能：12864发送命令
//参数：byCmd 要发送的数据命令
void Lcd12864SendCmd(u8 byCmd )
{
    //v_Lcd12864CheckBusy_f() ;
	  delay_ms(D);
    SET_INC;
    SET_WRITE;
    CLR_EN;
    GPIOD->ODR=byCmd ;    //将数据放入端口位的低八位
    delay_ms(D);
    delay_ms(D);
    SET_EN;
    delay_ms(D);
    delay_ms(D);
    CLR_EN;
    SET_READ;
    SET_INC;
}
//功能：12864发送数据
//参数：byData 要发送的数据
void Lcd12864SendData( u8 byData )    
{
    //Lcd12864CheckBusy();
	  delay_ms(D);
    SET_DATA;
    SET_WRITE;
    CLR_EN;
    GPIOD->ODR=byData ;  //将数据放入低八位数据端口
    delay_ms(D);
    delay_ms(D);
    SET_EN;  
    delay_ms(D);
    delay_ms(D);
    CLR_EN;
    SET_READ;
    SET_INC;
}
//功能：12864整体初始化
//参数：无
void Lcd12864Init( void )                  //初始化
{
	  lcd12864_IO_init();
    Lcd12864SendCmd( 0x30 ) ;              //基本指令集
    delay_ms(500);
    Lcd12864SendCmd( 0x01 ) ;              //清屏
    delay_ms(500);
    Lcd12864SendCmd( 0x06 ) ;              //光标右移
    delay_ms(500);
    Lcd12864SendCmd( 0x0c ) ;              //开显示
}
void Lcd12864ClerScreen(void)
{
    Lcd12864SendCmd( 0x01 ) ;
}
//功能：进行XY地址转换
//参数：无
void Lcd12864SetAddress( u8 x, u8 y )  //地址转换
{
    u8 byAddress ;
    switch( y )
    {
        case 0 :    byAddress = 0x80 + x ;
            break;
        case 1 :    byAddress = 0x90 + x ;
            break ;
        case 2 :    byAddress = 0x88 + x ;
            break ;
        case 3 :    byAddress = 0x98 + x ;
            break ;
        default :
            break ;
    }
    Lcd12864SendCmd( byAddress ) ;
}
//功能：12864显示字符串
//参数：X坐标，Y坐标，pData 字符串指针或数组首地址
//说明:两个空格代表一个空格
void Lcd12864PutString( u8 x, u8 y, u8 *pData )
{
    Lcd12864SetAddress( x, y );
    while( *pData != '\0' )
    {
        Lcd12864SendData( *pData++ ) ;
    }
}       
//功能：12864显示一个十进制数字
//参数：XY坐标，num 要显示的数字
//范围：0-255
void Lcd12864PutNum(u8 x, u8 y, long num)
{
    u16 Temp;
    Temp=num%100000;  //取出实际的部分
		Lcd12864SetAddress(x,y);
		Lcd12864SendData(table_num[Temp/10000]);      //提出要用的数字
		Lcd12864SendData(table_num[Temp/1000%10]);
		Lcd12864SendData(table_num[Temp/100%10]);
    Lcd12864SendData(table_num[Temp/10%10]);
    Lcd12864SendData(table_num[Temp%10]);
}
//功能：12864显示初始化
//参数;无
void display_12864(void)
{
	  Lcd12864PutString(0,0,"IP:192.168.1.16");	 
	  Lcd12864PutString(0,1,"PORT:1400");	
	  Lcd12864PutString(0,2,"NowSTATUS:");	
	  Lcd12864PutString(0,3,"信道:");
}
//画图函数 入口参数为数组指针
void Draw_pic(const unsigned char *p)
{ 
	unsigned char i,j;

	Lcd12864SendCmd(0x34);   //打开扩充指令集，关闭绘图显示
	for(j=0;j<32;j++)	 //一、二行
	{
		Lcd12864SendCmd(0x80+j);   //垂直坐标
		Lcd12864SendCmd(0x80);     //水平坐标，AC指针会自动横着加一
		for(i=0;i<16;i++)
		{
			Lcd12864SendData(*p++);  
		}
	}
	
	for(j=0;j<32;j++)	 //三、四行
	{
		Lcd12864SendCmd(0x80+j);   //垂直坐标
		Lcd12864SendCmd(0x88);     //水平坐标，AC指针会自动横着加一
		for(i=0;i<16;i++)
		{
			Lcd12864SendData(*p++);  
		}
	}
	Lcd12864SendCmd(0x36);   //打开绘图显示
	Lcd12864SendCmd(0x30);   //回到基本指令集
}



#endif















