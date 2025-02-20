#include "tofsense.h"

#define mode_flag  2 		//0:�������ģʽ   1:��ѯ���ģʽ    2:����ģʽ
float sig1,sig2,sig3,sig4=0;
float status1,status2,status3,status4=0;
double MidFilter(double NewData);
/******************************************************************************
�������� �� Tof_Unpack_Data
�������� �� �����յ�ģ���һ֡���ݴ������������������������ݴ���ṹ�������
��    �� �� ��
�� �� ֵ �� ��
���÷������ڿ����ж��е���Tof_Unpack_Data();���и�ֵ
֧��ģ�飺TOFSense/TOFSense-P/PS/F/FP
*******************************************************************************/
void Tof_Unpack_Data()
{
	uint8_t tof_flag,tof_id;
	tof_flag = g_nts_frame0.UnpackData(u_rx_buf,TOF_data_length);		//��ȡ�����־λ
	tof_id = g_nts_frame0.result.id;									//��ȡ��֡���ݵ�ģ��ID
	if(tof_flag == 1){													//����ɹ�
		TOF[tof_id].ID = tof_id;
		TOF[tof_id].signal_strength = g_nts_frame0.result.signal_strength;
		TOF[tof_id].status = g_nts_frame0.result.dis_status;
		
		// // ���״̬Ϊ0�Ҿ�����[10,2000]֮�䣬���ɼ�ֵ�����������һ�βɼ�ֵ������ֱ��ʹ����һ�βɼ�ֵ
		// if((!TOF[tof_id].status)&&(g_nts_frame0.result.dis<2000)){
		// 	TOF[tof_id].dis = g_nts_frame0.result.dis;
		// 	TOF[tof_id].lastdis=TOF[tof_id].dis;
		// }
		// else{
		// 	TOF[tof_id].dis = TOF[tof_id].lastdis;
		// }
		if( ( TOF[tof_id].status==0 || TOF[tof_id].status==4 || TOF[tof_id].status==11 || TOF[tof_id].status==12) && g_nts_frame0.result.dis<4 && g_nts_frame0.result.dis>0){
			TOF[tof_id].dis = g_nts_frame0.result.dis;
		} 
		//else TOF[tof_id].dis = -0.001;
		// �����ݽ����˲�
		//TOF[tof_id].dis = MidFilter(TOF[tof_id].dis);
	}
	dis1=TOF[1].dis*1000;
	dis2=TOF[2].dis*1000;
	dis3=TOF[3].dis*1000;
	dis4=TOF[4].dis*1000;
	sig1=TOF[1].signal_strength;
	sig2=TOF[2].signal_strength;
	sig3=TOF[3].signal_strength;
	sig4=TOF[4].signal_strength;
	status1=TOF[1].status;
	status2=TOF[2].status;
	status3=TOF[3].status;
	status4=TOF[4].status;
	
}
/******************************************************************************
�������� �� TofM_Unpack_Data
�������� �� �����յ�ģ���һ֡���ݴ������������������������ݴ���ṹ�������
��    �� �� ��
�� �� ֵ �� ��
���÷������ڿ����ж��е���TofM_Unpack_Data();���и�ֵ
֧��ģ�飺TOFSense-M/MS
*******************************************************************************/
void TofM_Unpack_Data()
{
	uint8_t tofm_flag,tofm_id;

	tofm_flag = g_ntsm_frame0.UnpackData(u_rx_buf,TOF_data_length);		//��ȡ�����־λ
	tofm_id = g_ntsm_frame0.id;											//��ȡ��֡���ݵ�ģ��ID
	if(tofm_flag == 1)													//����ɹ�
	{
		TOF_M[tofm_id].ID = tofm_id;
		TOF_M[tofm_id].pixel_count = g_ntsm_frame0.pixel_count;
		for(int i=0;i<TOF_M[tofm_id].pixel_count;i++)
		{
			TOF_M[tofm_id].dis[i] = g_ntsm_frame0.pixels[i].dis;
			TOF_M[tofm_id].status[i] = g_ntsm_frame0.pixels[i].dis_status;
			TOF_M[tofm_id].signal_strength[i] = g_ntsm_frame0.pixels[i].signal_strength;
		}
	}
}



void HAL_Delay_ms(uint16_t nms)	   //����0-8191ms
{
		__HAL_TIM_SetCounter(&htim2, 0);//htim1

		__HAL_TIM_ENABLE(&htim2);

		while(__HAL_TIM_GetCounter(&htim2) < (nms));
		/* Disable the Peripheral */
		__HAL_TIM_DISABLE(&htim2);
}

// ��λ������ֵ�˲�
double FilterStack[3] = {0};
double MidFilter(double NewData)
{
	FilterStack[0] = FilterStack[1];
	FilterStack[1] = FilterStack[2];
	FilterStack[2] = NewData;
	if(FilterStack[0] > FilterStack[1])
	{
		if(FilterStack[1] > FilterStack[2])
			return FilterStack[1];
		else if(FilterStack[0] > FilterStack[2])
			return FilterStack[2];
		else
			return FilterStack[0];
	}
	else
	{
		if(FilterStack[0] > FilterStack[2])
			return FilterStack[0];
		else if(FilterStack[1] > FilterStack[2])
			return FilterStack[2];
		else
			return FilterStack[1];
	}
}
