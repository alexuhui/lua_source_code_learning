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

#ifndef luastate_h
#define luastate_h 

#include "luaobject.h"

#define LUA_EXTRASPACE sizeof(void*) 
#define G(L) ((L)->l_G)

#define STEPMULADJ 200
#define GCSTEPMUL 200 
#define GCSTEPSIZE 1024  //1kb
#define GCPAUSE 100

// size for string cache
#define STRCACHE_M 53
#define STRCACHE_N 2

typedef TValue* StkId;

/**
 * 虚拟机中执行函数的基础
 */
struct CallInfo {
    /**
     * func成员变量指明了函数位于stack中的位置
     */
    StkId func;
    /**
     * top成员变量则指明被调用函数的栈顶位置。
     */
    StkId top;
    /**
     * nresult成员变量指明了被调用函数一共要返回多少个值
     */
    int nresult;
    /**
     * callstatus成员变量则指明了函数调用的状态，
     * 最常用的状态则是CIST_LUA，表明当前调用是一个Lua函数(Lua closure)。
     */
    int callstatus;
    /**
     * 该函数调用的函数的 CallInfo 指针
     */
    struct CallInfo* next;
    /**
     * 调用该函数的函数的 CallInfo 指针
     */
    struct CallInfo* previous;
};

/**
 *  虚拟机的线程结构
 */
typedef struct lua_State {
    CommonHeader;       // gc header, all gcobject should have the commonheader
    /**
     * 栈起始指针, 
     * TValue 数组，用来暂存虚拟机运行过程中的临时变量
     * StkId是TValue∗的一个别名
     */
    StkId stack;
    /**
     * 对应 LX 的extra部分？
     * 从这里开始，栈不能被使用
     */
    StkId stack_last;
    /**
     * 栈顶指针，调用函数时动态改变
     * top变量表示当前被调用的函数，它在虚拟机栈的栈顶位置（栈底是被调用函数所在位置的下一个位置）。
     */
    StkId top;
    // 栈（数组）大小
    int stack_size;
    /**
     * 保护模式中
     * 当抛出异常时，跳出逻辑
     */
    struct lua_longjmp* errorjmp;
    // 状态
    int status;
    struct lua_State* previous;
    /**
     * 和lua_State生命周期一致的函数调用信息
     * base_ci的func指针指向了栈的首个位置
     * 可以理解成函数调用栈的栈底（整个调用过程可以看做链式栈，base_ci就是链头）
     */
    struct CallInfo base_ci;
    /**
     * 当前运行的CallInfo
     * Lua虚拟机主线程实例化时的ci指针是指向base_ci的。
     */
    struct CallInfo* ci;
    int nci;
    // 指向全局状态
    struct global_State* l_G;
    // 错误函数位于栈的位置
    ptrdiff_t errorfunc;
    // 进行函数调用的次数
    int ncalls;
    struct GCObject* gclist;
} lua_State;

// only for short string
struct stringtable {
    struct TString** hash;
    unsigned int nuse;
    unsigned int size;
};
/**
 *  虚拟机全局状态
 */
