/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Remote.h
  * @brief   This file contains all the function prototypes for
  *          the App_Remote.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_REMOTE_H__
#define __APP_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "BoardCAN.h"
/*YOUR CODE*/

/**
 * @brief 初始化遥控数据APP
 *
 * @param Board_Role 当前主控角色
 */
void App_Remote_Init(SteeringWheel_Infantry_BoardCAN_Role_e Board_Role);

/**
 * @brief 更新本地DR16或板间CAN遥控数据
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Remote_Update(uint32_t Now_ms);

/**
 * @brief 获取遥控数据在线状态
 *
 * @return bool true表示当前遥控数据源在线
 */
bool App_Remote_Get_Online_State(void);

/**
 * @brief 获取右侧X轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_X(void);

/**
 * @brief 获取右侧Y轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_Y(void);

/**
 * @brief 获取左侧X轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_X(void);

/**
 * @brief 获取左侧Y轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_Y(void);

/**
 * @brief 获取拨轮数据
 *
 * @return float 归一化到-1~1的拨轮数据
 */
float App_Remote_Get_Dial_Wheel(void);

/**
 * @brief 获取左侧三档开关状态
 *
 * @return BoardCAN_Remote_ThreeKey_e 左侧三档开关状态
 */
BoardCAN_Remote_ThreeKey_e App_Remote_Get_Left_ThreeKey(void);

/**
 * @brief 获取右侧三档开关状态
 *
 * @return BoardCAN_Remote_ThreeKey_e 右侧三档开关状态
 */
BoardCAN_Remote_ThreeKey_e App_Remote_Get_Right_ThreeKey(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_REMOTE_H__ */
