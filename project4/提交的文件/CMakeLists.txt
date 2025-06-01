cmake_minimum_required(VERSION 3.10)
project(OptimalImageLib VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "OpenMP found: ${OpenMP_CXX_FLAGS}")
else()
    message(STATUS "OpenMP not found")
endif()

add_compile_options(-g)
# 启用SIMD支持
if(MSVC)
    add_compile_options(/W4 /arch:AVX2)
    message(STATUS "MSVC compiler detected, enabling AVX2")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -std=c++17)
    # 检查是否支持不同的SIMD指令集
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-mavx2 COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag(-mavx COMPILER_SUPPORTS_AVX)
    check_cxx_compiler_flag(-msse4.2 COMPILER_SUPPORTS_SSE42)
    
    if(COMPILER_SUPPORTS_AVX2)
        add_compile_options(-mavx2)
        message(STATUS "AVX2 support enabled")
    elseif(COMPILER_SUPPORTS_AVX)
        add_compile_options(-mavx)
        message(STATUS "AVX support enabled")
    elseif(COMPILER_SUPPORTS_SSE42)
        add_compile_options(-msse4.2)
        message(STATUS "SSE4.2 support enabled")
    endif()
endif()

# 设置库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 添加库源文件
set(LIBRARY_SOURCES
    optimal_image.cpp
    optimal_image_ops.cpp
)

# 创建动态库
add_library(optimal_image SHARED ${LIBRARY_SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(optimal_image PUBLIC OpenMP::OpenMP_CXX)
endif()

# 设置库属性
set_target_properties(optimal_image PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER optimal_image.h)

# 安装库和头文件
install(TARGETS optimal_image
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)

# 添加demo可执行文件
add_executable(demo demo.cpp)
target_link_libraries(demo optimal_image)

# 添加性能测试可执行文件
add_executable(performancetest performancetest.cpp)
target_link_libraries(performancetest optimal_image)

# 添加测试
enable_testing()
add_test(NAME demo_test COMMAND demo) 