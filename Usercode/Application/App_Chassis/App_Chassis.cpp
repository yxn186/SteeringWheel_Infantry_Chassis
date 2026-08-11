/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis.cpp
  * @brief   App层底盘库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Chassis.h"
#include "PID.h"
#include "SteeringWheel_Chassis_Calculation.h"
#include "bsp_fdcan.h"
#include "Application/App_CAN/App_CAN.h"
#include "DJI_Motor.h"
#include "CAN_Interface.h"
#include "App_Chassis_Config.h"
#include <cstdint>


//大疆电机组类
Class_DJI_Motor_Group Wheel_Motor_Group;
Class_DJI_Motor_Group Steering_Motor_Group;

//大疆电机类
Class_DJI_Motor Wheel_Motor[4];
Class_DJI_Motor Steering_Motor[4];

//舵轮底盘解算轮类
Class_SteeringWheel_Chassis_Calculation SteeringWheel_Chassis_Calculation;

//PID库
Class_PID Wheel_Motor_PID[4];
Class_PID Steering_Motor_PID[4];

//底盘全部大疆电机在线状态，便于运行时观察
bool All_Motors_Online = false;

/**
 * @brief 更新并检查底盘全部大疆电机的在线状态
 *
 * @return true 4个轮电机和4个舵电机均已收到合法反馈且未超时
 * @return false 任一电机尚未收到合法反馈或反馈已经超时
 */
static bool App_Chassis_All_Motors_Online(void)
{
    bool All_Motors_Online = true;

    for (uint8_t i = 0; i < 4; i++)
    {
        Wheel_Motor[i].Update_Online_State();
        Steering_Motor[i].Update_Online_State();

        if (!Wheel_Motor[i].Get_Online_State() || !Steering_Motor[i].Get_Online_State())
        {
            All_Motors_Online = false;
        }
    }

    return All_Motors_Online;
}

/**
 * @brief 清零底盘全部电机和PID输出，并发送零控制帧
 *
 * 上电反馈未就绪或运行中任一电机掉线时调用，避免电调继续执行
 * 最近一次非零控制命令。
 */
static void App_Chassis_Force_Zero_Output(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        Wheel_Motor_PID[i].Reset();
        Steering_Motor_PID[i].Reset();

        Wheel_Motor[i].Set_Out(0);
        Steering_Motor[i].Set_Out(0);
    }

    Wheel_Motor_Group.Push_Data();
    Steering_Motor_Group.Push_Data();
}

/**
 * @brief 底盘CAN接收回调函数
 * 
 * @param CAN_Index CAN总线索引
 * @param id CAN报文ID
 * @param data CAN报文数据
 * @param length CAN报文数据长度
 */
void App_Chassis_CAN_RX_Callback(uint8_t CAN_Index,uint16_t id,const uint8_t *data,uint8_t length)
{
    if (CAN_Index == 1)
    {
        Wheel_Motor_Group.Process_CAN_Feedback(id,data,length);
    }
    else if (CAN_Index == 3)
    {
        Steering_Motor_Group.Process_CAN_Feedback(id,data,length);
    }
}

