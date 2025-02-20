/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor_control.h"
#include "PosLock.h"
#include "BMI088driver.h"
#include "KalmanFilter.h"
#include "AngleLock.h"
#include "Liner.h"
#include "bsp-protocol.h"
#include "lk_bsp_can.h"
#include "MS40X.h"
#include "math.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define mode_flag  2 		//0:主动输出模式   1:查询输出模式    2:级联模式
#define pi 3.1415926
#define StardardSpeed 11000
#define SlowStardardSpeed 6000  //处理特殊情况时的速度
#define INIT_DEGREE 5500        //别忘了bsp-protocol里还有一样的define
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int32_t set_spd = 2000;
uint8_t DirFlag=0;

float dis1,dis2,dis3,dis4=0;

uint8_t TOF_ID = 1;			//仅用作级联时更改查询命令
uint8_t u_tx_buf[8] = {0x57,0x10,0xff,0xff,0x00,0xff,0xff,0x63};  //查询命令
uint8_t u_rx_buf[400];

uint32_t TOF_data_length=0;		//DMA接收数据长度
void TOFRefresh(void);

uint8_t flag=0;
uint8_t SlowSpeedFlag = 0;
//int32_t set_spd = normspeed;
int32_t tar = 90;
fp32 gyro[3], accel[3], temp;
double yaw=0;
float timeflag=0;
EularAngle_t EularAngle ={
	.yaw=0,
	.pitch=0,
	.roll=0
};
Liner_t Liner ={
	.left=0,
	.right=0,
	.front=0,
	.back=0,
};
Speed_t Speed={
	.speedx=0,
	.speedy=0,
	.speedw=0,
};
Speed_t speedcum={
  .speedx=0,
  .speedy=0,
  .speedw=0,
};
void Front(void);
void Back(void);
void Right(void);
void Left(void);
void AdpSpeedCon(char dir);
void SpeedCumulativeRun(Speed_t *speed,Liner_t *liner,char dir);
void SlowSpeedCumulativeRun(Speed_t *speed,Liner_t *liner,char dir);
void SpeedCumulativeBreak(Speed_t *speed,Liner_t *liner,char dir,char block);
void SlowSpeedCumulativeBreak(Speed_t *speed,Liner_t *liner,char dir,char block);
void SpeedCumulativeCirCle(Speed_t *speed,char dir,char nextdir,char block);
uint32_t TOFSignalFlag(char dir);
char InvDir(char dir);
uint8_t RuntoBreakFlag(char dir);
float BreaktoChangeFlag(char dir);
char map[400];
//char map[]="F0L0B0L0F0L0F0R0F1R0B0L0R0F0L0F0R0F1L0R0B1L0F0L0F0B0R1B0L2B0F0L0B0L1F0B0L0B1R0B0F0L0F0L0B1R0L0F0R1F1L0F0R0L0B0R0F0R0F0L0F0L0B0L0B2S0";
//char map[]="F2S0";'S'
//char map[]="FLS";
//char map_temp[]={"FF00LL00BB00LL00FF00LL00FF00RR20FF00RR20BB20RR00BB00LL20LL00RR00FF00LL02FF02LL00BB02FL20SS00"};
//------------------------------------------------|1-----------------------
//1号情况
//char map[]="BB00LR20SS00SS";
//2号情况
//char map[]="RR00LF00SS00SS";
//3号情况
//char map[]="LL00FB40SS00SS";
//4号情况
//char map[]="FF60LL20RB20SS00SS";
//5号情况
//char map[]="RR00LF00SS00SS";




char *RunFrame			=	map;
char *MapFrame			=	map+1;
char *BlockFrame		=	map+2;
char *ChangeDis			=	map+3;
char *NextMapFrame	=	map+4;

uint8_t left[4]={0};
uint8_t right[4]={0};
uint8_t front[4]={0};
uint8_t back[4]={0};
uint8_t PauseBlockFlag = 0;//暂停标志位
uint32_t AskNum = 0;//停止状态计数
uint32_t InitNum = 0;//初始状态计数
uint32_t tempRoadNum = 0;
uint32_t tempRoadNumNext = 0;

