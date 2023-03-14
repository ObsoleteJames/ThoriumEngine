#pragma once

#define ARRAY_H

#include <type_traits>
#include <stdint.h>
#include <functional>

template<typename T1, typename T2>
struct TPair
{
public:
	T1 Key;
	T2 Value;
};

enum class EResizeMode : uint8_t
{
	RM_DOUBLE_SIZE = 0, // Doubles the capacity until the requested size is met.
	RM_ADD_HALF, // Multiply capcacity by 1.5 until the requested size is met.
	RM_EXACT, // Scales the capacity to the requested size.
};

// Use std::vector or custom vector class.
#if 0
#include <vector>
template<typename T>
class TArray : public std::vector<T>
{
public:
	void Add(const T& obj) { push_back(obj); }
	void Clear() { clear(); }
	void Reserve(size_t size, bool) { reserve(size); }

	T* Data() { return data(); }
	const T* Data() const { return data(); }

	size_t Size() const { return size(); }
	size_t Capacity() const { return capacity(); }

	void Erase(const_iterator i) { erase(i); }
	void Erase(const_iterator first, const_iterator last) { erase(first, last); }

	const_iterator Find(const T& obj) { return std::find(begin(), end(), obj); }
	const const_iterator Find(const T& obj) const { return std::find(begin(), end(), obj); }

	void SetResizeMode(const EResizeMode&) {} // does nothing.

};
#else
//typedef size_t SizeType;
using SizeType = size_t;

template<typename T>
class TReverseIterator
{
public:
	typedef typename std::remove_pointer<T>::type baseT;

	TReverseIterator(T* p) : ptr(p) {}
	T& operator*() const { return *ptr; }

	TReverseIterator<T>& operator++() { ptr--; return *this; }
	TReverseIterator<T> operator++(int) { ptr--; return *this; }
	TReverseIterator<T> operator+(SizeType i) { return TReverseIterator<T>(ptr - (i == -1 ? 0 : i)); }

	const TReverseIterator<T>& operator++() const { ptr--; return *this; }
	const TReverseIterator<T> operator++(int) const { ptr--; return *this; }
	const TReverseIterator<T> operator+(SizeType i) const { return TReverseIterator<T>(ptr - (i == -1 ? 0 : i)); }

	bool operator==(const TReverseIterator<T>& i) const { return ptr == i.ptr; }
	bool operator!=(const TReverseIterator<T>& i) const { return ptr != i.ptr; }

	bool operator>=(const TReverseIterator<T>& i) const { return ptr < i.ptr; }
	bool operator>(const TReverseIterator<T>& i) const { return ptr <= i.ptr; }

	//operator baseT*() { if constexpr (std::is_pointer<T>::value) return *ptr; else return ptr; }
	//operator const baseT*() const { if constexpr (std::is_pointer<T>::value) return *ptr; else return ptr; }

	operator T() { return *ptr; }
	operator const T() const { return *ptr; }

	baseT* operator->() { if constexpr (std::is_pointer<T>::value) return (baseT*)*ptr; return (baseT*)ptr; }
	const baseT* operator->() const { if constexpr (std::is_pointer<T>::value) return (baseT*)*ptr; return (baseT*)ptr; }

public:
	T* ptr;
};

template<typename T>
class TIterator
{
public:
	typedef typename std::remove_pointer<T>::type baseT;

	TIterator(T* p) : ptr(p) {}
	T& operator*() const { return *ptr; }

	TIterator(const TReverseIterator<T>& r) { ptr = r.ptr; }

	TIterator<T>& operator++() { ptr++; return *this; }
	TIterator<T> operator++(int) { ptr++; return *this; }
	TIterator<T> operator+(SizeType i) { return TIterator<T>(ptr + (i == -1 ? 0 : i)); }

	const TIterator<T>& operator++() const { ptr++; return *this; }
	const TIterator<T> operator++(int) const { ptr++; return *this; }
	const TIterator<T> operator+(SizeType i) const { return TIterator<T>(ptr + (i == -1 ? 0 : i)); }

