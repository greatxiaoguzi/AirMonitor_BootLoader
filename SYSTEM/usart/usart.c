#include "sys.h"
#include "usart.h"	  
#include <stdio.h>                                                                                            
//////////////////////////////////////////////////////////////////////////////////
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "include.h"					//ucos 使用
#endif
//********************************************************************************
//V1.3修改说明
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
//////////////////////////////////////////////////////////////////////////////////
 
///加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//串口1支持printf函数
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/

//变量和缓存区
static uint8_t Step = 0;    //串口数据接收步骤 
static uint8_t Step3 = 0;
uint8_t UartRecBuf[USART_REC_LEN];  //字节大小100个字
uint8_t WIFIRecDataFlag = 0;    //接收数据标志
//uint8_t WifiSetFlag = 0;   //开机起始的时候为零
uint8_t JQRec[15];       //甲醛传感器接收到的数据缓冲区
uint8_t jQRecSta = 0;     //记录甲醛传感器接收到的数据的实际位数
uint8_t JQRecFlag = 0;    //甲醛数据接收完成标志
uint8_t JQDataDealRes = 0; //甲醛数据处理结果
void PB11ConfigUart(uint8_t Mode)
{
	switch(Mode)
	{
		case UartMode:
		{    
			//配置PB11为浮空输入
			{GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=4<<12;}
			//配置PB10为复用推挽输出模式
			{GPIOB->CRH&=0XFFFFF0FF;GPIOB->CRH|=11<<8;}
		}break;
		case TFTMode:
		{    //配置PB11为通用推挽输出模式
			{GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}
			//配置PB10为通用推挽输出模式
			{GPIOB->CRH&=0XFFFFF0FF;GPIOB->CRH|=3<<8;}
		}break;
		default:break;
	}
}
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
//数组状态量,起始码，起始码，字节长度，
//uint8_t USART_TX_BUF[USART_TEC_LEN];         //发送缓冲区,暂时不用  
//初始化IO 串口1 串口3 串口4
//bound:波特率
//串口1： TX PA.9   RX	PA.10  串口3：TX PB10   RX	 PB11  串口4： TXPC10   RXPC11
//注意：串口1优先级高于串口3
void uart_init(u32 bound1,u32 bound2,u32 bound4)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART1，GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_UART4, ENABLE);	//使能USART1，USART4时钟时钟
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//    USART_DeInit(USART1);  //复位串口1
//    USART_DeInit(USART3);
//    USART_DeInit(UART4);
  
    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9  
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10		
	//USART2_TX	  PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
	//USART2_RX   PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10	
    //USART4_TX PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化C10	
		//USART4_RX	  PC11d
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PC11
      
     //USART1 初始化设置
    USART_InitStructure.USART_BaudRate = bound1;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART1, &USART_InitStructure); //初始化串口1
    //USART3 初始化设置
    USART_InitStructure.USART_BaudRate = bound2;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口3
     //USART4 初始化设置
    USART_InitStructure.USART_BaudRate = bound4;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(UART4, &USART_InitStructure); //初始化串口4   
    
	//Usart1 NVIC 配置   WIFI数据传输
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 
    //Usart2 NVIC 配置   WIFI配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 
    //Usart4 NVIC 配置   甲醛传感器数据接收
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;//抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器 
	
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//开启中断1
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//开启中断3
    USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//开启中断4
    USART_Cmd(USART1, DISABLE);                    //使能串口1
    USART_Cmd(USART2, DISABLE);
    USART_Cmd(UART4, DISABLE);
