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

#include "../clib/luaaux.h"
#include "luabase.h"

const lua_Reg reg[] = {
	{ "_G", luaB_openbase },
	{ NULL, NULL },
};

/**
 * 这个函数只需要将虚拟机实例指针传入即可。
 * 它的主要工作就是将基础函数注册到全局表_G中，比如常用的print函数等。
 */
void luaL_openlibs(struct lua_State* L) {
	for (int i = 0; i < sizeof(reg) / sizeof(reg[0]); i++) {
		lua_Reg r = reg[i];
		if (r.name != NULL && r.func != NULL) {
			luaL_requiref(L, r.name, r.func, 1);
		}
	}
}