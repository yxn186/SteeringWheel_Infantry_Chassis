# Application source files
set(USER_APP_SOURCES
    # App/App_Chassis.cpp
)
# set() 创建名为 USER_APP_SOURCES 的 CMake 列表变量。
# 每创建一个 App 源文件，就把相对 Usercode 目录的路径写在这里，例如：
#
# set(USER_APP_SOURCES
#     App/App_Chassis.cpp
#     App/App_Communication.cpp
# )

# FreeRTOS task source files
set(USER_TASK_SOURCES
    # Task/User_Task.cpp
)
# Task 层的写法与 App 相同，例如：
#
# set(USER_TASK_SOURCES
#     Task/User_Task.cpp
# )

# Combine all Usercode source files
set(USERCODE_SOURCES
    ${USER_APP_SOURCES}
    ${USER_TASK_SOURCES}
)
# ${变量名} 表示读取变量保存的内容。
# 合并后只需要把 USERCODE_SOURCES 一次性加入最终 ELF。

# Add Usercode sources directly to the final ELF target
if(USERCODE_SOURCES)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${USERCODE_SOURCES}
    )
endif()
# if(USERCODE_SOURCES) 让 App/Task 目录暂时为空时也能正常配置工程。
# target_sources() 会把源文件直接加入根 CMake 创建的最终 ELF。
# PRIVATE 表示这些源文件只属于当前主控目标。

# Make Usercode headers visible to this ELF target
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
# 在本文件中，CMAKE_CURRENT_SOURCE_DIR 表示当前工程的 Usercode 目录。
# 因此推荐使用带层级的头文件写法：
#
# #include "App/App_Chassis.h"
# #include "Task/User_Task.h"
#
# 头文件不需要列入 USER_APP_SOURCES 或 USER_TASK_SOURCES；
# target_include_directories() 只负责告诉编译器去哪里寻找头文件。
