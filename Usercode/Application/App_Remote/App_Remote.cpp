/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Remote.cpp
  * @brief   App层遥控数据接收
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Remote.h"
#include "Application/App_CAN/App_CAN.h"
#include "DR16.h"
#include "Remote_Data_Reception.h"

#define App_Remote_DR16_Alive_Detection_Period_ms 100U

//DR16和舵轮步兵遥控数据接收层对象
Class_DR16 DR16;
Class_SteeringWheel_Infantry_Remote_Data_Reception Remote_Data_Reception;

//当前主控是否直接连接DR16
bool DR16_Local_Board = false;
uint32_t DR16_Last_Alive_Detection_Time = 0;

/**
 * @brief 初始化遥控数据APP
 *
 * @param Board_Role 当前主控角色
 */
void App_Remote_Init(SteeringWheel_Infantry_BoardCAN_Role_e Board_Role)
{
    Remote_Data_Reception.Init(&DR16,AppCAN_Get_BoardCAN(),Board_Role);
    DR16_Local_Board = Remote_Data_Reception.Is_DR16_Local_Board();
    DR16_Last_Alive_Detection_Time = 0;

    //只有DR16安装板才初始化UART5和启动接收DMA
    if (DR16_Local_Board)
    {
        DR16.Init(&huart5);
    }
}

/**
 * @brief 更新本地DR16或板间CAN遥控数据
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Remote_Update(uint32_t Now_ms)
{
    if (DR16_Local_Board)
    {
        DR16.Task_1ms_Data_Calculate();

        if (Now_ms - DR16_Last_Alive_Detection_Time >= App_Remote_DR16_Alive_Detection_Period_ms)
        {
            DR16_Last_Alive_Detection_Time = Now_ms;
            DR16.Task_100ms_Alive_Detection();
        }
    }

    //安装板读取本地DR16并向CAN2发送，非安装板从CAN2读取
    Remote_Data_Reception.Update(Now_ms);
}

/**
 * @brief 获取遥控数据在线状态
 *
 * @return bool true表示当前遥控数据源在线
 */
bool App_Remote_Get_Online_State(void)
{
    return Remote_Data_Reception.Get_Online_State();
}

/**
 * @brief 获取右侧X轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_X(void)
{
    return Remote_Data_Reception.Get_Right_X();
}

/**
 * @brief 获取右侧Y轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_Y(void)
{
    return Remote_Data_Reception.Get_Right_Y();
}

/**
 * @brief 获取左侧X轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_X(void)
{
    return Remote_Data_Reception.Get_Left_X();
}

/**
 * @brief 获取左侧Y轴摇杆数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_Y(void)
{
    return Remote_Data_Reception.Get_Left_Y();
}

/**
 * @brief 获取拨轮数据
 *
 * @return float 归一化到-1~1的拨轮数据
 */
float App_Remote_Get_Dial_Wheel(void)
{
    return Remote_Data_Reception.Get_Dial_Wheel();
}

/**
 * @brief 获取左侧三档开关状态
 *
 * @return BoardCAN_Remote_ThreeKey_e 左侧三档开关状态
 */
BoardCAN_Remote_ThreeKey_e App_Remote_Get_Left_ThreeKey(void)
{
    return Remote_Data_Reception.Get_Left_ThreeKey();
}

/**
 * @brief 获取右侧三档开关状态
 *
 * @return BoardCAN_Remote_ThreeKey_e 右侧三档开关状态
 */
BoardCAN_Remote_ThreeKey_e App_Remote_Get_Right_ThreeKey(void)
{
    return Remote_Data_Reception.Get_Right_ThreeKey();
}
