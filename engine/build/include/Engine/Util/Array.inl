#pragma once

#ifndef ARRAY_H
#include "Array.h"
#endif

#include <stdexcept>
#include <cstdarg>

template<typename T>
TArray<T>::TArray() : _data(nullptr)
{
	Reserve(2, false);
}

template<typename T>
TArray<T>::TArray(SizeType size) :  _data(nullptr)
{
	Resize(size);
}

template<typename T>
TArray<T>::TArray(const TArray<T>& other)
{
	Reserve(other._size > 1 ? other._size : 2);

	//memcpy(_data, other._data, other._size * sizeof(T));

	for (SizeType i = 0; i < other._size; i++)
	{
		if constexpr (!std::is_pointer<T>::value && std::is_copy_constructible<T>::value)
			new (&_data[i]) T(other._data[i]);
		else
			_data[i] = other[i];
	}

	_size = other._size;
}

template<typename T>
TArray<T>::TArray(std::initializer_list<T> list) : _data(nullptr)
{
	Reserve(list.size());

	for (auto& obj : list)
		this->Add(obj);
}

template<typename T>
TArray<T>::~TArray()
{
	if constexpr (std::is_destructible<T>::value)
		for (SizeType i = 0; i < _size; i++)
			_data[i].~T();

	free(_data);
}

template<typename T>
TArray<T>& TArray<T>::operator=(const TArray<T>& other)
{
	Clear();
	Reserve(other.Size(), false);
	//_size = other._size;
	_size = 0;

	for (auto& obj : other)
		this->Add(obj);

	//memcpy(_data, other._data, other._size * sizeof(T));
	return *this;
}

template<typename T>
TArray<T>& TArray<T>::operator=(const std::initializer_list<T>& list)
{
	Clear();
	Reserve(list.size());

	for (auto& obj : list)
		this->Add(obj);

	return *this;
}

template<typename T>
T& TArray<T>::operator[](SizeType i)
{
	return _data[i];
}

template<typename T>
const T& TArray<T>::operator[](SizeType i) const
{
	return _data[i];
}

template<typename T>
void TArray<T>::Add(const T& obj)
{
	Reserve(_size + 1);
	_size++;

	//if (std::is_class<T>::value)
	//	_data[_size - 1].T();

	// Call the constructor on the newly added element.
	if constexpr (!std::is_pointer<T>::value && std::is_copy_constructible<T>::value)
		new (&_data[_size - 1]) T(obj);
	else
		*last() = obj;

#if 1
	// commented out sinse we use the copy constructor instead.
	//_data[_size - 1] = obj;
#else
	memcpy((T*)last(), &obj, sizeof(T));
#endif
}

template<typename T>
void TArray<T>::Add()
{
	Reserve(_size + 1);
	_size++;

	// Call the constructor on the newly added element.
	if constexpr (std::is_constructible<T>::value)
		new (&_data[_size - 1]) T();
	else
		memset(&_data[_size - 1], 0, sizeof(T));
}

template<typename T>
void TArray<T>::PopBack()
{
	_size--;
	
	if constexpr (std::is_destructible<T>::value)
		_data[_size].~T();
}

template<typename T>
void TArray<T>::Clear()
{
	if constexpr (std::is_class<T>::value && std::is_destructible<T>::value)
		for (TIterator<T> it = begin(); it != end(); it++)
			(*it).~T();

	_size = 0;
}

template<typename T>
void TArray<T>::Reserve(SizeType size, bool bCopy /*= true*/)
{
	if (size <= _capacity)
		return;

	SizeType oldCap = _capacity;
	if (_capacity == 0)
		_capacity = 2;

	while (_capacity < size)
		_capacity += (_capacity / 2);

	T* oldData = _data;
	_data = (T*)malloc(_capacity * sizeof(T));
	//_data = new T[_capacity]();

	if (oldData && oldCap > 0 && bCopy)
		memcpy(_data, oldData, oldCap * sizeof(T));
	
	if (oldData)
		free(oldData);
}

template<typename T>
void TArray<T>::Resize(SizeType size)
{
	if (_size >= size)
		return;

	SizeType prevSize = _size;

	_size = size;
	Reserve(_size);

	for (auto it = At(prevSize); it != end(); it++)
		new (it.ptr) T(); // call the constructor for the newly added elements.
}

template<typename T>
void TArray<T>::Erase(const TIterator<T>& index)
{
	if (index == end())
		throw std::out_of_range("");

	if (std::is_class<T>::value && std::is_destructible<T>::value)
		(*index).~T();

	SizeType _i = ((SizeType)index.ptr - (SizeType)_data) / sizeof(T);
	SizeType amount = _size - _i - 1;
	if (amount != 0)
		memcpy(index.ptr, index.ptr + 1, amount * sizeof(T));

	_size--;
}

template<typename T>
void TArray<T>::Erase(const TIterator<T>& first, const TIterator<T>& last)
{
	if (last == end() && first == begin())
		return Clear();

	if (last > end() || first >= last)
		throw std::out_of_range("TArray::Erase index out of range");

	SizeType removeAmount = ((SizeType)last.ptr - (SizeType)first.ptr) / sizeof(T);
	//SizeType findex = ((SizeType)first - (SizeType)data) / sizeof(T);
	SizeType lindex = ((SizeType)last.ptr - (SizeType)_data) / sizeof(T);

	if (std::is_class<T>::value && std::is_destructible<T>::value)
		for (TIterator<T> it = first; it != last; it++)
			(*it).~T();

	if (lindex != _size)
		memcpy(first.ptr, last.ptr, (_size - lindex - 1) * sizeof(T));

	_size -= removeAmount;
}

template<typename T>
SizeType TArray<T>::Index(const TReverseIterator<T>& it)
{
	return ((SizeType)it.ptr - (SizeType)_data) / sizeof(T);
}

template<typename T>
SizeType TArray<T>::Index(const TIterator<T>& it)
{
	return ((SizeType)it.ptr - (SizeType)_data) / sizeof(T);
}

template<typename T>
void TArray<T>::Insert(const T& obj, SizeType index)
{
	if (index >= _size)
		return;

	SizeType amount = _size - index;
	Reserve(_size + 1);

	if (amount != 0)
		memcpy(&_data[index + 1], &_data[index], amount * sizeof(T));

	if constexpr (!std::is_pointer<T>::value && std::is_copy_constructible<T>::value)
		new (&_data[index]) T(obj);
	else
		_data[index] = obj;

	_size++;
}

template<typename T>
typename TIterator<T> TArray<T>::Find(const T& obj, SizeType offset)
{
	for (SizeType i = offset; i < _size; i++)
		if (_data[i] == obj)
			return TIterator<T>(&_data[i]);

	return end();
}

template<typename T>
const typename TIterator<T> TArray<T>::Find(const T& obj, SizeType offset) const
{
	for (SizeType i = offset; i < _size; i++)
		if (_data[i] == obj)
			return TIterator<T>(&_data[i]);

	return end();
}

template<typename T>
TIterator<T> TArray<T>::Find(std::function<bool(const T& index)> check, SizeType offset /*= 0*/)
{
	for (SizeType i = offset; i < _size; i++)
		if (check(_data[i]))
			return TIterator<T>(&_data[i]);

	return end();
}

template<typename T>
const TIterator<T> TArray<T>::Find(std::function<bool(const T& index)> check, SizeType offset /*= 0*/) const
{
	for (SizeType i = offset; i < _size; i++)
		if (check(_data[i]))
			return TIterator<T>(&_data[i]);

	return end();
}