typedef enum
{
	MOTION_INIT=0,
	MOTION_START,
	MOTION_RUN,
	MOTION_BREAK,
	MOTION_CIRCLE,//圆弧插补
  MOTION_CHANGE,//换向状态
  MOTION_STOP,//停止状态
	MOTION_PAUSE,//暂停状态
}MOTION_STATE;
volatile double run2offflag=0;
volatile double changeflag=0;
volatile uint32_t startspeed=0;
MOTION_STATE MotionState = MOTION_INIT;
//MOTION_STATE MotionState = MOTION_OFFSET;

uint16_t motionflag=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM10_Init();
  MX_CAN1_Init();
  MX_TIM11_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM12_Init();
  MX_TIM13_Init();
  MX_CAN2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

//	/**@Communication */
	CommReceiveInit();
	

//	/**@IMU */
	while(BMI088_init()){};
	

//	/**@Motor */
	motor_init_4p();

//	/**@TOFSense */	
	//while(!(dis1||dis2||dis3||dis4)){};

//	/**@GreyScaleSensor */
	LineRead(&Liner);

//  /**@MS4005 */
	vApp_User_CAN_Configuration();
	
//	strncpy(map,map_temp,strlen(map_temp));
	__HAL_TIM_CLEAR_IT(&htim10,TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim11,TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim12,TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim13,TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&htim10);
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_Delay(3000);
	HAL_TIM_Base_Start_IT(&htim12);
	HAL_TIM_Base_Start_IT(&htim13);
	__HAL_UART_ENABLE_IT(&huart3,UART_IT_IDLE);	
	PlatformConAbsAng(INIT_DEGREE);
		
