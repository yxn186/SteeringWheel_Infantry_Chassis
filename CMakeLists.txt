cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings 设置 C 语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name 设置工程名称
set(CMAKE_PROJECT_NAME SteeringWheel_Infantry_Chassis)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
#它会生成 compile_commands.json，方便 clangd，也方便检查你的 .cpp 有没有真正参与编译。

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for C, CXX and ASM languages
enable_language(C CXX ASM)
#启用C CXX ASM语言支持
# C：编译 CubeMX 生成的 .c 文件。
# CXX：编译你的 .cpp 文件，调用 arm-none-eabi-g++。
# ASM：编译 STM32 启动汇编文件。

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})
#↑创建最终 ELF 目标；在本工程中 ${CMAKE_PROJECT_NAME} 展开为 SteeringWheel_Infantry_Chassis

# Require C++17 for C++ sources belonging to this ELF
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
#target_compile_features() 要求这个目标里的 .cpp 至少按照 C++17 编译。
#PRIVATE 表示这个要求只作用于当前 ELF，不向其他目标传播。
#必须放在 add_executable() 后面，因为目标尚未创建之前，不能给它设置编译特性。
#它不负责寻找 .cpp，只是规定：以后加入这个目标的 .cpp 应该采用 C++17。

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
#↑FreeRTOS、HAL、启动文件以及 freertos.c 都是通过这里进入工程的，绝对不能删除。

# Add user application and task sources
add_subdirectory(Usercode)
#↑表示 CMake 配置工程时，继续读取：Usercode/CMakeLists.txt
# 需要注意，它不会自动编译 Usercode 中的所有文件。
# 它只是进入该目录并执行里面的 CMakeLists.txt。之后我们会在 Usercode/CMakeLists.txt 中用 target_sources() 把 App、Task 源文件加入最终 ELF。
# 因此，实际添加这一行时必须确保下面这个文件已经存在：
# Usercode/CMakeLists.txt

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# ============================================================
# YXN_ECF common library
# ============================================================

set(YXN_ECF_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/YXN_ECF"
)
# ↑ 定义 ECF 根目录；顶层 CMakeLists.txt 中的 CMAKE_CURRENT_SOURCE_DIR
#   就是当前 Chassis 工程的根目录。

# YXN_ECF 使用方法：
#
# 1. 把需要使用的模块源文件加入最终 ELF，例如 PID：
# target_sources(${CMAKE_PROJECT_NAME} PRIVATE
#     ${YXN_ECF_DIR}/2_module/Alg/PID/PID.cpp
# )
#
# 2. 添加该模块的头文件搜索路径：
# target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
#     ${YXN_ECF_DIR}/2_module/Alg/PID
# )
#
# 只接入当前主控实际需要且已适配 STM32H723 的模块，不要递归编译整个 YXN_ECF。

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # ${YXN_ECF_DIR}/2_module/Alg/PID/PID.cpp
    # Add user sources here
    ${YXN_ECF_DIR}/module/Chassis_Calculation/SteeringWheel_Chassis_Calculation/SteeringWheel_Chassis_Calculation.cpp
    ${YXN_ECF_DIR}/bsp/CAN/FDCAN/bsp_fdcan.cpp
    ${YXN_ECF_DIR}/bsp/CAN/FDCAN/fdcan_adapter.cpp
    ${YXN_ECF_DIR}/module/Motor/DJI_Motor/DJI_Motor.cpp
    ${YXN_ECF_DIR}/algorithm/PID/PID.cpp
    ${YXN_ECF_DIR}/module/SteeringWheel_Infantry/BoardCAN/BoardCAN.cpp
)

#头文件路径和宏定义区域
# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    # ${YXN_ECF_DIR}/2_module/Alg/PID
    ${YXN_ECF_DIR}/module/Chassis_Calculation/SteeringWheel_Chassis_Calculation
    ${YXN_ECF_DIR}/bsp/CAN/FDCAN
    ${YXN_ECF_DIR}/bsp/CAN
    ${YXN_ECF_DIR}/module/Motor/DJI_Motor
    ${YXN_ECF_DIR}/bsp/CAN
    ${YXN_ECF_DIR}/algorithm/PID
    ${YXN_ECF_DIR}/module/SteeringWheel_Infantry/BoardCAN
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

#C++ 链接兼容处理
# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    #stm32cubemx 链接
    #负责把 CubeMX 配置的包含路径、宏、驱动和 FreeRTOS 内容关联到最终 ELF。

    # Add user defined libraries
)
