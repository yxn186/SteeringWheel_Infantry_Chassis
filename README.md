# SteeringWheel Infantry Chassis

本文档当前只说明本工程的 CMake 组织方式，目标是解释如何将：

- STM32CubeMX 生成代码；
- `Usercode/App` 应用代码；
- `Usercode/Task` FreeRTOS 任务代码；
- `YXN_ECF` 公共模块；

一起编译并链接进最终的 `SteeringWheel_Infantry_Chassis.elf`。

## 1. 当前构建关系

```text
Core、Drivers、Middlewares
        │
        └── cmake/stm32cubemx/CMakeLists.txt
                         │
Usercode/App/*.c,*.cpp ──┤
Usercode/Task/*.c,*.cpp ─┼── SteeringWheel_Infantry_Chassis.elf
选中的 YXN_ECF 模块 ─────┘
```

工程采用“源文件直接加入最终 ELF”的方式。App、Task 和当前选中的 ECF 文件都不是静态库，因此不需要 `--whole-archive`。

## 2. CMake 基础语法

### 2.1 调用命令

CMake 命令使用圆括号：

```cmake
command(argument1 argument2)
```

例如：

```cmake
add_executable(${CMAKE_PROJECT_NAME})
```

表示创建一个可执行目标。STM32 工程中的“可执行目标”最终就是 ELF 固件。

### 2.2 注释

以 `#` 开头的内容是注释：

```cmake
# This is a comment
```

注释不会参与配置和编译。

### 2.3 变量

`set()` 用于定义变量：

```cmake
set(CMAKE_PROJECT_NAME SteeringWheel_Infantry_Chassis)
```

`${变量名}` 用于读取变量：

```cmake
add_executable(${CMAKE_PROJECT_NAME})
```

展开后相当于：

```cmake
add_executable(SteeringWheel_Infantry_Chassis)
```

### 2.4 列表

CMake 列表不写逗号，每个元素使用空格或换行分隔：

```cmake
set(SOURCE_FILES
    App/App_Chassis.cpp
    Task/User_Task.cpp
)
```

### 2.5 路径

建议给组合得到的路径加双引号：

```cmake
"${CMAKE_CURRENT_SOURCE_DIR}/YXN_ECF"
```

这样路径中即使出现空格，也不会被拆成多个参数。

### 2.6 PRIVATE

现代 CMake 的配置通常附着在某个 target 上：

```cmake
target_sources(target PRIVATE source.cpp)
```

`PRIVATE` 表示该配置只属于当前 target，不向其他 target 传播。

## 3. 顶层 CMakeLists.txt

### 3.1 CMake 最低版本

```cmake
cmake_minimum_required(VERSION 3.22)
```

表示配置本工程至少需要 CMake 3.22。

### 3.2 C 语言标准

```cmake
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
```

- `CMAKE_C_STANDARD 11`：`.c` 文件按 C11 编译。
- `CMAKE_C_STANDARD_REQUIRED ON`：编译器必须满足该标准。
- `CMAKE_C_EXTENSIONS ON`：允许 GNU C 扩展。

CubeMX、HAL 和 FreeRTOS 的大量文件都是 C 文件，所以这些配置必须保留。

### 3.3 构建类型

```cmake
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
```

含义是：如果外部没有指定构建类型，就使用 Debug。

`if()` 和 `endif()` 必须成对出现。

### 3.4 工程名和编译数据库

```cmake
set(CMAKE_PROJECT_NAME SteeringWheel_Infantry_Chassis)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
```

第二行会生成 `compile_commands.json`。它可供 clangd 使用，也能用于检查某个 `.cpp` 是否真正获得了编译命令。

### 3.5 创建工程并启用语言

```cmake
project(${CMAKE_PROJECT_NAME})
enable_language(C CXX ASM)
```

- `C` 编译 `.c`；
- `CXX` 编译 `.cpp`；
- `ASM` 编译 STM32 启动汇编。

本工程工具链已将 C++ 编译器设置为 `arm-none-eabi-g++`。

### 3.6 创建最终 ELF

```cmake
add_executable(${CMAKE_PROJECT_NAME})
```

该命令创建最终目标。后续所有 `target_*()` 命令都是继续给这个目标增加属性。

