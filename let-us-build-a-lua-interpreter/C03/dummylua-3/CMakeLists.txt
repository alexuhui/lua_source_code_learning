cmake_minimum_required(VERSION 3.0)

# set the project name
project(dummylua)

set(COMMON_SRC common/luabase.c common/luainit.c common/luamem.c common/luaobject.c
	common/luastate.c common/luastring.c common/luatable.c)
set(CLIB_SRC clib/luaaux.c)
set(VM_SRC vm/luado.c vm/luagc.c vm/luavm.c vm/luafunc.c vm/luaopcodes.c)
set(COMPILER_SRC compiler/luazio.c compiler/lualexer.c compiler/luaparser.c compiler/luacode.c)
set(TEST_SRC test/p1_test.c test/p2_test.c test/p3_test.c test/p4_test.c test/p5_test.c)
set(SRC ${COMMON_SRC} ${CLIB_SRC} ${VM_SRC} ${TEST_SRC} ${COMPILER_SRC})

# add the executable
add_executable(dummylua main.c ${SRC})

IF(NOT WIN32)
	target_link_libraries(dummylua m)
ENDIF()

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