#include "include.h"

iapfun jump2app;  //定义用于跳转函数
uint16_t iapbuf[1024];  //定义1K的缓存用于iap写
//写APP函数
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节)
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t appsize)
{
	uint16_t t;
	uint16_t i=0;
	uint16_t temp;
	uint32_t fwaddr=appxaddr;//当前写入的地址
	uint8_t *dfu=appbuf;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(uint16_t)dfu[1]<<8;
		temp+=(uint16_t)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

//程序跳转
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}
//iap更新数据
uint8_t IAP_Update_Data(uint8_t *filename,uint8_t x0,uint8_t y0)
{
	uint32_t i=0;
	UINT br;
	uint8_t res;
	uint16_t ReadLen = 1024;
	FIL *F_iapdata;
	uint8_t *databuf;		
	databuf = (uint8_t *)mymalloc(1024);  //申请1K个字节的空间
	if(databuf==NULL)
	{
		return 0x01;
	}
	F_iapdata = (FIL *)mymalloc(sizeof(FIL));
	if(F_iapdata==NULL)
	{
		myfree(databuf);
		return 0x02;
	}
	res=f_open(F_iapdata,(const TCHAR*)filename,FA_READ);
	if(res==0)
	{
		while(1)
		{
			res = f_read(F_iapdata,databuf,ReadLen,(UINT *)&br);
			if(br!=1024) ReadLen = br;
			if(res||br==0)break;
			iap_write_appbin(FLASH_APP1_ADDR+i,databuf,ReadLen);
			i += 1024;
			LED0 = !LED0;
			LCD_ShowNum(x0-12,y0,100*i/F_iapdata->fsize,3,36,GREEN,BLACK);
			Show_Str(x0+68,y0+25,20,16,"%",16,0,GREEN,BLACK);
		}
		f_close(F_iapdata);
	}
	else
	{
		return 0x03;
	}
	myfree(databuf);
	myfree(F_iapdata);
	return 0;
}




























