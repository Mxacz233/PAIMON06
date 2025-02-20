/*-------------------------------------功能码说明-------------------------------------

	RxBuffer[1]为功能码的值。

	0x01   （all）	接收成功回复信号

	0xa1   （上2下）通信初始化
	0xb1   （上2下）急停避让控制信号
	0xc1   （上2下）路径内路段总数
	0xc2   （上2下）路径信息（行进方向、路段总长和停止时距离）
	0xd0   （上2下）撞击信号（撞击方向）
	0xd1		(上2下) 摄像头信号

	0x21   （下2上）请求检测宝藏信号
	0x31   （下2上）发送当前位置（当前所在路段号，测距正常端的方向，正常端方向离墙的距离）

-------------------------------------功能码说明结束-----------------------------------*/




#include "bsp-protocol.h"

uint8_t RxBuffer[8];
uint8_t RxFlag = 0;
uint8_t ErrFlag = 0;

uint8_t VerifyVal = 0;

uint8_t CheckTreasureFlag = 0;
uint8_t SendPositionFlag = 0;
uint8_t TempPosition[3]={0};//TP[0]是现在小车位于第几路段,TP[1]是距离哪个方向TP[2]多少个两厘米
//-----------接收成功回复标志位--------------
uint8_t Flag_0x21 = 0;		//请求检测宝藏信号
uint8_t Flag_0x31 = 0;		//发送当前位置（当前所在路段号，测距正常端的方向，正常端方向离墙的距离）
//-------------------------------------------


//---------协议解析标志位与数据位------------
uint8_t StopFlagUpper = 0;
uint8_t RoadsNumUpper = 0;  //值就是路段总数，不需要加1
uint8_t CrashFlagUpper = 0;
uint8_t CrashDirectionUpper = 0;
uint16_t RoadInfoUpper[4][50] = {0};
uint8_t RoadInfoFinishFlag = 0;//接收路段信息完成
uint8_t CameraDirTemp = 0;
uint8_t CameraDirPro = 0;
uint8_t GimbalDirTemp = 0;
//-------------------------------------------


//---------协议发送标志位与数据位------------
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
	RxFlag = 0;//清除标志
		
	//-------向上位机发送接收成功信号的变量初始化-----

	ReceiveRxBuffer[0] = 0xaa;
	ReceiveRxBuffer[1] = 0x01;
	ReceiveRxBuffer[2] = 0x00;
	ReceiveRxBuffer[3] = 0x00;
	ReceiveRxBuffer[4] = 0x00;
	ReceiveRxBuffer[5] = 0x00;
	ReceiveRxBuffer[6] = 0x00;
	ReceiveRxBuffer[7] = 0x55;
	//----向上位机发送接收成功信号的变量初始化结束----
	
	
