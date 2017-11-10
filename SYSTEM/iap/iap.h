#ifndef __IAP__H
#define __IAP__H
#include "sys.h"

typedef void (*iapfun)(void);  //定义函数指针用于程序跳转
	
#define FLASH_APP1_ADDR	  0x0800E000  	//第一个应用程序起始地址(存放在FLASH)
											//保留0X08000000~0X0800FFFF的空间为Bootloader使用（64KB）

void iap_load_app(uint32_t appxaddr);			//跳转到APP程序执行
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t applen);	//在指定地址开始,写入bin

uint8_t IAP_Update_Data(uint8_t *filename,uint8_t x0,uint8_t y0);
#endif






















