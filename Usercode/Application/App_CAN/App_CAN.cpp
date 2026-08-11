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
#include "Application/App_Chassis_Attitude/App_Chassis_Attitude.h"
#include "Application/App_Fire/App_Fire.h"
#include "bsp_fdcan.h"
#include "CAN_Interface.h"
#include "fdcan_adapter.h"
#include "BoardCAN.h"

//FDCAN适应层类对象初始化
Class_FDCAN_Adapter FDCAN1_Adapter;
Class_FDCAN_Adapter FDCAN2_Adapter;
Class_FDCAN_Adapter FDCAN3_Adapter;

//舵轮步兵板间通信类对象
static Class_SteeringWheel_Infantry_BoardCAN BoardCAN;


/**
 * @brief 处理FDCAN1接收帧
 *
 * @param header FDCAN接收帧头
 * @param buffer FDCAN接收数据区
 */
static void FDCAN1_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr) return;

    if (header.IdType != FDCAN_STANDARD_ID || header.RxFrameType != FDCAN_DATA_FRAME || header.DataLength != FDCAN_DLC_BYTES_8) return;
    
    uint16_t CAN_ID = static_cast<uint16_t>(header.Identifier);
    uint8_t Length = FDCAN_Convert_DLC_To_Length(header.DataLength);

    //CAN1同时承载底盘轮电机和可选拨弹盘反馈，各模块只处理自己的CAN ID
    App_Chassis_CAN_RX_Callback(1,CAN_ID,buffer,Length);
    App_Fire_Process_CAN_Message(CAN_ID,buffer,Length);
}

/**
 * @brief 处理FDCAN2板间通信和云台Yaw反馈
 *
 * @param header FDCAN接收帧头
 * @param buffer FDCAN接收数据区
 */
static void FDCAN2_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr) return;

    if (header.IdType != FDCAN_STANDARD_ID || header.RxFrameType != FDCAN_DATA_FRAME || header.DataLength != FDCAN_DLC_BYTES_8) return;
    
    uint16_t CAN_ID = static_cast<uint16_t>(header.Identifier);
    uint8_t Length = FDCAN_Convert_DLC_To_Length(header.DataLength);

    //CAN2同时承载遥控共享数据和云台Yaw电机反馈
    BoardCAN.Process_CAN_Message(CAN_ID,buffer,Length,HAL_GetTick());
    App_Chassis_Attitude_Process_CAN_Message(CAN_ID,buffer,Length);
}

/**
 * @brief 处理FDCAN3接收帧
 *
 * @param header FDCAN接收帧头
 * @param buffer FDCAN接收数据区
 */
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

    //板间通信对象初始化
    BoardCAN.Init(&FDCAN2_Adapter,SteeringWheel_Infantry_BoardCAN_Role_e::Chassis);

    CAN_Init(&hfdcan1, FDCAN1_Rx_Callback);
    CAN_Init(&hfdcan2, FDCAN2_Rx_Callback);
    CAN_Init(&hfdcan3, FDCAN3_Rx_Callback);
}

/**
 * @brief 获取舵轮步兵板间通信对象指针
 *
 * @return Class_SteeringWheel_Infantry_BoardCAN* 板间通信对象指针
 */
Class_SteeringWheel_Infantry_BoardCAN *AppCAN_Get_BoardCAN(void)
{
    return &BoardCAN;
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
