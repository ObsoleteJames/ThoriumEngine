
#include "Util/Assert.h"
#include "Util/Core.h"
#include <cstdlib>

#ifdef _WIN32
#include "windows.h"
#endif

void _util_assert(bool test, const FString& funcsig, const FString& message)
{
	FString msg = funcsig + " Assertion failed!\n\n" + message;
#ifdef IS_DEV
	msg += "\n\nPress retry to debug.";
#endif

#ifdef _WIN32
	int r = MessageBoxA(NULL, msg.c_str(), "Thorium Engine", MB_ABORTRETRYIGNORE | MB_ICONERROR);
#ifdef IS_DEV
	if (r == IDRETRY)
		_CrtDbgBreak();
#endif
	if (r == IDIGNORE)
		return;
#endif

	std::exit(1);
}
