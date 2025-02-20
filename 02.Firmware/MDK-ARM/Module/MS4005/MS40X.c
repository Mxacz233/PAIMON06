#include "MS40X.h"

void PlatformConAbsAng(int32_t degree)
{
	uint8_t TxData[8] = {0xA3, 0x00, 0x00, 0x00, 0x50, 0x46, 0x00, 0x00};
	TxData[4]=*(uint8_t *)(&degree);
	TxData[5]=*((uint8_t *)(&degree)+1);
	TxData[6]=*((uint8_t *)(&degree)+2);
	TxData[7]=*((uint8_t *)(&degree)+3);
	vApp_User_CAN1_TxMessage(TxData, 8);
}
void PlatformConAcCAng(int32_t degree)
{
	uint8_t TxData[8] = {0xA7, 0x00, 0x00, 0x00, 0x50, 0x46, 0x00, 0x00};
	TxData[4]=*(uint8_t *)(&degree);
	TxData[5]=*((uint8_t *)(&degree)+1);
	TxData[6]=*((uint8_t *)(&degree)+2);
	TxData[7]=*((uint8_t *)(&degree)+3);
	vApp_User_CAN1_TxMessage(TxData, 8);
}
