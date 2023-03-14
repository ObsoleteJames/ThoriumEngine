
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
	msg += "\n\nPress yes to debug.";
#endif

#ifdef _WIN32
#ifdef IS_DEV
	int r = MessageBoxA(NULL, msg.c_str(), "Thorium Engine", MB_YESNO | MB_ICONERROR);
	if (r == IDYES)
		_CrtDbgBreak();
#else
	int r = MessageBoxA(NULL, msg.c_str(), "Thorium Engine", MB_OK | MB_ICONERROR);
#endif
#endif
	std::exit(1);
}
