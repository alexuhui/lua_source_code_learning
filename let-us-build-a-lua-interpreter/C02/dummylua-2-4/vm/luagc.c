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

#include "luagc.h"
#include "../common/luamem.h"
#include "../common/luastring.h"
#include "../common/luatable.h"

#define GCMAXSWEEPGCO 25

#define gettotalbytes(g) (g->totalbytes + g->GCdebt)
#define white2gray(o) resetbits((o)->marked, WHITEBITS)
/** 灰色标记为黑色 */
#define gray2black(o) l_setbit((o)->marked, BLACKBIT)
/** 黑色标记为灰色 */
#define black2gray(o) resetbit((o)->marked, BLACKBIT)
#define sweepwholelist(L, list) sweeplist(L, list, MAX_LUMEM)

struct GCObject* luaC_newobj(struct lua_State* L, lu_byte tt_, size_t size) {
/**
 * 分配可GC对象
 */
    struct global_State* g = G(L);
    struct GCObject* obj = (struct GCObject*)luaM_realloc(L, NULL, 0, size);
    /**
     * 这里经过一系列的运算的精妙之处的哪里？
     * 似乎本质就是：obj->marked = g->currentwhite 啊
     */
    obj->marked = luaC_white(g);
    /**
     * 把 allgc 链表当前头部设置到 obj下个节点
     * 结合 g->allgc = obj; 把obj插入到链表头部
     */
    obj->next = g->allgc;
    /**
     * 设置 obj 类型
     */
    obj->tt_ = tt_;
    g->allgc = obj;

    return obj;
}

/**
 * 标记对象
 */
void reallymarkobject(struct lua_State* L, struct GCObject* gco) {
    struct global_State* g = G(L);
    /**
     * 白色标记成灰色
     * 怎么没有看到是否被引用的判断？
     */
    white2gray(gco);

    switch(gco->tt_) {
        case LUA_TTHREAD:{
            linkgclist(gco2th(gco), g->gray);            
        } break;
        case LUA_TTABLE:{
            linkgclist(gco2tbl(gco), g->gray);
        } break;
        case LUA_SHRSTR:{ 
            gray2black(gco);
            struct TString* ts = gco2ts(gco);
            g->GCmemtrav += sizelstring(ts->shrlen);
        } break;
        case LUA_LNGSTR: {
            gray2black(gco);
            struct TString* ts = gco2ts(gco);
            g->GCmemtrav += sizelstring(ts->u.lnglen);
        } break;
        default:break;
    }
}

/**
 * 获取需要“借贷”（预支）的内存
 */
static l_mem get_debt(struct lua_State* L) {
    struct global_State* g = G(L);
    int stepmul = g->GCstepmul; 
    l_mem debt = g->GCdebt;
    /**
     * 为负数表示之前预支的内存尚未用完
     */
    if (debt <= 0) {
        return 0;
    }

    debt = debt / STEPMULADJ + 1;
    debt = debt >= (MAX_LMEM / STEPMULADJ) ? MAX_LMEM : debt * g->GCstepmul;

    return debt; 
}

// mark root
static void restart_collection(struct lua_State* L) {
    struct global_State* g = G(L);
    g->gray = g->grayagain = NULL;
    /**
     * 标记主线程（白变灰再变黑）
     */
    markobject(L, g->mainthread); 
    markvalue(L, &g->l_registry);
}

static lu_mem traverse_thread(struct lua_State* L, struct lua_State* th) {
    TValue* o = th->stack;
    for (; o < th->top; o ++) {
        markvalue(L, o);
    }

    return sizeof(struct lua_State) + sizeof(TValue) * th->stack_size + sizeof(struct CallInfo) * th->nci;
}

static lu_mem traverse_strong_table(struct lua_State* L, struct Table* t) {
    for (unsigned int i = 0; i < t->arraysize; i++) {
        markvalue(L, &t->array[i]); 
    }

    for (int i = 0; i < twoto(t->lsizenode); i++) {
        Node* n = getnode(t, i);
        if (ttisnil(getval(n))) {
            TValue* deadkey = getwkey(n);
            deadkey->tt_ = LUA_TDEADKEY;
        }
        else {
            markvalue(L, getwkey(n));
            markvalue(L, getval(n));
        }
    }

    return sizeof(struct Table) + sizeof(TValue) * t->arraysize + sizeof(Node) * twoto(t->lsizenode);
}
/**
 * 标记传播
 */
static void propagatemark(struct lua_State* L) {
    struct global_State* g = G(L);
    if (!g->gray) {
        return;
    }
    struct GCObject* gco = g->gray;
    gray2black(gco);
    lu_mem size = 0;

    switch(gco->tt_) {
        case LUA_TTHREAD:{
            black2gray(gco);
            struct lua_State* th = gco2th(gco);
            g->gray = th->gclist;
            linkgclist(th, g->grayagain);
            size = traverse_thread(L, th);
        } break;
        case LUA_TTABLE:{
            struct Table* t = gco2tbl(gco);
            g->gray = t->gclist;
            size = traverse_strong_table(L, t);
        } break;
        default:break;
    }

    g->GCmemtrav += size;
}

static void propagateall(struct lua_State* L) {
    struct global_State* g = G(L);
    while(g->gray) {
        propagatemark(L);
    }
}

static void atomic(struct lua_State* L) {
    struct global_State* g = G(L);
    g->gray = g->grayagain;
    g->grayagain = NULL;

    g->gcstate = GCSinsideatomic;
    propagateall(L);
    g->currentwhite = cast(lu_byte, otherwhite(g));

    luaS_clearcache(L);
}