### 3.7 指定 C++17

```cmake
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
```

它要求属于该目标的 `.cpp` 至少使用 C++17。

这条命令必须写在 `add_executable()` 后，因为目标必须先存在，才能为目标配置编译特性。

### 3.8 接入 CubeMX 生成代码

```cmake
add_subdirectory(cmake/stm32cubemx)
```

`add_subdirectory()` 会进入指定目录并执行其中的 `CMakeLists.txt`。

HAL、FreeRTOS、启动文件和 `Core/Src/freertos.c` 都通过 CubeMX 子目录进入最终目标，因此不能删除这条命令。

不要把手写 Usercode 放进 `cmake/stm32cubemx/CMakeLists.txt`，因为该文件属于生成配置，后续重新生成工程时可能改变。

### 3.9 接入 Usercode

```cmake
add_subdirectory(Usercode)
```

这条命令只负责执行 `Usercode/CMakeLists.txt`，不会自动扫描或编译 Usercode 中的文件。

`Usercode/CMakeLists.txt` 必须存在，否则配置阶段会报错。

### 3.10 链接 CubeMX 接口

```cmake
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
)
```

`stm32cubemx` 为最终目标提供 CubeMX 配置的头文件路径、宏定义、HAL、FreeRTOS 和工具链库。

## 4. Usercode/CMakeLists.txt

推荐目录：

```text
Usercode/
├── App/
├── Task/
└── CMakeLists.txt
```

### 4.1 明确列出 App 源文件

```cmake
set(USER_APP_SOURCES
    # App/App_Chassis.cpp
)
```

真正创建文件后，写成：

```cmake
set(USER_APP_SOURCES
    App/App_Chassis.cpp
    App/App_Communication.cpp
)
```

这些路径相对 `Usercode/CMakeLists.txt` 所在的 `Usercode` 目录。

明确列出源文件的优点：

- 能一眼看到哪些文件参与构建；
- 路径错误时 CMake 立即报错；
- 新文件不会因为被放错目录而自动进入固件；
- 不依赖 `GLOB_RECURSE` 和 `VerifyGlobs.cmake`。

### 4.2 明确列出 Task 源文件

```cmake
set(USER_TASK_SOURCES
    # Task/User_Task.cpp
)
```

任务文件创建后，去掉注释：

```cmake
set(USER_TASK_SOURCES
    Task/User_Task.cpp
)
```

### 4.3 合并列表

```cmake
set(USERCODE_SOURCES
    ${USER_APP_SOURCES}
    ${USER_TASK_SOURCES}
)
```

此时 `USERCODE_SOURCES` 保存 App 与 Task 的全部源文件。

### 4.4 直接加入最终 ELF

```cmake
if(USERCODE_SOURCES)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${USERCODE_SOURCES}
    )
endif()
```

- `if(USERCODE_SOURCES)`：列表非空时才执行。
- `target_sources()`：把源文件直接加入最终 ELF。
- `PRIVATE`：源文件只属于当前主控目标。

使用条件判断后，即使 App 和 Task 暂时为空，工程也能完成 CMake 配置。

### 4.5 添加 Usercode 头文件路径

```cmake
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
```

在该文件中，`CMAKE_CURRENT_SOURCE_DIR` 表示 `Usercode` 目录。

推荐的包含方式：

```cpp
#include "App/App_Chassis.h"
#include "Task/User_Task.h"
```

头文件不需要加入 `USER_APP_SOURCES` 或 `USER_TASK_SOURCES`。`target_include_directories()` 只负责告诉编译器去哪里寻找头文件。

## 5. FreeRTOS weak 任务与 C++

CubeMX 当前生成：

```c
__weak void InitTask_Function(void *argument);
__weak void Main_Task_Function(void *argument);
```

并在 `MX_FREERTOS_Init()` 中通过 `osThreadNew()` 创建任务。

任务函数不需要出现在 `main.c`。`main.c` 只负责：

```text
osKernelInitialize()
MX_FREERTOS_Init()
osKernelStart()
```

### 5.1 C++ 强实现必须使用 C 链接名

在 `Usercode/Task/User_Task.cpp` 中：