//	MotionState=MOTION_STOP;
////转换test
//	RoadsNumUpper = 3;
	
	
//	发送test
//	CheckTreasureFlag = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  protocol_analysis();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{	
	if(htim->Instance == TIM10)//200Hz
	{
		TOFRefresh();
	}	
	if(htim->Instance == TIM11)//1000Hz
	{
		BMI088_read(gyro, accel, &temp);
		EularAngle.accel[0]=accel[0];
		EularAngle.accel[1]=accel[1];
		EularAngle.accel[2]=accel[2];
		EularAngle.gyro[0]=gyro[0];
		EularAngle.gyro[1]=gyro[1];
		EularAngle.gyro[2]=gyro[2];
		YawSolve(&EularAngle);
		if(MotionState!=MOTION_INIT) yaw=EularAngle.yaw;
		LineRead(&Liner);
	}
	if(htim->Instance == TIM12)//100Hz
	{
		// 运动状态机
    switch (MotionState)
    {
      case MOTION_INIT://起始状态
      {
        // 执行200次
        motionflag++;
		
        if(motionflag>200 && RoadInfoFinishFlag == 1)//路段信息接收完成
        {
					EularAngle.yaw=0;
          motionflag=0;
          MotionState=MOTION_RUN;
        }
        break;
      }
		case MOTION_START://起始状态
    {
				if(*MapFrame=='L')
				{
					if(abs(Speed.speedy)>=SlowStardardSpeed || dis4<150+185*(*BlockFrame-'0'+1))
					{
						Speed.speedx=0;
						MotionState=MOTION_RUN;
					}
					Speed.speedx=-0;
					Speed.speedy-=200;
				}
				if(*MapFrame=='R')
				{
					if(abs(Speed.speedy)>=SlowStardardSpeed || dis2<150+185*(*BlockFrame-'0'+1))
					{
						Speed.speedx=0;
						MotionState=MOTION_RUN;
					}
					Speed.speedx=-0;
					Speed.speedy+=200;
				}
				break;
    }
    case MOTION_RUN://运动状态
    {
			RoadInfoFinishFlag = 0;		//路段信息接收完成标志位归零
//			if(SlowSpeedFlag==1)
//			{
//				SlowSpeedCumulativeRun(&speedcum,&Liner,*MapFrame);			
//			}	
//			else 
//			{	
//				SpeedCumulativeRun(&speedcum,&Liner,*MapFrame);
//			}
//			Speed.speedx=speedcum.speedx;
//			Speed.speedy=speedcum.speedy;
			//下面这个值在圆弧插补中加入了状态为4的情况，mark
			if((RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12 ) 
				)//判断是否需要刹车
			{
				MotionState=MOTION_BREAK;

			}
			break;
    }
    case MOTION_BREAK://刹车状态
    {
//			if(SlowSpeedFlag==1)
//			{
//				SlowSpeedCumulativeBreak(&speedcum,&Liner,*MapFrame,*BlockFrame);		
//			}	
//			else 
//			{	
//				SpeedCumulativeBreak(&speedcum,&Liner,*MapFrame,*BlockFrame);
//			}
			SpeedCumulativeBreak(&speedcum,&Liner,*MapFrame,*BlockFrame);
			Speed.speedx=speedcum.speedx;
			Speed.speedy=speedcum.speedy;//之前的一格现在变成两格了！记得改！
//----------------------------------------8月1日版本--------------------------------------------------------------------------------------------------
//		  if(*NextMapFrame!='S' &&
////			if(*NextMapFrame!='S' && (*BlockFrame - '0') < 4 && 
//					(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) && 
//					BreaktoChangeFlag(*MapFrame)<125+185*(*BlockFrame-'0'+1) &&
//					BreaktoChangeFlag(*MapFrame)>25+185*(*BlockFrame-'0'+1) &&
//			(InvDir(*NextMapFrame)!=*MapFrame && *NextMapFrame!=*MapFrame))//判断是否需要换向
//			{
//				MotionState=MOTION_CIRCLE;
//			}
//			//长距直接跳过圆弧
//			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 8 ) &&
////			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 8 || (*BlockFrame - '0') >= 6) &&
////			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 6) &&
//					(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) && 
//					BreaktoChangeFlag(*MapFrame)<150+185*(*BlockFrame-'0'))
//			{
//				MotionState=MOTION_CHANGE;
//			} 
//			//180度换向或者下一步前进时不改变方向的两种情况下直接跳过圆弧插补
//			if(*NextMapFrame!='S' &&
//				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&
//			(InvDir(*NextMapFrame)==*MapFrame || *NextMapFrame==*MapFrame) &&
//			BreaktoChangeFlag(*MapFrame)<150+185*(*BlockFrame-'0'))
//			{
//				MotionState=MOTION_CHANGE;
//			}
//----------------------------------------8月1日版本end-----------------------------------------------------------------------------------------------			

//----------------------------------------长距跳过圆弧版本---------------------------------------------------------------------------------------------	
			
			//=====================圆弧插补============================
			if(*NextMapFrame!='S' && (*BlockFrame - '0') < 6 && (*ChangeDis - '0') < 6 &&
					(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) && 
					TOFSignalFlag(*MapFrame)>2 &&
					BreaktoChangeFlag(*MapFrame)<125+185*(*BlockFrame-'0'+1) &&
					BreaktoChangeFlag(*MapFrame)>10+185*(*BlockFrame-'0'+1) &&
					(InvDir(*NextMapFrame)!=*MapFrame && *NextMapFrame!=*MapFrame) &&
					*RunFrame == *MapFrame
				)//(排除掉撞击的情况)判断是否需要换向
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CIRCLE;
			}
			
			
			
//			//长距直接跳过圆弧
//			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 8 ) &&
////			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 8 || (*BlockFrame - '0') >= 6) &&
////			if( (*NextMapFrame=='S' || ((*ChangeDis) - '0') >= 6) &&
//					(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) && 
//					BreaktoChangeFlag(*MapFrame)<150+185*(*BlockFrame-'0'))
//			{
//				MotionState=MOTION_CHANGE;
//			} 
			
			
			//===============前进方向与测距方向相反时情况===========================
			if(InvDir(*RunFrame)==*MapFrame && 																		//前进方向与测距方向相反，跳过圆弧
				*NextMapFrame!='S' &&	(*ChangeDis - '0') < 6 &&											//排除掉末尾路段情况
				(tempRoadNum != 0 || TempCrushFlag != 1) && 												//排除掉需要撞击的情况(留给下面处理)
//				(*BlockFrame - '0') < 6 &&                                    		//排除掉长距情况
				(InvDir(*NextMapFrame)!=*MapFrame && *NextMapFrame != *MapFrame) && //排除掉180度换向或者方向不变的情况
				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&	
				TOFSignalFlag(*MapFrame)>2 &&
				BreaktoChangeFlag(*MapFrame)<183+185*(*BlockFrame-'0') &&
				BreaktoChangeFlag(*MapFrame)>147+185*(*BlockFrame-'0'))
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
			}
			
//			//===============前进方向与测距方向相反时情况（长距）=================
//			if(InvDir(*RunFrame)==*MapFrame && 															//前进方向与测距方向相反，跳过圆弧
//				*NextMapFrame!='S' &&	(*ChangeDis - '0') < 6 &&								//排除掉末尾路段情况
//				(tempRoadNum != 0 || TempCrushFlag != 1) && 									//排除掉需要撞击的情况(留给下面处理)
//				(*BlockFrame - '0') >= 6 &&                                   //处理长距情况
//				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&	
//				TOFSignalFlag(*MapFrame)>2 &&
//				BreaktoChangeFlag(*MapFrame)<168+185*(*BlockFrame-'0') &&
//				BreaktoChangeFlag(*MapFrame)>148+185*(*BlockFrame-'0'))
//			{
//				SlowSpeedFlag = 0;//将慢速标志位归零
//				MotionState=MOTION_CHANGE;
//				SlowSpeedFlag = 0;//这个标志位是为了特殊情况下减速，暂时用不上
//			}
			
			
			

			//=============180度换向或者方向不变时跳过圆弧插补============
			if(
				(*NextMapFrame!='S' &&
				(InvDir(*NextMapFrame)==*MapFrame || *NextMapFrame==*MapFrame)
				) && //上面是跳过圆弧的情况
				*RunFrame != *MapFrame &&
				(tempRoadNum != 0 || TempCrushFlag != 1) && //排除掉需要撞击的情况(留给下面处理)
				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&	
				TOFSignalFlag(*MapFrame)>2 &&
				BreaktoChangeFlag(*MapFrame)<166+185*(*BlockFrame-'0') &&
				BreaktoChangeFlag(*MapFrame)>144+185*(*BlockFrame-'0'))
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
			}
			
			
			//===============测距距离超过阈值时跳过圆弧插补=============
			if(
				(*NextMapFrame!='S' && 																						//排除掉末尾路段的情况
				(tempRoadNum != 0 || TempCrushFlag != 1) && 											//排除掉需要撞击的情况(留给下面处理)
				(InvDir(*NextMapFrame)!=*MapFrame && *NextMapFrame!=*MapFrame)&&	//排除掉180换向或者方向不变的情况
				InvDir(*RunFrame)!=*MapFrame &&																		//排除掉前进与测距方向反向的情况
				(*ChangeDis - '0') < 6) &&																				//排除掉弧距过长的情况
																																				//上面是跳过圆弧的情况
				(*BlockFrame - '0') >= 6 &&																			
				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&		
				TOFSignalFlag(*MapFrame)>0 &&
				BreaktoChangeFlag(*MapFrame)>129+185*(*BlockFrame-'0') &&
				BreaktoChangeFlag(*MapFrame)<154+185*(*BlockFrame-'0'))  //这个阈值需要单独调
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
				SlowSpeedFlag = 0;//这个标志位是为了特殊情况下减速
			}
			

			
			//==================处理末尾路段情况========================
			//三种情况：
			//1.末尾路段不动，只是为了转向；
			//2.末尾路段前进，前进方向和测距方向反向；
			//3.到达终点，前进方向和测距方向反向。
			if(*NextMapFrame=='S' && (*ChangeDis - '0') < 6 &&
				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&
				TOFSignalFlag(*MapFrame)>2 &&
				BreaktoChangeFlag(*MapFrame)<156+185*(*BlockFrame-'0') &&  
				BreaktoChangeFlag(*MapFrame)>132+185*(*BlockFrame-'0')
				)
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
			}

			//===================处理撞击的情况=========================
			if((tempRoadNum == 0 && TempCrushFlag == 1) &&  //处理撞击情况：已经改成直接处理撞击标志位（之前的处理方式是下一帧前进方向和这一帧测距方向相同）
				(*ChangeDis - '0') < 6 && 
				(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&
				TOFSignalFlag(*MapFrame)>2 &&
				BreaktoChangeFlag(*MapFrame)<170+185*(*BlockFrame-'0') &&  
				BreaktoChangeFlag(*MapFrame)>135+185*(*BlockFrame-'0')
			)
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
			}
			
			//===================处理弧距过长的问题=====================
			if(
				((*ChangeDis - '0') >= 6) && 
			(RuntoBreakFlag(*MapFrame)==0 || RuntoBreakFlag(*MapFrame)==4 || RuntoBreakFlag(*MapFrame)==11 || RuntoBreakFlag(*MapFrame)==12) &&
			TOFSignalFlag(*MapFrame)>2 &&
			(InvDir(*NextMapFrame)!=*MapFrame && *NextMapFrame!=*MapFrame) &&
			BreaktoChangeFlag(*MapFrame)<153+185*(*BlockFrame-'0') &&  //可能要改阈值 稍微改大一点
			BreaktoChangeFlag(*MapFrame)>135+185*(*BlockFrame-'0')
			)
			{
				SlowSpeedFlag = 0;//将慢速标志位归零
				MotionState=MOTION_CHANGE;
				
			}
			
			//==============处理仅仅转向的情况===========================
			if(*MapFrame == 'S') MotionState=MOTION_CHANGE;
			
			
