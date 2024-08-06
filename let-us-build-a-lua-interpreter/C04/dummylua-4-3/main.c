#include "test/p6_test.h"
#include "test/p7_test.h"
#include "test/p8_test.h"
#ifdef _WINDOWS_PLATFORM_
#include <process.h>
#endif

int main(int argc, char** argv) {
	p6_test_main();
	// p7_test_main();
	// p8_test_main();

#ifdef _WINDOWS_PLATFORM_
	system("pause");
#endif

	return 0;
}
