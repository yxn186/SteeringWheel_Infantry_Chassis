/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Attitude_Config.h
  * @brief   底盘与云台姿态关系配置
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_CHASSIS_ATTITUDE_CONFIG_H__
#define __APP_CHASSIS_ATTITUDE_CONFIG_H__

//云台Yaw电机在CAN2上的只读反馈配置
#define Chassis_Attitude_Yaw_Motor_CAN_Number                  2
#define Chassis_Attitude_Yaw_Motor_ID                          2
#define Chassis_Attitude_Yaw_Motor_Mechanical_Zero_Degree      0.0f    //云台正前方对应的编码器角度
#define Chassis_Attitude_Yaw_Motor_Direction                   1.0f    //云台逆时针转动时相对角应为正

#endif /* __APP_CHASSIS_ATTITUDE_CONFIG_H__ */