//----------------------------------------长距跳过圆弧版本end---------------------------------------------------------------------------------------			
			break;
    }
			case MOTION_CIRCLE://圆弧插补
			{
				SpeedCumulativeCirCle(&speedcum,*MapFrame,*NextMapFrame,*BlockFrame);
				Speed.speedx=speedcum.speedx;
				Speed.speedy=speedcum.speedy;
				if(	(RuntoBreakFlag(*NextMapFrame)==0 || RuntoBreakFlag(*NextMapFrame)==4 || RuntoBreakFlag(*NextMapFrame)==11 || RuntoBreakFlag(*NextMapFrame)==12) &&
						TOFSignalFlag(*MapFrame)>5 &&
						BreaktoChangeFlag(InvDir(*NextMapFrame))>75+185*(*ChangeDis-'0'+1) &&
						BreaktoChangeFlag(InvDir(*NextMapFrame))<175+185*(*ChangeDis-'0'+1))//判断是否需要换向
				{
					MotionState=MOTION_CHANGE;
				}
				//180度换向或者下一步前进时不改变方向的两种情况下直接跳过圆弧插补
				if(InvDir(*NextMapFrame)==*MapFrame || *NextMapFrame==*MapFrame || *RunFrame!=*MapFrame)//180度换向直接跳过圆弧插补
				{
					MotionState=MOTION_CHANGE;
				}
				break;
			}
      case MOTION_CHANGE://换向状态
      {
				
				if(*NextMapFrame=='S' || *MapFrame == 'S') 
				{
					MotionState=MOTION_STOP;
					//摄像头拍照云台转向
//					if			 	(CameraDirPro == 0)		PlatformConAbsAng(INIT_DEGREE);					//测距方向是前，摄像头转向后方
//					else if 	(CameraDirPro == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//测距方向是左，摄像头转向右方
//					else if	 	(CameraDirPro == 2)		PlatformConAbsAng(INIT_DEGREE - 18000);	//测距方向是后，摄像头转向前方
//					else if	 	(CameraDirPro == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //测距方向是右，摄像头转向左方
					
				}
				else if(tempRoadNum == 0 && TempCrushFlag == 1)  //改成了直接检测撞击标志位
				{
					Speed.speedx=0;
					Speed.speedy=0;
					MotionState=MOTION_RUN;
				}
				else MotionState=MOTION_RUN;
				TempCrushFlag = 0;
				tempRoadNum++;      //路段计数递增
//================================宝藏解析标志轮换==================================
				if			(RoadInfoUpper[0][tempRoadNum] == 0) *RunFrame = 'F';
				else if	(RoadInfoUpper[0][tempRoadNum] == 1) *RunFrame = 'L';
				else if	(RoadInfoUpper[0][tempRoadNum] == 2) *RunFrame = 'B';
				else if	(RoadInfoUpper[0][tempRoadNum] == 3) *RunFrame = 'R';
				else if	(RoadInfoUpper[0][tempRoadNum] == 4) *RunFrame = 'S';
				
				if			(RoadInfoUpper[1][tempRoadNum] == 0) *MapFrame = 'F';
				else if	(RoadInfoUpper[1][tempRoadNum] == 1) *MapFrame = 'L';
				else if	(RoadInfoUpper[1][tempRoadNum] == 2) *MapFrame = 'B';
				else if	(RoadInfoUpper[1][tempRoadNum] == 3) *MapFrame = 'R';
				else if	(RoadInfoUpper[1][tempRoadNum] == 4) *MapFrame = 'S';

				*BlockFrame				= RoadInfoUpper[2][tempRoadNum] + '0';
				*ChangeDis				= RoadInfoUpper[3][tempRoadNum] + '0';

				if			(RoadInfoUpper[0][tempRoadNum + 1] == 0) *NextMapFrame = 'F';
				else if	(RoadInfoUpper[0][tempRoadNum + 1] == 1) *NextMapFrame = 'L';
				else if	(RoadInfoUpper[0][tempRoadNum + 1] == 2) *NextMapFrame = 'B';
				else if	(RoadInfoUpper[0][tempRoadNum + 1] == 3) *NextMapFrame = 'R';
				else if	(RoadInfoUpper[0][tempRoadNum + 1] == 4) *NextMapFrame = 'S';
//================================宝藏解析标志轮换end================================

  

//==============================云台随测距方向转向===================================
//				if 			(*MapFrame == 'F')	PlatformConAbsAng(INIT_DEGREE + 18000);    //记得在协议解析的地方也有摄像头转向
//				else if (*MapFrame == 'L')	PlatformConAbsAng(INIT_DEGREE - 9000);
//				else if (*MapFrame == 'B')	PlatformConAbsAng(INIT_DEGREE);
//				else if (*MapFrame == 'R')	PlatformConAbsAng(INIT_DEGREE + 9000);  
//==============================云台随测距方向转向end===================================
				break;
      }
      case MOTION_STOP://停止状态
      {
        Speed.speedx=0;
        Speed.speedy=0;

//        云台转向程序
//				if(AskNum == 1)
//				{
//					if			 	(RoadInfoUpper[1][RoadsNumUpper - 1] == 0)		PlatformConAbsAng(INIT_DEGREE);					//测距方向是前，摄像头转向后方
//					else if 	(RoadInfoUpper[1][RoadsNumUpper - 1] == 1)		PlatformConAbsAng(INIT_DEGREE + 9000);	//测距方向是左，摄像头转向右方
//					else if	 	(RoadInfoUpper[1][RoadsNumUpper - 1] == 2)		PlatformConAbsAng(INIT_DEGREE + 18000);	//测距方向是后，摄像头转向前方
//					else if	 	(RoadInfoUpper[1][RoadsNumUpper - 1] == 3)		PlatformConAbsAng(INIT_DEGREE - 9000);  //测距方向是右，摄像头转向左方
//				} 

				if(AskNum == 1) 
				{	
					CheckTreasureFlag = 1;//当每条路径第====三十====次进入STOP状态时请求检测宝藏(接收到回复后标志位会清零)
				}		
				AskNum++;							  //防止反复请求检测
				
				if(RoadInfoFinishFlag == 1)//路段信息接收完成
				{
//					RunFrame			= map;
//					MapFrame			=	map+1;				//地图帧中的四位数据都从头调用字符串
//					BlockFrame		=	map+2;				
//					ChangeDis			=	map+3;
//					NextMapFrame	=	map+4;
					tempRoadNum 	= 0;        //路段计数归零
					MotionState=MOTION_RUN;		//调换到开始状态
					AskNum = 0;				      	//进入STOP状态计数归零，准备下一次宝藏检测请求                        。。。再不能用就亖了
				}
				break;
      }
			case MOTION_PAUSE://暂停状态
			{
				Speed.speedx=0;
        Speed.speedy=0;
				if(HAL_GPIO_ReadPin(BLO_GPIO_Port, BLO_Pin) == 0)
				{
					MotionState=MOTION_RUN;	
					PauseBlockFlag = 0;
				}
			
			}
    }
		if(MotionState!=MOTION_INIT) Speed.speedw=-Ang_PID_Absolute(fmod(yaw,360),0);
		else  Speed.speedw=0;
		if(PauseBlockFlag == 0)
		{
			if(HAL_GPIO_ReadPin(BLO_GPIO_Port, BLO_Pin) == 1)
			{
				MotionState=MOTION_PAUSE;
				PauseBlockFlag = 1;
			}
		}
		M2006_motion_vector(Speed.speedx,Speed.speedy,Speed.speedw);
	}
	
	
	
	
	if (htim->Instance == TIM13)
	{
	sendUART();
	}
}

