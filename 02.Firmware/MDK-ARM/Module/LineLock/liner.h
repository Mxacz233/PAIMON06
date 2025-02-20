#ifndef __LINER_H
#define __LINER_H

#include "main.h"

//四向巡线状态结构体
typedef struct {
	uint8_t left;
	uint8_t right;
	uint8_t front;
	uint8_t back;
}Liner_t;



void LineRead(Liner_t *liner_t);
int32_t SpeedOffset(uint8_t linerdir);
void DirectLiner(char dir,int32_t dirspeed,Liner_t *liner_t,Speed_t *speed_t);
#endif 