```cpp
extern "C" void InitTask_Function(void *argument)
{
    // Initialization
}

extern "C" void Main_Task_Function(void *argument)
{
    for (;;)
    {
        osDelay(1);
    }
}
```

函数名、大小写和参数必须与 `freertos.c` 完全一致。

`extern "C"` 只关闭 C++ 函数名修饰，函数体内部仍然可以使用 C++ 类和对象。

如果省略 `extern "C"`，链接器会同时看到：

```text
C weak symbol
C++ mangled strong symbol
```

它们不是同一个符号，运行时仍可能进入 CubeMX 的 weak 空任务。

### 5.2 为什么直接 target_sources 最稳妥

`User_Task.cpp` 通过 `target_sources()` 直接生成对象文件并进入最终链接。链接器同时看到同名 weak 和 strong 符号时，会选择 strong 符号。

如果任务实现被放进普通静态库，weak 符号已经满足引用，链接器可能不会从 `.a` 中提取任务对象。这时才需要 `--whole-archive`。

本工程当前不创建 Usercode 静态库，所以不需要 `--whole-archive`。

## 6. 接入 YXN_ECF

当前 ECF 是 Git Submodule，并采用以下主要层级：

```text
YXN_ECF/
├── 1_bsp/
└── 2_module/
    └── Alg/
        ├── PID/
        ├── LowPassFilter/
        ├── MyMath/
        └── SlopePlaning/
```

它是可选模块集合，不应该被整个递归加入编译。

### 6.1 定义 ECF 根目录

在顶层 `CMakeLists.txt` 中：

```cmake
set(YXN_ECF_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/YXN_ECF"
)
```

顶层文件中的 `CMAKE_CURRENT_SOURCE_DIR` 表示当前 Chassis 工程根目录。

### 6.2 以 PID 为例加入源文件

PID 的真实路径是：

```text
YXN_ECF/2_module/Alg/PID/PID.cpp
YXN_ECF/2_module/Alg/PID/PID.h
```

加入实现：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${YXN_ECF_DIR}/2_module/Alg/PID/PID.cpp
)
```

加入头文件搜索路径：

```cmake
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${YXN_ECF_DIR}/2_module/Alg/PID
)
```

之后 App 和 Task 都属于同一个最终目标，所以两处都可以：

```cpp
#include "PID.h"
```

### 6.3 source 与 include 的区别

| 命令 | 作用 | 缺失时的典型结果 |
|---|---|---|
| `target_sources()` | 编译 `.c/.cpp` 并链接实现 | `undefined reference` |
| `target_include_directories()` | 让编译器找到 `.h` | `No such file or directory` |
| `target_link_libraries()` | 链接另一个已创建的库目标 | 当前直接源文件方案不需要 |

### 6.4 模块依赖必须一起接入

例如 `SlopePlaning.cpp` 使用 `MyMath.h`，所以需要同时添加：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${YXN_ECF_DIR}/2_module/Alg/SlopePlaning/SlopePlaning.cpp
    ${YXN_ECF_DIR}/2_module/Alg/MyMath/MyMath.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${YXN_ECF_DIR}/2_module/Alg/SlopePlaning
    ${YXN_ECF_DIR}/2_module/Alg/MyMath
)
```

### 6.5 不要一次性编译整个 ECF

当前 ECF 中仍有部分模块包含：

- `stm32f4xx_*.h`；
- `stm32f103xb.h`；
- `core_cm4.h`；
- 尚未在当前主控生成的 `tim.h`、`i2c.h` 或其他外设头文件。

本工程使用 STM32H723/Cortex-M7，因此每接入一个 BSP 或硬件模块前，都要先检查芯片系列、HAL API 和外设依赖。

PID、低通滤波等纯算法模块适合作为第一批接入对象。

## 7. 为什么当前不使用静态库

| 方案 | 对象文件是否直接进入链接 | 复杂度 | 当前建议 |
|---|---:|---:|---:|
| `target_sources()` | 是 | 低 | 推荐 |
| OBJECT library | 是 | 中 | 模块明显增多后考虑 |
| STATIC library | 按未解析引用提取 | 中 | ECF 独立发布时考虑 |
| STATIC + `--whole-archive` | 强制提取 | 高 | weak 任务库等特殊情况 |

