/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Control.h
  * @brief   This file contains all the function prototypes for
  *          the App_Chassis_Control.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_CONTROL_H__
#define __APP_CHASSIS_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief 初始化底盘遥控控制APP
 */
void App_Chassis_Control_Init(void);

/**
 * @brief 更新底盘遥控控制状态
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Chassis_Control_Update(uint32_t Now_ms);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_CONTROL_H__ */
