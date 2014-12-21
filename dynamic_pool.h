/* Copyright (c) 2013 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef INCLUDED_MEMORY_POOL
#define INCLUDED_MEMORY_POOL

#define MSC_VERSION _MSC_VER != 0
#include <assert.h>
#define ENSURE(x) assert(x)
#ifndef _DEBUG
# define NDEBUG
#endif

/**
 * @author Jorma Rebane
 * @date 2013.06.27
 *
 * We're using a SIZE based Pool.
 * This is useful if you use memory pools a lot and want
 * to share the pools accross many different types.
 *
 * The STL-compatible bucket_allocator<T> is an interface on top of a
 * collection of Thread-Local memory pools, indexed by the request size in bytes.
 * The maximum request size for bucket_allocator<T> is 1024 bytes. For all other cases
 * the standard malloc is used instead.
 */

//#include <lib/debug.h> // ENSURE
#include <stdlib.h> // malloc/free
#include <string.h> // memmove
#include <stdint.h> // uint32_t
#include <memory>	// std::allocator




#define ENABLE_POOL_DEBUGGING_DEBUG 1 // enable pool debugging in Debug mode?
#define ENABLE_POOL_DEBUGGING_RELEASE 0 // enable pool debugging in Release mode?


// For auto-calculating the initial capacity of pools. Very crude.
// This is why you should always 'know better' and give a size hint to your pool!
#define POOL_AUTOSIZE(TSIZE) ((TSIZE <= 128) ? 8192/TSIZE : (8192*2)/TSIZE)


// maximum bucket size for the global allocator
#define POOL_MAX_BUCKET_SIZE 1024



#if MSC_VERSION
# pragma warning(disable:4200) // so annoying
#endif




// dummy type for setting the dynamic_pool GC value
typedef bool pool_gc_t;
const pool_gc_t use_gc = (pool_gc_t)true;
const pool_gc_t no_gc = (pool_gc_t)false;


#ifdef NDEBUG // release?
	#define POOL_NDEBUG !ENABLE_POOL_DEBUGGING_RELEASE // is pool debugging disabled? pool:no-debug
	#define POOL_DEBUG ENABLE_POOL_DEBUGGING_RELEASE // is pool debugging enabled? pool:debug
#else
	#define POOL_NDEBUG !ENABLE_POOL_DEBUGGING_DEBUG // is pool debugging disabled? pool:no-debug
	#define POOL_DEBUG ENABLE_POOL_DEBUGGING_DEBUG // is pool debugging enabled? pool:debug
#endif


#define POOL_GUARDBYTES ((void*)0xCAFED00D)





/**
 * Our memory pool is a SIZE based pool, meaning you specify the size
 * of a single object as the template parameter.
 * If the pool has no more available elements, alloc() returns NULL.
 * You can return objects to the pool by using  dealloc().
 * You can clear the state of the pool with clear() - this will make the
 * allocator forget its free list resets the available number of objects.
 */
class pool
{
public:
	// Size of the actual pool object is 24 bytes,
	// the allocated data follows right after the pool.
	// We could use ushort for available/freed, but that would
	// misalign the data. This favors newer x86/x64 model performance.
	struct node
	{
	#if POOL_NDEBUG
		union {
			node* next;	// free list next pointer
			char data[];	// the actual data
		};
	#else
		// We need the debug facility to detect writes into deleted handles
		// Writing into a handle after deleting it will cause the freed 
		// list to segfault. This will allow us to catch that.
		node* next;		// free list next pointer
		union {
			void* guard;	// guard bytes
			char data[];	// the actual data
		};
	#endif
	};

	unsigned sizeOf;	// sizeof each element
	unsigned available;	// number of unhanded allocations
	unsigned freed;		// number in free list
	node* list;			// free list pointer
	char* end;			// pointer to the end of the buffer (first invalid address)
	int _padding[1];	// padding data to get buffer on 8-byte alignment

	char buffer[];// the actual buffer contained in this object

public:

	/**
	 * Creates a new pool<TSIZE> object. Pool is a variable size object
	 * allocated as sizeof(pool) + TSIZE*capacity + alignto(8192)
	 * @param size_of Size of each element
	 * @param capacity Number of objects to reserve space for
	 * @return A new pool object
	 */
	static pool* create(unsigned size_of, const unsigned capacity)
	{
		#if POOL_DEBUG
			size_of += sizeof(node*); // debug mode needs room for an extra ptr
		#else
			if(size_of < sizeof(node*))
				size_of = sizeof(node*); // ensure minimum size of node* is achieved for release mode
		#endif
		if(size_of % 4 != 0) // have to align to 4 byte boundary?
			size_of = size_of + 4 - size_of % 4;

		// calculate size of this structure + buffer size
		int allocSize = sizeof(pool) + size_of * capacity;
		// align alloc size to a 8192 byte chunk
		allocSize = (allocSize % 8192) ? allocSize + 8192 - (allocSize % 8192) : allocSize;

		// now calculate the 'usable' buffer size for our end pointer
		int usableBuffer = (allocSize - sizeof(pool));	// pool struct is not usable space
		usableBuffer -= usableBuffer % size_of;			// remove any misaligned bytes from the end

		pool* p = (pool*)malloc(allocSize); // this is usally aligned to 8 byte boundary :(
		p->sizeOf = size_of;
		p->available = usableBuffer / size_of;			// number of nodes available
		p->freed = 0;
		p->list = NULL;
		p->end = p->buffer + usableBuffer;
		return p;
	}

	/**
	 * Clears the current state of the pool and resets
	 * all the variables of the pool
	 */
	inline void clear()
	{
		available = (end - buffer) / sizeOf;
		freed = 0;
		list = NULL;
	}

	/**
	 * Destroys the pool
	 */
	inline static void destroy(pool* p)
	{
		free(p);
	}

	// unsafe version for dynamic_pool
	inline void* _alloc_new()
	{
		#if POOL_NDEBUG
			return (buffer + sizeOf * --available);	
		#else
			return ((node*)(buffer + sizeOf * --available))->data;
		#endif
	}

	// unsafe version for dynamic_pool
	inline void* _alloc_freed() // gets from the freed list
	{
		--freed;
		#if POOL_NDEBUG
			void* ptr = (void*)list;
		#else
			ENSURE(list->guard == POOL_GUARDBYTES && "Invalid write to dealloc()-ed pointer detected!");
			void* ptr = list->data;
		#endif
		list = list->next; // unshift the linked list
		return ptr;
	}

	// unsafe version for dynamic_pool
	inline void _dealloc(void* ptr)
	{
		++freed;
		#if  POOL_NDEBUG
			((node*)ptr)->next = list;	// shift the linked list
			list = (node*)ptr;			// store the beginning of the linked list
		#else
			node* n = (node*)((char*)ptr - sizeof(void*));
			n->guard = POOL_GUARDBYTES;
			n->next = list;
			list = n;
		#endif
	}

	/**
	 * @return A new object from this pool, or NULL if no more handles available.
	 */
	inline void* alloc() // gets a new item
	{
		if(freed) 
		{
			--freed;
			#if  POOL_NDEBUG
				void* ptr = (void*)list;
			#else
				ENSURE(list->guard == POOL_GUARDBYTES && "Invalid write to dealloc()-ed pointer detected!");
				void* ptr = list->data; 
			#endif
			list = list->next; // unshift the linked list
			return ptr;
		} 
		else if(available) 
		{
			#if POOL_NDEBUG
				return (buffer + sizeOf * --available);	
			#else
				return ((node*)(buffer + sizeOf * --available))->data;
			#endif
		}
		return NULL; // its empty
	}

	/**
	 * Deallocates a pointer
	 * @param ptr Pointer 
	 */
	inline void dealloc(void* ptr) // puts into the freed list
	{
		++freed;
		#if POOL_NDEBUG
			((node*)ptr)->next = list;	// shift the linked list
			list = (node*)ptr;			// store the beginning of the linked list
		#else
			ENSURE(is_owner(ptr) && "Given pointer does not belong to this pool!");
			node* n = (node*)((char*)ptr - sizeof(void*));
			n->guard = POOL_GUARDBYTES;
			n->next = list;
			list = n;
		#endif
	}

	/**
	 * @return TRUE if the pointer is in range of this pool
	 */
	inline bool is_owner(void* ptr) const
	{
		return buffer <= ptr && ptr < end;
	}

	/**
	 * @return Number of objects currently allocated
	 */
	inline unsigned alloc_count() const
	{
		return ((end - buffer) / sizeOf) - (available + freed); // reserved - free
	}

	/**
	 * @return Number of objects available to allocate
	 */
	inline unsigned free_count() const
	{
		return available + freed;
	}

	/**
	 * @return Number of objects reserved
	 */
	inline unsigned reserve_count() const
	{
		return (end - buffer) / sizeOf;
	}

	/**
	 * @return Number of objects currently allocated
	 */
	inline unsigned alloc_bytes() const
	{
		return (end - buffer) - (available + freed) * sizeOf; // reserved - free
	}

