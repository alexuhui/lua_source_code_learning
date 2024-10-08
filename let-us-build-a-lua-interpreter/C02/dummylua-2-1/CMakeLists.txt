cmake_minimum_required(VERSION 3.0)

# set the project name
project(dummylua)

set(COMMON_SRC common/luamem.c common/luaobject.c common/luastate.c)
set(CLIB_SRC clib/luaaux.c)
set(VM_SRC vm/luado.c)
set(TEST_SRC test/p1_test.c test/p1_my_test.c)
set(SRC ${COMMON_SRC} ${CLIB_SRC} ${VM_SRC} ${TEST_SRC})

# add the executable
add_executable(dummylua main.c ${SRC})

IF (WIN32)
	target_compile_definitions(dummylua PRIVATE _WINDOWS_PLATFORM_=1)
ENDIF()
# 添加 /utf-8 编译器选项
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

target_include_directories(dummylua PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}/common"
                          "${CMAKE_CURRENT_SOURCE_DIR}/clib"
                          "${CMAKE_CURRENT_SOURCE_DIR}/compiler"
                          "${CMAKE_CURRENT_SOURCE_DIR}/vm"
                          "${CMAKE_CURRENT_SOURCE_DIR}/test"
                          )