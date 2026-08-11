/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Fire_Config.h
  * @brief   发射机构应用配置
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_FIRE_CONFIG_H__
#define __APP_FIRE_CONFIG_H__

//define Gimbal是0 Chassis是1
#define SteeringWheel_Infantry_Board_Gimbal                  0
#define SteeringWheel_Infantry_Board_Chassis                 1

//define拨弹盘安装位置，默认使用上供弹结构
#ifndef SteeringWheel_Infantry_Feeder_Location
#define SteeringWheel_Infantry_Feeder_Location               SteeringWheel_Infantry_Board_Gimbal
#endif

//发射机构均使用CAN1上的M3508
#define Fire_Motor_CAN_Number                                 1
#define Fire_Friction_Motor_Count                             4
#define Fire_Friction_Motor_0_ID                              1
#define Fire_Friction_Motor_1_ID                              2
#define Fire_Friction_Motor_2_ID                              3
#define Fire_Friction_Motor_3_ID                              4
#define Fire_Feeder_Motor_ID                                  5

//四摩擦轮线速度目标和安装方向
#define Fire_Friction_Target_Speed_m_s                        15.0f
#define Fire_Friction_Wheel_Radius_m                          0.03f
#define Fire_Friction_Motor_0_Direction                       1.0f
#define Fire_Friction_Motor_1_Direction                      -1.0f
#define Fire_Friction_Motor_2_Direction                       1.0f
#define Fire_Friction_Motor_3_Direction                      -1.0f

//摩擦轮速度环参数
#define Fire_Friction_PID_Kp_s                                0.0f
#define Fire_Friction_PID_Ki_s                                0.0f
#define Fire_Friction_PID_Kd_s                                0.0f
#define Fire_Friction_PID_ErrorInt_High_s                     0.0f
#define Fire_Friction_PID_ErrorInt_Low_s                      0.0f
#define Fire_Friction_PID_Out_High                            0.0f
#define Fire_Friction_PID_Out_Low                             0.0f

//拨弹盘每发转角和方向
#define Fire_Feeder_Angle_Per_Bullet_Degree                   45.0f
#define Fire_Feeder_Motor_Direction                           1.0f

//拨弹盘角度环和速度环参数
#define Fire_Feeder_PID_Kp_a                                  0.0f
#define Fire_Feeder_PID_Ki_a                                  0.0f
#define Fire_Feeder_PID_Kd_a                                  0.0f
#define Fire_Feeder_PID_ErrorInt_High_a                       0.0f
#define Fire_Feeder_PID_ErrorInt_Low_a                        0.0f
#define Fire_Feeder_PID_Max_Speed_Rad_s                       0.0f
#define Fire_Feeder_PID_Kp_s                                  0.0f
#define Fire_Feeder_PID_Ki_s                                  0.0f
#define Fire_Feeder_PID_Kd_s                                  0.0f
#define Fire_Feeder_PID_ErrorInt_High_s                       0.0f
#define Fire_Feeder_PID_ErrorInt_Low_s                        0.0f
#define Fire_Feeder_PID_Out_High                              0.0f
#define Fire_Feeder_PID_Out_Low                               0.0f

//堵转检测和单发完成条件
#define Fire_Feedback_Wait_Timeout_ms                         500
#define Fire_Friction_Stall_Detection_Delay_ms                500
#define Fire_Stall_Confirm_Time_ms                            300
#define Fire_Friction_Stall_Speed_m_s                         1.0f
#define Fire_Feeder_Stall_Speed_Rad_s                         0.2f
#define Fire_Feeder_Finish_Angle_Error_Degree                 1.0f
#define Fire_Feeder_Finish_Speed_Rad_s                        0.1f

#endif /* __APP_FIRE_CONFIG_H__ */
