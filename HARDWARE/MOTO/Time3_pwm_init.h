#ifndef _Time3_pwm_init
#define _Time3_pwm_init
#include "sys.h"
extern uint8_t Loop50HzFlag;
void Timer2_Init(uint32_t period);
void Tim3_init(void);
void SetPWMVal(uint16_t pwm1,uint16_t pwm2);	



#endif



