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

//小陀螺模式参数，左拨杆上、右拨杆下时生效
#define Chassis_Control_Spin_W_Z                           6.0f     //底盘自旋角速度，单位rad/s
#define Chassis_Control_Spin_W_Z_Direction                 1.0f

//底盘跟随云台参数，左拨杆上、右拨杆中时生效
#define Chassis_Control_Follow_Gimbal_PID_Kp_a             0.05f    //相对角误差到旋转速度的比例系数
#define Chassis_Control_Follow_Gimbal_PID_Ki_a             0.0f
#define Chassis_Control_Follow_Gimbal_PID_Kd_a             0.0f
#define Chassis_Control_Follow_Gimbal_PID_ErrorInt_High_a  0.0f
#define Chassis_Control_Follow_Gimbal_PID_ErrorInt_Low_a   0.0f
#define Chassis_Control_Follow_Gimbal_Max_W_Z              3.0f     //跟随模式最大角速度，单位rad/s
#define Chassis_Control_Follow_Gimbal_Output_Direction    -1.0f

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_CONTROL_CONFIG_H__ */
