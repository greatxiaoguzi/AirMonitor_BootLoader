#ifndef GSM__C
#define GSM__C
#include"include.h"	
//GSM模块初始化配置
//握手，设置短信格式，提醒
u8 MsgFlag=0;
extern  GUI_CONST_STORAGE GUI_FONT GUI_FontHZ12;
extern  GUI_CONST_STORAGE GUI_FONT GUI_FontHZ16;
extern  GUI_CONST_STORAGE GUI_FONT GUI_FontHZ24;
void GSMInit(void)
{
	  GUI_SetFont(&GUI_FontHZ16);	
	  GUI_DispStringAt("GSM握手连接中...", 70,60);
	  printf("AT\r\n");
	  Hand("OK",255);
	  if(!Hand("OK",255))		
		{
		  GUI_DispStringAt("GSM初始化失败!", 70,80);
			beep(1,2);
		}
		else
		{
				ClearBuf();	
				GUI_DispStringAt("GSM初始化成功",70,80);
				
				printf("AT+CNMI=2,1\r\n");                     //设置短信到来时提醒
			  Hand("OK",200);
				GUI_DispStringAt("GSM信息唤醒成功", 70,100);
				ClearBuf();
				delay_ms(2000);
				
				printf("AT+CMGF=1\r\n");                     //设置短信文本模式
			  Hand("OK",200);
				GUI_DispStringAt("GSM短信文本设置成功",70,120);
				ClearBuf();
				delay_ms(2000);
				
				printf("AT+CMGD=1\r\n");                     //删除第一条短信
			  Hand("OK",200);
				GUI_DispStringAt("短信删除设置成功", 70,140);
				ClearBuf();
				delay_ms(2000);	
	  }
}
//读取短信
void 	RecMesageDeal(void)
{
    if(strstr(Uart2InfBuf,"+CMTI")!=NULL)    //是否有短信来到
		{
			  printf("AT+CMGR=1\r\n");               // 发送读取指令
			  delay_ms(1000);		
        Hand("OK",200);			
				if(strstr(Uart2InfBuf,"SOpenJDQ1")!=NULL)
				   JDQ1=0;
				else if(strstr((const char*)Uart2InfBuf,"SClosJDQ1")!=NULL)
				   JDQ1=1;
				else if(strstr((const char*)Uart2InfBuf,"SOpenJDQ2")!=NULL)
				   JDQ2=0;
				else if(strstr((const char*)Uart2InfBuf,"SClosJDQ2")!=NULL)
				   JDQ2=1;
				else if(strstr((const char*)Uart2InfBuf,"SOpenJDQ3")!=NULL)
				   JDQ3=0;
				else if(strstr((const char*)Uart2InfBuf,"SClosJDQ3")!=NULL)
				   JDQ3=1;
				else if(strstr((const char*)Uart2InfBuf,"SOpenJDQ4")!=NULL)
				   JDQ4=0;
				else if(strstr((const char*)Uart2InfBuf,"SClosJDQ4")!=NULL)
				   JDQ4=1;
				else if(strstr((const char*)Uart2InfBuf,"SGetMsg")!=NULL)
				   MsgFlag=1; 
			 ClearBuf();                                                    
			 printf("AT+CMGD=1\r\n");                    // 发送读取指令
			 delay_ms(2000);
				Hand("OK",200);
			 ClearBuf();
		}
}
void SendMessage(void)
{
	  char D1[3],D2[3],D3[3],D4[3];	  
	  D1[0]=User_data[0]/100+0X30;
	  D1[1]=User_data[0]/10%10+0X30;
	  D1[2]=User_data[0]%10+0X30;
	  D2[0]=User_data[1]/100+0X30;
	  D2[1]=User_data[1]/10%10+0X30;
	  D2[2]=User_data[1]%10+0X30;
	  D3[0]=User_data[2]/100+0X30;
	  D3[1]=User_data[2]/10%10+0X30;
	  D3[2]=User_data[2]%10+0X30;
    D4[0]=User_data[3]/100+0X30;
	  D4[1]=User_data[3]/10%10+0X30;
	  D4[2]=User_data[3]%10+0X30;
	  ClearBuf();                           //清除串口缓存
    printf("AT+CMGS=\"%s\"\r\n",PhoneNumber); //要发送的电话号码 
	  delay_ms(1000); 
	  Hand(">",200);
	  printf("Envrmnt Status\r\n");
	  printf("Temp1:%s'C\r\n",D1);
	  printf("Temp2:%s'C\r\n",D2);
	  printf("Fire:%s\r\n",D3);
	  printf("Damp:%s%%\r\n",D4);
		printf("JDQ Status\r\n");
		if((GPIOC->ODR&0X01)==0x00)
			printf("JDQ1 Opened!\r\n");
		else
			printf("JDQ1 Close!\r\n");
		if((GPIOC->ODR&0X02)==0x00)
			printf("JDQ2 Opened!\r\n");
		else
			printf("JDQ2 Close!\r\n");
	  if((GPIOC->ODR&0X04)==0x00)
			printf("JDQ3 Opened!\r\n");
		else
			printf("JDQ3 Close!\r\n");
		if((GPIOC->ODR&0X08)==0x00)
			printf("JDQ4 Opened!\r\n");
		else
			printf("JDQ4 Close!\r\n");
		if((GPIOC->ODR&0X10)==0x00)
			printf("JDQ5 Opened!\r\n");
		else
			printf("JDQ5 Close!\r\n");
	  UsartSendDex(3,0X1A);
	  Hand("OK",200);
	  ClearBuf();     	 
}
void CallPhone(void)
{
    ClearBuf(); 
	  printf("ATD%s;\r\n",PhoneNumber);
	  delay_ms(6000);
	  printf("ATH\r\n");
}
//GPRS通信功能连接测试，需要发送以下AT指令
//IP和端口号地址
//  AT+CGCLASS="B"
//  AT+CGDCONT=1,"IP","CMNET"
//  AT+CGATT=1
//  AT+CIPCSGP=1,"CMNET"
void GPRSBuiltNet(u8 *IP,u8 *Port)
{
    printf("AT+CGCLASS=\"B\"\r\n");              //设置移动后台类别
	  Hand("OK",200);
	  printf("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n"); //采用互联网协议（ IP），接入点为"CMNET"
	  Hand("OK",200);
	  printf("AT+CGATT=1\r\n");                   //用于设置附着和分离 GPRS 业务。发送： AT+CGATT=1，附着 GPRS 业务。
	  Hand("OK",200);
	  printf("AT+CIPCSGP=1,\"CMNET\"\r\n");       //用于设置 CSD 或 GPRS 链接模式。发送： AT+CIPCSGP=1, "CMNET"，设置为 GPRS 连接，接入点为”CMNET”
	  Hand("OK",200);
	  delay_ms(500);
	  printf("AT+CLPORT=\"TCP\",\"2000\"\r\n");   //用于设置本地端口号。发送： AT+CLPORT="TCP","8888"，即设置 TCP连接本地端口号为 8888
	  Hand("OK",200);
	  //printf("%s%s\r\n",IP,Port);
	  printf("AT+CIPSTART=\"TCP\",\"113.111.214.69\",\"8086\"\r\n");//用于建立 TCP 连接或注册 UDP 端口号。发送： AT+CIPSTART="TCP","113.111.214.69","8086"，模块将建立一个 TCP 连接，连接目标地址为： 113.111.214.69，连接端口为 8086，连接成功会返回： CONNECT OK
	  Hand("CONNECT OK",200);
}
void GPRSSendMSg(char *p)
{
	  u8 key;
	  static u8 num=5;              //如果第一次发送不成功则进行第二次发送，指导发送完成
	  while(num)
		{
				printf("AT+CIPSEND\r\n");
				key=Hand(">",200);        //接收到消息则进行处理
				if(key)                   //有回信则说明模块可以发送消息
				{
						//输入要发送的内容或者信息
						printf("%s",p);
						UsartSendDex(3,0X1A);    //输入必要的十六进制数
						Hand("OK",200);		
						break;			
				}	  	  
				else
				{
						num--;
				}
	   }
		if(num==0)
			num=5;
}
void AdminNowStat(void)
{
    printf("AT+CIPSTATUS\r\n");//用于查询当前连接状态
	  Hand("OK",200);
	  printf("AT+CIPCLOSE\r\n");//用于关闭 TCP/UDP 连接
	  Hand("OK",200);
	  printf("AT+CIPSHUT\r\n");//用于关闭移动场景
	  Hand("OK",200);
	
}
//握手信息
u8 Hand(char *a,u8 Time)
{
	while(Time--)
	{
    if(strstr(Uart2InfBuf,a)!=NULL)
		{
			  return 1;
		    break;
		}
		delay_ms(10);
	}
	return 0;
}
#endif





