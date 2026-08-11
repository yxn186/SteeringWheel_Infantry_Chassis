/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Attitude.cpp
  * @brief   底盘与云台姿态关系应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#include "App_Chassis_Attitude.h"
#include "App_Chassis_Attitude_Config.h"
#include "Application/App_CAN/App_CAN.h"
#include "Application/App_IMU/App_IMU.h"
#include "DJI_Motor.h"
#include <math.h>

#define App_Chassis_Attitude_Degree_To_Radian     0.017453292519943295f

//底盘只读取Yaw电机反馈，不向CAN2发送GM6020控制帧
static Class_DJI_Motor_Group Yaw_Motor_Read_Only_Group;
static Class_DJI_Motor Yaw_Motor_Read_Only;

static App_Chassis_Attitude_States_e App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Uninitialized;
static bool App_Chassis_Attitude_Init_Finished = false;
static float App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree = 0.0f;
static float App_Chassis_Attitude_Chassis_Continuous_Yaw_Degree = 0.0f;
static float App_Chassis_Attitude_Gimbal_World_Yaw_Degree = 0.0f;

/**
 * @brief 将角度限制到(-180,180]范围
 *
 * @param Angle_Degree 需要处理的角度，单位degree
 * @return float 包角后的角度，单位degree
 */
static float App_Chassis_Attitude_Wrap_Angle_Degree(float Angle_Degree)
{
    while (Angle_Degree > 180.0f)
    {
        Angle_Degree -= 360.0f;
    }

    while (Angle_Degree <= -180.0f)
    {
        Angle_Degree += 360.0f;
    }

    return Angle_Degree;
}

/**
 * @brief 初始化CAN2云台Yaw电机只读对象和姿态状态
 */
void App_Chassis_Attitude_Init(void)
{
    App_Chassis_Attitude_Init_Finished = false;
    App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Uninitialized;

    Yaw_Motor_Read_Only_Group.Init(AppCAN_Get_Interface(Chassis_Attitude_Yaw_Motor_CAN_Number),DJI_Motor_6020);
    Yaw_Motor_Read_Only.Init(DJI_Motor_6020,Chassis_Attitude_Yaw_Motor_ID,&Yaw_Motor_Read_Only_Group);

    App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree = 0.0f;
    App_Chassis_Attitude_Chassis_Continuous_Yaw_Degree = 0.0f;
    App_Chassis_Attitude_Gimbal_World_Yaw_Degree = 0.0f;

    App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Waiting_Data;
    App_Chassis_Attitude_Init_Finished = true;
}

/**
 * @brief 更新Yaw电机在线状态和两板姿态关系
 */
void App_Chassis_Attitude_Update(void)
{
    if (!App_Chassis_Attitude_Init_Finished)
    {
        return;
    }

    Yaw_Motor_Read_Only.Update_Online_State();

    if (!Yaw_Motor_Read_Only.Get_Online_State())
    {
        App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Waiting_Data;
        return;
    }

    //编码器给出云台相对底盘角，可直接用于云台坐标系平移解算
    App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree = App_Chassis_Attitude_Wrap_Angle_Degree(
        (Yaw_Motor_Read_Only.Get_Angle() - Chassis_Attitude_Yaw_Motor_Mechanical_Zero_Degree) *
        Chassis_Attitude_Yaw_Motor_Direction);

    if (!App_IMU_Get_Ready_State())
    {
        App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Waiting_Data;
        return;
    }

    //底盘IMU给出底盘世界系角度，与相对角相加得到云台世界系角度
    App_Chassis_Attitude_Chassis_Continuous_Yaw_Degree = App_IMU_Get_Continuous_Yaw_Degree();
    App_Chassis_Attitude_Gimbal_World_Yaw_Degree = App_Chassis_Attitude_Chassis_Continuous_Yaw_Degree + App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree;

    App_Chassis_Attitude_States = App_Chassis_Attitude_States_e::Ready;
}

