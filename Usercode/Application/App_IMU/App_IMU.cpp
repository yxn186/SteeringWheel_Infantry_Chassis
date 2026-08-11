/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_IMU.cpp
  * @brief   底盘IMU应用层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "App_IMU.h"
#include "App_IMU_Config.h"
#include "IMU_Bias_Calibration.h"
#include "Mahony.h"
#include "Rotation3D.h"
#include "bmi088.h"
#include <math.h>
/*YOUR CODE*/

#define App_IMU_Degree_To_Radian                 0.017453292519943295f      //角度转弧度系数
#define App_IMU_Gravity_m_s2                     9.80665f                   //标准重力加速度

//BMI088驱动、启动校准和姿态解算对象
static Class_BMI088 App_IMU_BMI088;
static Class_IMU_Bias_Calibration App_IMU_Bias_Calibration;
static Class_Mahony App_IMU_Mahony;

//应用状态和坐标变换结果
static App_IMU_States_e App_IMU_States = App_IMU_States_e::Uninitialized;
static Matrix3f_t App_IMU_Sensor_To_Body_Matrix;
static Quaternionf_t App_IMU_Quaternion;
static Quaternionf_t App_IMU_Yaw_Zero_Quaternion;

//初始化过程只执行一次的步骤标志
static bool App_IMU_Calibration_Started = false;
static bool App_IMU_Calibration_Finished = false;
static bool App_IMU_Yaw_Zero_Initialized = false;
static bool App_IMU_Continuous_Yaw_Initialized = false;

static uint32_t App_IMU_Last_Attitude_Update_Time = 0;

//提供给底盘控制层使用的姿态角，单位degree
static float App_IMU_Yaw_Degree = 0.0f;
static float App_IMU_Pitch_Degree = 0.0f;
static float App_IMU_Roll_Degree = 0.0f;
static float App_IMU_Continuous_Yaw_Degree = 0.0f;
static float App_IMU_Last_Yaw_Degree = 0.0f;

//校准后的机体系角速度和加速度
static float App_IMU_GYRO_X_Rad_s = 0.0f;
static float App_IMU_GYRO_Y_Rad_s = 0.0f;
static float App_IMU_GYRO_Z_Rad_s = 0.0f;
static float App_IMU_ACC_X_m_s2 = 0.0f;
static float App_IMU_ACC_Y_m_s2 = 0.0f;
static float App_IMU_ACC_Z_m_s2 = 0.0f;

/**
 * @brief 读取BMI088数据并转换到底盘机体系
 *
 * @param ACC_Body_G 输出机体系三轴加速度，单位g
 * @param GYRO_Body_DPS 输出机体系三轴角速度，单位degree/s
 */
static void App_IMU_Get_Body_Data(Vector3f_t *ACC_Body_G,Vector3f_t *GYRO_Body_DPS)
{
    Vector3f_t ACC_Sensor_G =
    {
        App_IMU_BMI088.Get_ACC_X_G(),
        App_IMU_BMI088.Get_ACC_Y_G(),
        App_IMU_BMI088.Get_ACC_Z_G()
    };
    Vector3f_t GYRO_Sensor_DPS =
    {
        App_IMU_BMI088.Get_GYRO_X_DPS(),
        App_IMU_BMI088.Get_GYRO_Y_DPS(),
        App_IMU_BMI088.Get_GYRO_Z_DPS()
    };

    //安装矩阵只负责方向变换，不在这里混入零偏和单位换算
    *ACC_Body_G = Rotation3D_Matrix_Multiply_Vector(&App_IMU_Sensor_To_Body_Matrix,&ACC_Sensor_G);
    *GYRO_Body_DPS = Rotation3D_Matrix_Multiply_Vector(&App_IMU_Sensor_To_Body_Matrix,&GYRO_Sensor_DPS);
}

/**
 * @brief 使用校准后的数据更新姿态
 *
 * @param Now_ms 当前系统时间，单位ms
 * @param ACC_Body_G 机体系三轴加速度，单位g
 * @param GYRO_Body_DPS 机体系三轴角速度，单位degree/s
 */
