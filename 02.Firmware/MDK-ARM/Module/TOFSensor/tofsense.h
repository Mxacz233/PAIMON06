#ifndef TOFSENSE_H
#define TOFSENSE_H

#include "nlink_tofsense_frame0.h"
#include "nlink_tofsensem_frame0.h"
#include "usart.h"
#include "dma.h"
#include "tim.h"

/**比较隐蔽的外部添加
	*@AddThisFuncIn"stm32f4xx_it.c"@USER CODE BEGIN USART3_IRQn 1
	if(__HAL_UART_GET_FLAG( &huart3, UART_FLAG_IDLE ) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);       	//清除串口空闲中断标志位
		HAL_UART_AbortReceive(&huart3);				//关闭DMA传输

		TOF_data_length = sizeof(u_rx_buf) - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx); //计算接收数据长度

		if(TOF_data_length == 16)
		{
			Tof_Unpack_Data();
		}
		else
		{
			TofM_Unpack_Data();
		}
		HAL_UART_Receive_DMA(&huart3,u_rx_buf,sizeof(u_rx_buf));
	}
	*@AddThisHandlein"usart.h"@USER CODE BEGIN Private defines
	extern DMA_HandleTypeDef hdma_usart3_rx;
	extern DMA_HandleTypeDef hdma_usart3_tx;
*/

/*TOFSense/TOFSense-P/PS/F/FP等型号的解包数据结构体*/
typedef struct {
	uint8_t ID;

	float dis;					//解包后的距离数据
	uint8_t status;				//解包后的信号状态指示
	uint16_t signal_strength;	//解包后的信号强度
	float lastdis;
}Tofsense;
static Tofsense TOF[8];


/*TOFSenseM/MS型号的解包数据结构体*/
typedef struct {
	uint8_t pixel_count;		//8x8 or 4x4
	uint8_t ID;

	float dis[64];					//解包后的距离数据
	uint8_t status[64];				//解包后的信号状态指示
	uint16_t signal_strength[64];	//解包后的信号强度
}Tofsense_M;
static Tofsense_M TOF_M[8];



extern Tofsense TOF[8];

extern float sig1,sig2,sig3,sig4;

void Tof_Unpack_Data(void);
void TofM_Unpack_Data(void);
void HAL_Delay_ms(uint16_t nms);
void TofRenew(void);
uint8_t TOFInit(void);

#endif
