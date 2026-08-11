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
#include "Application/App_Chassis/App_Chassis.h"
#include "Application/App_Chassis_Attitude/App_Chassis_Attitude.h"
#include "Application/App_Remote/App_Remote.h"
#include "PID.h"

static Class_PID Chassis_Follow_Gimbal_PID;
static bool Chassis_Follow_Gimbal_Active = false;

/**
 * @brief 读取左摇杆并生成云台坐标系下的平移目标
 *
 * @param Speed_X 输出的云台前后方向速度，单位m/s
 * @param Speed_Y 输出的云台左右方向速度，单位m/s
 */
static void App_Chassis_Control_Get_Gimbal_Coordinate_Target(float *Speed_X,float *Speed_Y)
{
    if ((Speed_X == nullptr) || (Speed_Y == nullptr))
    {
        return;
    }

    *Speed_X = App_Remote_Get_Left_Y() * Chassis_Control_Normal_Max_Speed_X * Chassis_Control_Remote_Speed_X_Direction;
    *Speed_Y = App_Remote_Get_Left_X() * Chassis_Control_Normal_Max_Speed_Y * Chassis_Control_Remote_Speed_Y_Direction;
}

/**
 * @brief 离开跟随模式并清除角度环历史状态
 */
static void App_Chassis_Control_Stop_Follow_Gimbal(void)
{
    if (Chassis_Follow_Gimbal_Active)
    {
        Chassis_Follow_Gimbal_PID.Reset();
        Chassis_Follow_Gimbal_Active = false;
    }
}

/**
 * @brief 进入无力状态并清除跟随角度环
 */
static void App_Chassis_Control_No_Power(void)
{
    App_Chassis_Control_Stop_Follow_Gimbal();
    App_Chassis_No_Power();
}

/**
 * @brief 初始化底盘遥控控制APP
 */
void App_Chassis_Control_Init(void)
{
    App_Remote_Init(SteeringWheel_Infantry_BoardCAN_Role_e::Chassis);

    Chassis_Follow_Gimbal_PID.Reset();
    Chassis_Follow_Gimbal_PID.Kp_a = Chassis_Control_Follow_Gimbal_PID_Kp_a;
    Chassis_Follow_Gimbal_PID.Ki_a = Chassis_Control_Follow_Gimbal_PID_Ki_a;
    Chassis_Follow_Gimbal_PID.Kd_a = Chassis_Control_Follow_Gimbal_PID_Kd_a;
    Chassis_Follow_Gimbal_PID.ErrorInt_High_a = Chassis_Control_Follow_Gimbal_PID_ErrorInt_High_a;
    Chassis_Follow_Gimbal_PID.ErrorInt_Low_a = Chassis_Control_Follow_Gimbal_PID_ErrorInt_Low_a;
    Chassis_Follow_Gimbal_PID.Speed_Target_High = Chassis_Control_Follow_Gimbal_Max_W_Z;
    Chassis_Follow_Gimbal_PID.Speed_Target_Low = -Chassis_Control_Follow_Gimbal_Max_W_Z;
    Chassis_Follow_Gimbal_Active = false;
}

