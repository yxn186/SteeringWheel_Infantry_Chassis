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

bool Global_Init_Finished = false;

extern "C" void InitTask_Function(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();

    //底盘初始化
    App_Chassis_Init();

    //CAN初始化
    AppCAN_Init();

    //底盘遥控控制初始化
    App_Chassis_Control_Init();

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
/**
* @brief Function implementing the Main_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Main_Task_Function */
extern "C" void Main_Task_Function(void *argument)
{
    /* USER CODE BEGIN Main_Task_Function */

    /* Infinite loop */
    for(;;)
    {
        //底盘遥控控制更新
        App_Chassis_Control_Update(HAL_GetTick());

        osDelay(1);
    }
    /* USER CODE END Main_Task_Function */
}
