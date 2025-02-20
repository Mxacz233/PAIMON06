#include "KalmanFilter.h"
#include "math.h"

Kalman_t Kalman = {
.Q_angle = 0.001f,
.Q_bias = 0.003f,
.R_measure = 0.03f
};

//四元数参数
#define Kp 1.50f
#define Ki 0.005f // 这里的KpKi是用于调整加速度计修正陀螺仪的速度
#define halfT 0.005f // 采样周期的一半，用于求解四元数微分方程时计算角增量
float DCMgb[3][3];
float q0=1,q1=0,q2=0,q3=0;// 初始姿态四元数
float exInt = 0,eyInt = 0,ezInt = 0;//当前加计测得的重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差的积分
float yawtemp,pitchtemp,rolltemp=0;
float offset=0;

//二阶卡尔曼滤波
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt)
{
    double rate = newRate - Kalman->bias;
    Kalman->angle += dt * rate;

    Kalman->P[0][0] += dt * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0] + Kalman->Q_angle);
    Kalman->P[0][1] -= dt * Kalman->P[1][1];
    Kalman->P[1][0] -= dt * Kalman->P[1][1];
    Kalman->P[1][1] += Kalman->Q_bias * dt;

    double S = Kalman->P[0][0] + Kalman->R_measure;
    double K[2];
    K[0] = Kalman->P[0][0] / S;
    K[1] = Kalman->P[1][0] / S;

    double y = newAngle - Kalman->angle;
    Kalman->angle += K[0] * y;
    Kalman->bias += K[1] * y;

    double P00_temp = Kalman->P[0][0];
    double P01_temp = Kalman->P[0][1];

    Kalman->P[0][0] -= K[0] * P00_temp;
    Kalman->P[0][1] -= K[0] * P01_temp;
    Kalman->P[1][0] -= K[1] * P00_temp;
    Kalman->P[1][1] -= K[1] * P01_temp;

    return Kalman->angle;
};
//一阶互补滤波
float Complementary_Filter_x(float angle_m, float gyro_m)
{
	 static float angle;
	 float K1 =0.02; 
   angle = K1 * angle_m+ (1-K1) * (angle + gyro_m * 0.01);
	 return angle;
}
//快速开方取倒数
float fastsqrt(float x)
{
	float halfx=0.5f*x;
	float y=x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

float YawFusionSlove(float ax,float ay,float az,float gx,float gy,float gz)
{
	float yaw=0;
//	float q0temp,q1temp,q2temp,q3temp;//四元数暂存变量，求解微分方程时要用
//	uint8_t i;
//	float matrix[9]={1.f,  0.0f,  0.0f, 0.0f,  1.f,  0.0f, 0.0f,  0.0f,  1.f };//初始化矩阵
//  float vx, vy, vz;//当前姿态计算得来的重力在三轴上的分量
//  float ex, ey, ez;//当前加计测得的重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差
//	float norm;//矢量的模或四元数的范数
//	//四元数预先计算
//	float q0q0 = q0*q0;
//  float q0q1 = q0*q1;
//  float q0q2 = q0*q2;
//  float q0q3 = q0*q3;
//  float q1q1 = q1*q1;
//  float q1q2 = q1*q2;
//  float q1q3 = q1*q3;
//  float q2q2 = q2*q2;
//  float q2q3 = q2*q3;
//  float q3q3 = q3*q3;
//	//加速度计处于自由落体状态时不进行姿态解算，因为会产生分母无穷大的情况
//	if(ax*ay*az==0)	return 0;
//	////单位化加速度计
//	norm = fastsqrt(ax*ax + ay*ay + az*az); 
//  ax = ax * norm;
//  ay = ay * norm;
//  az = az * norm;
//	//陀螺仪积分估计重力向量(机体坐标系) 
//  vx = 2*(q1q3 - q0q2);												
//  vy = 2*(q0q1 + q2q3);
//  vz = q0q0 - q1q1 - q2q2 + q3q3 ;
//	//计算测得的重力与计算得重力间的误差，向量外积可以表示这一误差
//  ex = (ay*vz - az*vy); //+ (my*wz - mz*wy);                     
//  ey = (az*vx - ax*vz); //+ (mz*wx - mx*wz);
//  ez = (ax*vy - ay*vx); //+ (mx*wy - my*wx);
//  //用上面求出误差进行积分
//  exInt = exInt + ex * Ki;								 
//  eyInt = eyInt + ey * Ki;
//  ezInt = ezInt + ez * Ki;
//  //将误差PI后补偿到陀螺仪
//  gx = gx + Kp*ex + exInt;					   		  	
//  gy = gy + Kp*ey + eyInt;
//  gz = gz + Kp*ez + ezInt;//这里的gz由于没有观测者进行矫正会产生漂移，表现出来的就是积分自增或自减
//  //下面进行姿态的更新，也就是四元数微分方程的求解
//  q0temp=q0;//暂存当前值用于计算
//  q1temp=q1;//网上传的这份算法大多没有注意这个问题，在此更正
//  q2temp=q2;
//  q3temp=q3;
//  //采用一阶毕卡解法，相关知识可参见《惯性器件与惯性导航系统》P212
//  q0 = q0temp + (-q1temp*gx - q2temp*gy -q3temp*gz)*halfT;
//  q1 = q1temp + (q0temp*gx + q2temp*gz -q3temp*gy)*halfT;
//  q2 = q2temp + (q0temp*gy - q1temp*gz +q3temp*gx)*halfT;
//  q3 = q3temp + (q0temp*gz + q1temp*gy -q2temp*gx)*halfT;
//	//单位化四元数 
//  norm = fastsqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
//  q0 = q0 * norm;
//  q1 = q1 * norm;
//  q2 = q2 * norm;  
//  q3 = q3 * norm;
//	//矩阵R 将惯性坐标系(n)转换到机体坐标系(b) 
//	matrix[0] = q0q0 + q1q1 - q2q2 - q3q3;// 11(前列后行)
//	matrix[1] = 2.f * (q1q2 + q0q3);	    // 12
//	matrix[2] = 2.f * (q1q3 - q0q2);	    // 13
//	matrix[3] = 2.f * (q1q2 - q0q3);	    // 21
//	matrix[4] = q0q0 - q1q1 + q2q2 - q3q3;// 22
//	matrix[5] = 2.f * (q2q3 + q0q1);	    // 23
//	matrix[6] = 2.f * (q1q3 + q0q2);	    // 31
//	matrix[7] = 2.f * (q2q3 - q0q1);	    // 32
//	matrix[8] = q0q0 - q1q1 - q2q2 + q3q3;// 33
	//四元数转换成欧拉角(Z->Y->X) 
	yawtemp  += gz*57.324841f*0.002f;																							// yaw轴不受加速度计的修正，故直接积分
	//零漂补正
	offset+=0.0012;
//  pitchtemp = -asin(2.f * (q1q3 - q0q2))* 57.3f;                                 	// pitch(负号要注意) 
//  rolltemp 	= atan2(2.f * q2q3 + 2.f * q0q1, q0q0 - q1q1 - q2q2 + q3q3)* 57.3f ; 	// roll
//  for(i=0;i<9;i++)
//  {
//    *(&(DCMgb[0][0])+i) = matrix[i];
//  }
	
	//互补滤波
	yaw=Complementary_Filter_x(yawtemp,gz);
	//卡尔曼滤波
	yaw=Kalman_getAngle(&Kalman,yawtemp,gz,0.01);

	return yaw+offset;
}


#define dt 0.001f //采样周期
#define Samptime 2.0f // 采样期，单位秒
int16_t g_GetZero_Offset = 0; //实际采样次数
double Gyroz_offset = 0; //在1KHz下的每次零漂补偿
void YawSolve(EularAngle_t *eularangle)
{	
	//三位滑动均值滤波
	eularangle->gyro[2]=IMU_Filter(eularangle->gyro[2]);
	//采样期
	if( g_GetZero_Offset < Samptime/dt)
	{
		Gyroz_offset += eularangle->gyro[2] * dt / Samptime;		/* 进行积分 */
		g_GetZero_Offset++;
	}
	//计算期
	else
	{
		eularangle->gyro[2]-=Gyroz_offset;
		eularangle->yaw+=eularangle->gyro[2]*57.324841f*0.001f;
	}
}


//三位滑动均值滤波
double DataStack[3] = {0};
double IMU_Filter(double data)
{
	double datasum=0;
	for(uint8_t i=3-1;i>=1;i--)
	{
		DataStack[i] = DataStack[i-1];
	}
	DataStack[0] = data;
	for(uint8_t i=0; i<3; i++)
	{
		datasum += DataStack[i];
	}
	datasum/=3.0;
	return datasum;
}

//一阶互补滤波
double lastdata=0;
double CompFilter(double data)
{
	float K=0.8;
	data=K*data+(1-K)*lastdata;
	lastdata=data;
	return data;
}
//离散二阶低通滤波

