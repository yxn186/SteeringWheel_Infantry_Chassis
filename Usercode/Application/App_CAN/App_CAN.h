/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_CAN.h
  * @brief   This file contains all the function prototypes for
  *          the App_CAN.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CAN_H__
#define __APP_CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "CAN_Interface.h"
/*YOUR CODE*/

/**
 * @brief 获取CAN接口类指针
 * 
 * @param CAN_Index 1-3
 * @return Class_CAN_Interface* CAN接口
 */
Class_CAN_Interface *AppCAN_Get_Interface(uint8_t CAN_Index);

void AppCAN_Init(void);





#ifdef __cplusplus
}
#endif

#endif /* __APP_CAN_H__ */
