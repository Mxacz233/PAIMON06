/*-------------------------------------������˵��-------------------------------------

	RxBuffer[1]Ϊ�������ֵ��

	0x01   ��all��	���ճɹ��ظ��ź�

	0xa1   ����2�£�ͨ�ų�ʼ��
	0xb1   ����2�£���ͣ���ÿ����ź�
	0xc1   ����2�£�·����·������
	0xc2   ����2�£�·����Ϣ���н�����·���ܳ���ֹͣʱ���룩
	0xd0   ����2�£�ײ���źţ�ײ������
	0xd1		(��2��) ����ͷ�ź�

	0x21   ����2�ϣ������ⱦ���ź�
	0x31   ����2�ϣ����͵�ǰλ�ã���ǰ����·�κţ���������˵ķ��������˷�����ǽ�ľ��룩

-------------------------------------������˵������-----------------------------------*/




#include "bsp-protocol.h"

uint8_t RxBuffer[8];
uint8_t RxFlag = 0;
uint8_t ErrFlag = 0;

uint8_t VerifyVal = 0;

uint8_t CheckTreasureFlag = 0;
uint8_t SendPositionFlag = 0;
uint8_t TempPosition[3]={0};//TP[0]������С��λ�ڵڼ�·��,TP[1]�Ǿ����ĸ�����TP[2]���ٸ�������
//-----------���ճɹ��ظ���־λ--------------
uint8_t Flag_0x21 = 0;		//�����ⱦ���ź�
uint8_t Flag_0x31 = 0;		//���͵�ǰλ�ã���ǰ����·�κţ���������˵ķ��������˷�����ǽ�ľ��룩
//-------------------------------------------


//---------Э�������־λ������λ------------
uint8_t StopFlagUpper = 0;
uint8_t RoadsNumUpper = 0;  //ֵ����·������������Ҫ��1
uint8_t CrashFlagUpper = 0;
uint8_t CrashDirectionUpper = 0;
uint16_t RoadInfoUpper[4][50] = {0};
uint8_t RoadInfoFinishFlag = 0;//����·����Ϣ���
uint8_t CameraDirTemp = 0;
uint8_t CameraDirPro = 0;
uint8_t GimbalDirTemp = 0;
//-------------------------------------------


//---------Э�鷢�ͱ�־λ������λ------------
uint8_t SendBuffer[8] = {0};
//-------------------------------------------

uint8_t ReceiveRxBuffer[8];
uint8_t ReceiveRxTempBuffer[8];
uint8_t ReceiveOKFlag = 0;
uint8_t TempCrushFlag = 0;

char test001[80];

void CommReceiveInit(void)
{
	HAL_UART_Receive_DMA(&huart1,(uint8_t*)RxBuffer,8);
}


