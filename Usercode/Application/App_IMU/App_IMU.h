/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_IMU.h
  * @brief   底盘IMU应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_IMU_H__
#define __APP_IMU_H__

#include "main.h"

/**
 * @brief 底盘IMU应用运行状态
 */
enum class App_IMU_States_e : uint8_t
{
    Uninitialized = 0,
    Initializing,
    Calibrating,
    Ready,
    Offline,
    Error
};

/**
 * @brief 初始化底盘IMU应用、BMI088驱动、校准器和姿态解算器
 */
void App_IMU_Init(void);

/**
 * @brief 更新BMI088采样、零偏校准和姿态解算
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_IMU_Update(uint32_t Now_ms);

/**
 * @brief 处理BMI088数据就绪中断
 *
 * @param GPIO_Pin 触发中断的GPIO引脚
 */
void App_IMU_Process_GPIO_EXTI(uint16_t GPIO_Pin);

/**
 * @brief 获取底盘IMU应用当前运行状态
 *
 * @return App_IMU_States_e 当前运行状态
 */
App_IMU_States_e App_IMU_Get_States(void);

/**
 * @brief 判断底盘IMU是否已经完成校准并输出有效姿态
 *
 * @return bool true表示姿态数据已经可以使用
 */
bool App_IMU_Get_Ready_State(void);

/**
 * @brief 判断BMI088是否在反馈超时时间内收到过完整数据
 *
 * @return bool true表示BMI088当前在线
 */
bool App_IMU_Get_Online_State(void);

/**
 * @brief 获取启动零偏校准进度
 *
 * @return float 校准进度，范围为0.0到1.0
 */
float App_IMU_Get_Calibration_Progress(void);

/**
 * @brief 获取姿态四元数的W分量
 *
 * @return float 四元数W分量
 */
float App_IMU_Get_Quaternion_W(void);

/**
 * @brief 获取姿态四元数的X分量
 *
 * @return float 四元数X分量
 */
float App_IMU_Get_Quaternion_X(void);

/**
 * @brief 获取姿态四元数的Y分量
 *
 * @return float 四元数Y分量
 */
float App_IMU_Get_Quaternion_Y(void);

/**
 * @brief 获取姿态四元数的Z分量
 *
 * @return float 四元数Z分量
 */
float App_IMU_Get_Quaternion_Z(void);

/**
 * @brief 获取启动零点下的包角Yaw
 *
 * @return float Yaw角，单位degree，范围为(-180,180]
 */
float App_IMU_Get_Yaw_Degree(void);

/**
 * @brief 获取机体系Pitch角
 *
 * @return float Pitch角，单位degree
 */
float App_IMU_Get_Pitch_Degree(void);

/**
 * @brief 获取机体系Roll角
 *
 * @return float Roll角，单位degree
 */
float App_IMU_Get_Roll_Degree(void);

/**
 * @brief 获取跨越正负180度后仍连续的Yaw角
 *
 * @return float 连续Yaw角，单位degree
 */
float App_IMU_Get_Continuous_Yaw_Degree(void);

/**
 * @brief 获取机体系X轴角速度
 *
 * @return float X轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_X_Rad_s(void);

/**
 * @brief 获取机体系Y轴角速度
 *
 * @return float Y轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_Y_Rad_s(void);

/**
 * @brief 获取机体系Z轴角速度
 *
 * @return float Z轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_Z_Rad_s(void);

/**
 * @brief 获取机体系X轴加速度
 *
 * @return float X轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_X_m_s2(void);

/**
 * @brief 获取机体系Y轴加速度
 *
 * @return float Y轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_Y_m_s2(void);

/**
 * @brief 获取机体系Z轴加速度
 *
 * @return float Z轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_Z_m_s2(void);

#endif /* __APP_IMU_H__ */

