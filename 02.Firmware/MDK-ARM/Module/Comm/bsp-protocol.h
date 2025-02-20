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
extern uint8_t TempPosition[3];//TP[0]是现在小车位于第几路段,TP[1]是距离哪个方向TP[2]多少个两厘米
//-------接收成功回复标志位----------
extern uint8_t Flag_0x21;		//请求检测宝藏信号
extern uint8_t Flag_0x31;		//发送当前位置（当前所在路段号，测距正常端的方向，正常端方向离墙的距离）
//-----------------------------------


//---------协议解析标志位与数据位------------
extern uint8_t StopFlagUpper;
extern uint8_t RoadsNumUpper;  //值就是路段总数，不需要加1
extern uint8_t CrashFlagUpper;
extern uint8_t CrashDirectionUpper;
extern uint16_t RoadInfoUpper[4][50];
extern uint8_t RoadInfoFinishFlag;
extern uint8_t CameraDirTemp;
extern uint8_t CameraDirPro;
extern uint8_t GimbalDirTemp;
//-------------------------------------------


//---------协议发送标志位与数据位------------
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
