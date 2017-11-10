#include "include.h"
//////////////////////////////////////////////////////////////////////////////////	  	  
 
//ÒÔÏÂÊÇSPIÄ£¿éµÄ³õÊ¼»¯´úÂë£¬ÅäÖÃ³ÉÖ÷»úÄ£Ê½£¬·ÃÎÊSD Card/W25X16/24L01/JF24C							  
//SPI¿Ú³õÊ¼»¯
//ÕâÀïÕëÊÇ¶ÔSPI1µÄ³õÊ¼»¯
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
  
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //¸´ÓÃÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);    //ÖÃÎ»

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ÉèÖÃSPIµ¥Ïò»òÕßË«ÏòµÄÊý¾ÝÄ£Ê½:SPIÉèÖÃÎªË«ÏßË«ÏòÈ«Ë«¹¤
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//ÉèÖÃSPI¹¤×÷Ä£Ê½:ÉèÖÃÎªÖ÷SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//ÉèÖÃSPIµÄÊý¾Ý´óÐ¡:SPI·¢ËÍ½ÓÊÕ8Î»Ö¡½á¹¹
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//Ñ¡ÔñÁË´®ÐÐÊ±ÖÓµÄÎÈÌ¬:Ê±ÖÓÐü¿Õ¸ß
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//Êý¾Ý²¶»ñÓÚµÚ¶þ¸öÊ±ÖÓÑØ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSSÐÅºÅÓÉÓ²¼þ£¨NSS¹Ü½Å£©»¹ÊÇÈí¼þ£¨Ê¹ÓÃSSIÎ»£©¹ÜÀí:ÄÚ²¿NSSÐÅºÅÓÐSSIÎ»¿ØÖÆ
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//¶¨Òå²¨ÌØÂÊÔ¤·ÖÆµµÄÖµ:²¨ÌØÂÊÔ¤·ÖÆµÖµÎª256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//Ö¸¶¨Êý¾Ý´«Êä´ÓMSBÎ»»¹ÊÇLSBÎ»¿ªÊ¼:Êý¾Ý´«Êä´ÓMSBÎ»¿ªÊ¼
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCÖµ¼ÆËãµÄ¶àÏîÊ½
	SPI_Init(SPI1, &SPI_InitStructure);  //¸ù¾ÝSPI_InitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯ÍâÉèSPIx¼Ä´æÆ÷
 
	SPI_Cmd(SPI1, ENABLE); //Ê¹ÄÜSPIÍâÉè
	
	SPI1_ReadWriteByte(0xff);//Æô¶¯´«Êä		 
}   
//SPI ËÙ¶ÈÉèÖÃº¯Êý
//SpeedSet:
//SPI_BaudRatePrescaler_2   2·ÖÆµ   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8·ÖÆµ   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16·ÖÆµ  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256·ÖÆµ (SPI 281.25K@sys 72M)
  
void SPI1_SetSpeed(uint8_t SpeedSet)
{
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=SpeedSet;	//ÉèÖÃSPI1ËÙ¶È,9MHz.
	SPI1->CR1|=1<<6; 		//SPI1Éè±¸Ê¹ÄÜ 
} 

