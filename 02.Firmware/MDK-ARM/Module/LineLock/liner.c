#include "liner.h"
	/*
		Ê®Áù½øÖÆ ¶þ½øÖÆ      ºÚ°×Öµ     
		-------- ----------- ----------
		0        0000        °×°×°×°× 
		1        0001        °×°×°×ºÚ 
		2        0010        °×°×ºÚ°× 
		3        0011        °×°×ºÚºÚ 
		4        0100        °×ºÚ°×°× 
		5        0101        °×ºÚ°×ºÚ 
		6        0110        °×ºÚºÚ°× 
		7        0111        °×ºÚºÚºÚ 
		8        1000        ºÚ°×°×°× 
		9        1001        ºÚ°×°×ºÚ 
		A        1010        ºÚ°×ºÚ°× 
		B        1011        ºÚ°×ºÚºÚ 
		C        1100        ºÚºÚ°×°× 
		D        1101        ºÚºÚ°×ºÚ 
		E        1110        ºÚºÚºÚ°× 
		F        1111        ºÚºÚºÚºÚ 
	*/
void LineRead(Liner_t *liner_t)
{
	liner_t->left =  HAL_GPIO_ReadPin(L1_GPIO_Port, L1_Pin)*8
									+HAL_GPIO_ReadPin(L2_GPIO_Port, L2_Pin)*4
									+HAL_GPIO_ReadPin(L3_GPIO_Port, L3_Pin)*2
									+HAL_GPIO_ReadPin(L4_GPIO_Port, L4_Pin)*1;
	
	liner_t->right = HAL_GPIO_ReadPin(R1_GPIO_Port, R1_Pin)*8
									+HAL_GPIO_ReadPin(R2_GPIO_Port, R2_Pin)*4
									+HAL_GPIO_ReadPin(R3_GPIO_Port, R3_Pin)*2
									+HAL_GPIO_ReadPin(R4_GPIO_Port, R4_Pin)*1;
	
	liner_t->front = HAL_GPIO_ReadPin(F1_GPIO_Port, F1_Pin)*8
									+HAL_GPIO_ReadPin(F2_GPIO_Port, F2_Pin)*4
									+HAL_GPIO_ReadPin(F3_GPIO_Port, F3_Pin)*2
									+HAL_GPIO_ReadPin(F4_GPIO_Port, F4_Pin)*1;
	
	liner_t->back =  HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)*8
									+HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin)*4
									+HAL_GPIO_ReadPin(B3_GPIO_Port, B3_Pin)*2
									+HAL_GPIO_ReadPin(B4_GPIO_Port, B4_Pin)*1;
}


#define OFFSETMIN 2000/2
//#define OFFSETMID 3500/2
#define OFFSETMAX 4000/2
int32_t SpeedOffset(uint8_t linerdir)
{
	switch(linerdir)
	{
		case 0x00: return 0;//¿ÕµØ
		case 0x01: return  OFFSETMAX;//´ó×óÆ«
		case 0x02: return  OFFSETMIN;//Ð¡×óÆ«
		case 0x03: return 0;//ÎÞ½â
		case 0x04: return -OFFSETMIN;//Ð¡ÓÒÆ«
		case 0x05: return 0;//ÎÞ½â
		case 0x06: return 0;//Ö±ÐÐ
		case 0x07: return 0;//ÎÞ½â
		case 0x08: return -OFFSETMAX;//´óÓÒÆ«
		case 0x09: return 0;//ÎÞ½â
		case 0x0A: return 0;//ÎÞ½â
		case 0x0B: return 0;//ÎÞ½â
		case 0x0C: return 0;//ÎÞ½â
		case 0x0D: return 0;//ÎÞ½â
		case 0x0E: return 0;//ÎÞ½â
		case 0x0F: return 0;//²æÂ·¿Ú
		default: return 0;
	}
}

void DirectLiner(char dir,int32_t dirspeed,Liner_t *liner_t,Speed_t *speed_t)
{
	uint8_t linesensor=0;
	switch (dir)
	{
		case 'F': linesensor=liner_t->front;break;
		case 'B': linesensor=liner_t->back;	break;
		case 'L': linesensor=liner_t->left;	break;
		case 'R': linesensor=liner_t->right;break;
	}
	int32_t speedvector_t[2]={0};
	speedvector_t[0]=dirspeed;
	speedvector_t[1]=SpeedOffset(linesensor);
	switch (dir)
	{
		case 'F':speed_t->speedx= speedvector_t[0];	speed_t->speedy= speedvector_t[1];	break;
		case 'B':speed_t->speedx=-speedvector_t[0];	speed_t->speedy=-speedvector_t[1];	break;
		case 'L':speed_t->speedx= speedvector_t[1];	speed_t->speedy=-speedvector_t[0];	break;
		case 'R':speed_t->speedx=-speedvector_t[1];	speed_t->speedy= speedvector_t[0];	break;
	}
}
