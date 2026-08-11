/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Chassis_Task.cpp
  * @brief   底盘任务库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Chassis_Task.h"
#include "bsp_fdcan.h"
#include "cmsis_os2.h"
#include "usb_device.h"
#include "Application/App_CAN/App_CAN.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "Application/App_Chassis_Control/App_Chassis_Control.h"
#include "Application/App_IMU/App_IMU.h"
#include "Application/App_Chassis_Attitude/App_Chassis_Attitude.h"
#include "Application/App_Fire/App_Fire.h"

bool Global_Init_Finished = false;

/**
 * @brief 初始化USB、底盘电机、姿态关系、发射机构、CAN、遥控和BMI088应用
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
extern "C" void InitTask_Function(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();

    //底盘初始化
    App_Chassis_Init();

    //先注册CAN2云台Yaw只读对象，再启动FDCAN接收
    App_Chassis_Attitude_Init();

    //初始化本板可选的拨弹盘对象
    App_Fire_Init(SteeringWheel_Infantry_BoardCAN_Role_e::Chassis);

    //CAN初始化
    AppCAN_Init();

    //底盘遥控控制初始化
    App_Chassis_Control_Init();

    //BMI088初始化和校准过程由Main Task继续推进
    App_IMU_Init();

    Global_Init_Finished = true;
    
    /* USER CODE BEGIN InitTask_Function */
    /* Infinite loop */
    for(;;)
    {
        osThreadTerminate(osThreadGetId());
    }
    /* USER CODE END InitTask_Function */
}


/* USER CODE BEGIN Header_Main_Task_Function */
/* USER CODE END Header_Main_Task_Function */

/**
 * @brief 每1ms更新底盘BMI088、姿态关系、遥控控制和发射机构
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
extern "C" void Main_Task_Function(void *argument)
{
    /* USER CODE BEGIN Main_Task_Function */

    /* Infinite loop */
    for(;;)
    {
        uint32_t Now_ms = HAL_GetTick();

        //更新底盘BMI088采样、校准和姿态解算
        App_IMU_Update(Now_ms);

        //更新云台相对底盘角度和两板世界系姿态
        App_Chassis_Attitude_Update();

        //底盘遥控控制更新
        App_Chassis_Control_Update(Now_ms);

        //更新本板拨弹盘和堵转保护
        App_Fire_Update(Now_ms);

        osDelay(1);
    }
    /* USER CODE END Main_Task_Function */
}