	/**
	 * @return Number of objects available to allocate
	 */
	inline unsigned free_bytes() const
	{
		return (available + freed) * sizeOf;
	}

	/**
	 * @return Number of objects reserved
	 */
	inline unsigned reserve_bytes() const
	{
		return end - buffer;
	}
};








/**
 * Dynamic pool is a memory pool that dynamically increases it size
 * to handle more and more requests whenever needed.
 *
 * @param GC [no_gc] Set to [use_gc] if you wish this dynamic_pool to trigger 
 *           garbage collection if a pool* gets empty
 */
template<pool_gc_t GC = no_gc> class dynamic_pool
{
public:
	pool** pools;
	unsigned pools_count;
	unsigned pools_capacity;
	unsigned pool_sizeOf;		// size in bytes of each element
	unsigned pool_sizehint;		// size hint for pool objects in number of elements


	/**
	 * Creates a new pool and automatically calculates pool size (quite aggressively).
	 */
	inline dynamic_pool(unsigned sizeOf) : pools(0), pools_count(0), pools_capacity(0), 
		pool_sizeOf(sizeOf), pool_sizehint(POOL_AUTOSIZE(sizeOf))
	{
	}

	/**
	 * Creates a new pool with the given pool size hint
	 */
	inline dynamic_pool(unsigned sizeOf, unsigned poolSizeHint) : pools(0), pools_count(0), pools_capacity(0), 
		pool_sizeOf(sizeOf), pool_sizehint(poolSizeHint)
	{
	}

	/**
	 * Destroys the entire pool
	 */
	inline ~dynamic_pool()
	{
		destroy();
	}

	/**
	 * This function destroys all the pools in this dynamic_pool
	 * The container array for pool objects is also destroyed
	 */
	void destroy()
	{
		int i = pools_count;
		if(!i) return; // early return if no pools
		while(i)
			pool::destroy(pools[--i]);

		free(pools);
		pools = NULL;
		pools_count = 0;
		pools_capacity = 0;
	}

	/**
	 * @return A new memory block of size <TSIZE>
	 */
	void* alloc() // gets a new handle
	{
		int i = pools_count;
		while(i) // backwards iteration
		{
			pool* p = pools[--i];
			if(p->freed) // any in the 'freed' list?
				return p->_alloc_freed();
			else if(p->available) // any in the buffer itself?
				return p->_alloc_new();
		}

		if(pools_count == pools_capacity)
		{
			pools_capacity += 4; // this is really bad
			pools = (pool**)realloc(pools, sizeof(pool*) * pools_capacity);
		}
		pool* p = pool::create(pool_sizeOf, pool_sizehint);
		pools[pools_count++] = p;
		return p->_alloc_new(); // this will definitely succeed
	}

	/**
	 * Deallocates a pointer by returning it to the pool
	 */
	void dealloc(void* ptr) // deletes an existing handle
	{
		int i = pools_count;
		while(i) // backwards iteration
		{
			pool* p = pools[--i];
			if(p->is_owner(ptr)) // belongs to this pool?
			{	
				p->_dealloc(ptr); // good. put it there.

				// @note We need general garbage collection for the _global_pool objects
				// run garbage collection if and only if:
				// 1) GC is enabled for this template
				// 2) there are more than 1 pools
				// 3) the pool is now empty
				if(GC && pools_count != 1 && p->alloc_count() == 0)
					erase_at(i);
				return;
			}
		}

		// on the global bucket_allocator this happens if you dealloc() from wrong thread
		// otherwise it's an invalid pointer or belongs to another pool
		ENSURE(false && "Pointer does not belong to this dynamic_pool!");
	}

	/**
	 * Clears all the pools. (!) Does NOT free any memory (!) Pools have their max capacity restored!
	 */
	void clear()
	{
		for(int i = pools_count; i; )
			pools[--i]->clear();
	}

	/**
	 * Destroys pools that are empty
	 */
	void clean_pools()
	{
		int i = pools_count;
		if(!i) return; // early return to avoid the free block in the end
		while(i)
		{
			pool* p = pools[--i];
			if(p->alloc_count() == 0)
			{
				erase_at(i);
				continue;
			}
		}
		if(pools_count == 0)
		{
			free(pools), pools = NULL;
			pools_capacity = 0;
		}
	}


private:
	void erase_at(int i) // erases the pool at the given index
	{
		pool** at = pools + i;
		pool::destroy(*at);
		if(int itemsToShift = pools_count - i - 1)
			memmove(at, at + 1, itemsToShift * sizeof(pool*)); // unshift the pools array
		--pools_count;
	}


public:
	/**
	 * @return Number of currently allocated objects
	 */
	unsigned alloc_count() const
	{
		unsigned count = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			count += pools[i]->alloc_count();
		return count;
	}
	
	/**
	 * @return Number of objects available for allocation
	 */
	unsigned free_count() const
	{
		unsigned count = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			count += pools[i]->free_count();
		return count;
	}

	/**
	 * @return Total number of objects reserved
	 */
	unsigned reserve_count() const
	{
		unsigned count = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			count += pools[i]->reserve_count();
		return count;
	}

	/**
	 * @return Number of bytes currently allocated
	 */
	unsigned alloc_bytes() const
	{
		unsigned numBytes = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			numBytes += pools[i]->alloc_bytes();
		return numBytes;
	}

	/**
	 * @return Number of bytes available for allocation
	 */
	unsigned free_bytes() const
	{
		unsigned numBytes = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			numBytes += pools[i]->free_bytes();
		return numBytes;
	}

	/**
	 * @return Number of bytes reserved for allocations
	 */
	unsigned reserve_bytes() const
	{
		unsigned numBytes = 0;
		for(unsigned i = 0; i < pools_count; ++i)
			numBytes += pools[i]->reserve_bytes();
		return numBytes;
	}
};