static void App_IMU_Update_Attitude(uint32_t Now_ms,const Vector3f_t &ACC_Body_G,const Vector3f_t &GYRO_Body_DPS)
{
    float ACC_X_G = ACC_Body_G.X - App_IMU_Bias_Calibration.Get_ACC_Bias_X_G();
    float ACC_Y_G = ACC_Body_G.Y - App_IMU_Bias_Calibration.Get_ACC_Bias_Y_G();
    float ACC_Z_G = ACC_Body_G.Z - App_IMU_Bias_Calibration.Get_ACC_Bias_Z_G();

    App_IMU_GYRO_X_Rad_s = (GYRO_Body_DPS.X - App_IMU_Bias_Calibration.Get_GYRO_Bias_X_DPS()) * App_IMU_Degree_To_Radian;
    App_IMU_GYRO_Y_Rad_s = (GYRO_Body_DPS.Y - App_IMU_Bias_Calibration.Get_GYRO_Bias_Y_DPS()) * App_IMU_Degree_To_Radian;
    App_IMU_GYRO_Z_Rad_s = (GYRO_Body_DPS.Z - App_IMU_Bias_Calibration.Get_GYRO_Bias_Z_DPS()) * App_IMU_Degree_To_Radian;

    App_IMU_ACC_X_m_s2 = ACC_X_G * App_IMU_Gravity_m_s2;
    App_IMU_ACC_Y_m_s2 = ACC_Y_G * App_IMU_Gravity_m_s2;
    App_IMU_ACC_Z_m_s2 = ACC_Z_G * App_IMU_Gravity_m_s2;

    //第一次更新先使用1ms，之后使用两组完整数据之间的真实时间
    float Dt_s = 0.001f;
    if (App_IMU_Last_Attitude_Update_Time != 0)
    {
        uint32_t Delta_Time_ms = Now_ms - App_IMU_Last_Attitude_Update_Time;
        if ((Delta_Time_ms > 0) && (Delta_Time_ms <= 100))
        {
            Dt_s = Delta_Time_ms * 0.001f;
        }
    }
    App_IMU_Last_Attitude_Update_Time = Now_ms;

    App_IMU_Mahony.Update(App_IMU_GYRO_X_Rad_s,
                          App_IMU_GYRO_Y_Rad_s,
                          App_IMU_GYRO_Z_Rad_s,
                          ACC_X_G,
                          ACC_Y_G,
                          ACC_Z_G,
                          Dt_s);

    Quaternionf_t Mahony_Quaternion =
    {
        App_IMU_Mahony.Get_Quaternion_W(),
        App_IMU_Mahony.Get_Quaternion_X(),
        App_IMU_Mahony.Get_Quaternion_Y(),
        App_IMU_Mahony.Get_Quaternion_Z()
    };

    //校准后的第一帧姿态作为启动Yaw零点，Pitch和Roll仍保留重力方向
    if (!App_IMU_Yaw_Zero_Initialized)
    {
        float Start_Yaw_Degree = 0.0f;
        Rotation3D_Quaternion_To_Yaw_Pitch_Roll_Degree(&Mahony_Quaternion,&Start_Yaw_Degree,nullptr,nullptr);

        float Half_Yaw_Offset_Rad = -0.5f * Start_Yaw_Degree * App_IMU_Degree_To_Radian;
        App_IMU_Yaw_Zero_Quaternion.W = cosf(Half_Yaw_Offset_Rad);
        App_IMU_Yaw_Zero_Quaternion.X = 0.0f;
        App_IMU_Yaw_Zero_Quaternion.Y = 0.0f;
        App_IMU_Yaw_Zero_Quaternion.Z = sinf(Half_Yaw_Offset_Rad);
        App_IMU_Yaw_Zero_Initialized = true;
    }

    Rotation3D_Quaternion_Multiply(&App_IMU_Yaw_Zero_Quaternion,&Mahony_Quaternion,&App_IMU_Quaternion);
    Rotation3D_Quaternion_To_Yaw_Pitch_Roll_Degree(&App_IMU_Quaternion,
                                                    &App_IMU_Yaw_Degree,
                                                    &App_IMU_Pitch_Degree,
                                                    &App_IMU_Roll_Degree);
    App_IMU_Yaw_Degree = Rotation3D_Wrap_Angle_Degree(App_IMU_Yaw_Degree);

    //对相邻两帧Yaw差值做包角，跨越正负180度时连续Yaw不会跳变
    if (!App_IMU_Continuous_Yaw_Initialized)
    {
        App_IMU_Continuous_Yaw_Degree = App_IMU_Yaw_Degree;
        App_IMU_Last_Yaw_Degree = App_IMU_Yaw_Degree;
        App_IMU_Continuous_Yaw_Initialized = true;
    }
    else
    {
        float Delta_Yaw_Degree = Rotation3D_Wrap_Angle_Degree(App_IMU_Yaw_Degree - App_IMU_Last_Yaw_Degree);
        App_IMU_Continuous_Yaw_Degree += Delta_Yaw_Degree;
        App_IMU_Last_Yaw_Degree = App_IMU_Yaw_Degree;
    }

    App_IMU_States = App_IMU_States_e::Ready;
}

