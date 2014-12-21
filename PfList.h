/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef PF_LIST_H
#define PF_LIST_H

/**
 * Need a custom implementation for the pathfinder
 * std::forward_list is too inefficient if you just 
 * wish to pop an element deep inside the linked list
 */

#include "dynamic_pool.h"



template<class T> struct PfListNode
{
	PfListNode* prev;
	PfListNode* next;
	T item;
};



template<class T> struct PfList
{
public:
	template<class U> struct _iter : std::iterator<std::bidirectional_iterator_tag, U>
	{
		typedef PfListNode<U> node;
		node* ptr;
		inline _iter() : ptr(NULL)		{}
		inline _iter(node* n) : ptr(n)	{}
		inline U& operator*() const		{ return ptr->item; }
		inline U* operator->() const	{ return &ptr->item; }
		inline _iter& operator++()		{ ptr = ptr->next; return *this; }
		inline _iter operator++(int)	{ _iter r(ptr); ptr = ptr->next; return r; }
		inline _iter& operator--()		{ ptr = ptr->prev; return *this; }
		inline _iter operator--(int)	{ _iter r(ptr); ptr = ptr->prev; return r; }
		inline bool operator==(const _iter& it) const { return ptr == it.ptr; }
		inline bool operator!=(const _iter& it) const { return ptr != it.ptr; }
		// one way conversion: iterator -> const_iterator
		inline operator _iter<U const>() const	{ return _iter<U const>(ptr); }
		inline operator bool() const			{ return ptr ? true : false; }
	};
	
	typedef PfListNode<T> node;
	typedef _iter<T> iterator;
	typedef _iter<T const> const_iterator;



private:
	node* Head;
	node* Tail;
	size_t Count;
	pool* Allocator;


public:

	inline node* new_node()
	{
		if(!Allocator) // if no allocator set, we need to fall back to a default one:
			Allocator = pool::create(sizeof(node), 1024); // reserve a lot of space ahead (trade speed for memory)
		if(node* n = (node*)Allocator->alloc())
			return n; // OK

		assert(false && "PfList::Allocator out of memory. Consider bigger sizehint in ResizeAllocator");
		return NULL;
	}
	inline void delete_node(node* n)
	{
		Allocator->dealloc(n);
	}

	/**
	 * Resizes the allocator
	 */
	void ResizeAllocator(int sizehint) 
	{
		if(Allocator)
		{
			clear();
			pool::destroy(Allocator); // destroy existing allocator
		}

		// create a new allocator
		Allocator = pool::create(sizeof(node), sizehint);
	}

	inline static pool* CreateCompatiblePool(int sizeHint)
	{
		return pool::create(sizeof(node), sizeHint);
	}


public:
	inline PfList() : Head(NULL), Tail(NULL), Count(0), Allocator(NULL)
	{
	}
	~PfList() 
	{
		if(Allocator) // only delete the allocator and we're done
		{
			pool::destroy(Allocator);
		}
		else // delete all nodes 1 by 1
		{
			node* n = Head;
			node* next;
			do {
				next = n->next;
				delete_node(n);
			} while(n = next);
		}
	}

	inline iterator begin() const	{ return Head; }
	inline iterator end() const		{ return NULL; }
	inline T front() const			{ return Head->item; }
	inline T back() const			{ return Tail->item; }
	inline bool empty() const		{ return Head == NULL; }
	inline size_t count() const		{ return Count; }
	inline size_t size() const		{ return Count; }

public:
	void clear()
	{
		if(!Head) return;
		if(Allocator) // if we have a specialized allocator, we can just clear the allocator
		{
			Allocator->clear(); // done! :)
		}
		else // in a general allocator we have to release 1 by 1
		{
			node* n = Head;
			node* next;
			do {
				next = n->next;
				delete_node(n);
			} while(n = next);
		}
		Head = NULL;
		Tail = NULL;
		Count = 0;
	}

	/**
	 * Erases item at the given iterator position.
	 * The iterator will be invalidated.
	 */
	void erase(iterator& it)
	{
		node* n = it.ptr;
		if(n == Head) Head = n->next;
		else if(n->prev) n->prev->next = n->next;
		if(n == Tail) Tail = n->prev;
		else if(n->next) n->next->prev = n->prev;
		delete_node(n);
		--Count;
		it.ptr = NULL; // invalidate iterator
	}

	T pop_front()
	{
		T item = Head->item;
		node* n = Head;
		if(Count == 1)
			Head = NULL, Tail = NULL;
		else {
			Head = Head->next;
			if(Head) Head->prev = NULL;
		}
		delete_node(n);
		--Count;
		return item;
	}

	iterator push_front(const T& item)
	{
		++Count;
		if(Head) {
			//node* n = new_node(NULL, Head, item);
			node* n = new_node();
			n->prev = NULL;
			n->next = Head;
			n->item = item;
			Head->prev = n;
			Head = n;
			return n;
		}
		//node* n = new_node(NULL, NULL, item);
		node* n = new_node();
		n->prev = NULL;
		n->next = NULL;
		n->item = item;
		Tail = Head = n;
		return n;
	}

	iterator push_back(const T& item)
	{
		++Count;
		if(Tail) {
			//node* n = new_node(Tail, NULL, item);
			node* n = new_node();
			n->prev = Tail;
			n->next = NULL;
			n->item = item;
			Tail->next = n;
			Tail = n;
			return n;
		}
		//node* n = new_node(NULL, NULL, item);
		node* n = new_node();
		n->prev = NULL;
		n->next = NULL;
		n->item = item;
		Head = Tail = n;
		return n;
	}

	/**
	 * Alias for insert_before
	 */
	inline iterator insert(iterator& it, const T& item)
	{
		return insert_before(it, item);
	}

	iterator insert_before(iterator& it, const T& item)
	{
		node* n = it.ptr;
		if(n == NULL) // push_back
			return push_back(item);
		++Count;
		//node* prev = new_node(n->prev, n, item);
		node* prev = new_node();
		prev->prev = n->prev;
		prev->next = n;
		prev->item = item;
		if(n->prev) n->prev->next = prev;
		n->prev = prev;
		if(n == Head) Head = prev;
		return prev;
	}

	iterator insert_after(iterator& it, const T& item)
	{
		node* n = it.ptr;
		if(n == NULL) // push_front
			return push_front(item);
		++Count;
		//node* next = new_node(n, n->next, item);
		node* next = new_node();
		next->prev = n;
		next->next = n->next;
		next->item = item;
		if(n->next) n->next->prev = next;
		n->next = next;
		if(n == Tail) Tail = next;
		return next;
	}
};

#endif // PF_LIST_H