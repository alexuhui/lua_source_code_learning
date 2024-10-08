#include "luazio.h"

/**
* 初始化文字读取模块
* @param L lua_State
* @param zio 字符读取模块
* @param reader 文件读取函数读取
* @param data 读取数据
*/
void luaZ_init(struct lua_State* L, Zio* zio, lua_Reader reader, void* data) {
	lua_assert(L);
	lua_assert(zio);
	lua_assert(data);

	zio->L = L;
	zio->data = data;
	zio->n = 0;
	zio->p = NULL;
	zio->reader = reader;
}

/**
 * 填充 Zio (从文件读取文本代码)
 */
int luaZ_fill(Zio* z) {
	int c = 0;

	size_t read_size = 0;
	z->p = (void*)z->reader(z->L, z->data, &read_size);
	
	if (read_size > 0) {
		z->n = (int)read_size;
		c = (int)(*z->p);

		z->p++;
		z->n--;
	}
	else {
		c = EOF;
	}
	
	return c;
}