void App_Chassis_Init(void)
{
    //初始化舵轮底盘解算库
    SteeringWheel_Chassis_Calculation.Init(Chassis_a, Chassis_b, Wheel_Radius, Max_WheelMotor_Linear_Speed);

    //初始化舵轮电机组
    //轮电机CAN1
    Wheel_Motor_Group.Init(AppCAN_Get_Interface(1), DJI_Motor_3508);
    //舵电机CAN3
    Steering_Motor_Group.Init(AppCAN_Get_Interface(3), DJI_Motor_6020);

    //轮电机初始化
    Wheel_Motor[0].Init(DJI_Motor_3508, Wheel_Motor_0_ID, &Wheel_Motor_Group);
    Wheel_Motor[1].Init(DJI_Motor_3508, Wheel_Motor_1_ID, &Wheel_Motor_Group);
    Wheel_Motor[2].Init(DJI_Motor_3508, Wheel_Motor_2_ID, &Wheel_Motor_Group);
    Wheel_Motor[3].Init(DJI_Motor_3508, Wheel_Motor_3_ID, &Wheel_Motor_Group);

    //舵电机初始化
    Steering_Motor[0].Init(DJI_Motor_6020, Steering_Motor_0_ID, &Steering_Motor_Group);
    Steering_Motor[1].Init(DJI_Motor_6020, Steering_Motor_1_ID, &Steering_Motor_Group);
    Steering_Motor[2].Init(DJI_Motor_6020, Steering_Motor_2_ID, &Steering_Motor_Group);
    Steering_Motor[3].Init(DJI_Motor_6020, Steering_Motor_3_ID, &Steering_Motor_Group);

    //配置PID参数
    for(uint8_t i = 0; i < 4; i++)
    {
        Wheel_Motor_PID[i].Kp_s = Wheel_Motor_PID_Kp_s;
        Wheel_Motor_PID[i].Ki_s = Wheel_Motor_PID_Ki_s;
        Wheel_Motor_PID[i].Kd_s = Wheel_Motor_PID_Kd_s;
        Wheel_Motor_PID[i].ErrorInt_High_s = Wheel_Motor_PID_ErrorInt_High_s;
        Wheel_Motor_PID[i].ErrorInt_Low_s = Wheel_Motor_PID_ErrorInt_Low_s;
        Wheel_Motor_PID[i].Integral_Stop_Near_Zero_Enable_s = Wheel_Motor_PID_Integral_Stop_Near_Zero_Enable_s;
        Wheel_Motor_PID[i].Integral_Stop_Target_Abs_Threshold_s = Wheel_Motor_PID_Integral_Stop_Target_Abs_Threshold_s;
        Wheel_Motor_PID[i].Integral_Stop_Error_Abs_Threshold_s = Wheel_Motor_PID_Integral_Stop_Error_Abs_Threshold_s;
        Wheel_Motor_PID[i].Out_High = Wheel_Motor_PID_Out_High;
        Wheel_Motor_PID[i].Out_Low = Wheel_Motor_PID_Out_Low;

        Steering_Motor_PID[i].Kp_s = Steering_Motor_PID_Kp_s;
        Steering_Motor_PID[i].Ki_s = Steering_Motor_PID_Ki_s;
        Steering_Motor_PID[i].Kd_s = Steering_Motor_PID_Kd_s;
        Steering_Motor_PID[i].ErrorInt_High_s = Steering_Motor_PID_ErrorInt_High_s;
        Steering_Motor_PID[i].ErrorInt_Low_s = Steering_Motor_PID_ErrorInt_Low_s;
        Steering_Motor_PID[i].Integral_Stop_Near_Zero_Enable_s = Steering_Motor_PID_Integral_Stop_Near_Zero_Enable_s;
        Steering_Motor_PID[i].Integral_Stop_Target_Abs_Threshold_s = Steering_Motor_PID_Integral_Stop_Target_Abs_Threshold_s;
        Steering_Motor_PID[i].Integral_Stop_Error_Abs_Threshold_s = Steering_Motor_PID_Integral_Stop_Error_Abs_Threshold_s;
        Steering_Motor_PID[i].Out_High = Steering_Motor_PID_Out_High;
        Steering_Motor_PID[i].Out_Low = Steering_Motor_PID_Out_Low;

        Steering_Motor_PID[i].Kp_a = Steering_Motor_PID_Kp_a;
        Steering_Motor_PID[i].Ki_a = Steering_Motor_PID_Ki_a;
        Steering_Motor_PID[i].Kd_a = Steering_Motor_PID_Kd_a;
        Steering_Motor_PID[i].ErrorInt_High_a = Steering_Motor_PID_ErrorInt_High_a;
        Steering_Motor_PID[i].ErrorInt_Low_a = Steering_Motor_PID_ErrorInt_Low_a;
        Steering_Motor_PID[i].FeedForward_Enable_a = Steering_Motor_PID_FeedForward_Enable_a;
        Steering_Motor_PID[i].Kf_a = Steering_Motor_PID_Kf_a;
        Steering_Motor_PID[i].FeedForward_High_a = Steering_Motor_PID_FeedForward_High_a;
        Steering_Motor_PID[i].FeedForward_Low_a = Steering_Motor_PID_FeedForward_Low_a;
        Steering_Motor_PID[i].Integral_Stop_Near_Zero_Enable_a = Steering_Motor_PID_Integral_Stop_Near_Zero_Enable_a;
        Steering_Motor_PID[i].Integral_Stop_Target_Abs_Threshold_a = Steering_Motor_PID_Integral_Stop_Target_Abs_Threshold_a;
        Steering_Motor_PID[i].Integral_Stop_Error_Abs_Threshold_a = Steering_Motor_PID_Integral_Stop_Error_Abs_Threshold_a;
        Steering_Motor_PID[i].Speed_Target_High = Steering_Motor_PID_Speed_Target_High;
        Steering_Motor_PID[i].Speed_Target_Low = Steering_Motor_PID_Speed_Target_Low;
    }
}

/**
 * @brief 更新底盘反馈、PID当前值和运动学解算状态
 *
 * @param Speed_X 底盘前后速度 前正后负 单位m/s
 * @param Speed_Y 底盘左右速度 左正右负 单位m/s
 * @param W_Z 底盘旋转速度 逆时针为正 单位rad/s
 */