void SpeedCumulativeRun(Speed_t *speed,Liner_t *liner,char dir)
{
  switch(dir){
    case 'F':{
      speed->speedx=StardardSpeed;
      speed->speedy=SpeedOffset(liner->front);
      break;
    }case 'B':{
      speed->speedx=-StardardSpeed;
      speed->speedy=-SpeedOffset(liner->back);
      break;
    }case 'L':{
      speed->speedx=SpeedOffset(liner->left);
      speed->speedy=-StardardSpeed;
      break;
    }case 'R':{
      speed->speedx=-SpeedOffset(liner->right);
      speed->speedy=StardardSpeed;
      break;
    }
  }
}

void SlowSpeedCumulativeRun(Speed_t *speed,Liner_t *liner,char dir)
{
  switch(dir){
    case 'F':{
      speed->speedx=SlowStardardSpeed;
      speed->speedy=SpeedOffset(liner->front);
			//speed->speedy=0;
      break;
    }case 'B':{
      speed->speedx=-SlowStardardSpeed;
      speed->speedy=-SpeedOffset(liner->back);
			//speed->speedy=0;
      break;
    }case 'L':{
      speed->speedx=SpeedOffset(liner->left);
      speed->speedy=-SlowStardardSpeed;
			//speed->speedx=0;
      break;
    }case 'R':{
      speed->speedx=-SpeedOffset(liner->right);
      speed->speedy=SlowStardardSpeed;
			//speed->speedx=0;
      break;
    }
  }
}