//使能串口3
}
//数据校验计算
uint8_t CheckSum(uint8_t *buf,uint8_t len)
{
    uint8_t i;
    uint8_t temp=0;
    for(i=0;i<len;i++)
    {
       temp^=*(buf+i);
    }
    return temp;
}
//判断帧数据包是否已完成接收
uint8_t FindPkg(uint8_t *buf)
{
    uint8_t i;
    for(i=0;i<Step;i++)
    {
       if(*(buf+i)=='+'&&*(buf+i+1)=='I') //两个帧头
       {
          if(*(buf+i+3)=='D')    //数据长度是否为此次接收到的实际长度
            return i;                    //返回真开始的地方
       }
    }
    return 0XFF;
}
//检查数据包
uint8_t CheckPkg(uint8_t *buf)
{
   uint8_t i;
   for(i=0;i<Step;i++)
	{
		if(*(buf+i-1)=='&')
		{
			if(*(buf+i)=='#')
				return 0;
		}
	}
	return 1;
}
//串口中断接收程序
//判断接收到的数据帧
//+IPD,0,7:数据区
//OK
uint8_t RemFlag = 0;
#if EN_USART1_RX   //如果使能了接收
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.进入中断保护
   OSIntEnter();    //进入OS中断保护
#endif
   if(USART1->SR&(1<<5))  //判断是否为串口1接收中断	
   {
      if(Step<USART_REC_LEN)  //小于实际的大小则继续接受
      {
         UartRecBuf[Step]=USART1->DR;	//读取接收到的数据
         Step++;        //接受次序
      }
      else
      {
         Step=0;  
		 memset((void *)&JQRec[0],0,USART_REC_LEN);  //清空缓冲区 		  
      }
      if(UartRecBuf[Step-1]=='D'&&RemFlag ==0)  //服务器发来了数据
      {
		if(UartRecBuf[Step-4]=='+')
			RemFlag = 1;
      }
	  if(RemFlag)
	  {
		if(UartRecBuf[Step-1]=='#')
		{
			if(UartRecBuf[Step-2]=='&')
			{
				RemFlag = 0;
				WIFIRecDataFlag = 1;
			}
		}
	  }
   }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  			//退出中断保护									 
#endif
}
#endif
//串口3中断函数
void USART3_IRQHandler(void)                	//串口1中断服务程序
{
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.进入中断保护
OSIntEnter();    
#endif	
	if(USART3->SR&(1<<5))  //接收中断
	{
		if(Step3<USART_REC_LEN)
		{
			
		}
		else
		{
			memset((void *)&UartRecBuf[0],sizeof(UartRecBuf),0); //如果错误的话清空缓存区
		}
		Step3++;     //接受次序
		if(Step3==USART_REC_LEN)
			Step3=0;
	}
#ifdef OS_TICKS_PER_SEC	 	
	OSIntExit();  					//退出中断保护				 		 
#endif	
}
//甲醛传感器接收数据处理
//串口屏显示不转成浮点数
//返回值;0正确  1错误
//(0X00*256+0X05)/100=0.05  mg
uint8_t DealHCHOData(uint8_t *buf)
{
//	if((*buf==0XA5) && (*(buf+1)==0X5A))
//	{
//		if(*(buf+8) == 0XAA)
//		{
//			SesorData[0].Jiaquanmg =*(buf+4)*256 + *(buf+5);       //mg值甲醛含量计算，这里没有换算成最后的实际值，实际值要除100
//			SesorData[0].Jiaquanppm = *(buf+6)*256 + *(buf+7);     //ppm值
//		}
//		else
//		{
//			SesorData[0].Jiaquanmg = 0;
//			SesorData[0].Jiaquanppm = 0;
//			return 1;
//		}
//	}
//	else
//	{
//		SesorData[0].Jiaquanmg = 0;
//		SesorData[0].Jiaquanppm = 0;
//		return 1;
//	} 
//	memset((void *)JQRec[0],10,0);  //清空缓冲区
	return 0;  
}
//发送甲醛数据读取命令
//帧格式：0XA5  0X5A 0X02 0X80 0XAA
void SendJQCmd(uint8_t const *buf)
{
  uint8_t i;
  for(i=0;i<5;i++)
  {    
	while((UART4->SR & 0x0040)==0x0000); 		 //等待发送完成
	UART4->DR = *(buf+i); 					 //写入数据
	delay_ms(10);  
  }
}
uint8_t FindPkgHead(uint8_t *buf)
{
   uint8_t i;
   for(i=0;i<jQRecSta;i++)
   {
     if((*(buf+i)==0XA5)&&(*(buf+i+1)==0X5A))
     {
          //if(*(buf+i+8)==0XAA)
          if(jQRecSta==(i+9))
          {
             return i;
          }
     }
   }
   return 0XFF;
}
//串口4中断服务函数用于甲醛数据接收
//0XA5  0X5A 0X06 0X80 0Xxx  0Xxx 0Xxx  0Xxx 0XAA
//   起始    数量  指令   甲醛mg     甲醛ppm  结束
void UART4_IRQHandler(void)                	//串口1中断服务程序
{
    uint8_t Inchar =0XFF;
    uint8_t Idx = 0;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.进入中断保护
	  OSIntEnter();    
#endif	
    if(UART4->SR&(1<<5))  //如果是接收到中断
    {
      Inchar = UART4->DR;
      if(jQRecSta<15)
      {        
        JQRec[jQRecSta] = Inchar;        //保存数据  
        jQRecSta++;   //用于字节接收
      }
      else
      {
        jQRecSta=0;
        memset((void *)&JQRec[0],10,0);  //清空缓冲区 
      }            
      Idx = FindPkgHead(JQRec);      //帧判断,寻找帧头
      if(Idx!=0XFF)
      {
            //处理数据包函数
            JQDataDealRes = DealHCHOData(&JQRec[Idx]);  //处理结果   
            if(JQDataDealRes == 0)
               JQDataDealRes = 1;  //数据正确处理
            else if(JQDataDealRes == 1)
               JQDataDealRes = 0;  //数据未正确处理完成
            jQRecSta=0;     //重新接收
      }     
      if(jQRecSta>15)
      {
        jQRecSta = 0; 
        memset((void *)&JQRec[0],10,0);  //清空缓冲区 
      }
   }
#ifdef OS_TICKS_PER_SEC	 	
	OSIntExit();  					//退出中断保护				 		 
#endif	
}