/**
 * Global pools use garbage collection (automatic cleanup on low capacity)
 */
typedef dynamic_pool<use_gc> global_pool_t;





/**
 * Gets a Thread-Local-Storage Pool suitable for the specified size request.
 * @note This only works for requestSize range [0, 1024]. Otherwise undefined behaviour. Use wisely.
 * @param requestSize Size of the memory request in bytes.
 * @return A Thread-Local-Storage Pool for this size request.
 */
global_pool_t* _get_tls_pool(uint32_t requestSize);





/**
 * A bucket allocator is a special allocator that divides allocations into fixed-size memory pools.
 * These pools are thread-local, so an assertion failure is triggered if delete is called from a wrong thread.
 * @note This special allocator pools allocations between [4..1024] bytes
 * @note General vector allocator for STL
 */
template<class T> class bucket_allocator : public std::allocator<T>
{
public:
	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T			value_type;
	template<class X> struct rebind { typedef bucket_allocator<X> other; };

	inline bucket_allocator() throw()  {}
	inline bucket_allocator(const bucket_allocator& a) throw() : std::allocator<T>(a) {}
	template<class X> inline bucket_allocator(const bucket_allocator<X>&) throw()  {}
	inline ~bucket_allocator() throw()  {}
	inline bucket_allocator select_on_container_copy_construction() const throw() { return *this; }

	inline pointer address(reference x) const throw()  { return &x; }
	inline const_pointer address(const_reference x) const throw()  { return &x; }
	inline size_type max_size() const throw()  { return size_t(-1) / sizeof(T); }
	inline void construct(pointer p, const_reference v) { ::new(p) T(v); }
	inline void destroy(pointer p) { p->~T(); (void)p; }

	/**
	 * @note This should be tested for all vectors:
	 * @note The size 1024 bytes is deduced from MTuner profiling data. Everything over that seems to be irrelevant.
	 */
	pointer allocate(size_type n)
	{
		const uint32_t requestSize = n * sizeof(T); // requested bytes
		if(requestSize <= POOL_MAX_BUCKET_SIZE) // default MAX: 1024 bytes
		{
			return (pointer)_get_tls_pool(requestSize)->alloc();
		}
		void* mem = malloc(requestSize);
		if(!mem)
		{
			throw std::bad_alloc();
		}
		return (pointer)mem;
	}

	void deallocate(pointer p, size_type n)
	{
		const uint32_t requestSize = n * sizeof(T); // requested bytes
		if(requestSize <= POOL_MAX_BUCKET_SIZE) // default MAX: 1024 bytes
		{
			_get_tls_pool(requestSize)->dealloc(p);
			return;
		}
		free(p);
	}
};





/**
 * Thread-Local-Storage node, specifically for tls_alloc / tls_free
 */
struct tls_node
{
	global_pool_t* gpool;
	char data[];
};


/**
 * Allocates bytes from Thread-Local-Storage memory pools buckets. An alternative to bucket_allocator<T>
 */
void* tls_alloc(const uint32_t numBytes);


/**
 * Deallocates bytes allocated by the tls_alloc
 */
void tls_free(void* ptr);

extern int TLSALLOCS;




#endif // INCLUDED_MEMORY_POOL