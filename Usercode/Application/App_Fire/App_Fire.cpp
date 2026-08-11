/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Fire.cpp
  * @brief   发射机构应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#include "App_Fire.h"
#include "App_Fire_Config.h"
#include "Application/App_CAN/App_CAN.h"
#include "Application/App_Remote/App_Remote.h"
#include "DJI_Motor.h"
#include "PID.h"
#include <math.h>

static Class_DJI_Motor_Group Fire_Motor_Group;
static Class_DJI_Motor Friction_Motor[Fire_Friction_Motor_Count];
static Class_DJI_Motor Feeder_Motor;
static Class_PID Friction_Motor_PID[Fire_Friction_Motor_Count];
static Class_PID Feeder_Motor_PID;

static SteeringWheel_Infantry_BoardCAN_Role_e Fire_Current_Board_Role = SteeringWheel_Infantry_BoardCAN_Role_e::Gimbal;
static App_Fire_States_e App_Fire_States = App_Fire_States_e::Disabled;
static bool App_Fire_Init_Finished = false;
static bool Fire_Friction_Local_Board = false;
static bool Fire_Feeder_Local_Board = false;
static bool Fire_Mode_Active = false;
static bool Fire_Stall_Latched = false;
static bool Fire_Feeder_Target_Initialized = false;
static bool Fire_Feeder_Shot_In_Progress = false;
static bool Fire_Single_Shot_Request = false;
static float Fire_Feeder_Target_Angle_Degree = 0.0f;
static uint32_t Fire_Mode_Enter_Time = 0;
static uint32_t Fire_Friction_Stall_Start_Time[Fire_Friction_Motor_Count] = {0};
static uint32_t Fire_Feeder_Stall_Start_Time = 0;

/**
 * @brief 获取摩擦轮电机ID
 *
 * @param Index 摩擦轮序号，范围0到3
 * @return uint8_t 对应的M3508电机ID
 */
static uint8_t App_Fire_Get_Friction_Motor_ID(uint8_t Index)
{
    const uint8_t Motor_ID[Fire_Friction_Motor_Count] = {Fire_Friction_Motor_0_ID,Fire_Friction_Motor_1_ID,Fire_Friction_Motor_2_ID,Fire_Friction_Motor_3_ID};
    return Motor_ID[Index];
}

/**
 * @brief 获取摩擦轮电机安装方向
 *
 * @param Index 摩擦轮序号，范围0到3
 * @return float 电机目标速度方向系数
 */
static float App_Fire_Get_Friction_Motor_Direction(uint8_t Index)
{
    const float Motor_Direction[Fire_Friction_Motor_Count] = {Fire_Friction_Motor_0_Direction,Fire_Friction_Motor_1_Direction,Fire_Friction_Motor_2_Direction,Fire_Friction_Motor_3_Direction};
    return Motor_Direction[Index];
}

/**
 * @brief 初始化摩擦轮和拨弹盘PID参数
 */
static void App_Fire_Init_PID(void)
{
    for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
    {
        Friction_Motor_PID[i].Reset();
        Friction_Motor_PID[i].Kp_s = Fire_Friction_PID_Kp_s;
        Friction_Motor_PID[i].Ki_s = Fire_Friction_PID_Ki_s;
        Friction_Motor_PID[i].Kd_s = Fire_Friction_PID_Kd_s;
        Friction_Motor_PID[i].ErrorInt_High_s = Fire_Friction_PID_ErrorInt_High_s;
        Friction_Motor_PID[i].ErrorInt_Low_s = Fire_Friction_PID_ErrorInt_Low_s;
        Friction_Motor_PID[i].Out_High = Fire_Friction_PID_Out_High;
        Friction_Motor_PID[i].Out_Low = Fire_Friction_PID_Out_Low;
    }

    Feeder_Motor_PID.Reset();
    Feeder_Motor_PID.Kp_a = Fire_Feeder_PID_Kp_a;
    Feeder_Motor_PID.Ki_a = Fire_Feeder_PID_Ki_a;
    Feeder_Motor_PID.Kd_a = Fire_Feeder_PID_Kd_a;
    Feeder_Motor_PID.ErrorInt_High_a = Fire_Feeder_PID_ErrorInt_High_a;
    Feeder_Motor_PID.ErrorInt_Low_a = Fire_Feeder_PID_ErrorInt_Low_a;
    Feeder_Motor_PID.Speed_Target_High = Fire_Feeder_PID_Max_Speed_Rad_s;
    Feeder_Motor_PID.Speed_Target_Low = -Fire_Feeder_PID_Max_Speed_Rad_s;
    Feeder_Motor_PID.Kp_s = Fire_Feeder_PID_Kp_s;
    Feeder_Motor_PID.Ki_s = Fire_Feeder_PID_Ki_s;
    Feeder_Motor_PID.Kd_s = Fire_Feeder_PID_Kd_s;
    Feeder_Motor_PID.ErrorInt_High_s = Fire_Feeder_PID_ErrorInt_High_s;
    Feeder_Motor_PID.ErrorInt_Low_s = Fire_Feeder_PID_ErrorInt_Low_s;
    Feeder_Motor_PID.Out_High = Fire_Feeder_PID_Out_High;
    Feeder_Motor_PID.Out_Low = Fire_Feeder_PID_Out_Low;
}

