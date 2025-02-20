#include "PosLock.h"
#define StardardSpeed 11000
#define SlowStardardSpeed 6000
float Pos_PID_Absolute(float ActualPos,float TargetPos)
{
    float Error;
    float LastError;
    float Integral;
    float Derivative;
    float Output;
    float Kp = -30;
    float Ki = 0.1;
    float Kd = 0.5;

    Error = TargetPos - ActualPos;
    Integral += Error;
    Derivative = Error - LastError;
    Output = Kp * Error + Ki * Integral + Kd * Derivative;

    LastError = Error;
		if(Output<100 && Output>-100) Output=0;
		if(Output>StardardSpeed) Output=StardardSpeed;
		if(Output<-StardardSpeed) Output=-StardardSpeed;

    return Output;
}

float Slow_Pos_PID_Absolute(float ActualPos,float TargetPos)
{
    float Error;
    float LastError;
    float Integral;
    float Derivative;
    float Output;
    float Kp = -30;
    float Ki = 0.1;
    float Kd = 0.5;

    Error = TargetPos - ActualPos;
    Integral += Error;
    Derivative = Error - LastError;
    Output = Kp * Error + Ki * Integral + Kd * Derivative;

    LastError = Error;
		if(Output<100 && Output>-100) Output=0;
		if(Output>SlowStardardSpeed) Output=SlowStardardSpeed;
		if(Output<-SlowStardardSpeed) Output=-SlowStardardSpeed;

    return Output;
}

