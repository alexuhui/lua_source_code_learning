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

#ifndef lua_parser_h
#define lua_parser_h

#include "../common/luaobject.h"
#include "../compiler/luazio.h"

#define UNOPR_PRIORITY 12

/**
 * 单目运算符
 */
typedef enum UnOpr {
	UNOPR_MINUS,// 负数
	UNOPR_LEN, // 长度计算 #
	UNOPR_BNOT, // 二进制 按位非
	UNOPR_NOT,  // 逻辑非运算

	NOUNOPR,  // ??
} UnOpr;

/**
 * 双目运算符
 */
typedef enum BinOpr {
	BINOPR_ADD = 0,
	BINOPR_SUB,
	BINOPR_MUL,
	BINOPR_DIV,
	BINOPR_IDIV,
	BINOPR_MOD,
	BINOPR_POW,
	BINOPR_BAND,
	BINOPR_BOR,
	BINOPR_BXOR,
	BINOPR_SHL,
	BINOPR_SHR,
	BINOPR_CONCAT,
	BINOPR_GREATER,
	BINOPR_LESS,
	BINOPR_EQ,
	BINOPR_GREATEQ,
	BINOPR_LESSEQ,
	BINOPR_NOTEQ,
	BINOPR_AND,
	BINOPR_OR,

	NOBINOPR,
} BinOpr;
/**
 * 表达式类型
 */
typedef enum expkind {
	VVOID,			// expression is void
	VNIL,			// expression is nil value
	VFLT,			// expression is float value
	VINT,			// expression is integer value
	VTRUE,			// expression is true value
	VFALSE,			// expression is false value
	VCALL,			// expression is a function call, info field of struct expdesc is represent instruction pc

	VLOCAL,			// expression is a local value, info field of struct expdesc is represent the pos of the stack
	VUPVAL,			// expression is a upvalue, ind is in use
	VINDEXED,		// ind field of struct expdesc is in use

	VK,				// expression is a constant, info field of struct expdesc is represent the index of k
	VJMP,
	VRELOCATE,		// expression can put result in any register, info field represents the instruction pc
	VNONRELOC,		// expression has result in a register, info field represents the pos of the stack
} expkind;

/**
 * 表达式信息
 * expdesc是非常重要的数据结构，用来临时存储表达式的重要变量。
 * 而在编译的过程中，往往又需要复用这个结构，以节约内存和提升效率
 */
typedef struct expdesc {
	expkind k;				// expkind
	union {
		int info;			
		lua_Integer i;		// for VINT
		lua_Number r;		// for VFLT

		struct {
			int t;		// the index of the upvalue or table
			int vt;		// whether 't' is a upvalue(VUPVAL) or table(VLOCAL)
			int idx;	// index (R/K)
		} ind;
	} u;
	int t;				// patch list of 'exit when true'
	int f;				// patch list of 'exit when false'
} expdesc;

// Token cache
typedef struct MBuffer {
	char* buffer;
	int n;
	int size;
} MBuffer;

typedef struct Labeldesc {
	struct TString* varname;
	int pc;
	int line;
} Labeldesc;

typedef struct Labellist {
	struct Labeldesc* arr;
	int n;
	int size;
} Labellist;
/**
 * 动态数据
 */
typedef struct Dyndata {
	struct {
		short* arr;
		int n;
		int size;
	} actvar;
	Labellist labellist;
} Dyndata;

typedef struct BlockCnt {
	struct BlockCnt* previous;
	lu_byte nactvar;
	int is_loop;
	int firstlabel;
} BlockCnt;

typedef struct FuncState {
	int firstlocal;
	struct FuncState* prev;
	struct LexState* ls;
	struct BlockCnt* bl;
	Proto* p;
	int pc;				// next code array index to save instruction
	int jpc;			// the position of the first test instruction
	int last_target;
	int nk;				// next constant array index to save const value
	int nups;			// next upvalues array index to save upval
	int nlocvars;		// the number of local values
	int nactvars;		// the number of activate values
	int np;				// the number of protos
	int freereg;		// free register index
} FuncState;

/**
 * 前缀表达式结构，类似链表
 */
typedef struct LH_assign {
	struct LH_assign* prev;
	expdesc	   v;
} LH_assign;

LClosure* luaY_parser(struct lua_State* L, Zio* zio, MBuffer* buffer, Dyndata* dyd, const char* name);
#endif