//SPIx ¶ÁÐ´Ò»¸ö×Ö½Ú
//TxData:ÒªÐ´ÈëµÄ×Ö½Ú
//·µ»ØÖµ:¶ÁÈ¡µ½µÄ×Ö½Ú
uint8_t SPI1_ReadWriteByte(uint8_t TxData)   //¶ÁÐ´nrf24l01
{		
	uint8_t retry=0;				 	
	while ((SPI1->SR&1<<1)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:·¢ËÍ»º´æ¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}
	SPI1->DR=TxData;           //Í¨¹ýÍâÉèSPIx·¢ËÍÒ»¸öÊý¾Ý
	retry=0;
	while((SPI1->SR&1<<0)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:½ÓÊÜ»º´æ·Ç¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}
	return SPI1->DR;           //·µ»ØÍ¨¹ýSPIx×î½ü½ÓÊÕµÄÊý,Êý¾Ý¼Ä´æÆ÷Ý					    			    
}
void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;    //½á¹¹ÌåÉêÃ÷
	SPI_InitTypeDef  SPI_InitStructure;     //½á¹¹ÌåÉêÃ÷
	//Ê¹ÄÜÊ±ÖÓ
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2 ,ENABLE );   //SPI2¹ÒÓÚAPB1ÉÏ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 |GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;          //¸´ÓÃÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	//³õÊ¼»¯SPI2
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ÉèÖÃSPIµ¥Ïò»òÕßË«ÏòµÄÊý¾ÝÄ£Ê½:SPIÉèÖÃÎªË«ÏßË«ÏòÈ«Ë«¹¤
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//ÉèÖÃSPI¹¤×÷Ä£Ê½:ÉèÖÃÎªÖ÷SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//ÉèÖÃSPIµÄÊý¾Ý´óÐ¡:SPI·¢ËÍ½ÓÊÕ8Î»Ö¡½á¹¹
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//Ñ¡ÔñÁË´®ÐÐÊ±ÖÓµÄÎÈÌ¬:Ê±ÖÓÐü¿Õ¸ß
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//Êý¾Ý²¶»ñÓÚµÚ¶þ¸öÊ±ÖÓÑØ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSSÐÅºÅÓÉÓ²¼þ£¨NSS¹Ü½Å£©»¹ÊÇÈí¼þ£¨Ê¹ÓÃSSIÎ»£©¹ÜÀí:ÄÚ²¿NSSÐÅºÅÓÐSSIÎ»¿ØÖÆ
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//¶¨Òå²¨ÌØÂÊÔ¤·ÖÆµµÄÖµ:²¨ÌØÂÊÔ¤·ÖÆµÖµÎª256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//Ö¸¶¨Êý¾Ý´«Êä´ÓMSBÎ»»¹ÊÇLSBÎ»¿ªÊ¼:Êý¾Ý´«Êä´ÓMSBÎ»¿ªÊ¼
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCÖµ¼ÆËãµÄ¶àÏîÊ½
	SPI_Init(SPI2, &SPI_InitStructure);  //¸ù¾ÝSPI_InitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯ÍâÉèSPIx¼Ä´æÆ÷
 
	SPI_Cmd(SPI2, ENABLE); //Ê¹ÄÜSPIÍâÉè
	
	//SPI2_ReadWriteByte(0xff);//Æô¶¯´«Êä		 
}   
//SPI ËÙ¶ÈÉèÖÃº¯Êý
//SpeedSet:
//SPI_BaudRatePrescaler_2   2·ÖÆµ   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8·ÖÆµ   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16·ÖÆµ  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256·ÖÆµ (SPI 281.25K@sys 72M)
  
void SPI2_SetSpeed(uint8_t SpeedSet)
{
	SPI2->CR1&=0XFFC7; 
	SPI2->CR1|=SpeedSet;	//ÉèÖÃSPI1ËÙ¶È,9MHz.
	SPI2->CR1|=1<<6; 		//SPI1Éè±¸Ê¹ÄÜ 
} 

//SPIx ¶ÁÐ´Ò»¸ö×Ö½Ú
//TxData:ÒªÐ´ÈëµÄ×Ö½Ú
//·µ»ØÖµ:¶ÁÈ¡µ½µÄ×Ö½Ú
uint8_t SPI2_ReadWriteByte(uint8_t TxData)   //¶ÁÐ´nrf24l01
{		
	uint8_t retry=0;				 	
	while ((SPI2->SR&1<<1)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:·¢ËÍ»º´æ¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}			  
	SPI2->DR=TxData;           //Í¨¹ýÍâÉèSPIx·¢ËÍÒ»¸öÊý¾Ý
	retry=0;
	while ((SPI2->SR&1<<0)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:½ÓÊÜ»º´æ·Ç¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}	  						    
	return SPI2->DR;           //·µ»ØÍ¨¹ýSPIx×î½ü½ÓÊÕµÄÊý,Êý¾Ý¼Ä´æÆ÷?				    			    
}

