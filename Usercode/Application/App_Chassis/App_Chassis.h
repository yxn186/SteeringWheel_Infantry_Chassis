/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis.h
  * @brief   This file contains all the function prototypes for
  *          the App_Chassis.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_H__
#define __APP_CHASSIS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/
/**
 * @brief 底盘CAN接收回调函数
 * 
 * @param CAN_Index CAN总线索引
 * @param id CAN报文ID
 * @param data CAN报文数据
 * @param length CAN报文数据长度
 */
void App_Chassis_CAN_RX_Callback(uint8_t CAN_Index,uint16_t id,const uint8_t *data,uint8_t length);






#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_H__ */