void SpeedCumulativeBreak(Speed_t *speed,Liner_t *liner,char dir,char block){
	int i=block-'0';
    switch(dir){
    case 'F':{
      speed->speedx=Pos_PID_Absolute(dis1,150+185*i);
      speed->speedy=SpeedOffset(liner->front);
			break;
    }case 'B':{
      speed->speedx=-Pos_PID_Absolute(dis3,150+185*i);
      speed->speedy=-SpeedOffset(liner->back);
			break;
    }case 'L':{
      speed->speedx=SpeedOffset(liner->left);
      speed->speedy=-Pos_PID_Absolute(dis4,150+185*i);
			break;
    }case 'R':{
      speed->speedx=-SpeedOffset(liner->right);
      speed->speedy=Pos_PID_Absolute(dis2,150+185*i);
			break;
    }
  }
}


void SlowSpeedCumulativeBreak(Speed_t *speed,Liner_t *liner,char dir,char block){
	int i=block-'0';
    switch(dir){
    case 'F':{
      speed->speedx=Slow_Pos_PID_Absolute(dis1,150+185*i);
      speed->speedy=SpeedOffset(liner->front);
			//speed->speedy=0;
			break;
    }case 'B':{
      speed->speedx=-Slow_Pos_PID_Absolute(dis3,150+185*i);
      speed->speedy=-SpeedOffset(liner->back);
			//speed->speedy=0;
			break;
    }case 'L':{
      speed->speedx=SpeedOffset(liner->left);
      speed->speedy=-Slow_Pos_PID_Absolute(dis4,150+185*i);
			//speed->speedx=0;
			break;
    }case 'R':{
      speed->speedx=-SpeedOffset(liner->right);
      speed->speedy=Slow_Pos_PID_Absolute(dis2,150+185*i);
			//speed->speedx=0;
			break;
    }
  }
}