/**
 * @brief 清除全部发射PID历史状态
 */
static void App_Fire_Reset_PID(void)
{
    for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
    {
        Friction_Motor_PID[i].Reset();
    }
    Feeder_Motor_PID.Reset();
}

/**
 * @brief 判断当前板是否存在需要控制的发射电机
 *
 * @return bool true表示本板安装了摩擦轮或拨弹盘
 */
static bool App_Fire_Has_Local_Motor(void)
{
    return Fire_Friction_Local_Board || Fire_Feeder_Local_Board;
}

/**
 * @brief 持续清零本板全部发射电机输出
 */
static void App_Fire_Force_Zero_Output(void)
{
    if (Fire_Friction_Local_Board)
    {
        for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
        {
            Friction_Motor[i].Set_Out(0);
        }
    }

    if (Fire_Feeder_Local_Board)
    {
        Feeder_Motor.Set_Out(0);
    }

    if (App_Fire_Has_Local_Motor())
    {
        Fire_Motor_Group.Push_Data();
    }
}

/**
 * @brief 更新本板发射电机在线状态
 */
static void App_Fire_Update_Online_States(void)
{
    if (Fire_Friction_Local_Board)
    {
        for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
        {
            Friction_Motor[i].Update_Online_State();
        }
    }

    if (Fire_Feeder_Local_Board)
    {
        Feeder_Motor.Update_Online_State();
    }
}

/**
 * @brief 判断本板全部发射电机反馈是否在线
 *
 * @return bool true表示本板已安装的发射电机均在线
 */
static bool App_Fire_All_Local_Motors_Online(void)
{
    if (Fire_Friction_Local_Board)
    {
        for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
        {
            if (!Friction_Motor[i].Get_Online_State())
            {
                return false;
            }
        }
    }

    if (Fire_Feeder_Local_Board && !Feeder_Motor.Get_Online_State())
    {
        return false;
    }

    return true;
}

/**
 * @brief 更新单个堵转确认计时器
 *
 * @param Stall_Condition true表示当前满足堵转条件
 * @param Stall_Start_Time 堵转计时起点
 * @param Now_ms 当前系统时间，单位ms
 * @return bool true表示堵转条件已经持续超过确认时间
 */
static bool App_Fire_Update_Stall_Timer(bool Stall_Condition,uint32_t *Stall_Start_Time,uint32_t Now_ms)
{
    if (Stall_Start_Time == nullptr)
    {
        return false;
    }

    if (!Stall_Condition)
    {
        *Stall_Start_Time = 0;
        return false;
    }

    if (*Stall_Start_Time == 0)
    {
        *Stall_Start_Time = Now_ms;
        return false;
    }

    return Now_ms - *Stall_Start_Time >= Fire_Stall_Confirm_Time_ms;
}

/**
 * @brief 锁止发射机构并持续发送零输出
 */
static void App_Fire_Latch_Stall(void)
{
    Fire_Stall_Latched = true;
    Fire_Feeder_Shot_In_Progress = false;
    Fire_Single_Shot_Request = false;
    App_Fire_States = App_Fire_States_e::Stalled;
    App_Fire_Reset_PID();
    App_Fire_Force_Zero_Output();
}

