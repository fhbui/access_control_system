cmake_minimum_required(VERSION 3.30.3)
project(program_rtos LANGUAGES C CXX)

# 设置编译器
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_VERSION: ${CMAKE_C_COMPILER_VERSION}")

set(MDK_INCLUDE_PATH 
    "J:/software/Keil_v5_32/ARM/ARMCC/include" 
    )

# 设置路径
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR} 
    ${PROJECT_SOURCE_DIR}/Code 
    ${PROJECT_SOURCE_DIR}/Code/BSP 
    ${PROJECT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include 
    ${PROJECT_SOURCE_DIR}/Drivers/CMSIS/Include 
    ${PROJECT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc 
    ${PROJECT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy 
    ${PROJECT_SOURCE_DIR}/Core/Inc
    ${MDK_INCLUDE_PATH}
    
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include 
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM4F 
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/LVGL/LVGL_SRC
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/LVGL/LVGL_SRC/lvgl
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/LVGL/LVGL_SRC/lvgl/examples/porting
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/LVGL/LVGL_SRC/lvgl/src

    )

# 自己的代码
file(GLOB_RECURSE MY_INCLUDES "Code/*.h" "Code/BSP/*.h")
file(GLOB_RECURSE MY_SOURCES "Code/*.c" "Code/BSP/*.c")

# CMSIS驱动代码
file(GLOB_RECURSE CMSIS_INCLUDES "Drivers/CMSIS/Device/ST/STM32F4xx/Include/*.h" "Drivers/CMSIS/Include/*.h" "Drivers/CMSIS/RTOS/Template")
# file(GLOB_RECURSE MY_SOURCES "Code/*.c" "Code/BSP/*.c")

# HAL库驱动代码
file(GLOB_RECURSE HAL_INCLUDES "Drivers/STM32F4xx_HAL_Driver/Inc/*.h" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/*.h")
file(GLOB_RECURSE HAL_SOURCES "Drivers/STM32F4xx_HAL_Driver/Src/*.c")

# MDK内置代码（标准C库）
file(GLOB_RECURSE MDK_INCLUDES "J:/software/Keil_v5_32/ARM/ARMCC/include/*.h")

# 核心代码
file(GLOB_RECURSE CORE_INCLUDES "Core/Inc/*.h" )
file(GLOB_RECURSE CORE_SOURCES  "Core/Src/*.c" )

file(GLOB_RECURSE INCLUDES ${MY_INCLUDES} ${HAL_INCLUDES} ${CORE_INCLUDES} ${MDK_INCLUDES})
file(GLOB_RECURSE SOURCES  ${MY_SOURCES} ${HAL_SOURCES} ${CORE_SOURCES})

# 将所有.c/.h文件集中到变量主要是为了add_executable，如果只是单纯编辑的话可以不用
add_executable(main ${SOURCES} ${INCLUDES})     #好像添加这句后检索C库就正常了，可能是之前一些cmake信息没被删除