void protocol_analysis(void)
{
	if(RxFlag)
	{
	RxFlag = 0;//�����־
		
	//-------����λ�����ͽ��ճɹ��źŵı�����ʼ��-----

	ReceiveRxBuffer[0] = 0xaa;
	ReceiveRxBuffer[1] = 0x01;
	ReceiveRxBuffer[2] = 0x00;
	ReceiveRxBuffer[3] = 0x00;
	ReceiveRxBuffer[4] = 0x00;
	ReceiveRxBuffer[5] = 0x00;
	ReceiveRxBuffer[6] = 0x00;
	ReceiveRxBuffer[7] = 0x55;
	//----����λ�����ͽ��ճɹ��źŵı�����ʼ������----
	
	
//------------------------------------------ͨ��Э�����---------------------------------------------------------------
	if(RxBuffer[0]==0xaa)						//�ж�֡ͷ
	{
	VerifyVal = 0x00;
	//����У���-------------
	for(uint8_t i=0;i<7;i++)
	{
		VerifyVal += RxBuffer[i];
	}
	VerifyVal = 0xff - (*(uint8_t*)(&VerifyVal));
	//����У��ͽ���---------
	if(VerifyVal == RxBuffer[7])				//�ж�16����У���
	{	
		/*-------------------------------------������˵��-------------------------------------

			RxBuffer[1]Ϊ�������ֵ��
			
			0x01   ��all��	���ճɹ��ظ��ź�
			
			0xa1   ����2�£�ͨ�ų�ʼ��
			0xb1   ����2�£���ͣ���ÿ����ź�
			0xc1   ����2�£�·����·������
			0xc2   ����2�£�·����Ϣ���н�����·���ܳ���ֹͣʱ���룩
			0xd0   ����2�£�ײ���źţ�ײ������
			0xd1		(��2��) ����ͷ�ź�
			
			0x21   ����2�ϣ������ⱦ���ź�
			0x31   ����2�ϣ����͵�ǰλ�ã���ǰ����·�κţ���������˵ķ��������˷�����ǽ�ľ��룩
			
		---------------------------------------������˵������---------------------------------*/
		
		
		
		if(RxBuffer[1]==0x01)			//�����ճɹ��ظ��źš�							�������ж�
		{
			if(RxBuffer[2]==0x21) 		Flag_0x21 = 1;//�Է��յ������ⱦ���ź�
			else if(RxBuffer[2]==0x31) 	Flag_0x31 = 1;
			else ErrFlag = 1;
		}
		else if(RxBuffer[1]==0xa1)		//��ͨ�ų�ʼ����								�������ж�
		{
			ReceiveRxBuffer[2] = 0xa1;
			ReceiveOKFlag = 1;
		}
		else if(RxBuffer[1]==0xb1)		//����ͣ���ÿ����źš�							�������ж�
		{
			ReceiveRxBuffer[2] = 0xb1;
			ReceiveOKFlag = 1;

			StopFlagUpper = 1;			//��ͣ���ñ�־λ��1���ǵ�ִ�����������ñ�־λ��
		}
		else if(RxBuffer[1]==0xc1)		//��·����·��������							�������ж�
		{
			ReceiveRxBuffer[2] = 0xc1;
			ReceiveOKFlag = 1;	
			RoadsNumUpper = RxBuffer[2];//RxBuffer[2]Ϊ·����·��������ֵ	
		}
		else if(RxBuffer[1]==0xd0)		//��ײ���źţ�ײ�����򣩡�					�������ж�
		{
			ReceiveRxBuffer[2] = 0xd0;
//		CrashDirectionUpper = RxBuffer[2];
			ReceiveOKFlag = 1;
			CrashFlagUpper = 1;			//ײ����־λ��1���ǵ�ִ�����������ñ�־λ��

		}
		else if(RxBuffer[1]==0xd1)		//������ͷ����					�������ж�
		{
			ReceiveRxBuffer[2] = 0xd1;
			ReceiveOKFlag = 1;
			CameraDirTemp = RxBuffer[2];//RxBuffer[2]Ϊ����ͷ����

		}
		else if(RxBuffer[1]==0xe0)
		{
			ReceiveRxBuffer[2] = 0xe0;
			ReceiveOKFlag = 1;
			GimbalDirTemp = RxBuffer[2];
			if			 	(GimbalDirTemp == 0)		PlatformConAbsAng(INIT_DEGREE - 18000);				
			else if 	(GimbalDirTemp == 1)		PlatformConAbsAng(INIT_DEGREE - 9000);	
			else if	 	(GimbalDirTemp == 2)		PlatformConAbsAng(INIT_DEGREE);	
			else if	 	(GimbalDirTemp == 3)		PlatformConAbsAng(INIT_DEGREE + 9000);  
		
		}
		else if(RxBuffer[1]==0xc2)		//��·����Ϣ���н�����·���ܳ���ֹͣʱ���룩���������ж�
		{
			ReceiveRxBuffer[2] = 0xc2;
			for(int i=0;i < RoadsNumUpper;i++)	//��·����Ϣ��������ж�
			{
				if(RxBuffer[2]==i)
				{
					ReceiveRxBuffer[3] = RxBuffer[2];
					ReceiveOKFlag = 1;
					RoadInfoUpper[0][i] = RxBuffer[3];//�н�����0123�ֱ��ʾǰ�����
					RoadInfoUpper[1][i] = RxBuffer[4];//ָ��С��ֹͣ�Ļ�׼������ķ���0123�ֱ��ʾǰ�����
					RoadInfoUpper[2][i] = RxBuffer[5];//ֹͣʱ���룬��λ��С����
					RoadInfoUpper[3][i] = RxBuffer[6];//��һ֡��㴦��෽�����ǽ���룬��λ��С������ʶͼʱ�ĸ��Ӵ�С��
					
					if(i == 0)   				//������Ҫײ�����������
					{
						if(RoadInfoUpper[1][0] > 3)
						{
							RoadInfoUpper[1][0] = RoadInfoUpper[1][0] - 4;
							TempCrushFlag = 1;
						}
						if			(RoadInfoUpper[0][0] == 0) *RunFrame = 'F';
						else if	(RoadInfoUpper[0][0] == 1) *RunFrame = 'L';
						else if	(RoadInfoUpper[0][0] == 2) *RunFrame = 'B';
						else if	(RoadInfoUpper[0][0] == 3) *RunFrame = 'R';
						
						if			(RoadInfoUpper[1][0] == 0) *MapFrame = 'F';
						else if	(RoadInfoUpper[1][0] == 1) *MapFrame = 'L';
						else if	(RoadInfoUpper[1][0] == 2) *MapFrame = 'B';
						else if	(RoadInfoUpper[1][0] == 3) *MapFrame = 'R';

						*BlockFrame				= RoadInfoUpper[2][0] + '0';
						*ChangeDis				= RoadInfoUpper[3][0] + '0';
					}
					
					if(i==(RoadsNumUpper - 1))//����ĩβ֡
					{
						if(RoadInfoUpper[1][RoadsNumUpper - 1] > 3)  	//���һ֡�ǲ���֡
						{
							CameraDirPro = RoadInfoUpper[1][RoadsNumUpper - 1] - 4;
							
							//��ʱת�����
							if			 	(CameraDirPro == 0)		PlatformConAbsAng(INIT_DEGREE);					//��෽����ǰ������ͷת���
							else if 	(CameraDirPro == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//��෽����������ͷת���ҷ�
							else if	 	(CameraDirPro == 2)		PlatformConAbsAng(INIT_DEGREE - 18000);	//��෽���Ǻ�����ͷת��ǰ��
							else if	 	(CameraDirPro == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //��෽�����ң�����ͷת����
							RoadInfoUpper[0][RoadsNumUpper - 1] = 4;
							RoadInfoUpper[1][RoadsNumUpper - 1] = 4;
							RoadInfoUpper[2][RoadsNumUpper - 1] = 0;
							RoadInfoUpper[3][RoadsNumUpper - 1] = 0;
							RoadInfoUpper[0][RoadsNumUpper] = 4;
							RoadInfoUpper[1][RoadsNumUpper] = 4;
							if(RoadsNumUpper == 1)
							{
								*RunFrame = 'S';
								*MapFrame = 'S';
								*BlockFrame = '0';
								*ChangeDis = '0';
							}
						}
						else																					//���һ֡������֡
						{
							CameraDirPro = RoadInfoUpper[1][RoadsNumUpper - 1];
							
							//��ʱת�����
							if			 	(CameraDirPro == 0)		PlatformConAbsAng(INIT_DEGREE);					//��෽����ǰ������ͷת���
							else if 	(CameraDirPro == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//��෽����������ͷת���ҷ�
							else if	 	(CameraDirPro == 2)		PlatformConAbsAng(INIT_DEGREE - 18000);	//��෽���Ǻ�����ͷת��ǰ��
							else if	 	(CameraDirPro == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //��෽�����ң�����ͷת����
							RoadInfoUpper[0][RoadsNumUpper] = 4;
							RoadInfoUpper[1][RoadsNumUpper] = 4;
							RoadInfoUpper[2][RoadsNumUpper] = 0;
							RoadInfoUpper[3][RoadsNumUpper] = 0;
							RoadInfoUpper[0][RoadsNumUpper + 1] = 4;
							RoadInfoUpper[1][RoadsNumUpper + 1] = 4;
						}
					
					
					}
					
					if(i == 1)					//��ʼ�����ؽ�����־
					{
						if			(RoadInfoUpper[0][0] == 0) *RunFrame = 'F';
						else if	(RoadInfoUpper[0][0] == 1) *RunFrame = 'L';
						else if	(RoadInfoUpper[0][0] == 2) *RunFrame = 'B';
						else if	(RoadInfoUpper[0][0] == 3) *RunFrame = 'R';
						else if (RoadInfoUpper[0][0] == 4) *RunFrame = 'S';
						
						if			(RoadInfoUpper[1][0] == 0) *MapFrame = 'F';
						else if	(RoadInfoUpper[1][0] == 1) *MapFrame = 'L';
						else if	(RoadInfoUpper[1][0] == 2) *MapFrame = 'B';
						else if	(RoadInfoUpper[1][0] == 3) *MapFrame = 'R';
						else if (RoadInfoUpper[1][0] == 4) *MapFrame = 'S';

						*BlockFrame				= RoadInfoUpper[2][0] + '0';
						*ChangeDis				= RoadInfoUpper[3][0] + '0';

						if			(RoadInfoUpper[0][1] == 0) *NextMapFrame = 'F';
						else if	(RoadInfoUpper[0][1] == 1) *NextMapFrame = 'L';
						else if	(RoadInfoUpper[0][1] == 2) *NextMapFrame = 'B';
						else if	(RoadInfoUpper[0][1] == 3) *NextMapFrame = 'R';
						else if	(RoadInfoUpper[0][1] == 4) *NextMapFrame = 'S';
						
						//==============================��̨���෽��ת��===================================
//						if 			(*MapFrame == 'F')	PlatformConAbsAng(INIT_DEGREE + 18000);
//						else if (*MapFrame == 'L')	PlatformConAbsAng(INIT_DEGREE + 27000);
//						else if (*MapFrame == 'B')	PlatformConAbsAng(INIT_DEGREE);
//						else if (*MapFrame == 'R')	PlatformConAbsAng(INIT_DEGREE + 9000);  
						//==============================��̨���෽��ת��end===================================
					}
					
					
					
					
					
					
					
					if(RoadsNumUpper > 4)  //��ǰ����
					{
						if(i == 3) RoadInfoFinishFlag = 1;//·����Ϣ������ɣ��ǵ����������־λ��
					}
					else
					{
						if(i == (RoadsNumUpper - 1)) RoadInfoFinishFlag = 1;//·����Ϣ������ɣ��ǵ����������־λ��
					}
					
				}
//				if(i==(RoadsNumUpper - 1) && RxBuffer[2]==(RoadsNumUpper - 1))
//				{

//					encodeRoads();
//					RoadInfoFinishFlag = 1;//·����Ϣ������ɣ��ǵ����������־λ��
//				}
				
			}
		}
		else ErrFlag = 1; //���������	
	}
	
	else ErrFlag = 1;	  //У��ʹ���
	VerifyVal = 0;
	}
	else ErrFlag = 1;	  //֡ͷ����
//-------------------------------------------ͨ��Э���������----------------------------------------------------------
	
	
	
	if(ReceiveOKFlag == 1)
	{
		//����У���-------------
		ReceiveRxBuffer[7] = 0x00;
		for(uint8_t i=0;i<7;i++)
		{
			ReceiveRxBuffer[7] += ReceiveRxBuffer[i];
		}
		ReceiveRxBuffer[7] = 0xff - (*(uint8_t*)(&ReceiveRxBuffer[7]));
		//����У��ͽ���---------
		for(int i=0;i<8;i++)
		{
			ReceiveRxTempBuffer[i] = ReceiveRxBuffer[i];
		}
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)ReceiveRxTempBuffer,8); //���ؽ��ճɹ��ظ��ź�
		for(int i=0;i<8;i++)
		{
			ReceiveRxBuffer[i]=0;
		}
		ReceiveOKFlag = 0;
	}
	}
}



void sendUART(void) //���ݷ���,����500Hz��������Ҳ�У���
{
	if(CheckTreasureFlag == 1 && Flag_0x21 == 0)		//��������ⱦ�ر�־λΪ1���������ⱦ�ػظ���δ�ɹ����ͼ�������
	{
		SendBuffer[0] = 0xaa;
		SendBuffer[1] = 0x21;
		SendBuffer[2] = 0x00;
		SendBuffer[3] = 0x00;
		SendBuffer[4] = 0x00;
		SendBuffer[5] = 0x00;
		SendBuffer[6] = 0x00;
		//����У���-------------
		SendBuffer[7] = 0;
		for(uint8_t i=0;i<7;i++)
		{
			SendBuffer[7] += SendBuffer[i];
		}
		SendBuffer[7] = 0xff - (*(uint8_t*)(&SendBuffer[7]));
		//����У��ͽ���---------
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuffer,8);
	}
	if(Flag_0x21 == 1) 				//��������ⱦ�ػظ��ɹ����ͽ�CheckTreasureFlag��0���ظ���־λҲ��0
	{	
		CheckTreasureFlag = 0;
		Flag_0x21 = 0;
	}
	if(SendPositionFlag == 1 && Flag_0x31 == 0)		//������͵�ǰλ�ñ�־λΪ1���ҷ��͵�ǰλ�ûظ���δ�ɹ����ͼ�������
	{
		SendBuffer[0] = 0xaa;
		SendBuffer[1] = 0x31;
		SendBuffer[2] = TempPosition[0];
		SendBuffer[3] = TempPosition[1];
		SendBuffer[4] = TempPosition[2];
		SendBuffer[5] = 0x00;
		SendBuffer[6] = 0x00;
		//����У���-------------
		SendBuffer[7] = 0;
		for(uint8_t i=0;i<7;i++)
		{
			SendBuffer[7] += SendBuffer[i];
		}
		SendBuffer[7] = 0xff - (*(uint8_t*)(&SendBuffer[7]));
		//����У��ͽ���---------
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuffer,8);
	}
	if(Flag_0x31 == 1) 				//��������ⱦ�ػظ��ɹ����ͽ�CheckTreasureFlag��0���ظ���־λҲ��0
	{								//��μǵ�ҲҪ�ŵ����SendPositionFlag��־λ�ĵط�ȥ����Ȼ��־λ�ᱻ������1��������������������������������
		SendPositionFlag = 0;
		Flag_0x31 = 0;
	}

}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		RxFlag = 1;
	}
}

