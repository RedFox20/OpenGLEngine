/**
 * Copyright (c) 2013 - Jorma Rebane
 * Pool allocator for smart_ptr<> handles.
 * Very specialized implementation for maximum efficiency.
 */
#include "smart_ptr.h"

#include <malloc.h> // malloc, free

namespace smart_pointer
{
	// fixed-size pool container, specific implementation for unsigned int handles
	struct pool
	{
		static const unsigned short CAPACITY = 1024; // 4*1024 = 4kb
		unsigned short available; // number of unhanded allocations
		unsigned short freed; // number in free list
		struct freenode { freenode* next; } *list; // free list
#pragma warning(push)
#pragma warning(disable:4200)
		unsigned buffer[]; // the actual buffer contained in this object
#pragma warning(pop)
		inline static void* operator new(size_t sz) // allocates the 'actual' size in bytes
		{
			return malloc(sz + sizeof(unsigned)*CAPACITY); // buffer[] is allocated <inside> this object
		}
		inline static void operator delete(void* mem) // need to delete with 'free'
		{
			free(mem); // we actually never delete any pools, but I put this here for the sake of correctness
		}
		inline pool() : available(CAPACITY), freed(0), list(0) // default inits the pool
		{
		}
		inline unsigned* get_new() // gets from the buffer
		{
			unsigned* ptr = buffer + --available; // get next
			*ptr = 1; // initialize
			return ptr;
		}
		inline unsigned* get_freed() // gets from the freed list
		{
			--freed;
			unsigned* ptr = (unsigned*)list; // cleverly reinterpret as an 'unsigned' ptr :)
			list = list->next; // unshift the linked list
			*ptr = 1; // initialize
			return ptr;
		}
		inline void put(unsigned* ptr) // puts into the freed list
		{
			++freed;
			freenode* f = (freenode*)ptr; // cleverly reinterpret as a 'freed' list object :)
			f->next = list; // current 'freed' list ptr is our next
			list = f; // and store the beginning of the linked list :)
		}
		inline bool is_owner(unsigned* ptr) const // true if the pointer is in range of this pool
		{
			return buffer <= ptr && ptr < (buffer + CAPACITY);
		}
		inline size_t live_count() const // gets the number of handles currently 'alive'
		{
			return freed + (CAPACITY - available);
		}
	};

	static pool** gPools = 0;		// array that contains our pools
	static size_t gPoolsCount = 0;	// exact length of gPools

	unsigned* new_handle() // gets a new handle
	{
		for(size_t i = 0; i < gPoolsCount; ++i)
		{
			pool* p = gPools[i];
			if(p->freed) // any in the 'freed' list?
				return p->get_freed();
			else if(p->available) // any in the buffer itself?
				return p->get_new();
		}
		// the overloaded pool::new actually calls malloc
		// and we actually never free any pools, since it will hamper smart_ptr<> performance
		gPools = (pool**)realloc(gPools, sizeof(pool*)*(gPoolsCount+1));
		pool* p = new pool();
		gPools[gPoolsCount++] = p;
		return p->get_new();
	}

	void delete_handle(unsigned* ptr) // deletes an existing handle
	{
		for(size_t i = 0; i < gPoolsCount; ++i)
		{
			pool* p = gPools[i];
			if(p->is_owner(ptr)) // belongs to this pool?
				p->put(ptr); // good. put it there.
		}
	}

	size_t live_count() // number of total handles currently 'alive'
	{
		size_t count = 0;
		for(size_t i = 0; i < gPoolsCount; ++i)
			count += gPools[i]->live_count();
		return count;
	}
	
}