static void App_Chassis_Update_State(float Speed_X,float Speed_Y,float W_Z)
{
    All_Motors_Online = App_Chassis_All_Motors_Online();

    //设置当前值
    for(uint8_t i = 0; i < 4; i++)
    {
        //轮电机：传入角速度
        Wheel_Motor_PID[i].Set_Current_Speed(Wheel_Motor[i].Get_AngleSpeed());

        //舵电机：传入角速度and角度
        float Steering_Motor_Current_Angle = 0.0f;
        if (i == 0)
        {
            Steering_Motor_Current_Angle = Steering_Motor[i].Get_Continuous_Angle() - Steering_Motor_0_Zero_Angle;
        }
        else if (i == 1)
        {
            Steering_Motor_Current_Angle = Steering_Motor[i].Get_Continuous_Angle() - Steering_Motor_1_Zero_Angle;
        }
        else if (i == 2)
        {
            Steering_Motor_Current_Angle = Steering_Motor[i].Get_Continuous_Angle() - Steering_Motor_2_Zero_Angle;
        }
        else
        {
            Steering_Motor_Current_Angle = Steering_Motor[i].Get_Continuous_Angle() - Steering_Motor_3_Zero_Angle;
        }

        Steering_Motor_PID[i].Set_Current_Speed(Steering_Motor[i].Get_AngleSpeed());
        Steering_Motor_PID[i].Set_Current_Angle(Steering_Motor_Current_Angle);

        SteeringWheel_Chassis_Calculation.Set_Current_Wheel_Motor_Data(i, Wheel_Motor[i].Get_AngleSpeed(), Steering_Motor[i].Get_AngleSpeed(), Steering_Motor_Current_Angle);
    }

    //传入外部目标
    SteeringWheel_Chassis_Calculation.Set_Target_Chassis_Data(Speed_X, Speed_Y, W_Z);

    //数据更新计算
    SteeringWheel_Chassis_Calculation.Update();
}

/**
 * @brief 底盘进入无力状态
 *
 * @details 保持底盘状态刷新，重置PID并持续发送零输出
 */
void App_Chassis_No_Power(void)
{
    App_Chassis_Update_State(0.0f,0.0f,0.0f);
    App_Chassis_Force_Zero_Output();
}

/**
 * @brief 底盘数据更新
 *
 * @param Speed_X 底盘前后速度 前正后负 单位m/s
 * @param Speed_Y 底盘左右速度 左正右负 单位m/s
 * @param W_Z 底盘旋转速度 逆时针为正 单位rad/s
 */
void App_Chassis_Update(float Speed_X,float Speed_Y,float W_Z)
{
    App_Chassis_Update_State(Speed_X,Speed_Y,W_Z);

    // 反馈未全部就绪时继续刷新底盘数据，但禁止PID计算和非零输出。
    if (!All_Motors_Online)
    {
        App_Chassis_Force_Zero_Output();
        return;
    }

    //目标计算完成 传入PID 进行计算 将Out传入电机
    for(uint8_t i = 0; i < 4; i++)
    {
        //设置轮电机目标速度
        Wheel_Motor_PID[i].Set_Speed_Target(SteeringWheel_Chassis_Calculation.Get_Target_Wheel_Angular_Speed(i));
        //PID计算
        Wheel_Motor_PID[i].Control_Speed_To_Out();
        //设置输出
        if (i == 0)
        {
            Wheel_Motor[i].Set_Out(Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_0_Direction);
        }
        else if (i == 1)
        {
            Wheel_Motor[i].Set_Out(Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_1_Direction);
        }
        else if (i == 2)
        {
            Wheel_Motor[i].Set_Out(Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_2_Direction);
        }
        else
        {
            Wheel_Motor[i].Set_Out(Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_3_Direction);
        }
        

        //设置舵电机目标角度
        Steering_Motor_PID[i].Set_Angle_Target(SteeringWheel_Chassis_Calculation.Get_Target_Steering_Angle(i));
        //PID计算
        Steering_Motor_PID[i].Control_Cascade();
        //设置输出
        if (i == 0)
        {
            Steering_Motor[i].Set_Out(Steering_Motor_PID[i].Get_Out() * Steering_Motor_0_Direction);
        }
        else if (i == 1)
        {
            Steering_Motor[i].Set_Out(Steering_Motor_PID[i].Get_Out() * Steering_Motor_1_Direction);
        }
        else if (i == 2)
        {
            Steering_Motor[i].Set_Out(Steering_Motor_PID[i].Get_Out() * Steering_Motor_2_Direction);
        }
        else
        {
            Steering_Motor[i].Set_Out(Steering_Motor_PID[i].Get_Out() * Steering_Motor_3_Direction);
        }
    }

    //PushOut
    Wheel_Motor_Group.Push_Data();
    Steering_Motor_Group.Push_Data();
}
