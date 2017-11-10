#include "sys.h"
#include "sd.h"
#include  "ff.h"
//#include "ucos_ii.h"
#include "Fat_API.h"
#include "malloc.h"
//#include "include.h"
FATFS *fs[2];  //为逻辑磁盘申请工作区

//文件类型列表
const uint8_t *FILE_TYPE_TBL[4][13]=
{
{"BIN"},			//BIN文件
{"INI"},			//LRC文件
{"TXT","C","H"},	//文本文件
{"BMP","JPG","JPEG","GIF"},//图片文件
};

//为文件系统申请内存
uint8_t exfuns_init(void)
{
	fs[0] =  (FATFS*)mymalloc(sizeof(FATFS)); //为sd卡工作区申请内存	
	fs[1] =  (FATFS*)mymalloc(sizeof(FATFS));//为FLASH工作区申请内存	
	if(fs[0]&&fs[1])
		return 0;
	else
		return 1;
}
//将小写字母转为大写字母,如果是数字,则保持不变.
uint8_t char_upper(uint8_t c)
{
	if(c<'A')return c;//数字,保持不变.
	if(c>='a')return c-0x20;//变为大写.
	else return c;//大写,保持不变
}
//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
uint8_t f_typetell(uint8_t *fname)
{
	uint8_t tbuf[5];
	uint8_t *attr='\0';//后缀名
	uint8_t i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')
			break;//偏移到了最后了.
		fname++;
	}
	if(i==250)
		return 0XFF;//错误的字符串.
 	for(i=0;i<5;i++)//得到后缀名
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)
		tbuf[i]=char_upper(tbuf[i]);//全部变为大写 
	for(i=0;i<4;i++)
	{
		for(j=0;j<13;j++)
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//此组已经没有可对比的成员了.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//找到了
			{
				return (i<<4)|j;   //0x00 0x10 0x20 0x30 0x40 0x50 大类文件
			}
		}
	}
	return 0XFF;//没找到		 			   
}	 
//得到磁盘剩余容量
//drv:磁盘编号("0:"/"1:")
//total:总容量	 （单位KB）
//free:剩余容量	 （单位KB）
//返回值:0,正常.其他,错误代码
uint8_t exf_getfree(uint8_t *drv,uint32_t *total,uint32_t *free)
{
	FATFS *fs1;
	uint8_t res;
    uint32_t fre_clust=0, fre_sect=0, tot_sect=0;
    //得到磁盘信息及空闲簇数量
    res =(uint32_t)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//得到总扇区数
	    fre_sect=fre_clust*fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512				  				//扇区大小不是512字节,则转换为512字节
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		*total=tot_sect>>1;	//单位为KB
		*free=fre_sect>>1;	//单位为KB 
 	}
	return res;
}	
//寻找文件路径过程
void FilePath_process(char * source,char *addstring)
{
	uint8_t count = 0;
	while(*source != '\0')
	{
		source++;
		count++;
	}
	if(*addstring == '.' && *(addstring+1) == '.')
	{
		if(*(source-1) == ':' && *(source-2) == '0' && count == 2) return;
		while(*source != '/') 
		{
			*source = '\0';
			source--;
		}
		*source = '\0';
		source--;
	} 
	else
	{
		*source = '/';
		source++;
		while(*addstring != '\0') 
		{
			*source = *addstring;
			source++;
			addstring++;
		}
  }
}
void ClearPath(char *path)
{
	path += 2;
	while(*path)
	{
		*path = '\0';
		path++;
	}
}

//void FatFs_Open(char *filename)
//{
//	OS_CPU_SR  cpu_sr;
//// 	UINT ss;
//	uint8_t res = 0;
//// 	FIL infile;
//	DIR root_file;
//	FILINFO file_name;
//// 	uint32_t file_size = 0;
//	#if _USE_LFN
//	char lfn[30];
////	file_name.lfname = (const TCHAR*)lfn;
//	file_name.lfsize = 30;
//	#endif
//	 OS_ENTER_CRITICAL();                  
//	
//	res = f_opendir(&root_file,(const TCHAR*)filename);//根目录的名字是""
//	filecount = 0;
//	while((f_readdir(&root_file, &file_name) == FR_OK) && file_name.fname[0])
//	{ 
//		if(filecount < SDMaxFileCount)
//		{
//			strcpy((char *)FileList[filecount],file_name.fname);//file_name.fname
//			filecount ++;
//		} 
//		else 
//		{
//			break;
//		}
//// 			printf((char*)FileList[filecount]);
//// 			printf("\r\n");
//    }
//	OS_EXIT_CRITICAL();
// 		printf("filecount = %d\r\n",filecount);
// printf("\r\n电子电路:\r\n\r\n");
// res = f_opendir(&root_file, "电子电路");        //打开目录	
// while((f_readdir(&root_file, &file_name) == FR_OK) && file_name.fname[0]){  
// 	printf((char *)file_name.fname);
// 	printf("\r\n");
// }
		
// retry = f_open(&infile,"SD.txt",FA_CREATE_NEW);    //创建新文件
// 	res = f_open(&infile, "test.txt", FA_OPEN_EXISTING | FA_READ);
// // f_write(&infile, "abcdefg",8, &ss);
// 	if(res != FR_OK) return;
// 	file_size = infile.fsize;
// 		printf("test.txt size: %ld\r\n",file_size);
// // 		TIME = 0;
// 	    res = f_read(&infile, SD_Buffer, BufferSize-1, &ss);
// // 		printf((char *)SD_Buffer);
// 			if (res != FR_OK) {
// 				printf("File read error!\r\n");
// // // // 				break;
// 			} 
// // 			if(ss<512)break;//读到文件末尾
//   f_close(&infile);    //关闭文件
//}
// void FileOpen_Txt(char *buf)
// {
// 	uint8_t res = 0;
// 	FIL infile;
//   uint32_t file_size = 0;	
// 	UINT ss;
// 	FilePath_process(FilePath,buf);
// 	res = f_open(&infile, FilePath, FA_OPEN_EXISTING | FA_READ);
// 	if(res != FR_OK) return;
// 	file_size = infile.fsize;
// 	printf("**.txt size: %ld\r\n",file_size);
// 	ClearPath((char *)SD_Buffer);
// 	res = f_read(&infile, SD_Buffer, BufferSize-1, &ss);
// 	if (res != FR_OK) {
// 		 printf("File read error!\r\n");
// 	} 
//   f_close(&infile);    //关闭文件
// 	FilePath_process(FilePath,"..");
// }