普通 ECF 函数被 App 引用时，做成 STATIC library 通常也能被正常提取；但当前直接源文件方式更容易学习和排查。

## 8. 添加新文件的标准流程

### 添加 App 文件

1. 创建 `Usercode/App/App_Chassis.cpp` 和对应头文件。
2. 在 `USER_APP_SOURCES` 中加入 `App/App_Chassis.cpp`。
3. 需要时在 Task 中包含 `App/App_Chassis.h`。
4. 重新构建。

### 添加 Task 文件

1. 创建 `Usercode/Task/User_Task.cpp`。
2. 在 `USER_TASK_SOURCES` 中加入 `Task/User_Task.cpp`。
3. 使用 `extern "C"` 实现与 CubeMX 完全一致的任务入口。
4. 重新构建并检查 ELF 符号。

### 添加 ECF 模块

1. 找到模块的真实 `.c/.cpp` 和 `.h` 路径。
2. 检查其 `#include`，确认依赖和芯片系列兼容。
3. 把实现加入顶层 `target_sources()`。
4. 把自身及依赖的目录加入 `target_include_directories()`。
5. 在 App/Task 中包含头文件并构建。

## 9. 构建与验证

### 9.1 配置和构建

```powershell
cmake --preset Debug
cmake --build --preset Debug -j 4
```

以上命令假设终端中的 `cmake` 与创建 `build/Debug` 缓存时使用的是同一套 CMake。可以先检查：

```powershell
Get-Command cmake
cmake --version
```

STM32Cube 扩展可能使用自己的 Bundled CMake，而系统 `PATH` 中也可能存在另一版本。不要让两个版本交替修改同一个构建目录；应持续使用 STM32Cube 当前配置的 CMake，或者为另一版本使用独立的新构建目录。

### 9.2 检查 `.cpp` 是否参与编译

在 `build/Debug/compile_commands.json` 中搜索：

```text
Usercode
YXN_ECF
```

对应 `.cpp` 应使用 `arm-none-eabi-g++`。

### 9.3 检查 RTOS 任务符号

```powershell
arm-none-eabi-nm -C build/Debug/SteeringWheel_Infantry_Chassis.elf |
    Select-String 'InitTask_Function|Main_Task_Function'
```

期望：

```text
T InitTask_Function
T Main_Task_Function
```

- `T`：Usercode 中的 strong 实现已进入 ELF。
- `W`：仍然只有 CubeMX weak 实现。

符号为 `T` 只能证明链接正确。最终运行仍应通过断点、LED、串口或调试变量确认任务被实际调度。

## 10. 常见错误

### `Usercode does not contain a CMakeLists.txt`

根 CMake 已调用 `add_subdirectory(Usercode)`，但缺少 `Usercode/CMakeLists.txt`。

### `Cannot find source file`

源文件列表中的路径拼写错误，或者文件尚未创建。

### `PID.h: No such file or directory`

缺少 PID 的 `target_include_directories()`。

### `undefined reference to Class_PID...`

只添加了 PID 头文件路径，没有将 `PID.cpp` 加入 `target_sources()`。

### 任务仍然显示为 `W`

检查：

1. `User_Task.cpp` 是否列入 `USER_TASK_SOURCES`；
2. 定义是否使用 `extern "C"`；
3. 函数名、大小写和参数是否与 `freertos.c` 一致；
4. 最终检查的 ELF 是否是刚刚重新链接得到的文件。

### 新文件没有自动参与编译

本工程故意采用显式源文件列表。创建文件后必须把路径加入相应 `set(...)` 列表。

## 11. 核心理解

```text
add_executable()
    创建最终 ELF 目标

add_subdirectory()
    执行另一个目录中的 CMakeLists.txt

target_sources()
    把实现文件加入目标

target_include_directories()
    为目标添加头文件搜索路径

target_compile_features()
    为目标指定 C++ 标准能力

target_link_libraries()
    将另一个 CMake 库目标关联到当前目标
```

只添加 include 路径不等于编译实现，只创建 `.cpp` 文件也不等于它会自动进入工程。源文件必须通过 CMake 成为最终目标的一部分。
