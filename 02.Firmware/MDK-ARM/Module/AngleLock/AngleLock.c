#include "AngleLock.h"

float Ang_PID_Absolute(float ActualAngle,float TargetAngle)
{
    float Error;
    float LastError;
    float Integral;
    float Derivative;
    float Output;
    float Kp = 300;
    float Ki = 0.2;
    float Kd = 0.5;

    Error = TargetAngle - ActualAngle;
    Integral += Error;
    Derivative = Error - LastError;
    Output = Kp * Error + Ki * Integral + Kd * Derivative;

    LastError = Error;
		if(Output<100 && Output>-100) Output=0;

    return Output;
}

