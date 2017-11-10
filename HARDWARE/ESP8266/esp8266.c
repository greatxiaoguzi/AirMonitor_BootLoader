#ifndef _ESP8266_C
#define _ESP8266_C	
#include "include.h"	
//STMflash读写部分申明定义
uint8_t Flash_Buffer[50];      //flash缓冲区50个字节
#define SIZE sizeof(Flash_Buffer)	 			  	//数组长度

//乐为IP:121.40.22.15
//esp8266Wifi模块驱动程序
//配置网络通信协议，IP地址，端口号

//乐为服务器密钥
uint8_t UserKey[60];
Esp8266 Esp8266Config={{"",""},{"TCP"},{""},8888,0,1,0};           //结构体实体化
NET NetData;  				//网络接收到的数据
uint8_t ReConfigFlag = 0XFF;  //是否重新配置WIFI
//开机配置Esp8266模块，检测是否在位成功
//功能:开机先读取FLASH，得到值后开始配置连接，否则如果收到上位机发过来的数据，则开始连接
void Esp8266_Config(void)
{
	uint8_t i;
	uint8_t *p=0;
	uint8_t *p1=0;
    uint8_t buf[20];
    uint8_t ConfigStep = 1;    //配置步骤
	uint8_t ConfigWriteFlag = 0XF0;
	uint8_t ErrorType;
	uint8_t ConfigStat;
    uint8_t SuccesFlag = 0;  //连接成功标志
    uint8_t LinkTimes = 0;
	//LCD_Clear(BLACK);   //清屏
	LCD_Fill(0,30,320,240,BLACK);
	ConfigStat = AT24CXX_ReadOneByte(0);
	if(KeyScan()||ConfigStat!=0XF7)  //如果要重新配置的话执行
	{
		switch(ConfigStat)
		{
			case 0xf6:ConfigStep =1;Show_Str(60,120,200,16,"请配置您的WIFI->",16,0,GREEN,BLACK);break;
			case 0xf5:ConfigStep =2;Show_Str(60,140,200,16,"请配置您的USERKEY->",16,0,GREEN,BLACK);break;
			case 0xf3:ConfigStep =3;Show_Str(60,160,200,16,"请配置您的ServerIP->",16,0,GREEN,BLACK);break;
		}		
		ConfigServerMode();  //设置为服务器模式
		Show_Str(60,120,200,16,"请配置您的WIFI->",16,0,GREEN,BLACK);  //
		p = UartRecBuf;
		while(1)
		{
			if(WIFIRecDataFlag)
			{
				WIFIRecDataFlag = 0;
				delay_ms(100);
				switch(ConfigStep)
				{
					case 1:
					{
						if((strstr(p,"%SSID")!=NULL)&&(strstr(p,"&#")!=NULL))
						{
							Show_Str(200,120,200,16,"写入中...  ",16,0,GREEN,BLACK);
							p1 = strstr((const char*)p,"%SSID");
							AT24CXX_Write(1,p1,150);  		//保存wifi的SSID和密码
							ConfigWriteFlag |= (1<<0);
							AT24CXX_WriteOneByte(0,ConfigWriteFlag);
							Show_Str(200,120,200,16,"WIFI已配置",16,0,GREEN,BLACK);
							Show_Str(60,140,200,16,"请配置您的USERKEY->",16,0,GREEN,BLACK);
							ConfigStep ++;
						}
						else
						{
							Show_Str(60,120,200,16,"WIFI未配置",16,0,GREEN,BLACK);							
						}
						ClearBuf();
					}break;
					case 2:
					{
						if((strstr(p,"%USERKEY")!=NULL)&&(strstr(p,"&#")!=NULL))
						{
							Show_Str(210,140,200,16,"写入中...  ",16,0,GREEN,BLACK);
							p1 = strstr((const char*)p,"%USERKEY");
							AT24CXX_Write(151,p1,50);
							ConfigWriteFlag |= (1<<1);
							AT24CXX_WriteOneByte(0,ConfigWriteFlag);
							Show_Str(210,140,200,16,"USERKEY已配置",16,0,GREEN,BLACK);
							Show_Str(60,160,200,16,"请配置您的ServerIP->",16,0,GREEN,BLACK);
							ConfigStep ++;
						}
						else
						{
							Show_Str(210,140,200,16,"USERKEY未配置",16,0,GREEN,BLACK);						
						} 
						ClearBuf();												
					}break;
					case 3:
					{
						if((strstr(p,"%SVN")!=NULL)&&(strstr(p,"&#") != NULL))
						{
							Show_Str(210,160,200,16,"写入中...  ",16,0,GREEN,BLACK);
							p1 = strstr((const char*)p,"%SVN");
							AT24CXX_Write(202,p1,30);
							ConfigWriteFlag |= (1<<2);
							AT24CXX_WriteOneByte(0,ConfigWriteFlag);
							Show_Str(210,160,200,16,"SVN已配置",16,0,GREEN,BLACK);
							Show_Str(60,180,200,16,"配置完成",16,0,GREEN,BLACK);
							ConfigStep ++;
						}
						else
						{
							Show_Str(210,160,200,16,"SVN未配置",16,0,GREEN,BLACK);						
						}
						ClearBuf();	
					}break;
					default:break;
				}
				if(ConfigStep>3)
					break;
				LED0 = !LED0;
			}
		}
	}
	//LCD_Clear(BLACK);
	LCD_Fill(0,20,320,240,BLACK);
    if(!Esp8266_ReStart())  //发送重新启动信号
    {
		Show_Str(60,40,200,16,"WIFI重启成功.",16,0,GREEN,BLACK);
        Esp8266Config.WifiStartFlag = 1;
    }
	else
	{
		Show_Str(60,40,200,16,"WIFI重启失败.",16,0,RED,BLACK);
        Esp8266Config.WifiStartFlag = 0;
		return;
	}
	delay_ms(500);
    if(!Esp8266_SetMode(0))   //设置模式
		Show_Str(60,60,200,16,"TCP模式设置成功.",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,60,200,16,"TCP模式设置失败.",16,0,RED,BLACK);
        return;
	}
	delay_ms(500);
	if(!Esp8266_ReStart())
		Show_Str(60,80,200,16,"WIFI重启成功.",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,80,200,16,"WIFI重启失败.",16,0,RED,BLACK);
        return;
	}
	delay_ms(500);
	if(!Esp8266_CheckNET())      //检查附近网络
	{
		Show_Str(60,100,200,16,"检查网络成功.",16,0,GREEN,BLACK);
	}
	else
	{
		Show_Str(60,100,200,16,"检查网络失败.",16,0,RED,BLACK);
        return;
	}
    //开始配置WIFI模块
	Show_Str(60,120,200,16,"WIFI设置中...",16,0,GREEN,BLACK);
	while(LinkTimes<3) //最多连接10次
	{
		 LinkTimes ++;
		 if(!WifiNETSet()&&SuccesFlag==0)      //进入设置，如果设置成功的话，执行相应部分
		 {
             SuccesFlag =1;
			 Show_Str(60,140,200,16,"WIFI设置成功",16,0,GREEN,BLACK);
		 }
		 else
		 {
			 Show_Str(60,140,200,16,"重新设置中.",16,0,GREEN,BLACK);
		 }
		 if(SuccesFlag==1)             //连接成功的话
		 {
			   	Show_Str(60,160,200,16,"WIFI连接中...",16,0,GREEN,BLACK);
				if(!Esp8266_ConnectWire())   //开始连接WIFI，连接成功的话
				{
					Esp8266Config.WifiLinkSuccFlag = 1;  //标志WIFI连接成功
					Show_Str(60,180,200,16,"WIFI连接成功",16,0,GREEN,BLACK);
					for(i=0;i<2;i++)
					{
						 LED0 = !LED0;
						 delay_ms(500);
					}
					break;
				}
			 	else//否则连接未成功
				{
					SuccesFlag=0;
					Esp8266Config.WifiLinkSuccFlag = 0;
					Show_Str(60,180,200,16,"WIFI连接失败",16,0,RED,BLACK);
				}
		 }
	}
	if(LinkTimes>=100)
		Show_Str(60,180,200,16,"WIFI连接失败",16,0,RED,BLACK);
