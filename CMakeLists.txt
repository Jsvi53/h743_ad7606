cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME h743_ad7606)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)



# ==================== 后构建处理 ====================
# 生成二进制和HEX文件
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMENT "生成二进制文件和Intel HEX文件"
)

# 显示详细的大小信息
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMAND ${CMAKE_SIZE} --format=sysv $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "显示内存使用详情"
)

# 生成反汇编文件（用于优化分析）
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${CMAKE_PROJECT_NAME}> > ${CMAKE_PROJECT_NAME}.lst
    COMMENT "生成反汇编列表文件"
)

# 拷贝编译命令文件
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${CMAKE_BINARY_DIR}/compile_commands.json 
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "拷贝compile_commands.json到根目录"
)

# ==================== 内存使用检查 ====================
# 检查Flash和RAM使用率（需要自定义脚本）
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== 内存使用分析 ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Flash大小限制: 512KB (STM32F103RET6)"
    COMMAND ${CMAKE_COMMAND} -E echo "RAM大小限制: 64KB"
    COMMENT "内存使用提醒"
)

# ==================== 可选：静态分析集成 ====================
# 可以集成PC-lint、Cppcheck等静态分析工具
option(ENABLE_STATIC_ANALYSIS "启用静态代码分析" OFF)
if(ENABLE_STATIC_ANALYSIS)
    find_program(CPPCHECK_EXECUTABLE cppcheck)
    if(CPPCHECK_EXECUTABLE)
        add_custom_target(cppcheck
            COMMAND ${CPPCHECK_EXECUTABLE}
            --enable=all
            --inconclusive
            --std=c11
            --suppress=missingIncludeSystem
            ${CMAKE_SOURCE_DIR}/Core/Src
            COMMENT "运行Cppcheck静态分析"
        )
    endif()
endif()

# ==================== 可选：单元测试支持 ====================
option(ENABLE_TESTING "启用单元测试" OFF)
if(ENABLE_TESTING)
    enable_testing()
    # 这里可以添加单元测试配置
endif()

# ==================== 开发者工具 ====================
# 创建便于开发的目标
add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program ${CMAKE_PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "烧录程序到STM32"
)

add_custom_target(debug
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
    COMMENT "启动OpenOCD调试服务器"
)

# 显示构建配置信息
message(STATUS "==================== 构建配置 ====================")
message(STATUS "项目名称: ${CMAKE_PROJECT_NAME}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "编译器: ${CMAKE_C_COMPILER}")
message(STATUS "编译标志: ${CMAKE_C_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "发布版编译标志: ${CMAKE_C_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "调试版编译标志: ${CMAKE_C_FLAGS_DEBUG}")
endif()
message(STATUS "链接标志: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "==================================================")