/**
 * @brief 初始化底盘IMU应用、BMI088驱动、校准器和姿态解算器
 */
void App_IMU_Init(void)
{
    App_IMU_States = App_IMU_States_e::Initializing;

    //安装矩阵决定传感器XYZ怎样映射到底盘的前、左、上方向
    App_IMU_Sensor_To_Body_Matrix.Data[0][0] = App_IMU_Mount_R00;
    App_IMU_Sensor_To_Body_Matrix.Data[0][1] = App_IMU_Mount_R01;
    App_IMU_Sensor_To_Body_Matrix.Data[0][2] = App_IMU_Mount_R02;
    App_IMU_Sensor_To_Body_Matrix.Data[1][0] = App_IMU_Mount_R10;
    App_IMU_Sensor_To_Body_Matrix.Data[1][1] = App_IMU_Mount_R11;
    App_IMU_Sensor_To_Body_Matrix.Data[1][2] = App_IMU_Mount_R12;
    App_IMU_Sensor_To_Body_Matrix.Data[2][0] = App_IMU_Mount_R20;
    App_IMU_Sensor_To_Body_Matrix.Data[2][1] = App_IMU_Mount_R21;
    App_IMU_Sensor_To_Body_Matrix.Data[2][2] = App_IMU_Mount_R22;

    //Mahony对象独立保存四元数和积分项
    App_IMU_Mahony.Init(App_IMU_Mahony_Kp,
                        App_IMU_Mahony_Ki,
                        App_IMU_Mahony_ACC_Norm_Min_G,
                        App_IMU_Mahony_ACC_Norm_Max_G);

    //将SPI、片选和量程配置交给BMI088驱动
    BMI088_Config_t BMI088_Config;
    BMI088_Config.SPI_Handler = App_IMU_SPI_Handler;
    BMI088_Config.ACC_CS_GPIOx = App_IMU_ACC_CS_GPIO_Port;
    BMI088_Config.ACC_CS_Pin = App_IMU_ACC_CS_Pin;
    BMI088_Config.ACC_CS_Active_Level = App_IMU_ACC_CS_Active_Level;
    BMI088_Config.GYRO_CS_GPIOx = App_IMU_GYRO_CS_GPIO_Port;
    BMI088_Config.GYRO_CS_Pin = App_IMU_GYRO_CS_Pin;
    BMI088_Config.GYRO_CS_Active_Level = App_IMU_GYRO_CS_Active_Level;
    BMI088_Config.ACC_INT_Pin = App_IMU_ACC_INT_Pin;
    BMI088_Config.GYRO_INT_Pin = App_IMU_GYRO_INT_Pin;
    BMI088_Config.ACC_Config_Value = App_IMU_ACC_Config_Value;
    BMI088_Config.ACC_Range_Value = App_IMU_ACC_Range_Value;
    BMI088_Config.ACC_Range_G = App_IMU_ACC_Range_G;
    BMI088_Config.GYRO_Bandwidth_Value = App_IMU_GYRO_Bandwidth_Value;
    BMI088_Config.GYRO_Range_Value = App_IMU_GYRO_Range_Value;
    BMI088_Config.GYRO_Range_DPS = App_IMU_GYRO_Range_DPS;
    BMI088_Config.Feedback_Timeout_ms = App_IMU_Feedback_Timeout_ms;

    if (!App_IMU_BMI088.Init(BMI088_Config))
    {
        App_IMU_States = App_IMU_States_e::Error;
    }
}

