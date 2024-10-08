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

#ifndef luagc_h
#define luagc_h

#include "../common/luastate.h"

// GCState
#define GCSpause        0
#define GCSpropagate    1
#define GCSatomic       2
#define GCSinsideatomic 3
#define GCSsweepallgc   4
#define GCSsweepend     5

// Color
#define WHITE0BIT       0
#define WHITE1BIT       1
#define BLACKBIT        2

// Bit operation
#define bitmask(b) (1<<b)
#define bit2mask(b1, b2) (bitmask(b1) | bitmask(b2))

#define resetbits(x, m) ((x) &= cast(lu_byte, ~(m)))
#define setbits(x, m) ((x) |= (m))
#define testbits(x, m) ((x) & (m))
#define resetbit(x, b) resetbits(x, bitmask(b)) 
#define l_setbit(x, b) setbits(x, bitmask(b))
#define testbit(x, b) testbits(x, bitmask(b))

#define WHITEBITS bit2mask(WHITE0BIT, WHITE1BIT)
#define luaC_white(g) (g->currentwhite & WHITEBITS)
#define otherwhite(g) (g->currentwhite ^ WHITEBITS)

#define iswhite(o) testbits((o)->marked, WHITEBITS)
#define isgray(o)  (!testbits((o)->marked, bitmask(BLACKBIT) | WHITEBITS))
#define isblack(o) testbit((o)->marked, bitmask(BLACKBIT))
#define isdeadm(ow, m) (!((m ^ WHITEBITS) & (ow)))

/**
 * 将对象转换成 GCUnion 类型，并返回 GCUnion 的 gc 字段（GCObject 类型）
 */
#define obj2gco(o) (&cast(union GCUnion*, o)->gc)
/**
 * 将对象转换成 GCUnion 类型，并返回 GCUnion 的 th 字段（lua_State 类型）
 */
#define gco2th(o)  check_exp((o)->tt_ == LUA_TTHREAD, &cast(union GCUnion*, o)->th)
#define gcvalue(o) ((o)->value_.gc)

#define iscollectable(o) \
    ((o)->tt_ == LUA_TTHREAD || \
	 (o)->tt_ == LUA_SHRSTR  || \
	 (o)->tt_ == LUA_LNGSTR  || \
	 (o)->tt_ == LUA_TTABLE  || \
	 (o)->tt_ == LUA_TLCL    || \
	 (o)->tt_ == LUA_TCCL)

/**
 * 标记GCobject
 */
#define markobject(L, o) if (iswhite(o)) { reallymarkobject(L, obj2gco(o)); }
/**
 * 标记可回收变量
 */
#define markvalue(L, o)  if (iscollectable(o) && iswhite(gcvalue(o))) { reallymarkobject(L, gcvalue(o)); }
/**
 * *** 这里还没看懂
 * 从调用情况看，字面意思是：
 * 先把  global_State 的 gray（灰色标记列表） 赋值给 lua_State 的 gclist 
 * gco 转换成 GCUnion 类型后的 th（lua_State）值，赋值给 global_State 的 gray（灰色标记列表） 
 */
#define linkgclist(gco, prev) { (gco)->gclist = prev; prev = obj2gco(gco); }

// try trigger gc
#define luaC_condgc(pre, L, pos) if (G(L)->GCdebt > 0) { pre; luaC_step(L); pos; } 
#define luaC_checkgc(L) luaC_condgc((void)0, L, (void)0)

struct GCObject* luaC_newobj(struct lua_State* L, int tt_, size_t size);
void luaC_step(struct lua_State* L);
void reallymarkobject(struct lua_State* L, struct GCObject* gc);
void luaC_freeallobjects(struct lua_State* L);

#endif 
