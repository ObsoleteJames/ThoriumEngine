#pragma once

#include "Object.h"
#include "ObjectHandle.h"
#include <functional>

template<typename... TArgs>
struct FDelegateBinding
{
	FObjectHandle receiver;
	void** funcPtr;
	std::function<void(TArgs...)> func;
};

template<typename... TArgs>
class TDelegate
{
public:
	TDelegate() = default;

	template<typename T, typename TFunc>
	void Bind(T* obj, TFunc func)
	{
		void** funcPtr = reinterpret_cast<void**>(&func);

		bindings.push_back(FDelegateBinding<TArgs...>());

		FDelegateBinding<TArgs...>& binding = bindings[bindings.size() - 1];
		binding.receiver = obj;
		binding.funcPtr = funcPtr;
		if constexpr (std::is_member_function_pointer<TFunc>::value)
		{
			binding.func = [obj, func](TArgs... args) {
				(obj->*func)(args...);
			};
		}
		else
		{
			binding.func = [obj, func](TArgs... args) {
				func(args...);
			};
		}
	}

	void Invoke(TArgs... args)
	{
		for (auto it = bindings.rbegin(); it != bindings.rend(); it++)
		{
			if (it->receiver.Get() != nullptr && it->func)
				it->func(args...);
		}
	}

	template<typename TFunc>
	void Remove(CObject* obj, TFunc func)
	{
		void** funcPtr = reinterpret_cast<void**>(&func);
		for (auto it = bindings.begin(); it != bindings.end(); it++)
		{
			if (it->receiver == obj && it->funcPtr == funcPtr)
			{
				bindings.erase(it);
				break;
			}
		}
	}

	void RemoveAll(CObject* obj)
	{
		/*for (auto it = bindings.rbegin(); it != bindings.rend(); it++)
		{
			if (it->receiver == obj)
			{
				bindings.erase(++(it.base()));
			}
		}*/
		for (auto i = bindings.size(); i > 0; i--)
		{
			if (bindings[i - 1].receiver == obj)
			{
				bindings.erase(bindings.begin() + (i - 1));
			}
		}
	}

private:
	std::vector<FDelegateBinding<TArgs...>> bindings;
};
