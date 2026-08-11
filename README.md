# 舵轮步兵底盘工程

本工程运行于 STM32H723VET6 底盘主控，负责四舵轮运动解算、八个底盘电机闭环、BMI088 姿态解算、云台相对角读取、DR16 遥控数据接收以及可选的下供弹拨弹盘。代码按照 BSP、Module、Application、Task 分层组织。

## 1. 代码分层

```text
Core / Drivers / USB_DEVICE
    CubeMX生成的HAL、外设和USB代码
YXN_ECF/bsp
    FDCAN、SPI、USART等硬件适配
YXN_ECF/module
    电机、DR16、BoardCAN、BMI088和舵轮底盘模块
YXN_ECF/algorithm
    PID、Mahony、Rotation3D等算法
Usercode/Application
    App_CAN、App_Remote、App_IMU、App_Chassis、
    App_Chassis_Attitude、App_Chassis_Control、App_Fire
Usercode/Chassis_Task
    初始化顺序和1 ms周期调度
```

Task 不直接操作电机或 HAL。运动模式由 App_Chassis_Control 选择，坐标关系由 App_Chassis_Attitude 提供，底层电机闭环和运动解算由 App_Chassis 完成。

## 2. 主要硬件与总线

| 总线 | 设备 | 说明 |
| --- | --- | --- |
| CAN1 | 4个M3508轮电机 | 电机ID 1~4，速度闭环 |
| CAN1 | M3508拨弹盘 | 默认ID 5，仅在拨弹盘位于底盘时注册 |
| CAN2 | BoardCAN | 两板共享DR16数据 |
| CAN2 | GM6020 Yaw反馈 | 只读对象，不发送Yaw控制帧 |
| CAN3 | 4个GM6020转向电机 | 舵角闭环 |
| SPI2 | BMI088 | 底盘连续Yaw和角速度 |
| UART5 | DR16 | 只有被配置为DR16安装板时才启动 |

CAN1 的轮电机使用 0x200 电流帧，ID 5 拨弹盘使用 0x1FF 电流帧，两组对象互不覆盖。CAN2 同时承载 BoardCAN 和云台 Yaw 反馈，回调按照 CAN ID 分别交给对应模块。

## 3. 初始化与周期更新

Init Task 的主要顺序为：

1. 初始化 USB。
2. 初始化底盘八电机、运动解算和姿态关系对象。
3. 初始化本板可选的拨弹盘对象。
4. 启动三路 FDCAN。
5. 初始化遥控控制与 BMI088。
6. 设置 `Global_Init_Finished` 后结束初始化线程。

Main Task 每 1 ms 更新 BMI088、底盘与云台姿态关系、遥控模式和发射机构。遥控离线、关键反馈缺失或模式未定义时，底盘进入无力并持续发送零输出。

## 4. 底盘控制链路

遥控数据经过以下路径进入底盘：

```text
UART5 DR16 或 CAN2 BoardCAN
        ↓
App_Remote
        ↓
App_Chassis_Control
        ↓
App_Chassis
        ↓
舵轮解算、PID和CAN输出
```

左摇杆生成以云台正方向为基准的 X/Y 平移目标。App_Chassis_Attitude 使用 CAN2 上的 GM6020 编码器角得到云台相对底盘角，再把云台坐标系速度旋转到底盘坐标系。

底盘 BMI088 提供底盘连续 Yaw。该角度与云台相对角相加，可得到启动零点下的云台世界系 Yaw，用于姿态观察和跟随关系。

## 5. 遥控模式

| 左拨杆 | 右拨杆 | 底盘行为 |
| --- | --- | --- |
| 中 | 上 | 普通平移，右摇杆交给云台 |
| 中 | 中 | 普通平移，右摇杆X控制底盘角速度 |
| 上 | 下 | 小陀螺，保持云台坐标系平移并高速自旋 |
| 上 | 中 | 底盘正方向闭环跟随云台 |
| 上 | 上 | 发射模式，底盘无力 |
| 其他 | 其他 | 无力 |

小陀螺模式每个周期根据当前云台相对角转换平移目标，因此底盘自旋时操作者的前后左右仍以云台朝向为基准。底盘跟随模式使用云台相对底盘角作为位置误差，由角度环产生底盘角速度。

速度上限、摇杆方向、小陀螺角速度和跟随 PID 位于 `App_Chassis_Control_Config.h`。轮电机、转向电机和舵轮几何参数位于 App_Chassis 对应配置中。

## 6. 发射机构

`App_Fire` 在底盘侧只根据 `SteeringWheel_Infantry_Feeder_Location` 决定是否注册 ID 5 拨弹盘。四个摩擦轮始终由云台侧控制。

- 左拨杆上、右拨杆上：两侧同时进入发射状态。
- 拨弹盘每次单发使连续角度目标增加配置的每发角度。
- 拨弹盘反馈超时或单发过程中持续低速：锁止为零输出。
- 锁止后必须退出双上模式，再重新进入双上模式才会重试。

拨轮真实数据尚未确认，`App_Fire_Check_Dial_Wheel_Trigger()` 保留触发判断位置。确认拨轮变化规律后，只需在这里产生一次单发请求。其他应用也可调用 `App_Fire_Request_Single_Shot()`。

拨弹盘位置、电机方向、每发角度、PID和堵转门限都在 `Usercode/Application/App_Fire/App_Fire_Config.h` 中设置。

## 7. 板卡位置切换

DR16 安装位置由公共库 `Remote_Data_Reception.cpp` 中的 `SteeringWheel_Infantry_DR16_Location` 决定。安装板从 UART5 获取数据并通过 CAN2 共享，非安装板只读取 CAN2。

拨弹盘安装位置由两侧相同的 `SteeringWheel_Infantry_Feeder_Location` 决定：

- `SteeringWheel_Infantry_Board_Gimbal`：拨弹盘在云台。
- `SteeringWheel_Infantry_Board_Chassis`：拨弹盘在底盘。

修改位置 define 后，两块主控必须同步同一配置并重新编译烧录。

## 8. 离线与堵转保护

App_Chassis 在无力状态下仍更新电机反馈、PID Current 和运动学状态，随后跳过非零控制并持续发送零电流帧。这样恢复反馈后不需要重新创建对象，也不会保留旧积分输出。

App_Fire 对本板发射电机独立检测反馈和堵转。任何本地发射电机锁止时，本板发射电机全部清零；重新进入双上模式会清除锁止并重新等待反馈。

当前 PID 参数和输出限幅默认均为 0，目标值与控制框架已经建立，但不会产生有效驱动力。实物调试时应先确认电机ID、方向、机械零位和 CAN 反馈，再逐步增加输出限幅与 PID 参数。

## 9. 配置与验证边界

新增 APP 源文件必须显式加入 `Usercode/CMakeLists.txt`。公共 BSP、Module 和 Algorithm 源文件由根目录 CMake 按明确列表接入，避免把整个公共库递归编译。

编译或链接成功不代表舵轮方向、BMI088安装方向、机械零位、坐标变换或堵转阈值已经通过硬件验证。首次上电应架空车体、限制输出，并准备可靠的急停或断电手段。
