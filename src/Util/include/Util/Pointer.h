#pragma once

template<typename T>
class TUniquePtr
{
public:
	TUniquePtr() = default;
	TUniquePtr(const TUniquePtr& other) : ptr(other.ptr) {
	}
	TUniquePtr(TUniquePtr&& other) : ptr(other.ptr) {
		other.ptr = nullptr;
	}
	TUniquePtr(T* obj) : ptr(obj) {}
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
	TUniquePtr<T>& operator=(TUniquePtr<T>&& other)
	{
		this->ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

private:
	T* ptr = nullptr;
};
