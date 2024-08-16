#ifndef yhlib_h
#define yhlib_h

#include "lua.h"
#include "lauxlib.h"

#define LUA_YHLIBNAME "yhlib"
LUAMOD_API int (luaopen_yhlib) (lua_State *L);

int pp(lua_State *L);

int testadd(lua_State *L, int a, int b);

#endif  //yhlib_h