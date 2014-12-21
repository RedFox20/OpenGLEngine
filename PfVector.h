/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef PF_VECTOR_H
#define PF_VECTOR_H

/**
 * Need a custom implementation for the pathfinder
 * std::vector is too inefficient - we need something
 * that scales a lot better.
 *
 * We will definitely loose some flexibility, but in this case
 * we need a hardcore performance vector with very little functionality
 */

#include <iterator>

template<class T> struct PfVector
{
	template<class U> struct _iter : std::iterator<std::bidirectional_iterator_tag, U>
	{
		U* ptr;
		inline _iter() : ptr(NULL) {}
		inline _iter(U* ptr) : ptr(ptr) {}
		inline U& operator*() const { return *ptr; }
		inline U* operator->() const { return ptr; }
		inline _iter& operator++() { ++ptr; return *this; }
		inline _iter operator++(int) { U* p = ptr; ++ptr; return p; }
		inline _iter& operator--() { --ptr; return *this; }
		inline _iter operator--(int) { U* p = ptr; --ptr; return p; }
		inline bool operator==(const _iter& it) const { return ptr == it.ptr; }
		inline bool operator!=(const _iter& it) const { return ptr != it.ptr; }
		// one way conversion: iterator -> const_iterator
		inline operator _iter<U const>() const { return _iter<U const>(ptr); }
		inline operator bool() const { return ptr ? true : false; }
	};
	typedef _iter<T> iterator;
	typedef _iter<T const> const_iterator;

	T* Buffer;
	size_t Length;		// Size
	size_t MaxLength;	// Capacity

	inline PfVector() : Buffer(NULL), Length(0), MaxLength(0) {}
	inline ~PfVector() { if(Buffer) delete Buffer; }

	inline T& operator[](size_t index) { return Buffer[index]; }
	inline const T& operator[](size_t index) const { return Buffer[index]; }
	inline size_t size() const { return Length; }
	inline size_t capacity() const { return MaxLength; }
	inline bool empty() const { return !Length; }


	inline iterator begin() const { return Buffer; }
	inline iterator end() const { return Buffer + Length; }
	inline iterator before_begin() const { return Buffer - 1; }
	inline iterator before_end() const { return Buffer + Length - 1; }
	inline T& front() { return *Buffer; }
	inline T& back() { return Buffer[Length - 1]; }


	// deallocates the buffer
	inline void deallocate()
	{
		if(Buffer) {
			delete Buffer, Buffer = NULL;
			Length = 0, MaxLength = 0;
		}
	}

	// clears the vector by zeroing the count; no deallocation
	inline void clear()
	{
		Length = 0;
	}

	// erases item at the given iterator position
	inline void erase(const iterator& it)
	{
		_unshift(it.ptr);
		--Length;
	}

	// erases the first occurrence of item from this vector
	inline void erase(const T& item)
	{
		T* it = Buffer;
		size_t count = Length;
		while(count) {
			if(*it == item) {
				_unshift(it);
				--Length;
				return; // we only remove one entry
			}
			++it, --count;
		}
	}

	// pop the last element and return it
	inline void pop_back(T& out)
	{
		out = Buffer[--Length];
	}

	// push an item to the end of the vector
	inline void push_back(const T& item)
	{
		if(Length == MaxLength) // resize required
			_expand();
		Buffer[Length++] = item;
	}

	// inserts an item inside the vector at the current place
	inline void insert(const iterator& it, const T& item)
	{
		T* src;
		if(Length == MaxLength) // resize required
		{
			size_t offset = size_t(it.ptr - Buffer) / sizeof(T);
			_expand();
			src = Buffer + offset;
		}
		else
		{
			src = it.ptr;
		}
		_shift(src);
		*src = item;
		++Length;
	}

	// inserts an item inside the vector after the specified element
	inline void insert_after(const iterator& it, const T& item)
	{
		T* src;
		if(Length == MaxLength) // resize required
		{
			size_t offset = size_t(it.ptr - Buffer) / sizeof(T);
			_expand();
			src = Buffer + offset + 1;
		}
		else
		{
			src = it.ptr + 1;
		}
		_shift(src);
		*src = item;
		++Length;
	}


	// shifts the vector forward from this point
	inline void _shift(T* src)
	{
		T* dst = src + 1; // shift forward --->
		size_t sz = size_t(Buffer + Length) - size_t(src); // end - src
		if(sz) memmove(dst, src, sz);
	}

	// unshifts the vector
	inline void _unshift(T* dst)
	{
		T* src = dst + 1; // shift backward <---
		size_t sz = size_t(Buffer + Length) - size_t(src); // end - src
		if(sz) memmove(dst, src, sz);
	}

	// internal resize of buffer
	inline void _expand()
	{
		MaxLength = Buffer ? (MaxLength + (MaxLength >> 1)) : 1024;
		Buffer = (T*)realloc(Buffer, sizeof(T) * MaxLength);
	}
};


#endif