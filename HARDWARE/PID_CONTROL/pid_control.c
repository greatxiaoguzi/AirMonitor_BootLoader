#ifndef __PID_CONTROL_C
#define __PID_CONTROL_C
#include "include.h"
//外部全局变量，数组Remote_Sour[2];
static float Setpoint = 0.0000;    //目标点,PID控制器要调整回的值
extern float Pitch,Roll,Yaw;      //X Y Z
extern float 	Remote_Sour[2];    //遥控接收数组缓冲区，外部数组。全局数组
float Remote_Prac[2];     //内部数组
float PID1_P=45,PID1_I=10,PID1_D=15;  //PID参数
//float PID2_P=45,PID2_I=13,PID2_D=15;
//e是角度偏差，KP,KI和KD使我们PID运算所需要的参数，需要我们不同的组合最终得出使小车能够站立的合理数值
float PID1(float e,float kp,float ki,float kd)
{
    static float es=0,sum=0;
    float r;
    sum+=e;
	  if(sum>8)
			sum=8;
		if(sum<-8)
			sum=-8;
    r = kp*e+ki*sum+kd*(e-es);
    es=e;
    return r; 
}
float PID2(float e,float kp,float ki,float kd)
{
    static float es=0,sum=0;
    float r;
    sum+=e;
	  if(sum>8)
			sum=8;
		if(sum<-8)
			sum=-8;
    r = kp*e+ki*sum+kd*(e-es);
    es=e;
    return r; 
}
void main_control(void)
{
	 float PID1_out,PID2_out,Pwm1,Pwm2;
	 static float Bias;
   Get_Acc_Gyro_values();//得到三轴数据并进行融合
	 Bias=Pitch-Setpoint;  //角度偏差值
	 PID1_out=PID1(Bias,PID1_P,PID1_I,PID1_D);
	 PID2_out=PID2(Bias,PID1_P,PID1_I,PID1_D);
	 if(Bias>30||Bias<-30)
		 Moto_PWM(0,0);
	 else
	 {
		 Pwm1=PID1_out;    //加上油门的值
		 Pwm2=PID2_out;
	   Moto_PWM(Pwm1,Pwm2);    //方位控制
	 }
}
#endif