/**
 * @brief 更新底盘遥控控制状态
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Chassis_Control_Update(uint32_t Now_ms)
{
    App_Remote_Update(Now_ms);

    //遥控数据离线时，底盘立即进入无力状态
    if (!App_Remote_Get_Online_State())
    {
        App_Chassis_Control_No_Power();
        return;
    }

    //获取遥控器三档开关状态
    const BoardCAN_Remote_ThreeKey_e Left_ThreeKey = App_Remote_Get_Left_ThreeKey();
    const BoardCAN_Remote_ThreeKey_e Right_ThreeKey = App_Remote_Get_Right_ThreeKey();

    //左中右上：右摇杆交给云台，底盘只使用左摇杆完成XY平移
    if ((Left_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle) && (Right_ThreeKey == BoardCAN_Remote_ThreeKey_e::Up))
    {
        App_Chassis_Control_Stop_Follow_Gimbal();

        if (!App_Chassis_Attitude_Get_Yaw_Motor_Online_State())
        {
            App_Chassis_No_Power();
            return;
        }

        float Gimbal_Speed_X = 0.0f;
        float Gimbal_Speed_Y = 0.0f;
        float Speed_X = 0.0f;
        float Speed_Y = 0.0f;
        App_Chassis_Control_Get_Gimbal_Coordinate_Target(&Gimbal_Speed_X,&Gimbal_Speed_Y);
        App_Chassis_Attitude_Transform_Gimbal_To_Chassis(Gimbal_Speed_X,Gimbal_Speed_Y,&Speed_X,&Speed_Y);

        App_Chassis_Update(Speed_X,Speed_Y,0.0f);
        return;
    }

    //双中：普通遥控模式，左摇杆平移仍以云台正方向为前方
    if ((Left_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle) && (Right_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle))
    {
        App_Chassis_Control_Stop_Follow_Gimbal();

        if (!App_Chassis_Attitude_Get_Yaw_Motor_Online_State())
        {
            App_Chassis_No_Power();
            return;
        }

        float Gimbal_Speed_X = 0.0f;
        float Gimbal_Speed_Y = 0.0f;
        float Speed_X = 0.0f;
        float Speed_Y = 0.0f;
        App_Chassis_Control_Get_Gimbal_Coordinate_Target(&Gimbal_Speed_X,&Gimbal_Speed_Y);
        App_Chassis_Attitude_Transform_Gimbal_To_Chassis(Gimbal_Speed_X,Gimbal_Speed_Y,&Speed_X,&Speed_Y);
        float W_Z = App_Remote_Get_Right_X() * Chassis_Control_Normal_Max_W_Z * Chassis_Control_Remote_W_Z_Direction;

        App_Chassis_Update(Speed_X,Speed_Y,W_Z);
        return;
    }

    //左上右下：平移目标保持在云台坐标系，底盘同时高速自旋
    if ((Left_ThreeKey == BoardCAN_Remote_ThreeKey_e::Up) && (Right_ThreeKey == BoardCAN_Remote_ThreeKey_e::Down))
    {
        App_Chassis_Control_Stop_Follow_Gimbal();

        if (!App_Chassis_Attitude_Get_Ready_State())
        {
            App_Chassis_No_Power();
            return;
        }

        float Gimbal_Speed_X = 0.0f;
        float Gimbal_Speed_Y = 0.0f;
        float Chassis_Speed_X = 0.0f;
        float Chassis_Speed_Y = 0.0f;
        App_Chassis_Control_Get_Gimbal_Coordinate_Target(&Gimbal_Speed_X,&Gimbal_Speed_Y);
        App_Chassis_Attitude_Transform_Gimbal_To_Chassis(Gimbal_Speed_X,Gimbal_Speed_Y,&Chassis_Speed_X,&Chassis_Speed_Y);

        App_Chassis_Update(Chassis_Speed_X,Chassis_Speed_Y,Chassis_Control_Spin_W_Z * Chassis_Control_Spin_W_Z_Direction);
        return;
    }

    //左上右中：将底盘正方向闭环对准云台正方向
    if ((Left_ThreeKey == BoardCAN_Remote_ThreeKey_e::Up) && (Right_ThreeKey == BoardCAN_Remote_ThreeKey_e::Middle))
    {
        if (!App_Chassis_Attitude_Get_Ready_State())
        {
            App_Chassis_Control_No_Power();
            return;
        }

        if (!Chassis_Follow_Gimbal_Active)
        {
            Chassis_Follow_Gimbal_PID.Reset();
            Chassis_Follow_Gimbal_Active = true;
        }

        float Gimbal_Speed_X = 0.0f;
        float Gimbal_Speed_Y = 0.0f;
        float Chassis_Speed_X = 0.0f;
        float Chassis_Speed_Y = 0.0f;
        App_Chassis_Control_Get_Gimbal_Coordinate_Target(&Gimbal_Speed_X,&Gimbal_Speed_Y);
        App_Chassis_Attitude_Transform_Gimbal_To_Chassis(Gimbal_Speed_X,Gimbal_Speed_Y,&Chassis_Speed_X,&Chassis_Speed_Y);

        Chassis_Follow_Gimbal_PID.Set_Angle_Target(0.0f);
        Chassis_Follow_Gimbal_PID.Set_Current_Angle(App_Chassis_Attitude_Get_Gimbal_Relative_Yaw_Degree());
        Chassis_Follow_Gimbal_PID.Control_Angle_To_Speed();

        float W_Z = Chassis_Follow_Gimbal_PID.Get_Speed_Target() * Chassis_Control_Follow_Gimbal_Output_Direction;
        App_Chassis_Update(Chassis_Speed_X,Chassis_Speed_Y,W_Z);
        return;
    }

    //双下以及当前未定义的其他拨杆组合均进入无力状态
    App_Chassis_Control_No_Power();
}
