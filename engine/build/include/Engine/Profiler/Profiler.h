#pragma once

#include "EngineCore.h"

#ifdef IS_DEV
#define TH_PROFILE_SCOPE(func) CProfilerScoped(func)

#define TH_BEGIN_RPROFILE(func) CProfilerManager::ProfileFunction(func)
#define TH_END_PROFILE() CProfilerManager::PopFunction();
#else
#define TH_PROFILE_SCOPE(func)

#define TH_BEGIN_RPROFILE(func)
#define TH_END_PROFILE()
#endif

class ENGINE_API CProfilerManager
{
public:
	static void ProfileFunction(const FString& name);
	static void PopFunction();

private:

};

class ENGINE_API CProfilerScoped
{
public:
	inline CProfilerScoped(const FString& name) { CProfilerManager::ProfileFunction(name); }
	inline ~CProfilerScoped() { CProfilerManager::PopFunction(); }
};