/**
 * @brief 进入左上右上的发射模式
 *
 * @param Now_ms 当前系统时间，单位ms
 */
static void App_Fire_Enter_Mode(uint32_t Now_ms)
{
    Fire_Mode_Active = true;
    Fire_Stall_Latched = false;
    Fire_Feeder_Target_Initialized = false;
    Fire_Feeder_Shot_In_Progress = false;
    Fire_Single_Shot_Request = false;
    Fire_Feeder_Target_Angle_Degree = 0.0f;
    Fire_Mode_Enter_Time = Now_ms;
    Fire_Feeder_Stall_Start_Time = 0;
    for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
    {
        Fire_Friction_Stall_Start_Time[i] = 0;
    }

    App_Fire_Reset_PID();
    App_Fire_States = App_Fire_States_e::Waiting_Feedback;
    App_Fire_Force_Zero_Output();
}

/**
 * @brief 退出发射模式并保持零输出
 */
static void App_Fire_Exit_Mode(void)
{
    Fire_Mode_Active = false;
    Fire_Feeder_Target_Initialized = false;
    Fire_Feeder_Shot_In_Progress = false;
    Fire_Single_Shot_Request = false;
    App_Fire_States = App_Fire_States_e::Disabled;
    App_Fire_Reset_PID();
    App_Fire_Force_Zero_Output();
}

/**
 * @brief 检查拨轮是否产生一次单发请求
 *
 * @return bool true表示本周期产生一次单发请求
 */
static bool App_Fire_Check_Dial_Wheel_Trigger(void)
{
    float Dial_Wheel = App_Remote_Get_Dial_Wheel();
    (void)Dial_Wheel;

    //暂不确定拨杆数据效果 此处判断留空
    return false;
}

/**
 * @brief 执行四摩擦轮速度闭环
 */
static void App_Fire_Control_Friction_Motors(void)
{
    if (!Fire_Friction_Local_Board)
    {
        return;
    }

    for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
    {
        float Direction = App_Fire_Get_Friction_Motor_Direction(i);
        float Current_Speed_m_s = Friction_Motor[i].Get_AngleSpeed() * Fire_Friction_Wheel_Radius_m;
        Friction_Motor_PID[i].Set_Speed_Target(Fire_Friction_Target_Speed_m_s * Direction);
        Friction_Motor_PID[i].Set_Current_Speed(Current_Speed_m_s);
        Friction_Motor_PID[i].Control_Speed_To_Out();
        Friction_Motor[i].Set_Out(static_cast<int16_t>(Friction_Motor_PID[i].Get_Out()));
    }
}

/**
 * @brief 执行拨弹盘角度和速度串级闭环
 */
static void App_Fire_Control_Feeder_Motor(void)
{
    if (!Fire_Feeder_Local_Board)
    {
        Fire_Single_Shot_Request = false;
        return;
    }

    if (!Fire_Feeder_Target_Initialized)
    {
        Fire_Feeder_Target_Angle_Degree = Feeder_Motor.Get_Continuous_Angle();
        Fire_Feeder_Target_Initialized = true;
    }

    if (Fire_Single_Shot_Request && !Fire_Feeder_Shot_In_Progress)
    {
        Fire_Feeder_Target_Angle_Degree += Fire_Feeder_Angle_Per_Bullet_Degree * Fire_Feeder_Motor_Direction;
        Fire_Feeder_Shot_In_Progress = true;
        Fire_Single_Shot_Request = false;
        Fire_Feeder_Stall_Start_Time = 0;
    }

    Feeder_Motor_PID.Set_Angle_Target(Fire_Feeder_Target_Angle_Degree);
    Feeder_Motor_PID.Set_Current_Angle(Feeder_Motor.Get_Continuous_Angle());
    Feeder_Motor_PID.Set_Current_Speed(Feeder_Motor.Get_AngleSpeed());
    Feeder_Motor_PID.Control_Cascade();
    Feeder_Motor.Set_Out(static_cast<int16_t>(Feeder_Motor_PID.Get_Out()));

    float Angle_Error_Degree = Fire_Feeder_Target_Angle_Degree - Feeder_Motor.Get_Continuous_Angle();
    if (Fire_Feeder_Shot_In_Progress && (fabsf(Angle_Error_Degree) <= Fire_Feeder_Finish_Angle_Error_Degree) && (fabsf(Feeder_Motor.Get_AngleSpeed()) <= Fire_Feeder_Finish_Speed_Rad_s))
    {
        Fire_Feeder_Shot_In_Progress = false;
        Fire_Feeder_Stall_Start_Time = 0;
    }
}

