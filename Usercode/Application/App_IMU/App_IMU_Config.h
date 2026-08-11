/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_IMU_Config.h
  * @brief   IMU应用配置
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_IMU_CONFIG_H__
#define __APP_IMU_CONFIG_H__

#include "main.h"
#include "spi.h"
#include "bmi088reg.h"

//BMI088硬件接口
#define App_IMU_SPI_Handler                     (&hspi2)                    //BMI088使用的SPI句柄

#define App_IMU_ACC_CS_GPIO_Port                CS1_ACCLE_GPIO_Port        //加速度计片选端口
#define App_IMU_ACC_CS_Pin                      CS1_ACCLE_Pin              //加速度计片选引脚
#define App_IMU_ACC_CS_Active_Level             GPIO_PIN_RESET            //加速度计片选低电平有效
#define App_IMU_ACC_INT_Pin                     INT1_ACCEL_Pin             //加速度数据就绪中断引脚

#define App_IMU_GYRO_CS_GPIO_Port               CS2_GYRO_GPIO_Port         //陀螺仪片选端口
#define App_IMU_GYRO_CS_Pin                     CS2_GYRO_Pin               //陀螺仪片选引脚
#define App_IMU_GYRO_CS_Active_Level            GPIO_PIN_RESET            //陀螺仪片选低电平有效
#define App_IMU_GYRO_INT_Pin                    INT3_GYRO_Pin              //陀螺仪数据就绪中断引脚

//BMI088采样配置
#define App_IMU_ACC_Config_Value                 (BMI088_ACC_NORMAL | BMI088_ACC_800_HZ | BMI088_ACC_CONF_MUST_Set) //加速度计800Hz正常模式
#define App_IMU_ACC_Range_Value                  BMI088_ACC_RANGE_6G       //写入寄存器的±6g量程值
#define App_IMU_ACC_Range_G                      6.0f                      //换算原始加速度时使用的量程

#define App_IMU_GYRO_Bandwidth_Value             (BMI088_GYRO_1000_116_HZ | BMI088_GYRO_BANDWIDTH_MUST_Set) //陀螺仪1000Hz输出、116Hz带宽
#define App_IMU_GYRO_Range_Value                 BMI088_GYRO_2000          //写入寄存器的±2000dps量程值
#define App_IMU_GYRO_Range_DPS                   2000.0f                   //换算原始角速度时使用的量程

#define App_IMU_Feedback_Timeout_ms              100                       //超过该时间没有完整数据就判定离线

//零偏校准参数
#define App_IMU_Bias_Target_Samples              800                       //一次校准总共检查的样本数
#define App_IMU_Bias_GYRO_Norm_Max_DPS           3.0f                      //低于该角速度才认为底盘静止
#define App_IMU_Bias_ACC_Norm_Min_G              0.9f                      //有效样本允许的最小重力模长
#define App_IMU_Bias_ACC_Norm_Max_G              1.1f                      //有效样本允许的最大重力模长

//Mahony参数
#define App_IMU_Mahony_Kp                        1.0f                      //加速度修正姿态的比例系数
#define App_IMU_Mahony_Ki                        0.0f                      //加速度修正姿态的积分系数
#define App_IMU_Mahony_ACC_Norm_Min_G            0.9f                      //低于该模长时不使用加速度修正
#define App_IMU_Mahony_ACC_Norm_Max_G            1.1f                      //高于该模长时不使用加速度修正

//传感器坐标系到底盘机体系的旋转矩阵，默认XYZ同向
#define App_IMU_Mount_R00                        1.0f                      //机体X轴中的传感器X轴分量
#define App_IMU_Mount_R01                        0.0f                      //机体X轴中的传感器Y轴分量
#define App_IMU_Mount_R02                        0.0f                      //机体X轴中的传感器Z轴分量
#define App_IMU_Mount_R10                        0.0f                      //机体Y轴中的传感器X轴分量
#define App_IMU_Mount_R11                        1.0f                      //机体Y轴中的传感器Y轴分量
#define App_IMU_Mount_R12                        0.0f                      //机体Y轴中的传感器Z轴分量
#define App_IMU_Mount_R20                        0.0f                      //机体Z轴中的传感器X轴分量
#define App_IMU_Mount_R21                        0.0f                      //机体Z轴中的传感器Y轴分量
#define App_IMU_Mount_R22                        1.0f                      //机体Z轴中的传感器Z轴分量

#endif /* __APP_IMU_CONFIG_H__ */

