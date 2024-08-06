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
#include "../common/luastring.h"
#include "../common/luatable.h"
#include "../vm/luafunc.h"

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

// this function will load c modules into _LOADED table, if glb is true, then the module will also 
// add to _G
int luaL_requiref(struct lua_State* L, const char* name, lua_CFunction func, int glb) {
	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED);

	TValue* top = luaL_index2addr(L, lua_gettop(L));
	if (novariant(top) == LUA_TNIL) {
		luaL_pop(L);
		luaL_createtable(L); // _LOADED = {}
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, LUA_LOADED); // set _LOADED to registry
	}

	luaL_getsubtable(L, -1, name);
	if (novariant(L->top - 1) == LUA_TNIL) {
		luaL_pop(L);

		luaL_pushcfunction(L, func);
		luaL_pushstring(L, name);
		luaL_pcall(L, 1, 1);

		lua_pushvalue(L, -1);
		lua_setfield(L, -3, name);
	}

	lua_remove(L, -2); // remove _LOADED

	if (glb) {
		lua_pushglobaltable(L);  // push _G into stack top

		TValue* o = index2addr(L, lua_gettop(L));
		lua_insert(L, -2, o);
		lua_pop(L);

		lua_pushstring(L, name);
		o = index2addr(L, lua_gettop(L));
		lua_insert(L, -2, o);
		lua_pop(L);

		luaL_settable(L, -3);
	}
	lua_pop(L); // pop _G

	return LUA_OK;
}

void luaL_pushinteger(struct lua_State* L, lua_Integer integer) {
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

void luaL_pushstring(struct lua_State* L, const char* str) {
    lua_pushstring(L, str); 
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

char* luaL_tostring(struct lua_State* L, int idx) {
    return lua_tostring(L, idx);
}

TValue* luaL_index2addr(struct lua_State* L, int idx) {
    return index2addr(L, idx);
}

/** 创建table */
int luaL_createtable(struct lua_State* L) {
    return lua_createtable(L);
}

/**
 * 将栈顶部两个值（对应key, value） 设置到table
 * @param L lua_State lua线程
 * @param idx table栈地址
 */
int luaL_settable(struct lua_State* L, int idx) {
    return lua_settable(L, idx);
}

int luaL_gettable(struct lua_State* L, int idx) {
    return lua_gettable(L, idx);  
}

int luaL_getglobal(struct lua_State* L) {
    return lua_pushglobaltable(L);
}

int luaL_getsubtable(struct lua_State* L, int idx, const char* name) {
	TValue* o = luaL_index2addr(L, idx);
	if (novariant(o) != LUA_TTABLE) {
		return LUA_ERRERR;
	}

	struct Table* table = gco2tbl(gcvalue(o));
	TValue* subtable = (TValue*)luaH_getstr(L, table, luaS_newliteral(L, name));
	if (subtable == luaO_nilobject) {
		lua_pushnil(L);
		return LUA_OK;
	}

	setobj(L->top, subtable);
	increase_top(L);

	return LUA_OK;
}

void luaL_pop(struct lua_State* L) {
    lua_pop(L); 
}

int luaL_stacksize(struct lua_State* L) {
    return lua_gettop(L);
}

/**
 * 读取文本内容 （分段，一次读BUFSIZE大小）
 * @param L lua_State
 * @param data 文件内容
 * @param sz 读取内容大小
 */
static char* getF(struct lua_State* L, void* data, size_t* sz) {
	LoadF* lf = (LoadF*)data;
	if (lf->n > 0) {
		*sz = lf->n;
		lf->n = 0;
	}
	else {
		/**
		* fread 函数是 C 语言标准库 <stdio.h> 中的一个函数，用于从文件中读取数据。
		* 其功能是从给定的文件流中读取指定数量的数据项，每个数据项的大小由指定的字节数决定。
		*/
		*sz = fread(lf->buff, sizeof(char), BUFSIZE, lf->f);
		lf->n = 0;
	}

	return lf->buff;
}

static void init_upval(struct lua_State* L) {
	StkId top = L->top - 1;
	LClosure* cl = gco2lclosure(gcvalue(top));
	if (cl->nupvalues > 0) {
		struct Table* t = gco2tbl(gcvalue(&G(L)->l_registry));
		TValue* _G = &t->array[LUA_GLOBALTBLIDX];
		setobj(cl->upvals[0]->v, _G);
	}
}

/**
 * 加载lua脚本
 * @param L lua_State
 * @param filename 脚本路径
 */
int luaL_loadfile(struct lua_State* L, const char* filename) {
	FILE* fptr = NULL;
	l_fopen(&fptr, filename, "rb");
	if (fptr == NULL)
	{
		printf("can not open file %s\n", filename);
		return LUA_ERRERR;
	}

	// init LoadF
	LoadF lf;
	lf.f = fptr;
	lf.n = 0;
	memset(lf.buff, 0, BUFSIZE);

	int ok = luaD_load(L, getF, &lf, filename);
	if (ok == LUA_OK) {
		init_upval(L);
	}
	fclose(fptr);

	return ok;
}