/**
 * @brief 检查摩擦轮和拨弹盘堵转状态
 *
 * @param Now_ms 当前系统时间，单位ms
 * @return bool true表示任意本地发射电机堵转
 */
static bool App_Fire_Check_Stall(uint32_t Now_ms)
{
    if (Fire_Friction_Local_Board && (Now_ms - Fire_Mode_Enter_Time >= Fire_Friction_Stall_Detection_Delay_ms))
    {
        for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
        {
            float Current_Speed_m_s = fabsf(Friction_Motor[i].Get_AngleSpeed() * Fire_Friction_Wheel_Radius_m);
            if (App_Fire_Update_Stall_Timer(Current_Speed_m_s < Fire_Friction_Stall_Speed_m_s,&Fire_Friction_Stall_Start_Time[i],Now_ms))
            {
                return true;
            }
        }
    }

    if (Fire_Feeder_Local_Board && Fire_Feeder_Shot_In_Progress)
    {
        if (App_Fire_Update_Stall_Timer(fabsf(Feeder_Motor.Get_AngleSpeed()) < Fire_Feeder_Stall_Speed_Rad_s,&Fire_Feeder_Stall_Start_Time,Now_ms))
        {
            return true;
        }
    }
    else
    {
        Fire_Feeder_Stall_Start_Time = 0;
    }

    return false;
}

/**
 * @brief 初始化本板发射电机、PID和安装位置
 *
 * @param Board_Role 当前主控角色
 */
void App_Fire_Init(SteeringWheel_Infantry_BoardCAN_Role_e Board_Role)
{
    App_Fire_Init_Finished = false;
    Fire_Current_Board_Role = Board_Role;
    Fire_Friction_Local_Board = Fire_Current_Board_Role == SteeringWheel_Infantry_BoardCAN_Role_e::Gimbal;
    Fire_Feeder_Local_Board = static_cast<uint8_t>(Fire_Current_Board_Role) == SteeringWheel_Infantry_Feeder_Location;

    if (App_Fire_Has_Local_Motor())
    {
        Fire_Motor_Group.Init(AppCAN_Get_Interface(Fire_Motor_CAN_Number),DJI_Motor_3508);
    }

    if (Fire_Friction_Local_Board)
    {
        for (uint8_t i = 0; i < Fire_Friction_Motor_Count; i++)
        {
            Friction_Motor[i].Init(DJI_Motor_3508,App_Fire_Get_Friction_Motor_ID(i),&Fire_Motor_Group);
        }
    }

    if (Fire_Feeder_Local_Board)
    {
        Feeder_Motor.Init(DJI_Motor_3508,Fire_Feeder_Motor_ID,&Fire_Motor_Group);
    }

    App_Fire_Init_PID();
    Fire_Mode_Active = false;
    Fire_Stall_Latched = false;
    Fire_Feeder_Target_Initialized = false;
    Fire_Feeder_Shot_In_Progress = false;
    Fire_Single_Shot_Request = false;
    Fire_Feeder_Target_Angle_Degree = 0.0f;
    Fire_Mode_Enter_Time = 0;
    Fire_Feeder_Stall_Start_Time = 0;
    App_Fire_States = App_Fire_States_e::Disabled;
    App_Fire_Init_Finished = true;
}

