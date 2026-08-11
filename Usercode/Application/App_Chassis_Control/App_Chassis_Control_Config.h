/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Control_Config.h
  * @brief   This file contains the chassis control configuration
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_CONTROL_CONFIG_H__
#define __APP_CHASSIS_CONTROL_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

//普通遥控模式最大速度配置
#define Chassis_Control_Normal_Max_Speed_X                 1.0f    //前后最大速度，单位m/s
#define Chassis_Control_Normal_Max_Speed_Y                 1.0f    //左右最大速度，单位m/s
#define Chassis_Control_Normal_Max_W_Z                     3.0f    //旋转最大角速度，单位rad/s

//遥控摇杆到目标速度的方向配置，实车方向相反时只修改对应系数
#define Chassis_Control_Remote_Speed_X_Direction           1.0f
#define Chassis_Control_Remote_Speed_Y_Direction          -1.0f
#define Chassis_Control_Remote_W_Z_Direction              -1.0f

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_CONTROL_CONFIG_H__ */
