/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Control.cpp
  * @brief   App层底盘遥控控制
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Chassis_Control.h"
#include "App_Chassis_Control_Config.h"
#include "Application/App_CAN/App_CAN.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "Remote_Data_Reception.h"

//舵轮步兵遥控数据接收类对象
static Class_SteeringWheel_Infantry_Remote_Data_Reception Remote_Data_Reception;

/**
 * @brief 初始化底盘遥控控制APP
 */
void App_Chassis_Control_Init(void)
{
    //DR16安装在云台板，底盘板通过板间CAN获取遥控数据
    Remote_Data_Reception.Init(nullptr,AppCAN_Get_BoardCAN(),SteeringWheel_Infantry_BoardCAN_Role_e::Chassis);
}

/**
 * @brief 更新底盘遥控控制状态
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Chassis_Control_Update(uint32_t Now_ms)
{
    Remote_Data_Reception.Update(Now_ms);

    //遥控数据离线时，底盘立即进入无力状态
    if (!Remote_Data_Reception.Get_Online_State())
    {
        App_Chassis_No_Power();
        return;
    }

    //获取遥控器三档开关状态
    const BoardCAN_Remote_ThreeKey_e Left_ThreeKey = Remote_Data_Reception.Get_Left_ThreeKey();
    const BoardCAN_Remote_ThreeKey_e Right_ThreeKey = Remote_Data_Reception.Get_Right_ThreeKey();

    //双中：普通遥控模式，当前直接使用底盘坐标系
    if ((Left_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle) && (Right_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle))
    {
        float Speed_X = Remote_Data_Reception.Get_Left_Y() * Chassis_Control_Normal_Max_Speed_X * Chassis_Control_Remote_Speed_X_Direction;
        float Speed_Y = Remote_Data_Reception.Get_Left_X() * Chassis_Control_Normal_Max_Speed_Y * Chassis_Control_Remote_Speed_Y_Direction;
        float W_Z = Remote_Data_Reception.Get_Right_X() * Chassis_Control_Normal_Max_W_Z * Chassis_Control_Remote_W_Z_Direction;

        App_Chassis_Update(Speed_X,Speed_Y,W_Z);
        return;
    }

    //双下以及当前未定义的其他拨杆组合均进入无力状态
    App_Chassis_No_Power();
}
