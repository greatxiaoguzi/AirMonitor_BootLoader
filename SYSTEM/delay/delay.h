#ifndef __DELAY_H
#define __DELAY_H 
#include "include.h"

void delay_init(void);
void delay_ms(uint16_t nms);
void delay_us(u32 nus);
void get_ms(unsigned long *count);
void Mydelay(uint16_t delay);
#endif

//------------------End of File----------------------------
