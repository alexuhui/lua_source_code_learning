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

#include "luaaux.h"
#include "../vm/luado.h"

/**
 * 默认的内存分配函数
 * @param ud 一个指向用户数据的指针，通常用于传递给分配函数以保持上下文信息。
 * @param ptr 表示要被重新分配的内存，一个指向已分配内存的指针，如果该参数不为 NULL，则表示需要释放这块内存。
 * @param osize 表示旧内存的大小（也就是ptr所指向的内存块的大小）
 * @param nsize 表示当前要开辟的新内存块大小。
 */
static void* l_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;

    // printf("l_alloc nsize:%ld\n", nsize);
    if (nsize == 0) {
        // nsize 为零表示释放内存
        free(ptr);
        return NULL;
    }

    /**
     * realloc 函数是 C 语言标准库中的一个函数，用于重新分配已分配的内存块
     * void* realloc(void* ptr, size_t size);
     * 
     * void* ptr：指向已分配内存的指针。如果 ptr 为 NULL，则 realloc 相当于调用 malloc 函数分配一块大小为 size 的内存。
     * size_t size：请求分配的新内存大小（以字节为单位）
     */ 
    return realloc(ptr, nsize);
}

/** 
 * 创建Lua虚拟机的函数
 * 创建lua线程实例，设置内存分配函数
 * 返回 lua_State 记录线程上下文信息
*/
struct lua_State* luaL_newstate() {
    // 设置默认的内存分配函数
    struct lua_State* L = lua_newstate(&l_alloc, NULL);
    return L;
}

void luaL_close(struct lua_State* L) {
    lua_close(L);
}

void luaL_pushinteger(struct lua_State* L, int integer) {
    lua_pushinteger(L, integer);
}

void luaL_pushnumber(struct lua_State* L, float number) {
    lua_pushnumber(L, number);
}

void luaL_pushlightuserdata(struct lua_State* L, void* userdata) {
    lua_pushlightuserdata(L, userdata);
}

void luaL_pushnil(struct lua_State* L) {
    lua_pushnil(L);
}

void luaL_pushcfunction(struct lua_State* L, lua_CFunction f) {
    lua_pushcfunction(L, f);
}

void luaL_pushboolean(struct lua_State* L, bool boolean) {
    lua_pushboolean(L, boolean);
}

// function call
typedef struct CallS {
    StkId func;
    int nresult;
} CallS;

static int f_call(lua_State* L, void* ud) {
    CallS* c = cast(CallS*, ud);
    luaD_call(L, c->func, c->nresult);
    return LUA_OK;
}

int luaL_pcall(struct lua_State* L, int narg, int nresult) {
    int status = LUA_OK;
    CallS c;
    c.func = L->top - (narg + 1);
    c.nresult = nresult; 

    status = luaD_pcall(L, &f_call, &c, savestack(L, L->top), 0);
    return status;
}

bool luaL_checkinteger(struct lua_State* L, int idx) {
    int isnum = 0;
    lua_tointegerx(L, idx, &isnum);
    if (isnum) {
        return true;
    }
    else {
        return false;
    }
}

lua_Integer luaL_tointeger(struct lua_State* L, int idx) {
    int isnum = 0;
    lua_Integer ret = lua_tointegerx(L, idx, &isnum);
    return ret;
}

lua_Number luaL_tonumber(struct lua_State* L, int idx) {
    int isnum = 0;
    lua_Number ret = lua_tonumberx(L, idx, &isnum);
    return ret;
}

void* luaL_touserdata(struct lua_State* L, int idx) {
    // TODO
    return NULL;
}

bool luaL_toboolean(struct lua_State* L, int idx) {
    return lua_toboolean(L, idx);
}

int luaL_isnil(struct lua_State* L, int idx) {
    return lua_isnil(L, idx);
}

void luaL_pop(struct lua_State* L) {
    lua_pop(L); 
}

int luaL_stacksize(struct lua_State* L) {
    return lua_gettop(L);
}
