/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Attitude.h
  * @brief   底盘与云台姿态关系应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_CHASSIS_ATTITUDE_H__
#define __APP_CHASSIS_ATTITUDE_H__

#include "main.h"

/**
 * @brief 底盘与云台姿态关系运行状态
 */
enum class App_Chassis_Attitude_States_e : uint8_t
{
    Uninitialized = 0,
    Waiting_Data,
    Ready
};

/**
 * @brief 初始化CAN2云台Yaw电机只读对象和姿态状态
 */
void App_Chassis_Attitude_Init(void);

/**
 * @brief 更新Yaw电机在线状态和两板姿态关系
 */
void App_Chassis_Attitude_Update(void);

/**
 * @brief 处理CAN2上的云台Yaw电机反馈
 *
 * @param CAN_ID CAN标准帧ID
 * @param Data CAN数据区
 * @param Length CAN数据长度
 * @return bool true表示该帧属于云台Yaw电机
 */
bool App_Chassis_Attitude_Process_CAN_Message(uint16_t CAN_ID,const uint8_t *Data,uint8_t Length);

/**
 * @brief 将云台坐标系速度转换到底盘坐标系
 *
 * @param Gimbal_Speed_X 云台前后方向速度，单位m/s
 * @param Gimbal_Speed_Y 云台左右方向速度，单位m/s
 * @param Chassis_Speed_X 输出的底盘前后方向速度，单位m/s
 * @param Chassis_Speed_Y 输出的底盘左右方向速度，单位m/s
 */
void App_Chassis_Attitude_Transform_Gimbal_To_Chassis(float Gimbal_Speed_X,float Gimbal_Speed_Y,float *Chassis_Speed_X,float *Chassis_Speed_Y);

/**
 * @brief 获取姿态关系模块运行状态
 *
 * @return App_Chassis_Attitude_States_e 当前运行状态
 */
App_Chassis_Attitude_States_e App_Chassis_Attitude_Get_States(void);

/**
 * @brief 判断底盘IMU和云台Yaw反馈是否均可使用
 *
 * @return bool true表示姿态关系数据有效
 */
bool App_Chassis_Attitude_Get_Ready_State(void);

/**
 * @brief 获取CAN2云台Yaw电机在线状态
 *
 * @return bool true表示Yaw电机反馈未超时
 */
bool App_Chassis_Attitude_Get_Yaw_Motor_Online_State(void);

/**
 * @brief 获取云台相对底盘的Yaw角
 *
 * @return float 云台相对Yaw角，单位degree，范围为(-180,180]
 */
float App_Chassis_Attitude_Get_Gimbal_Relative_Yaw_Degree(void);

/**
 * @brief 获取底盘启动零点下的连续Yaw角
 *
 * @return float 底盘连续Yaw角，单位degree
 */
float App_Chassis_Attitude_Get_Chassis_Continuous_Yaw_Degree(void);

/**
 * @brief 获取启动零点下的云台世界系Yaw角
 *
 * @return float 云台世界系Yaw角，单位degree
 */
float App_Chassis_Attitude_Get_Gimbal_World_Yaw_Degree(void);

#endif /* __APP_CHASSIS_ATTITUDE_H__ */
