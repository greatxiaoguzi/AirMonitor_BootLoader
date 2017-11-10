#ifndef __MPU6050__H
#define __MPU6050__H
#include "include.h"

void Mpu6050_init(void);
void Get_Acc_Gyro_values(void);
void display_XYZ(float x,float y,float z);
#endif

