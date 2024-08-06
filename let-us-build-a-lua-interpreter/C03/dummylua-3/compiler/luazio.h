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
#ifndef LUA_ZIO
#define LUA_ZIO

#include "../common/lua.h"
#include "../common/luaobject.h"

typedef char* (*lua_Reader)(struct lua_State* L, void* data, size_t* size);

#define MIN_BUFF_SIZE 32 
/**
 * 读取字符
 * 有点像滑动窗口，
 * ** 如果Zio模块还没读完，n > 0, 就把Zio的缓冲指针p后移
 * ** 否则，调用luaZ_fill方法填充Zio模块（读取一定长度的字符串到缓冲区）
 */
#define zget(z) (((z)->n--) > 0 ? (*(z)->p++) : luaZ_fill(z))
#define luaZ_resetbuffer(ls) (ls->buff->n = 0)
#define luaZ_buffersize(ls) (ls->buff->size)

/**
 * 文本读取缓存
 */
typedef struct LoadF {
    FILE* f;
    // 编码数组
    char buff[BUFSIZE]; // read the file stream into buff
	int n;		       // how many char you have read
} LoadF;

/**
 * 1)Zio模块通过外部指定的read函数，从Lua脚本文件中读取BUFSIZE个字符，
 *   并存入LoadF结构的buff中，它的n值记录了读取了多少个字符。
 * 2)Zio的p指针指向LoadF的buff数组，并且将LoadF中的n赋值给zio->n。
 * 3)将∗p赋值给lexstate->current，接着zio->p++，然后zio->n--。
 *   当zio->n小于等于0时，在下一次调用luaX_next函数时，Zio模块会重新从文件中读取新的BUFSIZE个字符，重置LoadF结构的buff数组、n、zio->p和zio->n。
 */
typedef struct Zio {
	lua_Reader reader;		// read buffer to p
	int n;					// the number of unused bytes
	char* p;				// the pointer to buffer
	void* data;				// structure which holds FILE handler
	struct lua_State* L;
} Zio;

/**
 * 初始化Zio结构体
 */
void luaZ_init(struct lua_State* L, Zio* zio, lua_Reader reader, void* data);

// if fill success, then it will return next character in ASCII table, or it will return -1
int luaZ_fill(Zio* z);	
#endif