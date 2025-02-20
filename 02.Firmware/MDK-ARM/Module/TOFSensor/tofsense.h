#ifndef TOFSENSE_H
#define TOFSENSE_H

#include "nlink_tofsense_frame0.h"
#include "nlink_tofsensem_frame0.h"
#include "usart.h"
#include "dma.h"
#include "tim.h"

/**�Ƚ����ε��ⲿ���
	*@AddThisFuncIn"stm32f4xx_it.c"@USER CODE BEGIN USART3_IRQn 1
	if(__HAL_UART_GET_FLAG( &huart3, UART_FLAG_IDLE ) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);       	//������ڿ����жϱ�־λ
		HAL_UART_AbortReceive(&huart3);				//�ر�DMA����

		TOF_data_length = sizeof(u_rx_buf) - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx); //����������ݳ���

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

/*TOFSense/TOFSense-P/PS/F/FP���ͺŵĽ�����ݽṹ��*/
typedef struct {
	uint8_t ID;

	float dis;					//�����ľ�������
	uint8_t status;				//�������ź�״ָ̬ʾ
	uint16_t signal_strength;	//�������ź�ǿ��
	float lastdis;
}Tofsense;
static Tofsense TOF[8];


/*TOFSenseM/MS�ͺŵĽ�����ݽṹ��*/
typedef struct {
	uint8_t pixel_count;		//8x8 or 4x4
	uint8_t ID;

	float dis[64];					//�����ľ�������
	uint8_t status[64];				//�������ź�״ָ̬ʾ
	uint16_t signal_strength[64];	//�������ź�ǿ��
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
