#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "main.h"

typedef struct {

    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;

    double KalmanAngleX;
    double KalmanAngleY;
} MPU6050_t;

// Kalman structure
typedef struct {
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_t;
// 欧拉角结构体
typedef struct {
	float accel[3];
	float gyro[3];
	float yaw;
	float pitch;
	float roll;
}EularAngle_t;

//EularAngle_t EularAngle ={
//	.yaw=0,
//	.pitch=0,
//	.roll=0
//};
float YawFusionSlove(float ax,float ay,float az,float gx,float gy,float gz);
void YawSolve(EularAngle_t *EularAngle);
double IMU_Filter(double data);
#endif

