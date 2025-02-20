#include "bsp_can.h"
#include "pid.h"
#include "stm32f4xx_hal.h"

PID_TypeDef motor_pid[4];


void motor_init_4p(void)
{
//	filter_to_can_1();
	filter_to_can_2();
	for(int i=0; i<4; i++)
	{	
		pid_init(&motor_pid[i]);
		motor_pid[i].f_param_init(&motor_pid[i],PID_Speed,16384,5000,10,0,8000,0,1.5,0.1,0); 
	}
}

void M2006_motion_vector(int x_speed,int y_speed,int turn_speed)
{
	//麦轮位置解算
	motor_pid[0].target = 0 + x_speed + y_speed + turn_speed;  
	motor_pid[1].target = 0 + x_speed - y_speed + turn_speed;
	motor_pid[2].target = 0 - x_speed - y_speed + turn_speed;
	motor_pid[3].target = 0 - x_speed + y_speed + turn_speed;
	
	//速度环PID（根据设定值进行PID计算）
	motor_pid[0].f_cal_pid(&motor_pid[0],moto_chassis[0].speed_rpm);   
	motor_pid[1].f_cal_pid(&motor_pid[1],moto_chassis[1].speed_rpm);   
	motor_pid[2].f_cal_pid(&motor_pid[2],moto_chassis[2].speed_rpm);   
	motor_pid[3].f_cal_pid(&motor_pid[3],moto_chassis[3].speed_rpm);   
	
	//将PID的计算结果通过CAN发送到电机
	set_moto_current(&hcan2, motor_pid[0].output,  
							 motor_pid[1].output,
                             motor_pid[2].output,
                             motor_pid[3].output);
}
