#pragma once

template<typename T>
class TList;

using SizeType = size_t;

template<typename T>
class TListIterator
{
public:
	TListIterator(TList<T>::Index* index) : value(index) {}
	T& operator*() { return value->value; }

	TListIterator<T>& operator++() { value = value->Next; return *this; }
	TListIterator<T>& operator++(int) { value = value->Next; return *this; }

	TListIterator<T>& operator++(SizeType i) { 
		for (int i =0; i < i && value->Next; i++)
			value = value->Next; 
		return *this;
	}

	bool operator==(const TListIterator<T>& i) const { return value == i.value; }
	bool operator!=(const TListIterator<T>& i) const { return value != i.value; }

	T* operator->() { return &value->value; }

public:
	TList<T>::Index* value;
};

// Linked List Class
template<typename T>
class TList
{
public:
	struct Index
	{
		T value;
		Index* Next = nullptr;
	};

public:
	TList();
	TList(const TList<T>&);
	~TList();

	TList<T>& operator=(const TList<T>&);

	T& operator[](SizeType i);
	const T& operator[](SizeType i) const;

	void Add(const T&);
	void Clear();
	
	SizeType Size() const;
	void Erase(SizeType i);
	void Erase(SizeType first, SizeType, last);

	TListIterator<T> At(SizeType index) const;

	TListIterator<T> begin() const;
	TListIterator<T> end() const;

	TListIterator<T> first() const;
	TListIterator<T> last() const;

private:
	SizeType size;
	Index* First;
};
