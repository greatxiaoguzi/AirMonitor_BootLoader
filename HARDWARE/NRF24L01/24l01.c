#include "include.h"
//////////////////////////////////////////////////////////////////////////////////	    
u8 User_data[6];
const u8 IP_MaT[5]={'0','L','O','N','G'};    //主机发送地址，自定义
const u8 IP_SLR0[5]={'0','L','O','N','G'};    //从机0接受地址
const u8 IP_SLR1[5]={'2','L','O','N','G'};     //从机1接受地址
const u8 IP_SLR2[5]={'3','L','O','N','G'};     //从机2接受地址
const u8 IP_SLR3[5]={'4','L','O','N','G'};     //从机3接受地址
const u8 IP_SLR4[5]={'5','L','O','N','G'};     //从机4接受地址
const u8 IP_SLR5[5]={'6','L','O','N','G'};     //从机5接受地址
//u8 Che_TAddr[5]={0xff,0xff,0xff,0xff,0xff};  //用于检查发送地址
//u8 Che_RAddr1[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址1
//u8 Che_RAddr2[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址2
//u8 Che_RAddr3[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址3
//u8 Che_RAddr4[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址4
//u8 Che_RAddr5[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址5
//u8 Che_RAddr6[5]={0xff,0xff,0xff,0xff,0xff}; //用于检查接受地址6
u8 Che_StaReg[2]={0xff,0x00};   //用于查看状态寄存器的值
u8 Chanl=0;                     //需要用到的通道值，全局变量标示
//u8  Rx_Succ=0;                //终端接收标志申明
//初始化24L01的IO口
void NRF24L01_Init(void)        //无线初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure; 
	
 	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	//引脚为CE和CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//SPI  IRQ
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //上拉输入
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB,GPIO_Pin_2|GPIO_Pin_8);
	SPI1_Init();    		//初始化SPI
	
	SPI_Cmd(SPI1,DISABLE);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//选择了串行时钟的稳态:时钟悬空低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第一个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	
	NRF24L01_CE=0; 	//使能24L01
	NRF24L01_CSN=1;	//SPI片选取消	 	
}
//检测24L01是否存在
//返回值:0，成功;1，失败	
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;
  NRF24L01_CSN=0;                 //使能SPI传输
  status =SPI1_ReadWriteByte(reg);//发送寄存器号
  SPI1_ReadWriteByte(value);      //写入寄存器的值
  NRF24L01_CSN=1;                 //禁止SPI传输	   
  return(status);       			//返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;
 	NRF24L01_CSN = 0;          //使能SPI传输
  SPI1_ReadWriteByte(reg);   //发送寄存器号
  reg_val=SPI1_ReadWriteByte(0XFF);//读取寄存器内容
  NRF24L01_CSN = 1;          //禁止SPI传输
  return(reg_val);           //返回状态值
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)//数组地址及其对应的长度
{
	u8 status,u8_ctr;	       
  NRF24L01_CSN = 0;           //使能SPI传输
  status=SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//读出数据
  NRF24L01_CSN=1;       //关闭SPI传输
  return status;        //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	NRF24L01_CSN = 0;          //使能SPI传输
  status = SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI1_ReadWriteByte(*pBuf++); //写入数据	 
  NRF24L01_CSN = 1;       //关闭SPI传输
  return status;          //返回读到的状态值
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
u8 NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
 	SPI1_SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	NRF24L01_CE=0;
  NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_USER_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CE=1;//启动发送	   
	while(NRF24L01_IRQ!=0);//等待发送完成
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		  NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
		  return MAX_TX; 
	}
	if(sta&TX_OK)//发送完成
	{
		  return TX_OK;
	}
	return 0xff;//其他原因发送失败
}
//启动NRF24L01接受一次数据
//txbuf:带接收的数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;
	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz(24L01的最大SPI时钟为10Mhz)
	sta=NRF24L01_Read_Reg(STATUS);          //读取状态寄存器的值
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_USER_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器
		return 0;
	}
	return 1;//没收到任何数据
}					    
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void NRF24L01_RX_Mode(u8 chanl)
{
	  //设置24L01+关闭，用于修改无线配置
	  NRF24L01_CE=0;
	  //基本接受应答使能，通信频率设定
	  NRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG,0x0f);//配置接收模式的基本参数;PWR_UP,EN_CRC,16BIT_CRC,
	  switch(chanl)       //选择具体通道
		{
			case 0:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x01);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x01);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			case 1:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x02);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x02);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			case 2:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x04);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x04);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			case 3:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x08);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x08);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			case 4:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x10);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x10);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			case 5:
			{
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x20);//应答设置，0x3f代表543210全都应答   11 1111
	        NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x20);//接收通道使能0x3f代表543210通道全都允许接收 11 1111
			}break;
			default: break;
		}
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RF_CH ,50);  //设置RF通信频率2.4G+设置值Mhz。范围0x00到0x7f
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启  
	  //设置从机的接收地
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P0,(u8*)IP_SLR0,5);//写RX0接收地址，从机地址
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P1,(u8*)IP_SLR1,5);//写RX1接收地址
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P2,(u8*)IP_SLR2,1);//写RX2接收地址
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P3,(u8*)IP_SLR3,1);//写RX3接收地址
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P4,(u8*)IP_SLR4,1);//写RX4接收地址
	  NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P5,(u8*)IP_SLR5,1);//写RX5接收地址
	  //设置有效的用户数据宽度为32字节
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P0,RX_USER_WIDTH);//选择通道0的用户有效数据宽度32字节 
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P1,RX_USER_WIDTH);//选择通道1的用户有效数据宽度32字节 
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P2,RX_USER_WIDTH);//选择通道2的用户有效数据宽度32字节 
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P3,RX_USER_WIDTH);//选择通道3的用户有效数据宽度32字节 
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P4,RX_USER_WIDTH);//选择通道4的用户有效数据宽度32字节 
	  NRF24L01_Write_Reg(NRF_WRITE_REG + RX_PW_P5,RX_USER_WIDTH);//选择通道5的用户有效数据宽度32字节 
		//CE为高,进入接收模式 
  	NRF24L01_CE = 1; 
}						 
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了
//CE为高大于10us,则启动发送.
void NRF24L01_TX_Mode(void)
{
    //无线模块待机模式，等待设置	
	  NRF24L01_CE=0;	    
  	//NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_MaT,TX_ADR_WIDTH);//写TX节点地址，主机 ，5个字节
  	//NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P0,(u8*)IP_SLR0,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK，5个字节	  

  	NRF24L01_Write_Reg(NRF_WRITE_REG + EN_AA,0x3f);     //使能通道543210的自动应答,全都应答    
  	NRF24L01_Write_Reg(NRF_WRITE_REG + EN_RXADDR,0x3f); //使能通道543210的接收地址,全都设置
    //设置自动重发
  	NRF24L01_Write_Reg(NRF_WRITE_REG + SETUP_RETR,0xff);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:15次
  	NRF24L01_Write_Reg(NRF_WRITE_REG + RF_CH,50);       //设置RF通道为40
  	NRF24L01_Write_Reg(NRF_WRITE_REG + RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF_WRITE_REG + CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	  NRF24L01_CE=1;//CE为高,10us后启动发送
}
void NRF_Set_Mode(u8 mode)    //NRF设置工作模式,通道设定
{
   NRF24L01_TX_Mode();	
	 NRF24L01_CE=0;
	 switch(mode)
	 {
		 case 1:
		 {
		    NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_SLR1,5);//写TX节点地址
	      NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P1,(u8*)IP_SLR1,5);//写RX1接收地址
		 }break;
		 case 2:
		 {
		    NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_SLR2,5);//写TX节点地址
	      NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P2,(u8*)IP_SLR2,5);//写RX2接收地址
		 }break;
		 case 3:
		 {
		    NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_SLR3,5);//写TX节点地址
	      NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P3,(u8*)IP_SLR3,1);//写RX3接收地址
		 }break;
		 case 4:
		 {
		    NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_SLR4,5);//写TX节点地址
	      NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P4,(u8*)IP_SLR4,1);//写RX4接收地址
		 }break;
		 case 5:
		 {
		    NRF24L01_Write_Buf(NRF_WRITE_REG + TX_ADDR,(u8*)IP_SLR5,5);//写TX节点地址
	      NRF24L01_Write_Buf(NRF_WRITE_REG + RX_ADDR_P5,(u8*)IP_SLR5,1);//写RX5接收地址
		 }break;
		 default : LED0=!LED0;break;
	 }
	  NRF24L01_CE=1;
}
void Chec_NRF(void)   //检查NRF24L01是否在位.	串口打印输出
{
	  u8 Succe=10;
    while(Succe--)	
	  {
				 if(NRF24L01_Check())
				 {
					   printf("NRF24L01 Error\n");
						 delay_ms(500);					 
				 }
				 else
				 {
					  printf("NRF24L01 OK\n");
						break;
				 }
	  }
}






