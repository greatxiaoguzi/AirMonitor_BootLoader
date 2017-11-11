/*
软件说明：新增芯片内部flash读写功能，用于参数保存
新增文件;eeprom.c   stm32f10x_flash.c

*/
//主函数：实现任务的创建以及子任务的实现
//任务堆栈的创建
#include "include.h"
#include "ff.h"
uint8_t CntFlag = 0;   //时间计数标志
//得到指定目录下的bin文件个数
uint16_t APP_Get_Tnum(uint8_t *path)
{
	DIR tdir;  			 //临时目录
	FILINFO tfileinfo;	//临时文件信息
	uint8_t res;
	uint16_t rval = 0;  //有效的文件个数	
	uint8_t *fn=0;
	
	tfileinfo.lfsize = _MAX_LFN*2+1;
	tfileinfo.lfname = mymalloc(tfileinfo.lfsize);
	res = f_opendir(&tdir,(const TCHAR*)path); 	      //打开指定目录
	if(res==FR_OK && tfileinfo.lfname != NULL)
	{
		while(f_readdir(&tdir,&tfileinfo) == FR_OK)
		{
			if(tfileinfo.fname[0]==0)
			{
				break;
			}
			fn = (uint8_t*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);
			res= f_typetell(fn);
			if((res&0xf0)==0x00)    //判断是否为bin文件，只有对BIN文件计数
				rval++;		
			LED0 = !LED0;
		}
	}
	myfree(tfileinfo.lfname);
	return rval;
}
//得到SD卡的状态
uint8_t GetSDCapcity(uint8_t x0,uint8_t y0,uint8_t width,uint8_t height,uint8_t DispFlag)
{
	BYTE res;
	uint32_t total=0,free=0;
	uint32_t Process;
	res = exf_getfree("0",&total,&free);  //得到SD卡的容量大小
	if(res)
	{
		Show_Str(x0,y0,100,20,"无SD卡存在",16,0,RED,LIGHTBLUE);
		return 1;
	}
	else
	{
		total = total>>10;
		free = free>>10;
		if(DispFlag)
		{
			LCD_DrawRectangle(x0,y0, x0+width,y0+height,GRAY);  //画矩形
			LCD_Fill(x0+1,y0+1,x0+width-1,y0+height-1,LGRAY);
			Process = (uint32_t)(total-free)*72/total;
			LCD_Fill(x0+1 ,y0+1 ,x0+1+Process+1,y0+height-1,DARKBLUE);
			
			Show_Str(x0+width,y0,100,12,"总:    MB",12,0,BLUE,LIGHTBLUE);
			LCD_ShowNum(x0+width+9,y0,total,5,12,BLACK,LIGHTBLUE);
			Show_Str(x0+width+59,y0,100,12,"剩:    MB",12,0,BLUE,LIGHTBLUE);
			LCD_ShowNum(x0+width+70,y0,free,5,12,BLACK,LIGHTBLUE);
		}
		return 0;		
	}
}
//加载APP
void UpdataApp(uint8_t *path)
{
	uint8_t res=0;
	LCD_Fill(6,31,472,310,BLACK);
	Show_Str(195,220,300,16,"固件正在更新",16,0,GREEN,BLACK);
	LCDDrawCir(240,140,40,RED,0,0);
	res = IAP_Update_Data(path,220,120);  //后两者为显示的数值的坐标
	switch(res)
	{
		case 0x00:
		{
			Show_Str(195,220,300,16,"固件更新完成",16,0,GREEN,BLACK);
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{
				Show_Str(195,220,200,16,"APP加载中......",16,0,GREEN,BLACK);
				iap_load_app(FLASH_APP1_ADDR);
			}
			else
				Show_Str(195,220,200,16,"App加载失败",16,0,RED,BLACK);
		}break;
		case 0x01:Show_Str(195,220,300,16,"内存申请失败",16,0,RED,BLACK);break;
	    case 0x02:Show_Str(195,220,300,16,"文件申请失败",16,0,RED,BLACK);break;
		case 0x03:Show_Str(195,220,300,16,"文件打开失败或无文件",16,0,RED,BLACK);break;
	}
}
//程序跳转
void ProgramJump(void)
{                                     
	if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
	{
		Show_Str(195,220,200,16,"APP加载中......",16,0,GREEN,BLACK);
		iap_load_app(FLASH_APP1_ADDR);
	}	
	else
	{
		Show_Str(195,220,200,16,"APP跳转失败",16,0,RED,BLACK);
		Close_FDN304P();
	}
}
void Load_AppUpdate_UI(void)
{
	//LCD_Clear(LIGHTBLUE);
	LCD_Fill(0,0,480,320,LIGHTBLUE);
	LCD_DrawRectangle(5,30,473,311,BLUE);  //画矩形
	LCD_Fill(6,31,472,310,BLACK);
	LCD_DrawLine(240, 32, 240, 311,BLUE);
	Show_Str(1,0,200,24,"系统APP更新",24,0,BLUE,LIGHTBLUE);
	Show_Str(350,16,200,12,"(短按更新，长按退出)",12,0,RED,LIGHTBLUE);
	GetSDCapcity(140,4,200,20,1);
}
//主函数
int main(void)
{
	DIR appdir;   //app目录
	FILINFO appfileinfo;//app文件信息
	uint8_t *pname;  //带路径的文件名
	uint8_t curindex = 0;  //当前索引
	uint8_t precurindex = 0;
	uint16_t totappnum = 0; 		//总的app文件总数
	uint8_t *appindextbl;
	uint8_t *fn=0;					//长文件名    
	uint8_t res;
	uint8_t orUpdateCnt = 0;
	uint16_t Cnt = 0;
	uint8_t key;
	uint8_t PageNum = 0;  //页数
	uint8_t FileExistFlag = 0;  //是否有文件存在标志
	sys_clock_init(9);   		             //初始化阶段	
	delay_init();      		                //延时初始化 
	StartMachineInit();                   //开机检测初始化
	uart_init(115200,9600,9600); 
	NVIC_Configuration();                    //设置中断分组
	EncoderInit();
	if(!KeyScan())  //只能返回0
	{
		TFTLCD_Init();      					//彩屏初始化    	
		//KEY_Init();
		LED_Init();
		mem_init();								//内存池初始化
		exfuns_init();
		f_mount(fs[0],"0:",1); 	//挂载SD卡 
		f_mount(fs[1],"1:",1); 	//挂载FLASH.    
		while(Font_Init())						//字库初始化检查是否OK
		{
	UPD:	
			LCD_Fill(0,0,480,320,LIGHTBLUE);
			while(SD_Initialize()&&Cnt<40)	//检测SD卡
			{
				LCD_ShowString(180,80,200,24,24,"SD Card Check Failed!",GREEN,LIGHTBLUE); //显示SD是否存在信息
				delay_ms(200);
				LCD_Fill(175,75,175+150,75+40,LIGHTBLUE);
				delay_ms(200);
				Cnt++;
			}
			Cnt = 0;
			LCD_ShowString(180,80,200,24,24,"SD Card OK",BLACK,LIGHTBLUE);
			LCD_ShowString(180,120,200,24,24,"Font Updating...",BLACK,LIGHTBLUE);
			key=update_font(150,160,24);	//更新字库
			while(key&&Cnt<10)			//更新失败
			{
				LCD_ShowString(180,120,200,24,24,"Font Update Failed!",BLACK,LIGHTBLUE);
				delay_ms(200);
				LCD_Fill(175,115,175+150,115+40,LIGHTBLUE);
				delay_ms(200);	
				Cnt++;			
			}
			LCD_ShowString(180,120,200,24,24,"Font Update Success!",BLACK,LIGHTBLUE);
			delay_ms(1500);	
			LCD_Clear(BLACK);//清屏
			break;
		}
		Cnt = 0;
		//按键进入选择模式
		if(!GetSDCapcity(195,195,200,30,0))  //进入固件更新模式
		{
			Load_AppUpdate_UI();
			totappnum = APP_Get_Tnum("0:/APPFILE");
			if(totappnum>34)  //总共可容纳34个BIN文件
				totappnum = 34;
			PageNum = totappnum/12+1;  //获得页数
			appfileinfo.lfsize = _MAX_LFN*2+1;						//长文件名最大长度
			appfileinfo.lfname = mymalloc(appfileinfo.lfsize);
			pname = mymalloc(appfileinfo.lfsize);				//为带路径的文件名分配内存		
			appindextbl = (uint8_t*)mymalloc(2*totappnum);				//申请2*totpicnum个字节的内存,用于存放app索引
			if(appfileinfo.lfname==NULL||pname==NULL)//内存分配出错
			{
				Show_Str(180,160,200,16,"内存分配失败",16,0,RED,LIGHTBLUE);
				return 0;
			}
			curindex = 0;
			res=f_opendir(&appdir,"0:/APPFILE"); 	//打开目录
			while(res == FR_OK)
			{
				//dir_sdi(&appdir,appindextbl[curindex]);//改变当前目录索引
				res = f_readdir(&appdir,&appfileinfo);		
				if(res!=FR_OK||appfileinfo.fname[0]==0)
				{
					break;
				}
				else 
				{
					fn = (uint8_t*)(*appfileinfo.lfname?appfileinfo.lfname:appfileinfo.fname);
					res = f_typetell(fn);
					if((res&0xf0)==0x00)  //是bin文件的话
					{				
						curindex ++;
						if(curindex>totappnum)
							break;
						appindextbl[curindex] = appdir.index;			
						if(curindex<=17)
						{
							strcpy((char*)pname,"SD:");				//复制路径(目录)
							strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
							Show_Str(25,32+(Cnt++)*16,215,16,pname,16,0,GREEN,BLACK);	//显示文件名称
							if(Cnt==17)
								Cnt = 0;
						}		
						else				
						{
							strcpy((char*)pname,"SD:");				//复制路径(目录)
							strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
							Show_Str(266,32+(Cnt++)*16,215,16,pname,16,0,GREEN,BLACK);//显示文件名称
						}
					}
				}
				res = 0;
			}
			if(curindex == 0)
				FileExistFlag = 0;
			else
				FileExistFlag = 1;
			curindex = 0;
			Show_Str(7,32,32,16,"->",16,0,RED,BLACK);
			while(FileExistFlag)
			{
				//key = KeyScan();
				if(Encoder_TYPE!=Encoder_Void && Encoder_TYPE!= Encoder_Pressed)  //如果没有按下是转动的话
				{
					if(Encoder_TYPE==Encoder_Forword)
					{
						curindex ++;
						if(curindex>=totappnum)
						{
							curindex = 0;
							if(totappnum>17)
								Show_Str(243,32+(totappnum-1-17)*16,32,16,"  ",16,0,RED,BLACK);
						}
					}
					else if(Encoder_TYPE==Encoder_Backword)
					{
						curindex --;
						if(curindex==255)
							curindex = totappnum-1;
					}
					
					if(curindex <= 16)   //当前操作为第一页
					{
						if(totappnum >1)
						{
							Show_Str(7,32+curindex*16,32,16,"->",16,0,RED,BLACK);
							if(precurindex==17)
								Show_Str(243,32+(precurindex-17)*16,32,16,"  ",16,0,RED,BLACK);
							else
								Show_Str(7,32+precurindex*16,32,16,"  ",16,0,RED,BLACK);
						}
						else
							Show_Str(7,32,32,16,"->",16,0,RED,BLACK);
						Encoder_TYPE = Encoder_Void;
					}
					else if(curindex>16 && curindex<totappnum)
					{
						if(curindex==totappnum-1)  //这里判断在第一个倒转到最后一个时清掉第一个的显示
						{
							if(Encoder_TYPE==Encoder_Backword)
								Show_Str(7,32+precurindex*16,32,16,"  ",16,0,RED,BLACK);
						}
						if(precurindex==16)
						{
							if(Encoder_TYPE == Encoder_Forword)
								Show_Str(7,32+precurindex*16,32,16,"  ",16,0,RED,BLACK);
						}
						else
							Show_Str(243,32+(precurindex-17)*16,32,16,"  ",16,0,RED,BLACK);
						if(precurindex == 17)
						{
							if(Encoder_TYPE == Encoder_Backword)
								Show_Str(243,32+(precurindex-17)*16,32,16,"  ",16,0,RED,BLACK);
						}
						Show_Str(243,32+(curindex-17)*16,32,16,"->",16,0,RED,BLACK);
						Encoder_TYPE = Encoder_Void;
					}
					else
					{
						//curindex = 0;
						if(totappnum>17)
						{
							Show_Str(7,32,32,16,"->",16,0,RED,BLACK);
							Show_Str(243,32+(totappnum-1-17)*16,32,16,"  ",16,0,RED,BLACK);
						}
						Encoder_TYPE = Encoder_Void;
					}
					precurindex = curindex;
				}
				else if(Encoder_TYPE == Encoder_Pressed)  //如果按下了旋钮的话
				{
					while(orUpdateCnt++<100 && GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)==0)
					{
						delay_ms(30);
					}
					if(orUpdateCnt >= 100)
					{
						FileExistFlag = 0;
						break;
					}
					dir_sdi(&appdir,appindextbl[curindex]);//改变当前目录索引
					res = f_readdir(&appdir,&appfileinfo);
					fn = (uint8_t*)(*appfileinfo.lfname?appfileinfo.lfname:appfileinfo.fname);
					strcpy((char*)pname,"0:/APPFILE/");				//复制路径(目录)
					strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
					break;   //跳出执行
				}
			}	
			if(FileExistFlag)
				UpdataApp(pname);    //更行系统app然后跳转
			else
			{
				LCD_Fill(6,31,472,310,BLACK);
				if(orUpdateCnt < 100)
					Show_Str(180,160,200,24,"文件不存在",24,0,RED,BLACK);
				else if(orUpdateCnt >= 100)
					Show_Str(180,160,200,24,"无更新",24,0,RED,BLACK);
			}
			delay_ms(1000);
		}
	}
	ProgramJump();  //跳转至APP程序
}
/*
if(curindex > totappnum-1)
{
	curindex = 0;//到末尾的时候,自动从头开始
	if(totappnum>17)
		Show_Str(243,32+(totappnum-1-17)*16,32,16,"  ",16,0,RED,BLACK);
}
if(curindex<=16)
{					
	Show_Str(7,32+curindex*16,32,16,"->",16,0,RED,BLACK);
	Show_Str(7,32+precurindex*16,32,16,"  ",16,0,RED,BLACK);
}
else
{
	if(curindex==17)
		Show_Str(7,32+16*16,32,16,"  ",16,0,RED,BLACK);  //消除最下面的一个->
	else
		Show_Str(243,32+(precurindex-17)*16,32,16,"  ",16,0,RED,BLACK);
	Show_Str(243,32+(curindex-17)*16,32,16,"->",16,0,RED,BLACK);
}
precurindex = curindex;


if(totappnum>17)
	Show_Str(243,32+(totappnum-1-17)*16,32,16,"  ",16,0,RED,BLACK);


*/











//end of file....


