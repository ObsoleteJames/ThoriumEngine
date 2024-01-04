#pragma once

template<typename T>
class TUniquePtr
{
public:
	TUniquePtr() = default;
	TUniquePtr(T* obj) {
		ptr = obj;
	}
	~TUniquePtr() {
		delete ptr;
	}

	T* operator->() { return ptr; }
	T& operator*() { return *ptr; }

	operator T* () { return ptr; }

	TUniquePtr<T>& operator=(T* newPtr)
	{
		if (ptr)
			delete ptr;

		ptr = newPtr;
		return *this;
	}

private:
	T* ptr;
};
