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

bool Global_Init_Finished = false;

extern "C" void InitTask_Function(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();
    
    /* USER CODE BEGIN InitTask_Function */
    /* Infinite loop */
    for(;;)
    {

        Global_Init_Finished = true;
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

        osDelay(1);
    }
    /* USER CODE END Main_Task_Function */
}