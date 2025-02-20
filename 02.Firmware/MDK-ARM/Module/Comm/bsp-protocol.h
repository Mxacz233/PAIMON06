#ifndef __BSP_PROTOCOL_H
#define __BSP_PROTOCOL_H

#include "main.h"
#include "usart.h"
#include "MS40X.h"


#define INIT_DEGREE 5500  

extern uint8_t RxBuffer[8];
extern uint8_t RxFlag;
extern uint8_t ErrFlag;

extern uint8_t VerifyVal;

extern uint8_t CheckTreasureFlag;
extern uint8_t SendPositionFlag;
extern uint8_t TempPosition[3];//TP[0]������С��λ�ڵڼ�·��,TP[1]�Ǿ����ĸ�����TP[2]���ٸ�������
//-------���ճɹ��ظ���־λ----------
extern uint8_t Flag_0x21;		//�����ⱦ���ź�
extern uint8_t Flag_0x31;		//���͵�ǰλ�ã���ǰ����·�κţ���������˵ķ��������˷�����ǽ�ľ��룩
//-----------------------------------


//---------Э�������־λ������λ------------
extern uint8_t StopFlagUpper;
extern uint8_t RoadsNumUpper;  //ֵ����·������������Ҫ��1
extern uint8_t CrashFlagUpper;
extern uint8_t CrashDirectionUpper;
extern uint16_t RoadInfoUpper[4][50];
extern uint8_t RoadInfoFinishFlag;
extern uint8_t CameraDirTemp;
extern uint8_t CameraDirPro;
extern uint8_t GimbalDirTemp;
//-------------------------------------------


//---------Э�鷢�ͱ�־λ������λ------------
extern uint8_t SendBuffer[8];
//-------------------------------------------

extern uint8_t ReceiveRxBuffer[8];
extern uint8_t ReceiveRxTempBuffer[8];
extern uint8_t ReceiveOKFlag;
extern uint8_t TempCrushFlag;
extern char test001[80];

void CommReceiveInit(void);
void protocol_analysis(void);
void sendUART(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void encodeRoads(void);
#endif