//------------------------------------------通信协议解析---------------------------------------------------------------
	if(RxBuffer[0]==0xaa)						//判断帧头
	{
	VerifyVal = 0x00;
	//计算校验和-------------
	for(uint8_t i=0;i<7;i++)
	{
		VerifyVal += RxBuffer[i];
	}
	VerifyVal = 0xff - (*(uint8_t*)(&VerifyVal));
	//计算校验和结束---------
	if(VerifyVal == RxBuffer[7])				//判断16进制校验和
	{	
		/*-------------------------------------功能码说明-------------------------------------

			RxBuffer[1]为功能码的值。
			
			0x01   （all）	接收成功回复信号
			
			0xa1   （上2下）通信初始化
			0xb1   （上2下）急停避让控制信号
			0xc1   （上2下）路径内路段总数
			0xc2   （上2下）路径信息（行进方向、路段总长和停止时距离）
			0xd0   （上2下）撞击信号（撞击方向）
			0xd1		(上2下) 摄像头信号
			
			0x21   （下2上）请求检测宝藏信号
			0x31   （下2上）发送当前位置（当前所在路段号，测距正常端的方向，正常端方向离墙的距离）
			
		---------------------------------------功能码说明结束---------------------------------*/
		
		
		
		if(RxBuffer[1]==0x01)			//“接收成功回复信号”							功能码判断
		{
			if(RxBuffer[2]==0x21) 		Flag_0x21 = 1;//对方收到请求检测宝藏信号
			else if(RxBuffer[2]==0x31) 	Flag_0x31 = 1;
			else ErrFlag = 1;
		}
		else if(RxBuffer[1]==0xa1)		//“通信初始化”								功能码判断
		{
			ReceiveRxBuffer[2] = 0xa1;
			ReceiveOKFlag = 1;
		}
		else if(RxBuffer[1]==0xb1)		//“急停避让控制信号”							功能码判断
		{
			ReceiveRxBuffer[2] = 0xb1;
			ReceiveOKFlag = 1;

			StopFlagUpper = 1;			//急停避让标志位置1（记得执行命令后清除该标志位）
		}
		else if(RxBuffer[1]==0xc1)		//“路径内路段总数”							功能码判断
		{
			ReceiveRxBuffer[2] = 0xc1;
			ReceiveOKFlag = 1;	
			RoadsNumUpper = RxBuffer[2];//RxBuffer[2]为路径内路段总数的值	
		}
		else if(RxBuffer[1]==0xd0)		//“撞击信号（撞击方向）”					功能码判断
		{
			ReceiveRxBuffer[2] = 0xd0;
//		CrashDirectionUpper = RxBuffer[2];
			ReceiveOKFlag = 1;
			CrashFlagUpper = 1;			//撞击标志位置1（记得执行命令后清除该标志位）

		}
		else if(RxBuffer[1]==0xd1)		//“摄像头方向”					功能码判断
		{
			ReceiveRxBuffer[2] = 0xd1;
			ReceiveOKFlag = 1;
			CameraDirTemp = RxBuffer[2];//RxBuffer[2]为摄像头方向

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
		else if(RxBuffer[1]==0xc2)		//“路径信息（行进方向、路段总长和停止时距离）”功能码判断
		{
			ReceiveRxBuffer[2] = 0xc2;
			for(int i=0;i < RoadsNumUpper;i++)	//“路径信息”序号码判断
			{
				if(RxBuffer[2]==i)
				{
					ReceiveRxBuffer[3] = RxBuffer[2];
					ReceiveOKFlag = 1;
					RoadInfoUpper[0][i] = RxBuffer[3];//行进方向，0123分别表示前左后右
					RoadInfoUpper[1][i] = RxBuffer[4];//指导小车停止的基准激光测距的方向，0123分别表示前左后右
					RoadInfoUpper[2][i] = RxBuffer[5];//停止时距离，单位是小格数
					RoadInfoUpper[3][i] = RxBuffer[6];//下一帧起点处测距方向反向距墙距离，单位是小格数（识图时的格子大小）
					
					if(i == 0)   				//处理需要撞击的特殊情况
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
					
					if(i==(RoadsNumUpper - 1))//处理末尾帧
					{
						if(RoadInfoUpper[1][RoadsNumUpper - 1] > 3)  	//最后一帧是不动帧
						{
							CameraDirPro = RoadInfoUpper[1][RoadsNumUpper - 1] - 4;
							
							//临时转向程序
							if			 	(CameraDirPro == 0)		PlatformConAbsAng(INIT_DEGREE);					//测距方向是前，摄像头转向后方
							else if 	(CameraDirPro == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//测距方向是左，摄像头转向右方
							else if	 	(CameraDirPro == 2)		PlatformConAbsAng(INIT_DEGREE - 18000);	//测距方向是后，摄像头转向前方
							else if	 	(CameraDirPro == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //测距方向是右，摄像头转向左方
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
						else																					//最后一帧是正常帧
						{
							CameraDirPro = RoadInfoUpper[1][RoadsNumUpper - 1];
							
							//临时转向程序
							if			 	(CameraDirPro == 0)		PlatformConAbsAng(INIT_DEGREE);					//测距方向是前，摄像头转向后方
							else if 	(CameraDirPro == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//测距方向是左，摄像头转向右方
							else if	 	(CameraDirPro == 2)		PlatformConAbsAng(INIT_DEGREE - 18000);	//测距方向是后，摄像头转向前方
							else if	 	(CameraDirPro == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //测距方向是右，摄像头转向左方
							RoadInfoUpper[0][RoadsNumUpper] = 4;
							RoadInfoUpper[1][RoadsNumUpper] = 4;
							RoadInfoUpper[2][RoadsNumUpper] = 0;
							RoadInfoUpper[3][RoadsNumUpper] = 0;
							RoadInfoUpper[0][RoadsNumUpper + 1] = 4;
							RoadInfoUpper[1][RoadsNumUpper + 1] = 4;
						}
					
					
					}
					
					if(i == 1)					//初始化宝藏解析标志
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
						
						//==============================云台随测距方向转向===================================
//						if 			(*MapFrame == 'F')	PlatformConAbsAng(INIT_DEGREE + 18000);
//						else if (*MapFrame == 'L')	PlatformConAbsAng(INIT_DEGREE + 27000);
//						else if (*MapFrame == 'B')	PlatformConAbsAng(INIT_DEGREE);
//						else if (*MapFrame == 'R')	PlatformConAbsAng(INIT_DEGREE + 9000);  
						//==============================云台随测距方向转向end===================================
					}
					
					
					
					
					
					
					
					if(RoadsNumUpper > 4)  //提前开跑
					{
						if(i == 3) RoadInfoFinishFlag = 1;//路段信息接收完成（记得消除这个标志位）
					}
					else
					{
						if(i == (RoadsNumUpper - 1)) RoadInfoFinishFlag = 1;//路段信息接收完成（记得消除这个标志位）
					}
					
				}
//				if(i==(RoadsNumUpper - 1) && RxBuffer[2]==(RoadsNumUpper - 1))
//				{

//					encodeRoads();
//					RoadInfoFinishFlag = 1;//路段信息接收完成（记得消除这个标志位）
//				}
				
			}
		}
		else ErrFlag = 1; //功能码错误	
	}
	
	else ErrFlag = 1;	  //校验和错误
	VerifyVal = 0;
	}
	else ErrFlag = 1;	  //帧头错误
//-------------------------------------------通信协议解析结束----------------------------------------------------------
	
	
	
	if(ReceiveOKFlag == 1)
	{
		//计算校验和-------------
		ReceiveRxBuffer[7] = 0x00;
		for(uint8_t i=0;i<7;i++)
		{
			ReceiveRxBuffer[7] += ReceiveRxBuffer[i];
		}
		ReceiveRxBuffer[7] = 0xff - (*(uint8_t*)(&ReceiveRxBuffer[7]));
		//计算校验和结束---------
		for(int i=0;i<8;i++)
		{
			ReceiveRxTempBuffer[i] = ReceiveRxBuffer[i];
		}
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)ReceiveRxTempBuffer,8); //返回接收成功回复信号
		for(int i=0;i<8;i++)
		{
			ReceiveRxBuffer[i]=0;
		}
		ReceiveOKFlag = 0;
	}
	}
}



void sendUART(void) //数据发送,放在500Hz（或慢点也行）里
{
	if(CheckTreasureFlag == 1 && Flag_0x21 == 0)		//如果请求检测宝藏标志位为1，且请求检测宝藏回复还未成功，就继续发送
	{
		SendBuffer[0] = 0xaa;
		SendBuffer[1] = 0x21;
		SendBuffer[2] = 0x00;
		SendBuffer[3] = 0x00;
		SendBuffer[4] = 0x00;
		SendBuffer[5] = 0x00;
		SendBuffer[6] = 0x00;
		//计算校验和-------------
		SendBuffer[7] = 0;
		for(uint8_t i=0;i<7;i++)
		{
			SendBuffer[7] += SendBuffer[i];
		}
		SendBuffer[7] = 0xff - (*(uint8_t*)(&SendBuffer[7]));
		//计算校验和结束---------
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuffer,8);
	}
	if(Flag_0x21 == 1) 				//如果请求检测宝藏回复成功，就将CheckTreasureFlag置0，回复标志位也置0
	{	
		CheckTreasureFlag = 0;
		Flag_0x21 = 0;
	}
	if(SendPositionFlag == 1 && Flag_0x31 == 0)		//如果发送当前位置标志位为1，且发送当前位置回复还未成功，就继续发送
	{
		SendBuffer[0] = 0xaa;
		SendBuffer[1] = 0x31;
		SendBuffer[2] = TempPosition[0];
		SendBuffer[3] = TempPosition[1];
		SendBuffer[4] = TempPosition[2];
		SendBuffer[5] = 0x00;
		SendBuffer[6] = 0x00;
		//计算校验和-------------
		SendBuffer[7] = 0;
		for(uint8_t i=0;i<7;i++)
		{
			SendBuffer[7] += SendBuffer[i];
		}
		SendBuffer[7] = 0xff - (*(uint8_t*)(&SendBuffer[7]));
		//计算校验和结束---------
		HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuffer,8);
	}
	if(Flag_0x31 == 1) 				//如果请求检测宝藏回复成功，就将CheckTreasureFlag置0，回复标志位也置0
	{								//这段记得也要放到清除SendPositionFlag标志位的地方去，不然标志位会被反复置1？？？？？？？？？？？？？？？？
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
    map[j] = dir[RoadInfoUpper[0][i]];//前进方向
		j++;
		map[j] = dir[RoadInfoUpper[1][i]];//测距方向
    j++;
		//原来的剩余格数是大格所以要除以2，但是现在都以小格为单位
    sprintf(map + j, "%x", RoadInfoUpper[2][i]);
    j += strlen(map + j);
		sprintf(map + j, "%x", RoadInfoUpper[3][i]); //新增了第二个距离
		j += strlen(map + j);
  }

  strcpy(map + j, "SS00SS"); 
}