void SpeedCumulativeCirCle(Speed_t *speed,char dir,char nextdir,char block)
{
	int i=block-'0';
	double changespeed=7000.0;
  switch(dir){
    case 'F':{
      speed->speedx=Pos_PID_Absolute(dis1,150+185*i);
      if(nextdir=='L'&&abs(speed->speedx)<=changespeed) speed->speedy=-(changespeed-abs(speed->speedx));
			if(nextdir=='R'&&abs(speed->speedx)<=changespeed) speed->speedy=+(changespeed-abs(speed->speedx));
			if(nextdir=='S') speed->speedy=0;
			break;
    }case 'B':{
      speed->speedx=-Pos_PID_Absolute(dis3,150+185*i);
      if(nextdir=='L'&&abs(speed->speedx)<=changespeed) speed->speedy=-(changespeed-abs(speed->speedx));
      if(nextdir=='R'&&abs(speed->speedx)<=changespeed) speed->speedy=+(changespeed-abs(speed->speedx));
      if(nextdir=='S') speed->speedy=0;
      break;
    }case 'L':{
      speed->speedy=-Pos_PID_Absolute(dis4,150+185*i);
      if(nextdir=='F'&&abs(speed->speedy)<=changespeed) speed->speedx=+(changespeed-abs(speed->speedy));
      if(nextdir=='B'&&abs(speed->speedy)<=changespeed) speed->speedx=-(changespeed-abs(speed->speedy));
      if(nextdir=='S') speed->speedx=0;
      break;
    }case 'R':{
      speed->speedy=Pos_PID_Absolute(dis2,150+185*i);
      if(nextdir=='F'&&abs(speed->speedy)<=changespeed) speed->speedx=+(changespeed-abs(speed->speedy));
      if(nextdir=='B'&&abs(speed->speedy)<=changespeed) speed->speedx=-(changespeed-abs(speed->speedy));
      if(nextdir=='S') speed->speedx=0;
      break;
    }
	}
}


