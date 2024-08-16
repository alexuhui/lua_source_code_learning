#include "yhlib.h"

static int yh_add(lua_State *L){
    int n = lua_gettop(L);  /* number of arguments */
    luaL_argcheck(L, n >= 1, 1, "at least one parameter expected");
    int isF = 0;
    lua_Integer iSum = 0;
    lua_Number fSum = 0.0;
    for (int i = 1; i <= n; i++)
    {
        if (isF) {
            lua_Number n = luaL_checknumber(L, i);
            fSum += n;
        }
        else
        {
            if(lua_isinteger(L, i)){
                lua_Integer n = lua_tointeger(L, i);
                iSum += n;
            }else
            {
                isF = 1;
                lua_Number n = luaL_checknumber(L, i);
                fSum += (lua_Number)iSum + n;
            }
        }
    }
    if(isF){
        lua_pushnumber(L, fSum);
    }else{
        lua_pushinteger(L, iSum);
    }
    return 1;
}

static const luaL_Reg  yh_funcs[] = {
  {"add", yh_add},
  {NULL, NULL}
};

LUAMOD_API int luaopen_yhlib (lua_State *L) {
  luaL_newlib(L, yh_funcs);
  return 1;
}