//#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
//	OSIntExit();  											 
//#endif
//#endif
/*串口发送数据函数
SendPacket:要发送的数据包
Len:数据数组大小*/
void UartSendText(char *text,uint8_t uartNum)
{
	uint8_t i;
	uint8_t Len;
	char *p;
	p = text;
	PB11ConfigUart(UartMode);
	Len=strlen(text);
	switch(uartNum)
	{
		case 1:
		{
		  for(i=0;i<Len;i++)
		  {                          			  
				while((USART1->SR & 0x0040)==0x0000);//等待串口发送完成 			  
				USART1->DR = *p++;//指针数组，数据包   
		  }
		}break;
		case 3:
		{
		  for(i=0;i<Len;i++)
		  {		   			  
				while((USART3->SR & 0x0040)==0x0000);
				USART3->DR = *p++;//指针数组，数据包
		  }  
		}break;
		case 4:
		{
		  for(i=0;i<Len;i++)
		  {               			   
			   while((UART4->SR & 0x0040)==0x0000);  //等待发送完成
			   UART4->DR = *p++;  //写入数据 
		  }
		}break;
        default:break;
//      SendPacket=Point;//指针重新指到开始出
	}
    PB11ConfigUart(TFTMode);  
}
//串口发送十六进制数据
void UsartSendDex(uint8_t Uartnum,uint8_t data)
{
	switch(Uartnum)
	{
		case 1:
		{
			while((USART1->SR&0X40)==0);
			USART1->DR = data;
		}break;
		case 3:
		{
			while((USART3->SR&0X40)==0);
			USART3->DR = data;
		}break;
		default:break;
	}
}
//串口缓冲区清零
void ClearBuf(void)
{
	uint8_t i;
	for(i=0;i<USART_REC_LEN-1;i++)
		UartRecBuf[i]= ' ';
	UartRecBuf[USART_REC_LEN-1]= '\0';
	Step=0;
	Step3=0;
}
//清除数据缓冲区
void ClearDataBuffer(uint8_t *Data)
{
   memset((void *)Data,sizeof(Data),0);
}





