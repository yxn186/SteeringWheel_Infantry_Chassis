/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Fire.h
  * @brief   发射机构应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_FIRE_H__
#define __APP_FIRE_H__

#include "main.h"
#include "BoardCAN.h"

/**
 * @brief 发射机构运行状态
 */
enum class App_Fire_States_e : uint8_t
{
    Disabled = 0,
    Waiting_Feedback,
    Running,
    Stalled
};

/**
 * @brief 初始化本板发射电机、PID和安装位置
 *
 * @param Board_Role 当前主控角色
 */
void App_Fire_Init(SteeringWheel_Infantry_BoardCAN_Role_e Board_Role);

/**
 * @brief 更新发射模式、电机闭环和堵转保护
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Fire_Update(uint32_t Now_ms);

/**
 * @brief 处理CAN1上的发射电机反馈
 *
 * @param CAN_ID CAN标准帧ID
 * @param Data CAN数据区
 * @param Length CAN数据长度
 * @return bool true表示该帧属于本板发射电机
 */
bool App_Fire_Process_CAN_Message(uint16_t CAN_ID,const uint8_t *Data,uint8_t Length);

/**
 * @brief 请求拨弹盘发射一发弹丸
 *
 * @details 请求只在发射模式有效且拨弹盘安装在本板时执行。
 */
void App_Fire_Request_Single_Shot(void);

/**
 * @brief 判断拨弹盘是否安装在当前主控
 *
 * @return bool true表示本板负责拨弹盘控制
 */
bool App_Fire_Is_Feeder_Local_Board(void);

/**
 * @brief 获取发射机构运行状态
 *
 * @return App_Fire_States_e 当前运行状态
 */
App_Fire_States_e App_Fire_Get_States(void);

/**
 * @brief 获取堵转锁止状态
 *
 * @return bool true表示发射电机已经锁止为零输出
 */
bool App_Fire_Get_Stall_Latched_State(void);

/**
 * @brief 获取摩擦轮运行状态
 *
 * @return bool true表示四摩擦轮正在执行速度闭环
 */
bool App_Fire_Get_Friction_Enabled_State(void);

/**
 * @brief 获取拨弹盘单发动作状态
 *
 * @return bool true表示拨弹盘正在转向下一发位置
 */
bool App_Fire_Get_Feeder_Shot_In_Progress_State(void);

/**
 * @brief 获取拨弹盘连续角度目标
 *
 * @return float 拨弹盘目标角度，单位degree
 */
float App_Fire_Get_Feeder_Target_Angle_Degree(void);

#endif /* __APP_FIRE_H__ */