static lu_mem freeobj(struct lua_State* L, struct GCObject* gco) {
    switch(gco->tt_) {
        case LUA_SHRSTR: {
            struct TString* ts = gco2ts(gco);
            luaS_remove(L, ts);
            lu_mem sz = sizelstring(ts->shrlen);
            luaM_free(L, ts, sz); 
            return sz; 
        } break;
        case LUA_LNGSTR: {
            struct TString* ts = gco2ts(gco);
            lu_mem sz = sizelstring(ts->u.lnglen);
            luaM_free(L, ts, sz);
        } break;
        case LUA_TTABLE: {
            struct Table* tbl = gco2tbl(gco);
            lu_mem sz = sizeof(struct Table) + tbl->arraysize * sizeof(TValue) + twoto(tbl->lsizenode) * sizeof(Node);
            luaH_free(L, tbl);
            return sz;
        } break;
        default:{
            lua_assert(0);
        } break;
    }
    return 0;
}

/**
 * 清理列表
 */
static struct GCObject** sweeplist(struct lua_State* L, struct GCObject** p, size_t count) {
    struct global_State* g = G(L);
    lu_byte ow = otherwhite(g);
    while (*p != NULL && count > 0) {
        lu_byte marked = (*p)->marked;
        if (isdeadm(ow, marked)) {
            struct GCObject* gco = *p;
            *p = (*p)->next;
            g->GCmemtrav += freeobj(L, gco);
        } 
        else {
            (*p)->marked &= cast(lu_byte, ~(bitmask(BLACKBIT) | WHITEBITS));
            (*p)->marked |= luaC_white(g);
            p = &((*p)->next);
        }
        count --; 
    }
    return (*p) == NULL ? NULL : p; 
}

static void entersweep(struct lua_State* L) {
    struct global_State* g = G(L);
    g->gcstate = GCSsweepallgc; 
    g->sweepgc = sweeplist(L, &g->allgc, 1);
}

static void sweepstep(struct lua_State* L) {
    struct global_State* g = G(L);
    if (g->sweepgc) {
        g->sweepgc = sweeplist(L, g->sweepgc, GCMAXSWEEPGCO);
        g->GCestimate = gettotalbytes(g);

        if (g->sweepgc) {
            return;
        }
    }
    g->gcstate = GCSsweepend;
    g->sweepgc = NULL;
}

static lu_mem singlestep(struct lua_State* L) {
    struct global_State* g = G(L);
    switch(g->gcstate) {
        case GCSpause: {
            g->GCmemtrav = 0;
            // 重置gc，标记主线程
            restart_collection(L);
            g->gcstate = GCSpropagate;
            return g->GCmemtrav;
        } break;
        case GCSpropagate:{
            g->GCmemtrav = 0;
            propagatemark(L);
            if (g->gray == NULL) {
                g->gcstate = GCSatomic;
            }
            return g->GCmemtrav;
        } break;
        case GCSatomic:{
            g->GCmemtrav = 0;
            if (g->gray) {
                propagateall(L);
            }
            atomic(L);
            entersweep(L);
            g->GCestimate = gettotalbytes(g);
            return g->GCmemtrav;
        } break;
        case GCSsweepallgc: {
            g->GCmemtrav = 0;
            sweepstep(L);
            return g->GCmemtrav;
        } break;
        case GCSsweepend: {
            g->GCmemtrav = 0;
            g->gcstate = GCSpause;
            return 0;
        } break;
        default:break;
    }

    return g->GCmemtrav;
}

static void setdebt(struct lua_State* L, l_mem debt) {
    struct global_State* g = G(L);
    lu_mem totalbytes = gettotalbytes(g);

    g->totalbytes = totalbytes - debt;
    g->GCdebt = debt;
}

// when memory is twice bigger than current estimate, it will trigger gc
// again
static void setpause(struct lua_State* L) {
    struct global_State* g = G(L);
    l_mem estimate = g->GCestimate / GCPAUSE;
    estimate = (estimate * g->GCstepmul) >= MAX_LMEM ? MAX_LMEM : estimate * g->GCstepmul;
    
    l_mem debt = g->GCestimate - estimate;
    setdebt(L, debt);
}

/**
 * 推进GC操作和状态变更
 */
void luaC_step(struct lua_State*L) {
    struct global_State* g = G(L);
    l_mem debt = get_debt(L);
    do {
        l_mem work = singlestep(L);
        debt -= work;
    }while(debt > -GCSTEPSIZE && G(L)->gcstate != GCSpause);
    
    if (G(L)->gcstate == GCSpause) {
        setpause(L);
    }
    else {
        debt = debt / g->GCstepmul * STEPMULADJ;
        setdebt(L, debt);
    }
}

void luaC_fix(struct lua_State* L, struct GCObject* o) {
    struct global_State* g = G(L);
    lua_assert(g->allgc == o);

    g->allgc = g->allgc->next;
    o->next = g->fixgc;
    g->fixgc = o;
    white2gray(o);
}

/**
 * 置灰
 * table 中 插入白色gc对象，把 table 重新设置为灰色
 */
void luaC_barrierback_(struct lua_State* L, struct Table* t, const TValue* o) {
    struct global_State* g = G(L);
    lua_assert(isblack(t) && iswhite(o));
    black2gray(t);
    linkgclist(t, g->grayagain);
}


void luaC_freeallobjects(struct lua_State* L) {
    struct global_State* g = G(L);
    g->currentwhite = WHITEBITS; // all gc objects must reclaim
    sweepwholelist(L, &g->allgc);
    sweepwholelist(L, &g->fixgc);
}
