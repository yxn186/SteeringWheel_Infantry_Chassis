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

    //注册电机
    for(uint8_t i = 0; i < 4; i++)
    {
        //注册轮电机
        Wheel_Motor_Group.Register_Motor(&Wheel_Motor[i]);
        //注册舵电机
        Steering_Motor_Group.Register_Motor(&Steering_Motor[i]);
    }
}

void App_Chassis_Update(float Speed_X,float Speed_Y,float W_Z)
{
    //设置当前值
    for(uint8_t i = 0; i < 4; i++)
    {
        //轮电机：传入角速度
        Wheel_Motor_PID[i].Set_Current_Speed(Wheel_Motor[i].Get_AngleSpeed());

        //舵电机：传入角速度and角度
        Steering_Motor_PID[i].Set_Current_Speed(Steering_Motor[i].Get_AngleSpeed());
        Steering_Motor_PID[i].Set_Current_Angle(Steering_Motor[i].Get_Continuous_Angle());

        SteeringWheel_Chassis_Calculation.Set_Current_Wheel_Motor_Data(i, Wheel_Motor[i].Get_AngleSpeed(), Steering_Motor[i].Get_AngleSpeed(), Steering_Motor[i].Get_Continuous_Angle());
    }

    //传入外部目标
    SteeringWheel_Chassis_Calculation.Set_Target_Chassis_Data(Speed_X, Speed_Y, W_Z);

    //数据更新计算
    SteeringWheel_Chassis_Calculation.Update();

    //目标计算完成 传入PID 进行计算 将Out传入电机
    for(uint8_t i = 0; i < 4; i++)
    {
        //设置轮电机目标速度
        Wheel_Motor_PID[i].Set_Speed_Target(SteeringWheel_Chassis_Calculation.Get_Target_Wheel_Angular_Speed(i));
        //PID计算
        Wheel_Motor_PID[i].Control_Speed_To_Out();
        //设置输出
        Wheel_Motor[i].Set_Out(Wheel_Motor_PID[i].Get_Out());
        

        //设置舵电机目标角度
        Steering_Motor_PID[i].Set_Angle_Target(SteeringWheel_Chassis_Calculation.Get_Target_Steering_Angle(i));
        //PID计算
        Steering_Motor_PID[i].Control_Cascade();
        //设置输出
        Steering_Motor[i].Set_Out(Steering_Motor_PID[i].Get_Out());
    }

    //PushOut
    Wheel_Motor_Group.Push_Data();
    Steering_Motor_Group.Push_Data();
}