/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Config.h
  * @brief   This file contains all the function prototypes for
  *          the App_Chassis_Config.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_CONFIG_H__
#define __APP_CHASSIS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/
//舵轮底盘相关参数配置

//底盘电机ID配置
//3508 ID 1-8
#define Wheel_Motor_0_ID                                                    1       //轮电机0 ID
#define Wheel_Motor_1_ID                                                    2       //轮电机1 ID
#define Wheel_Motor_2_ID                                                    3       //轮电机2 ID
#define Wheel_Motor_3_ID                                                    4       //轮电机3 ID

//6020 ID 1-7           
#define Steering_Motor_0_ID                                                 1       //舵电机0 ID
#define Steering_Motor_1_ID                                                 2       //舵电机1 ID
#define Steering_Motor_2_ID                                                 3       //舵电机2 ID
#define Steering_Motor_3_ID                                                 4       //舵电机3 ID

//舵轮零点角度设置 调车前先设置！！！
#define Steering_Motor_0_Zero_Angle                                         0.0f    //舵轮0零点角度
#define Steering_Motor_1_Zero_Angle                                         0.0f    //舵轮1零点角度
#define Steering_Motor_2_Zero_Angle                                         0.0f    //舵轮2零点角度
#define Steering_Motor_3_Zero_Angle                                         0.0f    //舵轮3零点角度

//电机方向设置 1为正向 -1为反向
#define Wheel_Motor_0_Direction                                             1       //轮电机0方向
#define Wheel_Motor_1_Direction                                             1       //轮电机1方向
#define Wheel_Motor_2_Direction                                             1       //轮电机2方向
#define Wheel_Motor_3_Direction                                             1       //轮电机3方向
#define Steering_Motor_0_Direction                                          1       //舵电机0方向
#define Steering_Motor_1_Direction                                          1       //舵电机1方向
#define Steering_Motor_2_Direction                                          1       //舵电机2方向
#define Steering_Motor_3_Direction                                          1       //舵电机3方向

//底盘参数设置      
#define Chassis_a                                                           0.25f    //旋转中心到前后舵轮中心的纵向距离 单位m
#define Chassis_b                                                           0.25f    //旋转中心到左右舵轮中心的横向距离 单位m
#define Wheel_Radius                                                        0.05f    //轮半径 单位m
#define Max_WheelMotor_Linear_Speed                                         20.0f    //轮电机最大线速度 单位m/s

//PID参数配置
//轮电机PID（速度环）
#define Wheel_Motor_PID_Kp_s                                                0.0f
#define Wheel_Motor_PID_Ki_s                                                0.0f
#define Wheel_Motor_PID_Kd_s                                                0.0f
#define Wheel_Motor_PID_ErrorInt_High_s                                     0.0f
#define Wheel_Motor_PID_ErrorInt_Low_s                                      0.0f
#define Wheel_Motor_PID_Integral_Stop_Near_Zero_Enable_s                    0
#define Wheel_Motor_PID_Integral_Stop_Target_Abs_Threshold_s                0.0f
#define Wheel_Motor_PID_Integral_Stop_Error_Abs_Threshold_s                 0.0f
#define Wheel_Motor_PID_Out_High                                            0.0f
#define Wheel_Motor_PID_Out_Low                                             0.0f

//舵电机PID（速度环）
#define Steering_Motor_PID_Kp_s                                             0.0f
#define Steering_Motor_PID_Ki_s                                             0.0f
#define Steering_Motor_PID_Kd_s                                             0.0f
#define Steering_Motor_PID_ErrorInt_High_s                                  0.0f
#define Steering_Motor_PID_ErrorInt_Low_s                                   0.0f
#define Steering_Motor_PID_Integral_Stop_Near_Zero_Enable_s                 0
#define Steering_Motor_PID_Integral_Stop_Target_Abs_Threshold_s             0.0f
#define Steering_Motor_PID_Integral_Stop_Error_Abs_Threshold_s              0.0f
#define Steering_Motor_PID_Out_High                                         0.0f
#define Steering_Motor_PID_Out_Low                                          0.0f

//舵电机PID（角度环）
#define Steering_Motor_PID_Kp_a                                             0.0f
#define Steering_Motor_PID_Ki_a                                             0.0f
#define Steering_Motor_PID_Kd_a                                             0.0f
#define Steering_Motor_PID_ErrorInt_High_a                                  0.0f
#define Steering_Motor_PID_ErrorInt_Low_a                                   0.0f
#define Steering_Motor_PID_FeedForward_Enable_a                             0
#define Steering_Motor_PID_Kf_a                                             0.0f
#define Steering_Motor_PID_FeedForward_High_a                               0.0f
#define Steering_Motor_PID_FeedForward_Low_a                                0.0f
#define Steering_Motor_PID_Integral_Stop_Near_Zero_Enable_a                 0
#define Steering_Motor_PID_Integral_Stop_Target_Abs_Threshold_a             0.0f
#define Steering_Motor_PID_Integral_Stop_Error_Abs_Threshold_a              0.0f
#define Steering_Motor_PID_Speed_Target_High                                0.0f
#define Steering_Motor_PID_Speed_Target_Low                                 0.0f





#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_CONFIG_H__ */
