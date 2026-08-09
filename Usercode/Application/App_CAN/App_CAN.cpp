/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_CAN.cpp
  * @brief   App层CAN
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_CAN.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "bsp_fdcan.h"
#include "CAN_Interface.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "fdcan_adapter.h"

//FDCAN适应层类对象初始化
Class_FDCAN_Adapter FDCAN1_Adapter;
Class_FDCAN_Adapter FDCAN2_Adapter;
Class_FDCAN_Adapter FDCAN3_Adapter;



static void FDCAN1_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr) return;

    if (header.IdType != FDCAN_STANDARD_ID || header.RxFrameType != FDCAN_DATA_FRAME || header.DataLength != FDCAN_DLC_BYTES_8) return;
    
    //进入接收进程函数
    App_Chassis_CAN_RX_Callback(1,static_cast<uint16_t>(header.Identifier),
                                buffer,FDCAN_Convert_DLC_To_Length(header.DataLength));
}

static void FDCAN2_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr) return;

    if (header.IdType != FDCAN_STANDARD_ID || header.RxFrameType != FDCAN_DATA_FRAME || header.DataLength != FDCAN_DLC_BYTES_8) return;
    
    //进入接收进程函数
    //待写 接入版间通讯
}

static void FDCAN3_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr) return;

    if (header.IdType != FDCAN_STANDARD_ID || header.RxFrameType != FDCAN_DATA_FRAME || header.DataLength != FDCAN_DLC_BYTES_8) return;
    
    //进入接收进程函数
    App_Chassis_CAN_RX_Callback(3,static_cast<uint16_t>(header.Identifier),buffer,FDCAN_Convert_DLC_To_Length(header.DataLength));
}

/**
 * @brief 初始化App层CAN
 * 
 */
void AppCAN_Init(void)
{
    FDCAN1_Adapter.Init(&hfdcan1);
    FDCAN2_Adapter.Init(&hfdcan2);
    FDCAN3_Adapter.Init(&hfdcan3);

    CAN_Init(&hfdcan1, FDCAN1_Rx_Callback);
    CAN_Init(&hfdcan2, FDCAN2_Rx_Callback);
    CAN_Init(&hfdcan3, FDCAN3_Rx_Callback);
}

/**
 * @brief 获取CAN适应接口指针
 * 
 * @param CAN_Index 1-3
 * @return Class_CAN_Interface* CAN适应层接口
 */
Class_CAN_Interface *AppCAN_Get_Interface(uint8_t CAN_Index)
{
    switch (CAN_Index)
    {
        case 1:
            return &FDCAN1_Adapter;
        case 2:
            return &FDCAN2_Adapter;
        case 3:
            return &FDCAN3_Adapter;
        default:
            return nullptr;
    }
}