typedef struct global_State {
    /**
     * Lua虚拟机“线程”类型的指针。
     * 实际上，是将LX结构中的lua_State类型成员l的地址赋值给它。
     * 也就是说，Lua虚拟机“主线程”实际上就是LX结构中的成员变量l。
     */
    struct lua_State* mainthread;
    /**
     * 内存分配/回收函数。
     */
    lua_Alloc frealloc;
    // 暂时未使用
    void* ud; 
    /**
     * 当Lua虚拟机抛出异常时，如果当前不处于保护模式，那么会直接调用panic函数。
     * 调用panic函数进行异常处理，通常是输出一些关键日志和伴随进程退出。
     */
    lua_CFunction panic;

    struct stringtable strt;
    TString* strcache[STRCACHE_M][STRCACHE_N];
    unsigned int seed;              // hash seed, just for string hash
    TString* memerrmsg;
    //gc fields
    /**
     * 1)gcstate：开始新一轮GC之后，当前GC所处的阶段（pause、propagate、atomic、sweep等）。
     */
    lu_byte gcstate;
    /**
     * 2)gcrunning：表示GC是否在进行。0为不允许，非0为允许。
     */

    /**
     * 3)currentwhite：当前新创建的GCObject的颜色。atomic阶段之前是white，atomic阶段之后会被设置为otherwhite。sweep阶段只清理颜色为white的GCObject。
     */
    lu_byte currentwhite;
    /**
     * 4)allgc：所有新创建的GCObject都会被放入allgc链表中。
     * 在sweep阶段会遍历allgc链表，将标记为白色的GCObject释放掉，将标记为黑色的GCObject重新设置为下一轮GC要被清理的白色。
     */
    struct GCObject* allgc;         // gc root set
    /**
     * 5)sweepgc：记录当前要被清理或重新标记的GCObject。
     */
    struct GCObject** sweepgc;
    /**
     * 6)gray：被标记为灰色的GCObject会被放入gray链表。
     */
    struct GCObject* gray;
    /**
     * 7)grayagain：被标记为黑色的GCObject（引用了在传播过程中新创建的对象后）重新被设置为灰色时，要放入grayagain链表中，Lua表通常会有这种操作。
     */
    struct GCObject* grayagain;
    struct GCObject* fixgc;         // objects can not collect by gc
    /**
     * 12)totalbytes：虚拟机预设内存总大小。
     */
    lu_mem totalbytes;
    /**
     * 13)GCdebt：GC的“借贷”值。预支的内存。每次预支，GCdebt 的值就减去预支内存大小
     * GCdebt 的值通常是个负值，创建新的GCObject时，GCdebt的值会加上新分配的内存大小
     * 当这个值大于0时就会触发GC，回收内存
     * *** Lua虚拟机真实的内存大小是totalbytes+GCdebt的结果。
     */
    l_mem GCdebt;                   // GCdebt will be negative
    /**
     * 一次GC操作可能会调用若干次dummylua项目中GC模块的single_step函数。
     * 每次执行single_step函数，处理的内存总量会被记录在GCmemtrav变量中。
     * 每当single_step函数执行完时，它会返回，然后被置0。
     * 返回值会被累加到临时变量中，当临时变量达到一定值的时候，会退出GC步骤操作。
     */
    lu_mem GCmemtrav;               // per gc step traverse memory bytes 
    /**
     * 15)GCestimate：一轮GC结束之后，Lua虚拟机的实际内存大小会被赋值到这个变量中。
     * 在sweep阶段结束后，totalbytes会默认被设置成GCestimate的两倍，并且GCdebt的值为-GCestimate的值
     */
    lu_mem GCestimate;              // after finish a gc cycle,it records total memory bytes (totalbytes + GCdebt)
    /**
     * 17)GCstepmul：一轮GC开始之后，用于调节GC步骤触发间隔以及单次GC步骤处理GCObject的数量用的。
     * 值越大间隔越小，单次GC步骤处理的GCObject越多，GC越积极
     */
    int GCstepmul;
} global_State;

// GCUnion
union GCUnion {
    struct GCObject gc;
    lua_State th;
    TString ts;
};

struct lua_State* lua_newstate(lua_Alloc alloc, void* ud);
void lua_close(struct lua_State* L);

void setivalue(StkId target, int integer);
void setfvalue(StkId target, lua_CFunction f);
void setfltvalue(StkId target, float number);
void setbvalue(StkId target, bool b);
void setnilvalue(StkId target);
void setpvalue(StkId target, void* p);
void setgco(StkId target, struct GCObject* gco);

void setobj(StkId target, StkId value);

void increase_top(struct lua_State* L);
void lua_pushcfunction(struct lua_State* L, lua_CFunction f);
void lua_pushinteger(struct lua_State* L, int integer);
void lua_pushnumber(struct lua_State* L, float number);
void lua_pushboolean(struct lua_State* L, bool b);
void lua_pushnil(struct lua_State* L);
void lua_pushlightuserdata(struct lua_State* L, void* p);
void lua_pushstring(struct lua_State* L, const char* str);

lua_Integer lua_tointegerx(struct lua_State* L, int idx, int* isnum);
lua_Number lua_tonumberx(struct lua_State* L, int idx, int* isnum);
bool lua_toboolean(struct lua_State* L, int idx);
int lua_isnil(struct lua_State* L, int idx);
char* lua_tostring(struct lua_State* L, int idx);

void lua_settop(struct lua_State* L, int idx);
int lua_gettop(struct lua_State* L);
void lua_pop(struct lua_State* L);
TValue* index2addr(struct lua_State* L, int idx);

#endif 