void SPI3_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
  
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI3, ENABLE );  	//SPI3Ê±ÖÓÊ¹ÄÜ
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );
	// I/O¿Ú³õÊ¼»¯ÅäÖÃ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //¸´ÓÃÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ÉèÖÃSPIµ¥Ïò»òÕßË«ÏòµÄÊý¾ÝÄ£Ê½:SPIÉèÖÃÎªË«ÏßË«ÏòÈ«Ë«¹¤
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//ÉèÖÃSPI¹¤×÷Ä£Ê½:ÉèÖÃÎªÖ÷SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//ÉèÖÃSPIµÄÊý¾Ý´óÐ¡:SPI·¢ËÍ½ÓÊÕ8Î»Ö¡½á¹¹
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//Ñ¡ÔñÁË´®ÐÐÊ±ÖÓµÄÎÈÌ¬:Ê±ÖÓÐü¿Õ¸ß
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//Êý¾Ý²¶»ñÓÚµÚ¶þ¸öÊ±ÖÓÑØ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSSÐÅºÅÓÉÓ²¼þ£¨NSS¹Ü½Å£©»¹ÊÇÈí¼þ£¨Ê¹ÓÃSSIÎ»£©¹ÜÀí:ÄÚ²¿NSSÐÅºÅÓÐSSIÎ»¿ØÖÆ
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//¶¨Òå²¨ÌØÂÊÔ¤·ÖÆµµÄÖµ:²¨ÌØÂÊÔ¤·ÖÆµÖµÎª256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//Ö¸¶¨Êý¾Ý´«Êä´ÓMSBÎ»»¹ÊÇLSBÎ»¿ªÊ¼:Êý¾Ý´«Êä´ÓMSBÎ»¿ªÊ¼
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCÖµ¼ÆËãµÄ¶àÏîÊ½
	SPI_Init(SPI3, &SPI_InitStructure);  //¸ù¾ÝSPI_InitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯ÍâÉèSPIx¼Ä´æÆ÷
 
	SPI_Cmd(SPI3, ENABLE); //Ê¹ÄÜSPIÍâÉè
	
	SPI3_ReadWriteByte(0xff);//Æô¶¯´«Êä		 
}

void SPI3_SetSpeed(uint8_t SpeedSet)
{
	SPI3->CR1&=0XFFC7; 
	SPI3->CR1|=SpeedSet;	//ÉèÖÃSPI1ËÙ¶È,9MHz.
	SPI3->CR1|=1<<6; 		//SPI1Éè±¸Ê¹ÄÜ 
} 

//SPIx ¶ÁÐ´Ò»¸ö×Ö½Ú
//TxData:ÒªÐ´ÈëµÄ×Ö½Ú
//·µ»ØÖµ:¶ÁÈ¡µ½µÄ×Ö½Ú
uint8_t SPI3_ReadWriteByte(uint8_t TxData)   //¶ÁÐ´Enc28j06
{		
	uint8_t retry=0;				 	
	while ((SPI3->SR&1<<1)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:·¢ËÍ»º´æ¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}			  
	SPI3->DR=TxData;           //Í¨¹ýÍâÉèSPIx·¢ËÍÒ»¸öÊý¾Ý
	retry=0;
	while ((SPI3->SR&1<<0)==0) //¼ì²éÖ¸¶¨µÄSPI±êÖ¾Î»ÉèÖÃÓë·ñ:½ÓÊÜ»º´æ·Ç¿Õ±êÖ¾Î»
	{
		retry++;
		if(retry>200)
			return 0;
	}	  						    
	return SPI3->DR;           //·µ»ØÍ¨¹ýSPIx×î½ü½ÓÊÕµÄÊý,Êý¾Ý¼Ä´æÆ÷?				    			    
}



