/**
 * @brief 更新BMI088初始化、零偏校准和姿态解算
 *
 * @param Now_ms 当前系统时间，单位ms
 */
void App_IMU_Update(uint32_t Now_ms)
{
    //底层Update先推进非阻塞初始化和ACC到GYRO的DMA采样链
    App_IMU_BMI088.Update(Now_ms);

    if (App_IMU_BMI088.Get_States() == BMI088_States_e::Error)
    {
        App_IMU_States = App_IMU_States_e::Error;
        return;
    }
    if (!App_IMU_BMI088.Get_Init_Finished())
    {
        App_IMU_States = App_IMU_States_e::Initializing;
        return;
    }

    //驱动初始化完成后只启动一次固定总窗口的静止校准
    if (!App_IMU_Calibration_Started)
    {
        App_IMU_Bias_Calibration.Start(App_IMU_Bias_Target_Samples,
                                       App_IMU_Bias_GYRO_Norm_Max_DPS,
                                       App_IMU_Bias_ACC_Norm_Min_G,
                                       App_IMU_Bias_ACC_Norm_Max_G);
        App_IMU_Calibration_Started = true;
        App_IMU_States = App_IMU_States_e::Calibrating;
    }

    //没有新的ACC与GYRO完整组合时不重复计算姿态
    if (!App_IMU_BMI088.Get_New_Sample_Flag())
    {
        if (App_IMU_BMI088.Get_States() == BMI088_States_e::Offline)
        {
            App_IMU_States = App_IMU_States_e::Offline;
        }
        return;
    }

    App_IMU_BMI088.Clear_New_Sample_Flag();

    Vector3f_t ACC_Body_G;
    Vector3f_t GYRO_Body_DPS;
    App_IMU_Get_Body_Data(&ACC_Body_G,&GYRO_Body_DPS);

    if (!App_IMU_Calibration_Finished)
    {
        IMU_Bias_Calibration_States_e Calibration_States =
            App_IMU_Bias_Calibration.Push_Sample(GYRO_Body_DPS.X,
                                                 GYRO_Body_DPS.Y,
                                                 GYRO_Body_DPS.Z,
                                                 ACC_Body_G.X,
                                                 ACC_Body_G.Y,
                                                 ACC_Body_G.Z);

        if (Calibration_States == IMU_Bias_Calibration_States_e::Finished)
        {
            //校准完成后从单位四元数重新开始，下一帧建立启动Yaw零点
            App_IMU_Calibration_Finished = true;
            App_IMU_Mahony.Reset();
            App_IMU_Last_Attitude_Update_Time = 0;
            App_IMU_States = App_IMU_States_e::Calibrating;
        }
        else if (Calibration_States == IMU_Bias_Calibration_States_e::No_Valid_Sample)
        {
            App_IMU_States = App_IMU_States_e::Error;
        }
        else
        {
            App_IMU_States = App_IMU_States_e::Calibrating;
        }
        return;
    }

    App_IMU_Update_Attitude(Now_ms,ACC_Body_G,GYRO_Body_DPS);
}

/**
 * @brief 将GPIO外部中断转交给BMI088驱动
 *
 * @param GPIO_Pin 产生中断的GPIO引脚
 */
void App_IMU_Process_GPIO_EXTI(uint16_t GPIO_Pin)
{
    App_IMU_BMI088.Process_GPIO_EXTI(GPIO_Pin);
}

/**
 * @brief 获取底盘IMU应用当前运行状态
 *
 * @return App_IMU_States_e 当前运行状态
 */
App_IMU_States_e App_IMU_Get_States(void)
{
    return App_IMU_States;
}