/**
 * @brief 处理CAN2上的云台Yaw电机反馈
 *
 * @param CAN_ID CAN标准帧ID
 * @param Data CAN数据区
 * @param Length CAN数据长度
 * @return bool true表示该帧属于云台Yaw电机
 */
bool App_Chassis_Attitude_Process_CAN_Message(uint16_t CAN_ID,const uint8_t *Data,uint8_t Length)
{
    if (!App_Chassis_Attitude_Init_Finished || (Data == nullptr) || (Length != 8))
    {
        return false;
    }

    const uint16_t Yaw_Feedback_CAN_ID = 0x204 + Chassis_Attitude_Yaw_Motor_ID;
    if (CAN_ID != Yaw_Feedback_CAN_ID)
    {
        return false;
    }

    Yaw_Motor_Read_Only_Group.Process_CAN_Feedback(CAN_ID,Data,Length);
    return true;
}

/**
 * @brief 将云台坐标系速度转换到底盘坐标系
 *
 * @param Gimbal_Speed_X 云台前后方向速度，单位m/s
 * @param Gimbal_Speed_Y 云台左右方向速度，单位m/s
 * @param Chassis_Speed_X 输出的底盘前后方向速度，单位m/s
 * @param Chassis_Speed_Y 输出的底盘左右方向速度，单位m/s
 */
void App_Chassis_Attitude_Transform_Gimbal_To_Chassis(float Gimbal_Speed_X,float Gimbal_Speed_Y,float *Chassis_Speed_X,float *Chassis_Speed_Y)
{
    if ((Chassis_Speed_X == nullptr) || (Chassis_Speed_Y == nullptr))
    {
        return;
    }

    float Relative_Yaw_Radian = App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree * App_Chassis_Attitude_Degree_To_Radian;
    float Cos_Yaw = cosf(Relative_Yaw_Radian);
    float Sin_Yaw = sinf(Relative_Yaw_Radian);

    *Chassis_Speed_X = Cos_Yaw * Gimbal_Speed_X - Sin_Yaw * Gimbal_Speed_Y;
    *Chassis_Speed_Y = Sin_Yaw * Gimbal_Speed_X + Cos_Yaw * Gimbal_Speed_Y;
}

/**
 * @brief 获取姿态关系模块运行状态
 *
 * @return App_Chassis_Attitude_States_e 当前运行状态
 */
App_Chassis_Attitude_States_e App_Chassis_Attitude_Get_States(void)
{
    return App_Chassis_Attitude_States;
}

/**
 * @brief 判断底盘IMU和云台Yaw反馈是否均可使用
 *
 * @return bool true表示姿态关系数据有效
 */
bool App_Chassis_Attitude_Get_Ready_State(void)
{
    return App_Chassis_Attitude_States == App_Chassis_Attitude_States_e::Ready;
}

/**
 * @brief 获取CAN2云台Yaw电机在线状态
 *
 * @return bool true表示Yaw电机反馈未超时
 */
bool App_Chassis_Attitude_Get_Yaw_Motor_Online_State(void)
{
    return Yaw_Motor_Read_Only.Get_Online_State();
}

/**
 * @brief 获取云台相对底盘的Yaw角
 *
 * @return float 云台相对Yaw角，单位degree，范围为(-180,180]
 */
float App_Chassis_Attitude_Get_Gimbal_Relative_Yaw_Degree(void)
{
    return App_Chassis_Attitude_Gimbal_Relative_Yaw_Degree;
}

/**
 * @brief 获取底盘启动零点下的连续Yaw角
 *
 * @return float 底盘连续Yaw角，单位degree
 */
float App_Chassis_Attitude_Get_Chassis_Continuous_Yaw_Degree(void)
{
    return App_Chassis_Attitude_Chassis_Continuous_Yaw_Degree;
}

/**
 * @brief 获取启动零点下的云台世界系Yaw角
 *
 * @return float 云台世界系Yaw角，单位degree
 */
float App_Chassis_Attitude_Get_Gimbal_World_Yaw_Degree(void)
{
    return App_Chassis_Attitude_Gimbal_World_Yaw_Degree;
}
