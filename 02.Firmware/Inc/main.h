/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tofsense.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern uint8_t u_rx_buf[400];
extern uint8_t u_tx_buf[8];

extern uint32_t TOF_data_length;

extern float dis1,dis2,dis3,dis4;
extern float status1,status2,status3,status4;

extern uint8_t TOF_ID;

extern char map[400];

extern char *RunFrame;
extern char *MapFrame;
extern char *BlockFrame;
extern char *ChangeDis;
extern char *NextMapFrame;





//速度结构体
typedef struct {
	int32_t speedx;
	int32_t speedy;
	int32_t speedw;
}Speed_t;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS1_GYRO_Pin GPIO_PIN_4
#define CS1_GYRO_GPIO_Port GPIOC
#define CS1_ACCEL_Pin GPIO_PIN_5
#define CS1_ACCEL_GPIO_Port GPIOC
#define R4_Pin GPIO_PIN_13
#define R4_GPIO_Port GPIOF
#define L1_Pin GPIO_PIN_14
#define L1_GPIO_Port GPIOF
#define R3_Pin GPIO_PIN_15
#define R3_GPIO_Port GPIOF
#define L2_Pin GPIO_PIN_0
#define L2_GPIO_Port GPIOG
#define R2_Pin GPIO_PIN_1
#define R2_GPIO_Port GPIOG
#define L3_Pin GPIO_PIN_7
#define L3_GPIO_Port GPIOE
#define R1_Pin GPIO_PIN_8
#define R1_GPIO_Port GPIOE
#define L4_Pin GPIO_PIN_9
#define L4_GPIO_Port GPIOE
#define F4_Pin GPIO_PIN_10
#define F4_GPIO_Port GPIOE
#define B1_Pin GPIO_PIN_11
#define B1_GPIO_Port GPIOE
#define F3_Pin GPIO_PIN_12
#define F3_GPIO_Port GPIOE
#define B2_Pin GPIO_PIN_13
#define B2_GPIO_Port GPIOE
#define F2_Pin GPIO_PIN_14
#define F2_GPIO_Port GPIOE
#define B3_Pin GPIO_PIN_15
#define B3_GPIO_Port GPIOE
#define F1_Pin GPIO_PIN_10
#define F1_GPIO_Port GPIOB
#define B4_Pin GPIO_PIN_11
#define B4_GPIO_Port GPIOB
#define B4G6_Pin GPIO_PIN_6
#define B4G6_GPIO_Port GPIOG
#define BLO_Pin GPIO_PIN_1
#define BLO_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