////////////////////////////////////////////////////////////////////
	Esp8266_BackIPAdd(); //返回模块的IP地址
	if(!Esp8266_OpenMoreConect())
		Show_Str(60,200,200,16,"多连接打开成功",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,200,200,16,"多连接打开失败",16,0,RED,BLACK);
		return;
	}
	delay_ms(500);
	//LCD_Clear(BLACK);   //清屏
	LCD_Fill(0,27,320,240,BLACK);
	Show_Str(60,30,200,16,"服务器连接中...",16,0,GREEN,BLACK);
	ErrorType = ReadServerPar();
	if(!ErrorType)   //从EEPROM读取服务器传参数
	{
		if(!Esp8266_ConectServer())
		{
			Esp8266Config.ServerLinkStat = 1;
			Show_Str(60,30,200,16,"服务器连接成功",16,0,GREEN,BLACK);
		}
		else
		{
			Esp8266Config.ServerLinkStat = 0; 
			Show_Str(60,30,200,16,"服务器连接失败",16,0,RED,BLACK);
			delay_ms(500);
			return;
		}
	}
	else
	{
		//Show_Str(60,30,200,16,"参数读取错误  ",16,0,RED,BLACK);
		LCD_ShowNum(60,30,ErrorType,2,16,RED,BLACK);
	}
	sprintf(buf,"服务器IP:%s",Esp8266Config.ServerIP);
	Show_Str(60,50,200,16,buf,16,0,GREEN,BLACK);
	sprintf(buf,"服务器PORT:%d",8080);
	Show_Str(60,70,200,16,buf,16,0,GREEN,BLACK);
	ClearBuf();
	delay_ms(2000);
}
//尝试重新连接服务器
void TryReConectServer(void)
{
	if(CurrentuiID==1||CurrentuiID==2)  //如果界面在这两者之间的话就显示
		Show_Str(5,3,100,12,"服务器连接中",12,0,GREEN,BLACK);
	if(!Esp8266_ConectServer())
	{
		if(CurrentuiID==1||CurrentuiID==2)
			Show_Str(5,3,100,12,"服务器已连接",12,0,GREEN,BLACK);
		Esp8266Config.ServerLinkStat = 1;
	}
	else
	{
		if(CurrentuiID==1||CurrentuiID==2)
			Show_Str(5,3,100,12,"服务器未连接",12,0,RED,BLACK);
		Esp8266Config.ServerLinkStat = 0;
	}
}
//尝试重新连接WIFI
void TryReConectWifi(void)
{
	if(!Esp8266_ConnectWire())  //连接成功的话
	{
		Esp8266Config.WifiLinkSuccFlag = 1;  //标志连接成功
	}
	else
	{
		Esp8266Config.WifiLinkSuccFlag = 0;
	}
}
//获得ESP8266的当前连接状态
uint8_t GetEsp8266CurrStat(void)
{
	ClearBuf();
	printf("AT+CIPSTATUS\r\n");
	if(!Hand("OK",WaitTime))             //如果成功
	{
		return 0;
	}
   else
   {
	   return 1;
   }
}
//模块重新启动 AT指令   AT+RST
//返回值：0成功  1失败
uint8_t Esp8266_ReStart(void)
{
	 ClearBuf();
	 printf("AT+RST\r\n");        //重新启动
	 if(!Hand("OK",WaitTime))             //如果成功
		 return 0;	   
   else
	   return 1;
}
//设置模块的工作方式
//ST模式
//AP模式
//ST+AP模式
//返回值： 0成功  1失败
uint8_t  Esp8266_SetMode(uint8_t Mode)
{
    ClearBuf();		
	if(Mode)
		printf("AT+CWMODE=2\r\n");    //设置AP模式
	else
		printf("AT+CWMODE=3\r\n");
    if(!Hand("OK",WaitTime))  //如果成功
    {
        delay_ms(500);
        ClearBuf();
        printf("AT+RST\r\n");         //重启生效
        if(!Hand("OK",WaitTime))  //握手成功的话
        {
            return 0;
        }
        else
          return 1;  //失败
    }	
    else
    return 1;			
}
//查看附近的无线网络
//返回值：0成功   1失败
uint8_t Esp8266_CheckNET(void)
{
    ClearBuf();
    printf("AT+CWLAP\r\n");       //查看可用的网络。返回当前连接的网络
    if(!Hand("OK",WaitTime))
        return 0;
    else
        return 1; 
}
//返回模块的IP地址说明已经连接路由器 返回OK
void Esp8266_BackIPAdd(void)
{
    ClearBuf();
    printf("AT+CIFSR\r\n");      //发送返回模块的IP地址
    Hand("OK",WaitTime);         //等待响应成功
}
//"%SSID=" + splitstring1 + "&%" + splitstring2 + "&#";
//发送要连接的无线网络
//返回值： 0成功 1失败
uint8_t Esp8266_ConnectWire(void)
{
    ClearBuf();
    //sprintf();
    printf("AT+CWJAP=\"%s\",\"%s\"\r\n",Esp8266Config.WireNet[0],Esp8266Config.WireNet[1]);  //发送无线网络ISSD和无线密码 
    if(!Hand("OK",1000))
        return 0;
    else
        return 1; 
}
//模块开启多连接
//返回值： 0成功 1失败
uint8_t Esp8266_OpenMoreConect(void)
{
    ClearBuf();
    printf("AT+CIPMUX=1\r\n");
    if(!Hand("OK",WaitTime))
        return 0;
    else
        return 1;
}
//开始要连接的服务器的IP地址和端口号
//入口参数：Proto：通信协议  IP:服务器IP地址  Port 服务器端口号
//返回值; 0成功 1失败
uint8_t Esp8266_ConectServer(void)
{
    ClearBuf();
    printf("AT+CIPSTART=%d,\"%s\",\"%s\",%d\r\n",Esp8266Config.SlrID,Esp8266Config.ComunitPro,Esp8266Config.ServerIP,Esp8266Config.ServerPort); //发送指令
    if(!Hand("OK",1000))
        return 0;
    else
        return 1;
}
//断开连接
//返回值： 0成功 1失败
uint8_t Esp8266_DisConectServer(void)
{
    ClearBuf();
    printf("AT+CIPCLOSE=%d\r\n",Esp8266Config.SlrID);
    if(!Hand("OK",WaitTime))
        return 0;
    else
        return 1; 
}
//
//设置模块为Server模式
//
uint8_t Esp8266_SetServerMode(uint16_t Port)
{
	ClearBuf();
	printf("AT+CIPSERVER=1,8080\r\n");
	if(!Hand("OK",WaitTime))
		return 0;
	else
		return 1; 
}
//192.168.4.1
uint8_t GetIPAddr(void)
{
	ClearBuf();
	printf("AT+CIFSR\r\n");
	if(!Hand("OK",WaitTime))
		return 0;
	else
		return 1;
}
//监听串口数据
//+IPD,0,10:1234567890
//UartRecBuf[50]
//返回值：模块的ID值
uint8_t ListenRec(void)
{
    if(WIFIRecDataFlag)  //接收到了服务器数据
    {
        WIFIRecDataFlag=0;
        if(strstr(UartRecBuf,"+IPD")!=NULL)
        {
             return 0;
        }
    }
    else
        return 1; //返回模块的ID值
}
//启动发送信息
//输入参数：*Data要发送的数据区，Len 要发送的数据的字节长度
//返回值; 0成功 1失败
uint8_t Esp8266_CIPSendData(char *Data,uint16_t Len)
{
    uint8_t key;
    //uint8_t MAClen;
    //MAClen = strlen(MAC);
    ClearBuf();
    printf("AT+CIPSEND=%d,%d\r\n",Esp8266Config.SlrID,Len);
    if(!Hand(">",WaitTime))  //握手成功
    {
        ClearBuf();
        printf("%s\r\n",Data);	 
        key=Hand("SEND OK",WaitTime);
        if(!key)  //确认数据发送成功	
        {
            ClearBuf();  //清除数据缓存区
            return 0;
        }
        else
            return 1;
    }
    else
    return 1;
}
//缓冲区	清零函数
//输入参数：缓冲区地址
void ClearBuffer(uint8_t *data)
{
    uint8_t i,len;
    len=strlen(data);
    for(i=0;i<len;i++)
    {
        data[i]=0;
    }
}
//尝试握手连接
//输入参数：*a要比较的文本值    Time 等待超时时间
//返回值：0成功  1失败
uint8_t Hand(char *a,uint16_t Time)
{
    while(Time--)
    {
        if(strstr(UartRecBuf,a)!=NULL)
        {
            return 0;
            break;
        }
        delay_ms(10);
    }
    return 1;
}
//读取服务器参数
uint8_t ReadServerPar(void)
{
	uint8_t i;
	uint8_t ConfigStat;		//配置状态
	//uint8_t buf[50];
	uint8_t *Start;   //字符串开始和结束的指针位置
	uint8_t StrLen;
	ConfigStat = AT24CXX_ReadOneByte(0);
	if((ConfigStat&0x02)==0x02)
	{
		ClearBuf();
		delay_ms(100);
		AT24CXX_Read(151,UartRecBuf,50);
		if(strstr(UartRecBuf,"%USER")==NULL)  //如果是没有发现指定字符串则返回
		{ 
			return 0X01;
		}
		else
		{
			StrLen = (strstr(UartRecBuf,"&#")) - (strstr(UartRecBuf,"%USER")+9);
			Start = strstr(UartRecBuf,"%USER") + 9;
			memcpy(UserKey,Start,StrLen);
		}
	}
	if((ConfigStat&0x04)==0x04)
	{
		ClearBuf();		
		delay_ms(100);
		AT24CXX_Read(202,UartRecBuf,30);
		if(strstr(UartRecBuf,"%SVN")==NULL)
		{
			ClearBuf();
			return 0X02;
		}
		else
		{
			StrLen = (strstr(UartRecBuf,"&%")) - (strstr(UartRecBuf,"%SVN")+5);
			Start = strstr(UartRecBuf,"%SVN") +5;
			for(i=0;i<StrLen;i++)
			{
				Esp8266Config.ServerIP[i] = *Start++;
			}
			StrLen = (strstr(UartRecBuf,"&#")) - (strstr(UartRecBuf,"&%")+2);
			Start = strstr(UartRecBuf,"&%") +2;
			Esp8266Config.ServerPort = (*Start-0x30)*1000+(*(Start+1)-0x30)*100+(*(Start+2)-0x30)*10+(*(Start+3)-0x30);
		}
	}
	return 0;
}
//Wifi连接设置处理
//数据格式：%SSID=guchenglong&%gcl19910621&#
//      7          18
//%SSID=TP-LINK_601&%qq18789463526qq&#
uint8_t WifiNETSet(void)
{
    uint8_t i;
	uint8_t ConfigStat;
    uint8_t *WifiSSID=0;                 //Wifi名称和密码指针分离
    uint8_t *Wifisecret=0;
    uint8_t Dist1;
    uint8_t Dist2;
	ConfigStat = AT24CXX_ReadOneByte(0);  //读取是否配置标志
	if((ConfigStat&0X01)==0X01)   				  //已配置
	{
		ReConfigFlag = 0;
		ClearBuf(); //清空缓冲
		delay_ms(100);
		AT24CXX_Read(1,UartRecBuf,150);  		//读出wifi的SSID和密码
	}
	else
		return 1; //错误状态
    if(strstr(UartRecBuf,"%SSID")==NULL)  //如果是没有发现指定字符串则返回
    {
         ClearBuf();            //清除缓冲区
         return 1;
    }
    WifiSSID = strstr(UartRecBuf,"%SSID")+6;       //指到SSID处
    Wifisecret = strstr(UartRecBuf,"&%")+2;  //指到密码地址处
    Dist1 =  strstr(UartRecBuf,"&%") - (strstr(UartRecBuf,"%SSID")+6);    //这儿搞了好长时间终于好了，SSID部分的值
    Dist2 = strstr(UartRecBuf,"&#") - (strstr(UartRecBuf,"&%")+2);      //密码到结束符
    if(*(WifiSSID-6)=='%')
    {
        for(i=0;i<Dist1;i++)
        {
            Esp8266Config.WireNet[0][i] = *WifiSSID; //保存SSID
            WifiSSID++;		
        }
        for(i=0;i<Dist2;i++)
        {
            Esp8266Config.WireNet[1][i] = *Wifisecret;  //保存密码
            Wifisecret++;
        }
        return 0;
    }
    else
    {
        ClearBuf();                   					//若么有接受到标志字符，则清空缓冲区
        return 1;
    }
}
//配置模块为服务器模式
void ConfigServerMode(void)
{	
	uint8_t *pIP =0;
	LCD_Clear(BLACK);//清屏
	if(!Esp8266_ReStart())  //发送重新启动信号
    {
		Show_Str(60,0,200,16,"WIFI重启成功",16,0,GREEN,BLACK);
	}
	else
	{
		Show_Str(60,0,200,16,"WIFI重启失败",16,0,RED,BLACK);
		return;
	}
	delay_ms(500);
	if(!Esp8266_SetMode(1))   		//设置为AP模式
		Show_Str(60,20,200,16,"AP模式设置成功",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,20,200,16,"AP模式设置失败",16,0,RED,BLACK);
		return;
	}
	delay_ms(500);
	if(!Esp8266_OpenMoreConect())	//打开多连接
		Show_Str(60,40,200,16,"多连接打开成功",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,40,200,16,"多连接打开失败",16,0,RED,BLACK);
		return;
	}
	delay_ms(500);
	if(!Esp8266_SetServerMode(8080))  //设置服务器模式	
		Show_Str(60,60,200,16,"本地服务器开启成功",16,0,GREEN,BLACK);
	else
	{
		Show_Str(60,60,200,16,"本地服务器开启失败.",16,0,RED,BLACK);
		return;
	}	
	delay_ms(500);
	GetIPAddr();
	delay_ms(500);
	pIP = strstr((char*)UartRecBuf,"+CIFSR:APIP") +12;
	Show_Str(110,80,200,16,pIP,16,0,GREEN,BLACK);
	Show_Str(60,80,100,16,"本地IP:",16,0,GREEN,BLACK);
	Show_Str(60,100,200,16,"端口:8080.",16,0,GREEN,BLACK);
	ClearBuf();
}
//网络数据处理
//输入参数：结构体  开关量  PWM值
//串口数据：+IPD,0,10:服务端数据
//信息数据格式: <数据内容格式>  分隔符  &         %SSID=" + splitstring1 + "&%" + splitstring2 + "&#
void NETDataDeal(void)
{
    uint8_t i;
    uint8_t *DataP;
    uint16_t num=0;
    if(strstr(UartRecBuf,"S1ON")!=NULL)
    {		
		LCD_ShowString(128,190,200,16,16,"S1 ON",GREEN,BLACK);
    }
    if(strstr(UartRecBuf,"S1OFF")!=NULL)
    {
		LCD_ShowString(128,190,200,16,16,"S1 OFF",GREEN,BLACK);
    }
    if(strstr(UartRecBuf,"S2ON")!=NULL)
    {
		LCD_ShowString(128,190,200,16,16,"S2 ON",GREEN,BLACK);
    }
    if(strstr(UartRecBuf,"S2OFF")!=NULL)
    {
		LCD_ShowString(128,190,200,16,16,"S2 OFF",GREEN,BLACK);
    }  
    if(strstr(UartRecBuf,"Val1=")!=NULL)
    {
        DataP = strstr(UartRecBuf,"Val1=") +5;    //指针移到数据处
        for(i=0;i<strlen(DataP)-6;i++)
        {
            if(*DataP!='&')
            {
                num=num*10+*DataP-'0';
                DataP++;
            }
            else
                break;
        }			 
//        Lcd12864PutNum(4,3,num);
        num=0;
    } 
    if(strstr(UartRecBuf,"Val2=")!=NULL)
    {
        DataP = strstr(UartRecBuf,"Val2=") +5;    //指针移到数据处
        for(i=0;i<strlen(DataP)-6;i++)
        {
            if(*DataP!='&')
            {
                num=num*10+*DataP-'0';
                DataP++;
            }
        else
            break;
        }
//        Lcd12864PutNum(6,3,num);
        num=0;
    } 
    Esp8266_CIPSendData("Received Success",strlen("Received Success"));	 
    ClearBuf();
}
#endif

