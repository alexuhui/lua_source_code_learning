cmake_minimum_required(VERSION 3.0)

# set the project name
project(lua_5_3_5)

# 添加 /utf-8 编译器选项
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

# 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
# 搜索 src 目录下的源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)

# 排除luac.c
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c)

# add the executable
add_executable(lua ${SOURCES})
