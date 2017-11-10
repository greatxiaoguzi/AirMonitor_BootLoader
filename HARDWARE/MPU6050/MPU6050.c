#include "include.h"

#define PRINT_ACCEL     (0x01)
#define PRINT_GYRO      (0x02)
#define PRINT_QUAT      (0x04)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)

#define MOTION          (0)
#define NO_MOTION       (1)

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (200)
#define FLASH_SIZE      (512)
#define FLASH_MEM_START ((void*)0x1800)
float Yaw,Roll,Pitch;
#define q30  1073741824.0f
float q0=1.0f,q1=0.0f,q2=0.0f,q3=0.0f;

static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};

static  unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}


static  unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010*/
     

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

static void run_self_test(void)
{
    int result;
//    char test_packet[4] = {0};
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x7) {
        /* Test passed. We can trust the gyro data here, so let's push it down
         * to the DMP.
         */
        float sens;
        unsigned short accel_sens;
        mpu_get_gyro_sens(&sens);
        gyro[0] = (long)(gyro[0] * sens);
        gyro[1] = (long)(gyro[1] * sens);
        gyro[2] = (long)(gyro[2] * sens);
        dmp_set_gyro_bias(gyro);
        mpu_get_accel_sens(&accel_sens);
        accel[0] *= accel_sens;
        accel[1] *= accel_sens;
        accel[2] *= accel_sens;
        dmp_set_accel_bias(accel);
		printf("setting bias succesfully ......\r\n");
    }
	else
	{
		printf("bias has not been modified ......\r\n");
	}

    /* Report results. 
//    test_packet[0] = 't';
//    test_packet[1] = result;
//    send_packet(PACKET_TYPE_MISC, test_packet);*/
}
void Mpu6050_init(void)
{
  	mpu_init();
	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_set_sample_rate(DEFAULT_MPU_HZ);
	dmp_load_motion_driver_firmware();
	dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
	dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
	        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
	        DMP_FEATURE_GYRO_CAL);
	dmp_set_fifo_rate(DEFAULT_MPU_HZ);
	mpu_set_dmp_state(1);
}
void display_XYZ(float x,float y,float z)
{
  if(x<0)
		{
		  //LCD_ShowString(120,10,200,16,16,"-");
			Pitch=fabs(Pitch);
			//LCD_ShowxNum(140,10,Pitch,3,16,0X80);
		}
		else 
    {
			//LCD_ShowString(120,10,200,16,16," ");
	    //LCD_ShowxNum(140,10,Pitch,3,16,0X80);
		}
		if(y<0)
		{
			//LCD_ShowString(120,30,200,16,16,"-");
			Roll=fabs(Roll);
		  //LCD_ShowxNum(140,30,Roll,3,16,0X80);
		}
		else 
    { 
			//LCD_ShowString(120,30,200,16,16," ");
      //LCD_ShowxNum(140,30,Roll,3,16,0X80);
		}
		if(z<0)
		{
		  //LCD_ShowString(120,50,200,16,16,"-");
			Yaw=fabs(Yaw);
			//LCD_ShowxNum(140,50,Yaw,3,16,0X80);
		}
		else 
    {
			//LCD_ShowString(120,50,200,16,16," ");
      //LCD_ShowxNum(140,50,Yaw,3,16,0X80);
    }
}
void Get_Acc_Gyro_values(void)
{
  unsigned long sensor_timestamp;
	short gyro[3], accel[3], sensors;
	unsigned char more;
	long quat[4];
	dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);	 
	if (sensors & INV_WXYZ_QUAT )
	{
	  //printf("in Calculating quaternion steps.....\r\n");
		q0=quat[0] / q30;
		q1=quat[1] / q30;
		q2=quat[2] / q30;
		q3=quat[3] / q30;
		Pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3;   // pitch
		Roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3;   // roll		 
		Yaw = 	atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;     //yaw
		//display_XYZ(Pitch,Roll,Yaw);
	}
}


