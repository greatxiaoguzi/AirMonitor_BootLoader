#include "sys.h"
#include "ff.h"
#include "w25q64.h"
#include "malloc.h"
#include "ili93xx.h"
#include "updatefont.h"
#define FONTINFOADDR 	(4916+100)*1024 
//字库信息结构体. 
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;

const uint8_t *GBK24_PATH = "0:/SYSTEM/FONT/GBK24.FON";		//GBK24的存放位置
const uint8_t *GBK16_PATH = "0:/SYSTEM/FONT/GBK16.FON";		//GBK24的存放位置
const uint8_t *GBK12_PATH = "0:/SYSTEM/FONT/GBK12.FON";		//GBK24的存放位置
const uint8_t *UNIGBK_PATH = "0:/SYSTEM/FONT/UNIGBK.BIN";		//GBK24的存放位置
//检查字库
//返回值 1 字库丢失出错
//       0 字库成功
uint8_t Font_Init(void)
{
	W25Q64_Init();	//初始化SPI flash
	SPI_Flash_Read((uint8_t*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//读出结构体的数据
	if(ftinfo.fontok!=0XAA)
		return 1;
	else
		return 0;
}
//显示当前字体更新进度
//x,y:坐标
//size:字体大小
//fsize:整个文件大小
//pos:当前文件指针位置
uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos)
{
	float prog;
	uint8_t t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
	if(t!=prog)
	{
		LCD_ShowString(x+3*size/2,y,240,320,size,"%",POINT_COLOR,BACK_COLOR);		
		t=prog;
		if(t>100)t=100;
		LCD_ShowNum(x,y,t,3,size,BLACK,LIGHTBLUE);//显示数值
	}
	return 0;					    
} 
//更新某一种字体
//x,y坐标，size字体大小，fxpath路径，fx更新的内容
//返回值:0成功 1失败
//
uint8_t update_fontx(uint16_t x,uint16_t y,uint8_t size,uint8_t *ftpath,uint8_t fx)
{
	uint32_t flashaddr = 0;
	FIL *fftemp;
	uint8_t *tempbuf;		//为字库写申请内存空间
	uint8_t res;
	uint16_t bread;
	uint32_t offx = 0;
	uint8_t rval = 0;
	fftemp = (FIL*)mymalloc(sizeof(FIL));
	tempbuf = mymalloc(4096);	//分配4k个字节空间
	if(fftemp==NULL)
		rval = 1;
	res = f_open(fftemp,(const TCHAR*)ftpath,FA_READ);
	if(res)
		rval = 2;	//打开文件失败
	if(rval==0)
	{
		switch(fx)
		{
			case 0:
				ftinfo.ugbkaddr = FONTINFOADDR+sizeof(ftinfo);	//信息头之后，紧跟UNIGBK转换码表
				ftinfo.ugbksize = fftemp->fsize;					//UNIGBK大小
				flashaddr = ftinfo.ugbkaddr;
				break;
			case 1:
				ftinfo.f12addr = ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK之后，紧跟GBK12字库
				ftinfo.gbk12size = fftemp->fsize;					//GBK12字库大小
				flashaddr = ftinfo.f12addr;						//GBK12的起始地址
				break;
			case 2:
				ftinfo.f16addr = ftinfo.f12addr+ftinfo.gbk12size;	//GBK12之后，紧跟GBK16字库
				ftinfo.gbk16size = fftemp->fsize;					//GBK16字库大小
				flashaddr = ftinfo.f16addr;						//GBK16的起始地址
				break;
			case 3:
				ftinfo.f24addr = ftinfo.f16addr+ftinfo.gbk16size;	//GBK16之后，紧跟GBK24字库
				ftinfo.gkb24size = fftemp->fsize;					//GBK24字库大小
				flashaddr = ftinfo.f24addr;						//GBK24的起始地址
				break;				
		}
		while(res==FR_OK)
		{
			res = f_read(fftemp,tempbuf,4096,(UINT *)&bread);		//读取4K的数据到缓存
			if(res!=FR_OK)break;//执行错误
			SPI_Flash_Write(tempbuf,flashaddr+offx,4096);
			offx+=bread;
			if(bread!=4096)break;			
		}
		f_close(fftemp);	//关闭文件
	}
	myfree(fftemp);			//释放内存
	myfree(tempbuf);		//释放内存
	return res;	
}

//
//更新字库文件：UNIGBK,GBK12,GBK16,GBK24一起更新
//x,y字体要显示的位置
//
uint8_t update_font(uint16_t x,uint16_t y,uint8_t size)
{
	uint8_t *gbk24_path = (uint8_t*)GBK24_PATH;
	uint8_t *gbk16_path = (uint8_t*)GBK16_PATH;
	uint8_t *gbk12_path = (uint8_t*)GBK12_PATH;
	uint8_t *unigbk_path = (uint8_t*)UNIGBK_PATH;
	uint8_t Res = 0XFF;
	ftinfo.fontok = 0XFF;
	
	SPI_Flash_Write((uint8_t *)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//清除之前字库的标志，防止更新到一半重启，导致字库部分丢失
	SPI_Flash_Read((uint8_t*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));
	
	LCD_ShowString(x,y,400,320,size,"Updating UNIGBK.BIN......",BLACK,LIGHTBLUE);
	Res = update_fontx(x+20*size/2,y,size,unigbk_path,0);			//更新UNIGBK.BIN
	if(Res)  return 1;
    LCD_ShowString(x,y+30,400,320,size,"Updating GBK12.BIN......",BLACK,LIGHTBLUE);
	
	Res=update_fontx(x+20*size/2,y,size,gbk12_path,1);			//更新GBK12.FON
	if(Res)  return 2;
	LCD_ShowString(x,y+60,400,320,size,"Updating GBK16.BIN......",BLACK,LIGHTBLUE);
	
	Res=update_fontx(x+20*size/2,y,size,gbk16_path,2);			//更新GBK16.FON
	if(Res)	 return 3;
	LCD_ShowString(x,y+85,400,320,size,"Updating GBK24.BIN......",BLACK,LIGHTBLUE);
	
	Res=update_fontx(x+20*size/2,y,size,gbk24_path,3);			//更新GBK24.FON
	if(Res)  return 4;
	//至此全部更新完成
	ftinfo.fontok = 0XAA;
	SPI_Flash_Write((uint8_t *)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//重新写入字库信息
	LCD_ShowString(x,y+80,400,320,size,"Font Update Finish",BLACK,LIGHTBLUE);
	return 0;//更新成功返回
}














































