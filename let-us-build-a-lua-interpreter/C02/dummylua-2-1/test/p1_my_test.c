#include "../clib/luaaux.h"

static int add_op(lua_State* L){
    int left = luaL_tointeger(L, -2);
    int right = luaL_tointeger(L, -1);
    printf("left + right : %d + %d = %d\n", left, right, left + right);
    luaL_pushinteger(L, 5);
    printf("----------- push 5\n");
    luaL_pushinteger(L, left + right);
    printf("----------- push left + right\n");
    luaL_pushinteger(L, 77);
    printf("----------- push 77\n");

    printf("------------------------- ci->func : %d\n", L->ci->func->value_.i);
    // 这个地方有点奇怪，需要返回 “返回值个数”，后面取值的时候才能正常
    return L->ci->nresult;
}

void test_add() { 
    struct lua_State* L = luaL_newstate();
    int a = 10, b = 20;
    luaL_pushcfunction(L, &add_op);
    luaL_pushinteger(L, a);
    luaL_pushinteger(L, b);

    printf("------------ 1111111 ci->func : %d\n", L->ci->func->value_.i);
    printf("****************** begin call\n");
    luaL_pcall(L, 2, 3);
    printf("****************** end call\n");
    printf("------------ 2222222 ci->func : %d\n", L->ci->func->value_.i);

    printf("other : %d\n", luaL_tointeger(L, -1));
    int result = luaL_tointeger(L, -2);
    printf("result : %d + %d = %d\n", a, b, result);
    printf("other : %d\n", luaL_tointeger(L, -3));

    printf("cur stack size %d\n",luaL_stacksize(L));
    luaL_pop(L);
    printf("final stack size %d\n",luaL_stacksize(L));

    luaL_close(L);
}