/**
 * @brief 判断底盘IMU是否已经完成校准并输出有效姿态
 *
 * @return bool true表示姿态数据已经可以使用
 */
bool App_IMU_Get_Ready_State(void)
{
    return App_IMU_States == App_IMU_States_e::Ready;
}

/**
 * @brief 判断BMI088是否在反馈超时时间内收到过完整数据
 *
 * @return bool true表示BMI088当前在线
 */
bool App_IMU_Get_Online_State(void)
{
    return App_IMU_BMI088.Get_Online_State();
}

/**
 * @brief 获取启动零偏校准进度
 *
 * @return float 校准进度，范围为0.0到1.0
 */
float App_IMU_Get_Calibration_Progress(void)
{
    return App_IMU_Bias_Calibration.Get_Progress();
}

/**
 * @brief 获取姿态四元数的W分量
 *
 * @return float 四元数W分量
 */
float App_IMU_Get_Quaternion_W(void)
{
    return App_IMU_Quaternion.W;
}

/**
 * @brief 获取姿态四元数的X分量
 *
 * @return float 四元数X分量
 */
float App_IMU_Get_Quaternion_X(void)
{
    return App_IMU_Quaternion.X;
}

/**
 * @brief 获取姿态四元数的Y分量
 *
 * @return float 四元数Y分量
 */
float App_IMU_Get_Quaternion_Y(void)
{
    return App_IMU_Quaternion.Y;
}

/**
 * @brief 获取姿态四元数的Z分量
 *
 * @return float 四元数Z分量
 */
float App_IMU_Get_Quaternion_Z(void)
{
    return App_IMU_Quaternion.Z;
}

/**
 * @brief 获取启动零点下的包角Yaw
 *
 * @return float Yaw角，单位degree，范围为(-180,180]
 */
float App_IMU_Get_Yaw_Degree(void)
{
    return App_IMU_Yaw_Degree;
}

/**
 * @brief 获取机体系Pitch角
 *
 * @return float Pitch角，单位degree
 */
float App_IMU_Get_Pitch_Degree(void)
{
    return App_IMU_Pitch_Degree;
}

/**
 * @brief 获取机体系Roll角
 *
 * @return float Roll角，单位degree
 */
float App_IMU_Get_Roll_Degree(void)
{
    return App_IMU_Roll_Degree;
}

/**
 * @brief 获取跨越正负180度后仍连续的Yaw角
 *
 * @return float 连续Yaw角，单位degree
 */
float App_IMU_Get_Continuous_Yaw_Degree(void)
{
    return App_IMU_Continuous_Yaw_Degree;
}

/**
 * @brief 获取机体系X轴角速度
 *
 * @return float X轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_X_Rad_s(void)
{
    return App_IMU_GYRO_X_Rad_s;
}

/**
 * @brief 获取机体系Y轴角速度
 *
 * @return float Y轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_Y_Rad_s(void)
{
    return App_IMU_GYRO_Y_Rad_s;
}

/**
 * @brief 获取机体系Z轴角速度
 *
 * @return float Z轴角速度，单位rad/s
 */
float App_IMU_Get_GYRO_Z_Rad_s(void)
{
    return App_IMU_GYRO_Z_Rad_s;
}

/**
 * @brief 获取机体系X轴加速度
 *
 * @return float X轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_X_m_s2(void)
{
    return App_IMU_ACC_X_m_s2;
}

/**
 * @brief 获取机体系Y轴加速度
 *
 * @return float Y轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_Y_m_s2(void)
{
    return App_IMU_ACC_Y_m_s2;
}

/**
 * @brief 获取机体系Z轴加速度
 *
 * @return float Z轴加速度，单位m/s^2
 */
float App_IMU_Get_ACC_Z_m_s2(void)
{
    return App_IMU_ACC_Z_m_s2;
}

/**
 * @brief 接收HAL GPIO外部中断并交给底盘IMU应用处理
 *
 * @param GPIO_Pin 产生中断的GPIO引脚
 */
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    App_IMU_Process_GPIO_EXTI(GPIO_Pin);
}

