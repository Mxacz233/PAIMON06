#include "KalmanFilter.h"
#include "math.h"

Kalman_t Kalman = {
.Q_angle = 0.001f,
.Q_bias = 0.003f,
.R_measure = 0.03f
};

//��Ԫ������
#define Kp 1.50f
#define Ki 0.005f // �����KpKi�����ڵ������ٶȼ����������ǵ��ٶ�
#define halfT 0.005f // �������ڵ�һ�룬���������Ԫ��΢�ַ���ʱ���������
float DCMgb[3][3];
float q0=1,q1=0,q2=0,q3=0;// ��ʼ��̬��Ԫ��
float exInt = 0,eyInt = 0,ezInt = 0;//��ǰ�ӼƲ�õ��������ٶ��������ϵķ������õ�ǰ��̬��������������������ϵķ��������Ļ���
float yawtemp,pitchtemp,rolltemp=0;
float offset=0;

//���׿������˲�
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
//һ�׻����˲�
float Complementary_Filter_x(float angle_m, float gyro_m)
{
	 static float angle;
	 float K1 =0.02; 
   angle = K1 * angle_m+ (1-K1) * (angle + gyro_m * 0.01);
	 return angle;
}
//���ٿ���ȡ����
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
//	float q0temp,q1temp,q2temp,q3temp;//��Ԫ���ݴ���������΢�ַ���ʱҪ��
//	uint8_t i;
//	float matrix[9]={1.f,  0.0f,  0.0f, 0.0f,  1.f,  0.0f, 0.0f,  0.0f,  1.f };//��ʼ������
//  float vx, vy, vz;//��ǰ��̬��������������������ϵķ���
//  float ex, ey, ez;//��ǰ�ӼƲ�õ��������ٶ��������ϵķ������õ�ǰ��̬��������������������ϵķ��������
//	float norm;//ʸ����ģ����Ԫ���ķ���
//	//��Ԫ��Ԥ�ȼ���
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
//	//���ٶȼƴ�����������״̬ʱ��������̬���㣬��Ϊ�������ĸ���������
//	if(ax*ay*az==0)	return 0;
//	////��λ�����ٶȼ�
//	norm = fastsqrt(ax*ax + ay*ay + az*az); 
//  ax = ax * norm;
//  ay = ay * norm;
//  az = az * norm;
//	//�����ǻ��ֹ�����������(��������ϵ) 
//  vx = 2*(q1q3 - q0q2);												
//  vy = 2*(q0q1 + q2q3);
//  vz = q0q0 - q1q1 - q2q2 + q3q3 ;
//	//�����õ������������������������������Ա�ʾ��һ���
//  ex = (ay*vz - az*vy); //+ (my*wz - mz*wy);                     
//  ey = (az*vx - ax*vz); //+ (mz*wx - mx*wz);
//  ez = (ax*vy - ay*vx); //+ (mx*wy - my*wx);
//  //��������������л���
//  exInt = exInt + ex * Ki;								 
//  eyInt = eyInt + ey * Ki;
//  ezInt = ezInt + ez * Ki;
//  //�����PI�󲹳���������
//  gx = gx + Kp*ex + exInt;					   		  	
//  gy = gy + Kp*ey + eyInt;
//  gz = gz + Kp*ez + ezInt;//�����gz����û�й۲��߽��н��������Ư�ƣ����ֳ����ľ��ǻ����������Լ�
//  //���������̬�ĸ��£�Ҳ������Ԫ��΢�ַ��̵����
//  q0temp=q0;//�ݴ浱ǰֵ���ڼ���
//  q1temp=q1;//���ϴ�������㷨���û��ע��������⣬�ڴ˸���
//  q2temp=q2;
//  q3temp=q3;
//  //����һ�ױϿ��ⷨ�����֪ʶ�ɲμ���������������Ե���ϵͳ��P212
//  q0 = q0temp + (-q1temp*gx - q2temp*gy -q3temp*gz)*halfT;
//  q1 = q1temp + (q0temp*gx + q2temp*gz -q3temp*gy)*halfT;
//  q2 = q2temp + (q0temp*gy - q1temp*gz +q3temp*gx)*halfT;
//  q3 = q3temp + (q0temp*gz + q1temp*gy -q2temp*gx)*halfT;
//	//��λ����Ԫ�� 
//  norm = fastsqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
//  q0 = q0 * norm;
//  q1 = q1 * norm;
//  q2 = q2 * norm;  
//  q3 = q3 * norm;
//	//����R ����������ϵ(n)ת������������ϵ(b) 
//	matrix[0] = q0q0 + q1q1 - q2q2 - q3q3;// 11(ǰ�к���)
//	matrix[1] = 2.f * (q1q2 + q0q3);	    // 12
//	matrix[2] = 2.f * (q1q3 - q0q2);	    // 13
//	matrix[3] = 2.f * (q1q2 - q0q3);	    // 21
//	matrix[4] = q0q0 - q1q1 + q2q2 - q3q3;// 22
//	matrix[5] = 2.f * (q2q3 + q0q1);	    // 23
//	matrix[6] = 2.f * (q1q3 + q0q2);	    // 31
//	matrix[7] = 2.f * (q2q3 - q0q1);	    // 32
//	matrix[8] = q0q0 - q1q1 - q2q2 + q3q3;// 33
	//��Ԫ��ת����ŷ����(Z->Y->X) 
	yawtemp  += gz*57.324841f*0.002f;																							// yaw�᲻�ܼ��ٶȼƵ���������ֱ�ӻ���
	//��Ư����
	offset+=0.0012;
//  pitchtemp = -asin(2.f * (q1q3 - q0q2))* 57.3f;                                 	// pitch(����Ҫע��) 
//  rolltemp 	= atan2(2.f * q2q3 + 2.f * q0q1, q0q0 - q1q1 - q2q2 + q3q3)* 57.3f ; 	// roll
//  for(i=0;i<9;i++)
//  {
//    *(&(DCMgb[0][0])+i) = matrix[i];
//  }
	
	//�����˲�
	yaw=Complementary_Filter_x(yawtemp,gz);
	//�������˲�
	yaw=Kalman_getAngle(&Kalman,yawtemp,gz,0.01);

	return yaw+offset;
}


#define dt 0.001f //��������
#define Samptime 2.0f // �����ڣ���λ��
int16_t g_GetZero_Offset = 0; //ʵ�ʲ�������
double Gyroz_offset = 0; //��1KHz�µ�ÿ����Ư����
void YawSolve(EularAngle_t *eularangle)
{	
	//��λ������ֵ�˲�
	eularangle->gyro[2]=IMU_Filter(eularangle->gyro[2]);
	//������
	if( g_GetZero_Offset < Samptime/dt)
	{
		Gyroz_offset += eularangle->gyro[2] * dt / Samptime;		/* ���л��� */
		g_GetZero_Offset++;
	}
	//������
	else
	{
		eularangle->gyro[2]-=Gyroz_offset;
		eularangle->yaw+=eularangle->gyro[2]*57.324841f*0.001f;
	}
}


//��λ������ֵ�˲�
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

//һ�׻����˲�
double lastdata=0;
double CompFilter(double data)
{
	float K=0.8;
	data=K*data+(1-K)*lastdata;
	lastdata=data;
	return data;
}
//��ɢ���׵�ͨ�˲�

