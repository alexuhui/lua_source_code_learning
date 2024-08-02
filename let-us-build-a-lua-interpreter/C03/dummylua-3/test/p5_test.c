
#include "p5_test.h"
#include "../common/lua.h"
#include "../clib/luaaux.h"

void p5_test_main() {
	// 创建了一个Lua虚拟机实例
	struct lua_State* L = luaL_newstate();
	// 为该Lua虚拟机加载了标准库
	luaL_openlibs(L);

	//对Lua代码进行编译
	//生成的虚拟机指令保存在Lua虚拟机实例L中
	int ok = luaL_loadfile(L, "./scripts/part05_test.lua");
	if (ok == LUA_OK) {
		// 运行已经编译好的虚拟机指令
		luaL_pcall(L, 0, 0);
	}

	//释放Lua虚拟机实例
	luaL_close(L);
}