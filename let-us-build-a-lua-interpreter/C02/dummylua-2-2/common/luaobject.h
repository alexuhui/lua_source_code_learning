/* Copyright (c) 2018 Manistein,https://manistein.github.io/blog/  

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.*/

#ifndef luaobject_h
#define luaobject_h 

#include "lua.h"

typedef struct lua_State lua_State;

typedef LUA_INTEGER lua_Integer;
typedef LUA_NUMBER lua_Number;
typedef unsigned char lu_byte;
typedef int (*lua_CFunction)(lua_State* L);
// 定义了一个函数指针类型 lua_Alloc
typedef void* (*lua_Alloc)(void* ud, void* ptr, size_t osize, size_t nsize);

// lua number type 
#define LUA_NUMINT (LUA_TNUMBER | (0 << 4))
#define LUA_NUMFLT (LUA_TNUMBER | (1 << 4))

// lua function type 
#define LUA_TLCL (LUA_TFUNCTION | (0 << 4))
#define LUA_TLCF (LUA_TFUNCTION | (1 << 4))
#define LUA_TCCL (LUA_TFUNCTION | (2 << 4))

// string type 
#define LUA_LNGSTR (LUA_TSTRING | (0 << 4))
#define LUA_SHRSTR (LUA_TSTRING | (1 << 4))

// GCObject
#define CommonHeader struct GCObject* next; lu_byte tt_; lu_byte marked
#define LUA_GCSTEPMUL 200

struct GCObject {
    CommonHeader;
};
/*
C 语言中的 union（联合体）是一种特殊的数据结构，它允许在相同的内存位置存储不同的数据类型。
union 的主要特点是它的大小等于其最大的成员大小，因为它将所有成员共享同一块内存区域。
这意味着，当你给 union 的一个成员赋值时，其他成员的值将被覆盖。
*/
typedef union lua_Value {
    /**
     * 可被垃圾回收的对象，userdata类型对象属于可回收
     */
    struct GCObject* gc;
    /** 
     * light userdata 类型变量
     * full userdata是受GC机制管控的，它就是上一段里所提的userdata类型
     * 而light userdata是需要由使用者自行释放的
    */
    void* p;
    /**
     * bool 变量 0 为false 1 为true
    */
    int b;
    /** 整型 */
    lua_Integer i;
    /** 浮点型 */ 
    lua_Number n;
    /** Light C Function的值，实际上就是存放函数指针 */
    lua_CFunction f;
// Value 是 lua_Value 的别名
} Value;

/*
Lua的基本类型包括：
nil类型、布尔类型、轻量用户数据(light userdata)类型、字符串类型、表类型、函数类型、完全用户数据（full userdata，又称userdata）类型和线程类型

Lua通过一个通用类型来表示所有类型的数据
*/
typedef struct lua_TValue {
    Value value_;
    // 定义在 lua.h 里的基本数据类型，以及上方的 number / function / string type
    int tt_;
    // TValue 是 lua_TValue 的别名
} TValue;

typedef struct TString {
    CommonHeader;
} TString;

#endif 
