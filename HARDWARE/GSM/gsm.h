#ifndef GSM__H
#define GSM__H
#include "include.h"
void GSMInit(void);
u8 Hand(char *a,u8 Time);
void 	RecMesageDeal(void);
void SendMessage(void);
void CallPhone(void);
void AdminNowStat(void);
void GPRSSendMSg(char *p);
void GPRSBuiltNet(u8 *IP,u8 *Port);

#endif