/**
 * @brief 更新发射模式、电机闭环和堵转保护
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_Fire_Update(uint32_t Now_ms)
{
    if (!App_Fire_Init_Finished)
    {
        return;
    }

    App_Fire_Update_Online_States();

    bool Fire_Mode_Selected = App_Remote_Get_Online_State() &&
                              (App_Remote_Get_Left_ThreeKey() == BoardCAN_Remote_ThreeKey_e::Up) &&
                              (App_Remote_Get_Right_ThreeKey() == BoardCAN_Remote_ThreeKey_e::Up);

    if (!Fire_Mode_Selected)
    {
        App_Fire_Exit_Mode();
        return;
    }

    if (!Fire_Mode_Active)
    {
        App_Fire_Enter_Mode(Now_ms);
    }

    if (Fire_Stall_Latched)
    {
        App_Fire_States = App_Fire_States_e::Stalled;
        App_Fire_Force_Zero_Output();
        return;
    }

    if (!App_Fire_All_Local_Motors_Online())
    {
        App_Fire_States = App_Fire_States_e::Waiting_Feedback;
        App_Fire_Force_Zero_Output();
        if (Now_ms - Fire_Mode_Enter_Time >= Fire_Feedback_Wait_Timeout_ms)
        {
            App_Fire_Latch_Stall();
        }
        return;
    }

    App_Fire_States = App_Fire_States_e::Running;

    if (App_Fire_Check_Dial_Wheel_Trigger())
    {
        App_Fire_Request_Single_Shot();
    }

    App_Fire_Control_Friction_Motors();
    App_Fire_Control_Feeder_Motor();

    if (App_Fire_Has_Local_Motor())
    {
        Fire_Motor_Group.Push_Data();
    }

    if (App_Fire_Check_Stall(Now_ms))
    {
        App_Fire_Latch_Stall();
    }
}

/**
 * @brief 处理CAN1上的发射电机反馈
 *
 * @param CAN_ID CAN标准帧ID
 * @param Data CAN数据区
 * @param Length CAN数据长度
 * @return bool true表示该帧属于本板发射电机
 */
bool App_Fire_Process_CAN_Message(uint16_t CAN_ID,const uint8_t *Data,uint8_t Length)
{
    if (!App_Fire_Init_Finished || (Data == nullptr) || (Length != 8))
    {
        return false;
    }

    bool Frame_Handled = false;
    if (Fire_Friction_Local_Board && (CAN_ID >= 0x201) && (CAN_ID < 0x201 + Fire_Friction_Motor_Count))
    {
        Frame_Handled = true;
    }
    if (Fire_Feeder_Local_Board && (CAN_ID == 0x200 + Fire_Feeder_Motor_ID))
    {
        Frame_Handled = true;
    }

    if (Frame_Handled)
    {
        Fire_Motor_Group.Process_CAN_Feedback(CAN_ID,Data,Length);
    }
    return Frame_Handled;
}

/**
 * @brief 请求拨弹盘发射一发弹丸
 *
 * @details 请求只在发射模式有效且拨弹盘安装在本板时执行。
 */
void App_Fire_Request_Single_Shot(void)
{
    if (Fire_Mode_Active && !Fire_Stall_Latched && Fire_Feeder_Local_Board)
    {
        Fire_Single_Shot_Request = true;
    }
}

/**
 * @brief 判断拨弹盘是否安装在当前主控
 *
 * @return bool true表示本板负责拨弹盘控制
 */
bool App_Fire_Is_Feeder_Local_Board(void)
{
    return Fire_Feeder_Local_Board;
}

/**
 * @brief 获取发射机构运行状态
 *
 * @return App_Fire_States_e 当前运行状态
 */
App_Fire_States_e App_Fire_Get_States(void)
{
    return App_Fire_States;
}

/**
 * @brief 获取堵转锁止状态
 *
 * @return bool true表示发射电机已经锁止为零输出
 */
bool App_Fire_Get_Stall_Latched_State(void)
{
    return Fire_Stall_Latched;
}

/**
 * @brief 获取摩擦轮运行状态
 *
 * @return bool true表示四摩擦轮正在执行速度闭环
 */
bool App_Fire_Get_Friction_Enabled_State(void)
{
    return Fire_Friction_Local_Board && (App_Fire_States == App_Fire_States_e::Running) && !Fire_Stall_Latched;
}

/**
 * @brief 获取拨弹盘单发动作状态
 *
 * @return bool true表示拨弹盘正在转向下一发位置
 */
bool App_Fire_Get_Feeder_Shot_In_Progress_State(void)
{
    return Fire_Feeder_Shot_In_Progress;
}

/**
 * @brief 获取拨弹盘连续角度目标
 *
 * @return float 拨弹盘目标角度，单位degree
 */
float App_Fire_Get_Feeder_Target_Angle_Degree(void)
{
    return Fire_Feeder_Target_Angle_Degree;
}