uint8_t RuntoBreakFlag(char dir){
  if (dir=='F') return status1;
  if (dir=='B') return status3;
  if (dir=='L') return status4;
  if (dir=='R') return status2;
  return 1;
}

uint32_t TOFSignalFlag(char dir){
	if (dir=='F') return sig1;
  if (dir=='B') return sig3;
  if (dir=='L') return sig4;
  if (dir=='R') return sig2;
	return 1;

}

float BreaktoChangeFlag(char dir){
  if (dir=='F') return dis1;
  if (dir=='B') return dis3;
  if (dir=='L') return dis4;
  if (dir=='R') return dis2;
  return 0;
}

char InvDir(char dir)
{
	if(dir=='F') return 'B';
	if(dir=='B') return 'F';
	if(dir=='L') return 'R';
	if(dir=='R') return 'L';
	if(dir=='S') return 'S';
	return 0;
}
void TOFRefresh(void){
	if(mode_flag == 0)													//主动输出模式
	{
	}
	else if(mode_flag == 1)												//查询输出模式
	{
		HAL_UART_Transmit_DMA(&huart3,u_tx_buf,sizeof(u_tx_buf));
		HAL_Delay(5);
	}
	else if(mode_flag == 2)												//级联模式
	{
		HAL_UART_Transmit_DMA(&huart3,u_tx_buf,sizeof(u_tx_buf));
//		HAL_Delay(5);
		
		TOF_ID++; 
		if(TOF_ID >= 5)
		{
			TOF_ID = 1;
		}
		u_tx_buf[4] = TOF_ID;										//更新ID
		u_tx_buf[7] = TOF_ID + 0x63;								//更新校验和
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
