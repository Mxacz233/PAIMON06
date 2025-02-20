/******************************************************************************
/// @brief
/// @copyright Copyright (c) 2017 <dji-innovations, Corp. RM Dept.>
/// @license MIT License
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction,including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense,and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished
/// to do so,subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
/// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
*******************************************************************************/

#include "can.h"
#include "bsp_can.h"


moto_measure_t moto_chassis[4] = {0};//4 chassis moto
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t              chassis_can_send_data[8];

void get_total_angle(moto_measure_t *p);
void get_moto_offset(moto_measure_t *ptr, uint8_t Data[8]);

/*******************************************************************************************
  * @Func		my_can_filter_init
  * @Brief    CAN1和CAN2滤波器配置
  * @Param		CAN_HandleTypeDef* hcan
  * @Retval		None
  * @Date     2015/11/30
 *******************************************************************************************/
extern CAN_HandleTypeDef hcan;


void filter_to_can_1(void)
{
    // add filter_0 to can_1
	HAL_StatusTypeDef HAL_Status;
	CAN_FilterTypeDef CAN_Filter;
	
	CAN_Filter.FilterBank = 0;	                    
	CAN_Filter.FilterMode = CAN_FILTERMODE_IDMASK; 	
	CAN_Filter.FilterScale= CAN_FILTERSCALE_32BIT; 	// 32BIT WIDTH
	CAN_Filter.FilterIdHigh = 0x0000;	            // 32BIT ID
	CAN_Filter.FilterIdLow  = 0x0000;
	CAN_Filter.FilterMaskIdHigh = 0x0000;           // 32BIT MASK
	CAN_Filter.FilterMaskIdLow  = 0x0000;
	CAN_Filter.FilterFIFOAssignment = CAN_FILTER_FIFO1;  //filter_0 -->  FIFO0
    CAN_Filter.FilterActivation = DISABLE;           // NOTE: deactivate it if not used!
	if( HAL_OK != HAL_CAN_ConfigFilter(&hcan1, &CAN_Filter))
	{
		Error_Handler();
	}

	HAL_Status = HAL_CAN_Start(&hcan1); 
	if(HAL_Status!=HAL_OK){
		Error_Handler();
	}	

	// Activate CAN RX notification (HERE WE attach FIFO0 to CAN1) called "CAN0" in manual
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{    
		Error_Handler();
	}
 
	// Activate CAN TX notification
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
		Error_Handler();
	}	
}


void filter_to_can_2(void)
{
	// add filter_0 to can_1
	HAL_StatusTypeDef HAL_Status;
	CAN_FilterTypeDef CAN_Filter;
	
	CAN_Filter.FilterBank = 15;	                    // <=27       
	CAN_Filter.FilterMode = CAN_FILTERMODE_IDMASK; 	
	CAN_Filter.FilterScale= CAN_FILTERSCALE_32BIT; 	// 32BIT WIDTH
	CAN_Filter.FilterIdHigh = 0x0000;	            // 32BIT ID
	CAN_Filter.FilterIdLow  = 0x0000;
	CAN_Filter.FilterMaskIdHigh = 0x0000;           // 32BIT MASK
	CAN_Filter.FilterMaskIdLow  = 0x0000;
	CAN_Filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;  //filter_0 -->  FIFO0
	CAN_Filter.FilterActivation = ENABLE;
	CAN_Filter.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&hcan2, &CAN_Filter);
	HAL_Status = HAL_CAN_Start(&hcan2);
	if(HAL_Status!=HAL_OK){
		Error_Handler();
	}	
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);	
 

}

//void can_filter_init(void)
//{
//    CAN_FilterTypeDef can_filter_st;
//    can_filter_st.FilterActivation = ENABLE;
//    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
//    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
//    can_filter_st.FilterIdHigh = 0x0000;
//    can_filter_st.FilterIdLow = 0x0000;
//    can_filter_st.FilterMaskIdHigh = 0x0000;
//    can_filter_st.FilterMaskIdLow = 0x0000;
//    can_filter_st.FilterBank = 0;
//	can_filter_st.SlaveStartFilterBank = 14;
//    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
//    HAL_CAN_ConfigFilter(&hcan2, &can_filter_st);
//    HAL_CAN_Start(&hcan2);
//    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
//}

//void my_can_filter_init_recv_all(CAN_HandleTypeDef* _hcan)
//{
//	//can1 &can2 use same filter config
//	CAN_FilterTypeDef		CAN_FilterConfigStructure;
//	static CanTxMsgTypeDef		Tx1Message;
//	static CanRxMsgTypeDef 		Rx1Message;

//	CAN_FilterConfigStructure.FilterMode = CAN_FILTERMODE_IDMASK;
//	CAN_FilterConfigStructure.FilterScale = CAN_FILTERSCALE_32BIT;
//	CAN_FilterConfigStructure.FilterIdHigh = 0x0000;
//	CAN_FilterConfigStructure.FilterIdLow = 0x0000;
//	CAN_FilterConfigStructure.FilterMaskIdHigh = 0x0000;
//	CAN_FilterConfigStructure.FilterMaskIdLow = 0x0000;
//	CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
//	CAN_FilterConfigStructure.FilterBank = 14;//can1(0-13)和can2(14-27)分别得到一半的filter
//	CAN_FilterConfigStructure.FilterActivation = ENABLE;

//	if(HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
//	{
//		//err_deadloop(); //show error!
//	}


