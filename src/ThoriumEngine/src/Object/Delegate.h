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

		bindings.Add();

		FDelegateBinding<TArgs...>& binding = *bindings.last();
		binding.receiver = obj;
		binding.funcPtr = funcPtr;
		binding.func = [=](TArgs... args) {
			if constexpr (std::is_member_function_pointer<TFunc>::value)
				(obj->*func)(args...);
			else
				func(args...);
		};
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
				bindings.Erase(it);
				break;
			}
		}
	}

	void RemoveAll(CObject* obj)
	{
		for (auto it = bindings.rbegin(); it != bindings.rend(); it++)
		{
			if (it->receiver == obj)
			{
				bindings.Erase(it);
			}
		}
	}

private:
	TArray<FDelegateBinding<TArgs...>> bindings;
};