	TIterator<T>& operator--() { ptr--; return *this; }
	TIterator<T> operator--(int) { ptr--; return *this; }

	const TIterator<T>& operator--() const { ptr--; return *this; }
	const TIterator<T> operator--(int) const { ptr--; return *this; }

	bool operator==(const TIterator<T>& i) const { return ptr == i.ptr; }
	bool operator!=(const TIterator<T>& i) const { return ptr != i.ptr; }

	bool operator>=(const TIterator<T>& i) const { return ptr >= i.ptr; }
	bool operator>(const TIterator<T>& i) const { return ptr > i.ptr; }

	operator baseT*() { if constexpr (std::is_pointer<T>::value) return *ptr; else return ptr; }
	operator const baseT*() const { if constexpr (std::is_pointer<T>::value) return *ptr; else return ptr; }

	baseT* operator->() { if constexpr(std::is_pointer<T>::value) return (baseT*)*ptr; else return (baseT*)ptr; }
	const baseT* operator->() const { if constexpr(std::is_pointer<T>::value) return (baseT*)*ptr; else return (baseT*)ptr; }

public:
	T* ptr;
};

template<typename T>
class TArray
{
public:
	TArray();
	TArray(const TArray<T>&);
	TArray(std::initializer_list<T> list);
	~TArray();

	TArray<T>& operator=(const TArray<T>&);
	TArray<T>& operator=(const std::initializer_list<T>& list);

	T& operator[](SizeType i);
	const T& operator[](SizeType i) const;

	void Add(const T&);
	void PopBack();
	void Clear();
	void Reserve(SizeType size, bool bCopy = true);
	void Resize(SizeType size);

	inline T* Data() { return _data; }
	inline const T* Data() const { return _data; }

	inline SizeType Size() const { return _size; }
	inline SizeType Capacity() const { return _capacity; }
	void Erase(const TIterator<T>& index);
	void Erase(const TIterator<T>& first, const TIterator<T>& last);

	// Iterators
	TIterator<T> Find(const T&, SizeType offset = 0);
	const TIterator<T> Find(const T&, SizeType offset = 0) const;

	TIterator<T> Find(std::function<bool(const T& index)> check, SizeType offset = 0);
	const TIterator<T> Find(std::function<bool(const T& index)> check, SizeType offset = 0) const;

	inline TIterator<T> At(SizeType index) { if (index >= _size) return end(); return TIterator<T>(_data + index); }
	inline const TIterator<T> At(SizeType index) const { if (index >= _size) return end(); return TIterator<T>(_data + index); }

	inline TIterator<T> begin() { return TIterator<T>(_data); }
	inline const TIterator<T> begin() const { return TIterator<T>(_data); }

	inline TIterator<T> end() { return TIterator<T>(_data + _size); }
	inline const TIterator<T> end() const { return TIterator<T>(_data + _size); }

	inline TReverseIterator<T> rbegin() { return TReverseIterator<T>(_data + (_size - 1)); }
	inline const TReverseIterator<T> rbegin() const { return TReverseIterator<T>(_data + (_size - 1)); }

	inline TReverseIterator<T> rend() { return TReverseIterator<T>(_data - 1); }
	inline const TReverseIterator<T> rend() const { return TReverseIterator<T>(_data - 1); }

	inline TIterator<T> last() { return TIterator<T>(_data + (_size - 1)); }
	inline const TIterator<T> last() const { return TIterator<T>(_data + (_size - 1)); }

	inline TIterator<T> first() { return begin(); }
	inline const TIterator<T> first() const { return begin(); }

	void SetResizeMode(const EResizeMode& mode);

protected:
	typedef typename std::remove_pointer<T>::type baseT;

	T* _data = nullptr;

	EResizeMode rm;
	SizeType _capacity = 0;
	SizeType _size = 0;
};

#include "Array.inl"
#endif