//	if(_hcan == &hcan){
//		_hcan->pTxMsg = &Tx1Message;
//		_hcan->pRxMsg = &Rx1Message;
//	}


//}

uint32_t FlashTimer;
/*******************************************************************************************
  * @Func			void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* _hcan)
  * @Brief    HAL库中标准的CAN接收完成回调函数，需要在此处理通过CAN总线接收到的数据
  * @Param		
  * @Retval		None 
  * @Date     2015/11/24
 *******************************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

		//ignore can1 or can2.
	switch(rx_header.StdId)
	{
		case CAN_2006Moto1_ID:
		case CAN_2006Moto2_ID:
		case CAN_2006Moto3_ID:
		case CAN_2006Moto4_ID:
			{
				static u8 i;
				i = rx_header.StdId - CAN_2006Moto1_ID;			
				get_moto_measure(&moto_chassis[i], rx_data);
			}
			break;		
	}
}

/*******************************************************************************************
  * @Func			void get_moto_measure(moto_measure_t *ptr, CAN_HandleTypeDef* hcan)
  * @Brief    接收2006电机通过CAN发过来的信息
  * @Param		
  * @Retval		None
  * @Date     2015/11/24
 *******************************************************************************************/
void get_moto_measure(moto_measure_t *ptr, uint8_t Data[8])
{

	ptr->last_angle = ptr->angle;
	ptr->angle = (uint16_t)(Data[0]<<8 | Data[1]) ;
	ptr->speed_rpm  = (int16_t)(Data[2]<<8 | Data[3]);
	ptr->real_current = (Data[4]<<8 | Data[5])*5.f/16384.f;

	ptr->hall = Data[6];
	
	
	if(ptr->angle - ptr->last_angle > 4096)
		ptr->round_cnt --;
	else if (ptr->angle - ptr->last_angle < -4096)
		ptr->round_cnt ++;
	ptr->total_angle = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle;
}

/*this function should be called after system+can init */
void get_moto_offset(moto_measure_t *ptr, uint8_t Data[8])
{
	ptr->angle = (uint16_t)(Data[0]<<8 | Data[1]) ;
	ptr->offset_angle = ptr->angle;
}

#define ABS(x)	( (x>0) ? (x) : (-x) )
/**
*@bref 电机上电角度=0， 之后用这个函数更新3510电机的相对开机后（为0）的相对角度。
	*/
void get_total_angle(moto_measure_t *p){
	
	int res1, res2, delta;
	if(p->angle < p->last_angle){			//可能的情况
		res1 = p->angle + 8192 - p->last_angle;	//正转，delta=+
		res2 = p->angle - p->last_angle;				//反转	delta=-
	}else{	//angle > last
		res1 = p->angle - 8192 - p->last_angle ;//反转	delta -
		res2 = p->angle - p->last_angle;				//正转	delta +
	}
	//不管正反转，肯定是转的角度小的那个是真的
	if(ABS(res1)<ABS(res2))
		delta = res1;
	else
		delta = res2;

	p->total_angle += delta;
	p->last_angle = p->angle;
}

void set_moto_current(CAN_HandleTypeDef* hcan, s16 iq1, s16 iq2, s16 iq3, s16 iq4)
{
	uint32_t box;
	
	chassis_tx_message.TransmitGlobalTime = DISABLE;
	chassis_tx_message.StdId = 0x200;
	chassis_tx_message.IDE = CAN_ID_STD;
	chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
	chassis_can_send_data[0] = (iq1 >> 8);
	chassis_can_send_data[1] = iq1;
	chassis_can_send_data[2] = (iq2 >> 8);
	chassis_can_send_data[3] = iq2;
	chassis_can_send_data[4] = (iq3 >> 8);
	chassis_can_send_data[5] = iq3;
	chassis_can_send_data[6] = (iq4 >> 8);
	chassis_can_send_data[7] = iq4;
	
	HAL_CAN_AddTxMessage(hcan,&chassis_tx_message,chassis_can_send_data,&box);
}

//void motor_init_4ch()
//{
//	  for(int i=0; i<4; i++)
//  {	
//    pid_init(&motor_pid[i]);
//    motor_pid[i].f_param_init(&motor_pid[i],PID_Speed,16384,5000,10,0,8000,0,1.5,0.1,0); 
//  }
//}

//void M2006_motion_vector(int x_speed,int y_speed,int turn_speed)
//{
//	//麦轮位置解算
//	motor_pid[0].target = 0 + x_speed + y_speed + turn_speed;  
//	motor_pid[1].target = 0 + x_speed - y_speed + turn_speed;
//	motor_pid[2].target = 0 - x_speed - y_speed + turn_speed;
//	motor_pid[3].target = 0 - x_speed + y_speed + turn_speed;
//	
//	//速度环PID（根据设定值进行PID计算）
//	motor_pid[0].f_cal_pid(&motor_pid[0],moto_chassis[0].speed_rpm);   
//	motor_pid[1].f_cal_pid(&motor_pid[1],moto_chassis[1].speed_rpm);   
//	motor_pid[2].f_cal_pid(&motor_pid[2],moto_chassis[2].speed_rpm);   
//	motor_pid[3].f_cal_pid(&motor_pid[3],moto_chassis[3].speed_rpm);   
//	
//	//将PID的计算结果通过CAN发送到电机
//	set_moto_current(&hcan1, motor_pid[0].output,  
//							 motor_pid[1].output,
//                             motor_pid[2].output,
//                             motor_pid[3].output);
//}