//char* encodeRoads(void) 
//{

//  char dir[4] = {'F', 'L', 'B', 'R'}; 
//  char* result = malloc(RoadsNumUpper * 4 + 3);
//  char* ptr = result;

//  for (int i = 0; i < RoadsNumUpper; i++) {
//    *ptr++ = dir[RoadInfoUpper[0][i]]; 
//    sprintf(ptr, "%d", RoadInfoUpper[2][i]);
//    ptr += strlen(ptr); 
//  }

//  strcpy(ptr, "S0");

//  return result;
//}

void encodeRoads(void) 
{
//	if(RoadInfoUpper[1][0] > 3)
//	{
//		RoadInfoUpper[1][0] = RoadInfoUpper[1][0] - 4;
//		TempCrushFlag = 1;
//	}
  char dir[4] = {'F', 'L', 'B', 'R'};
  memset(map, 0, sizeof(map));
  int i, j = 0;
  for(i=0; i<RoadsNumUpper; i++) 
  {
    map[j] = dir[RoadInfoUpper[0][i]];//ǰ������
		j++;
		map[j] = dir[RoadInfoUpper[1][i]];//��෽��
    j++;
		//ԭ����ʣ������Ǵ������Ҫ����2���������ڶ���С��Ϊ��λ
    sprintf(map + j, "%x", RoadInfoUpper[2][i]);
    j += strlen(map + j);
		sprintf(map + j, "%x", RoadInfoUpper[3][i]); //�����˵ڶ�������
		j += strlen(map + j);
  }

  strcpy(map + j, "SS00SS"); 
}
