cmake_minimum_required(VERSION 3.16)

# ==========================================
# 1. Toolchain Setup (交叉编译配置)
# ==========================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(TOOLCHAIN arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN}objdump)
set(CMAKE_SIZE ${TOOLCHAIN}size)

# ==========================================
# 2. Project Initialization (项目初始化)
# ==========================================
project(STM32_Linux_Project C ASM CXX)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==========================================
# 3. Compiler & Core Settings (内核与编译选项)
# ==========================================
# 直接拆开写，不使用带双引号的变量
add_compile_options(-mcpu=cortex-m3 -mthumb -Os -g -Wall -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m3 -mthumb")

# 芯片宏定义 (保持启用 HAL 库驱动)
add_compile_definitions(STM32F103xB USE_HAL_DRIVER)

# ==========================================
# 4. Sources & Executable (源码与可执行文件)
# ==========================================
# 搜集用户源码
file(GLOB_RECURSE USER_SOURCES
    "APP/Src/*.c"
    "BSP/Src/*.c"
    "Device/Src/*.c"
)

# 【关键修复一】：必须先创建可执行文件目标！
add_executable(${PROJECT_NAME} ${USER_SOURCES})

# 【关键修复二】：然后再加载 CubeMX 的子目录，因为子目录依赖上面的 ${PROJECT_NAME}
add_subdirectory(cmake/stm32cubemx)

# 添加头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE
    APP/Inc
    BSP/Inc
    Device/Inc  
)

# 链接CubeMX生成的库
target_link_libraries(${PROJECT_NAME} stm32cubemx)

# 移除cpp文件可能会引用的错误库依赖
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# ==========================================
# 5. Linker Script & Linker Options (链接器配置)
# ==========================================
# 【关键修复三】：更新为实际存在的脚本名称
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103XX_FLASH.ld)

target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3      # <- 替换到这里
    -mthumb              # <- 替换到这里
    -T${LINKER_SCRIPT}
    --specs=nano.specs    
    --specs=nosys.specs   
    -Wl,-Map=${PROJECT_NAME}.map 
    -Wl,--gc-sections     
)

# ==========================================
# 6. Post-Build: Generate .bin and .hex (生成烧录文件)
# ==========================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME} ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}
    COMMENT "Generating .hex, .bin and analyzing size..."
)

# ==========================================
# 7. Flash Target (自定义烧录命令)
# ==========================================
# 添加一个名为 "flash" 的伪目标，调用 openocd 进行烧录
add_custom_target(flash
    COMMAND sudo openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "program ${PROJECT_NAME}.hex verify reset exit"
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flashing target with OpenOCD..."
)