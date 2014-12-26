#pragma once
#include "AstarNode.h"
#include <vector>
using std::vector;


struct node_heap
{
	typedef AstarNode* T;
	T*  Data;      // data buffer
	int Size;      // number of used elements
	int Capacity;  // total capacity

	node_heap() : Data(0), Size(0), Capacity(0)
	{
	}
	node_heap(int capacity) : Data((T*)malloc(sizeof(T*)*capacity)), Size(0), Capacity(0)
	{
	}
	node_heap(const node_heap& other)          = delete; // NOCOPY
	node_heap& operator=(const node_heap& rhs) = delete; // NOCOPY
	~node_heap()
	{
		if (Data) free(Data);
	}
	node_heap(node_heap&& other)
	{
		Data = other.Data, Size = other.Size, Capacity = other.Capacity;
		other.Data = 0, other.Size = 0, other.Capacity = 0;
	}
	node_heap& operator=(node_heap&& fwd)
	{
		T* p = fwd.Data; int s = fwd.Size, c = fwd.Capacity;
		fwd.Data = Data, fwd.Size = Size, fwd.Capacity = Capacity;
		Data = p, Size = s, Capacity = c;
		return *this;
	}

	__forceinline T& operator[](int index) { return Data[index]; }
	__forceinline T operator[](int index) const { return Data[index]; }
	__forceinline int size() const { return Size; }
	__forceinline bool empty() const { return Size == 0; }
	__forceinline void clear() { Size = 0; }

	//// @brief Shifts the item upward as needed
	//// @note  The item does not have to exist, so this also works as insert
	static __forceinline void insert_up(T* ptr, int itemIndex, T item)
	{
		int current = itemIndex;
		int parent  = (current - 1) >> 1; // parent = (i - 1)/2
		const int itemScore = item->FScore;

		while (current > 0) // move upwards in the heap
		{
			T dataParent = ptr[parent];
			if (dataParent->FScore > itemScore)
			{
				ptr[current] = dataParent; // demote parent to current index
				current = parent; // move upwards
				parent  = (current - 1) >> 1; // parent = (i - 1)/2
			}
			else break;
		}
		ptr[current] = item; // write our value
	}

	//// @brief Shifts the item downward as needed
	//// @note  The item does not have to exist, so this also works as insert
	static __forceinline void insert_down(T* ptr, int size, int itemIndex, T item)
	{
		const int last = size - 1;
		int current    = itemIndex;
		int child      = (current << 1) + 1; // left_child = i*2 + 1
		const int itemScore = item->FScore;

		while (child < size) // move downwards in the heap
		{
			T dataChild = ptr[child];
			if (child < last) // test if rightChild is a better option
			{
				T rightChild = ptr[child+1];
				if (dataChild->FScore > rightChild->FScore)
					dataChild = rightChild, ++child;
			}
			if (itemScore > dataChild->FScore)
			{
				ptr[current] = dataChild; // promote child to new parent
				current = child; // move downwards
				child   = (current << 1) + 1; // left_child = i*2 + 1
			}
			else break;
		}
		ptr[current] = item; // write the final value
	}

	void insert(T item)
	{
		if (Size == Capacity)
		{
			int cap = Capacity;
			cap += 4 + (cap >> 1);  // += 50%
			if (int rem = cap % 8) // align up to 8
				cap += 8 - rem;
			Data = (T*)realloc(Data, sizeof(T)*(Capacity = cap));
		}
		insert_up(Data, Size++, item);
	}

	T pop()
	{
		T* ptr = Data;
		T poppedItem = ptr[0];

		// remove the last element and insert it to index 0
		int size = --Size;
		insert_down(ptr, size, 0, ptr[size]);
		return poppedItem;
	}

	void erase(T eraseItem)
	{
		T* ptr = Data;
		const int size = Size;
		for (int current = 0; current < size; ++current) // linear search
		{
			if (ptr[current] == eraseItem)
			{
				// pop last item and replace current item 
				// by inserting the popped item to the current index
				int newSize = --Size;
				insert_down(ptr, newSize, current, ptr[newSize]);
				return; // ok, erased
			}
		}
	}

	// instead of erase/insert, this function simply repositions
	// this item to a new correct index
	void repos(T repoItem)
	{
		T*  ptr  = Data;
		int size = Size;
		for (int current = 0; current < size; ++current) // linear search
		{
			if (ptr[current] == repoItem)
			{
				const int itemScore = repoItem->FScore;

				// parent score must always be lower, otherwise heap will be invalid
				int parent = (current - 1) >> 1; // parent
				if (parent >= 0 && ptr[parent]->FScore > itemScore) // parent is higher instead?
				{
					insert_up(ptr, current, repoItem);
					return;
				}

				// child score must always be higher, otherwise heap will be invalid
				int child = (current << 1) + 1; // left child
				if (child < size && ptr[child]->FScore < itemScore || 
					/*swap to right child:*/ 
					++child < size && ptr[child]->FScore < itemScore)
				{
					insert_down(ptr, size, current, repoItem);
				}
				return; // done
			}
		}
	}

	void print()
	{
		for (int i = 0; i < Size; ++i)
			printf("%d ", Data[i]->FScore);
		printf("\n");
	}

	void print_tree(int node, int level)
	{
		if (node >= Size)
			return;
		int left  = (node <<1 ) + 1;
		int right = left + 1;
		print_tree(left, level + 1);
		printf("%*s%d\n", level * 2, "", Data[node]->FScore);
		print_tree(right, level + 1);
	}
	void print_tree()
	{
		if (Size) print_tree(0, 0);
		else      printf("|-\n");
	}

	bool is_heap() const
	{
		return std::is_heap(Data, Data + Size, [](AstarNode* a, AstarNode* b) {
			return a->FScore > b->FScore;
		});
	}
	void make_heap()
	{
		std::make_heap(Data, Data + Size, [](AstarNode* a, AstarNode* b) {
			return a->FScore > b->FScore;
		});
	}
};

struct node_vect
{
	typedef AstarNode* T;
	static_assert(sizeof(T) == 4, "min_vector<T> sizeof(T) must be 4 for optimization reasons");
	T*  Data;
	int Size;
	int Capacity;

	node_vect() : Data(0), Size(0), Capacity(0)
	{
	}
	node_vect(int capacity) : Data((T*) malloc(sizeof(T)*capacity)), Size(0), Capacity(0)
	{
	}
	node_vect(const node_vect& other) = delete; // NOCOPY
	node_vect& operator=(const node_vect& rhs) = delete; // NOCOPY
	~node_vect()
	{
		if (Data) free(Data);
	}
	node_vect(node_vect&& other)
	{
		Data = other.Data, Size = other.Size, Capacity = other.Capacity;
		other.Data = 0, other.Size = 0, other.Capacity = 0;
	}
	node_vect& operator=(node_vect&& fwd)
	{
		T* p = fwd.Data; int s = fwd.Size, c = fwd.Capacity;
		fwd.Data = Data, fwd.Size = Size, fwd.Capacity = Capacity;
		Data = p, Size = s, Capacity = c;
		return *this;
	}

	__forceinline T& operator[](int index) { return Data[index]; }
	__forceinline const T& operator[](int index) const { return Data[index]; }
	__forceinline int size() const { return Size; }
	__forceinline bool empty() const { return Size == 0; }
	__forceinline void clear() { Size = 0; }

	//// @warning This algorithm always overflows by design to increase speed
	////          so it's only useful for shifting the entire array
	static void ptr_shift(T* ptr, int count)
	{
		T* p = ptr;
		int n = count;
		if (n >= 8)
		{
			__m128 prev1 = _mm_loadu_ps((float*) p);
			__m128 prev2 = _mm_loadu_ps((float*) p + 4);
			do
			{
				__m128 next1 = _mm_loadu_ps((float*) p + 8);
				__m128 next2 = _mm_loadu_ps((float*) p + 12);
				_mm_storeu_ps((float*) p + 1, prev1);
				_mm_storeu_ps((float*) p + 5, prev2);
				prev1 = next1;
				prev2 = next2;
				p += 8;
				n -= 8;
			} while (n >= 8);
			_mm_storeu_ps((float*) p + 1, prev1);
			_mm_storeu_ps((float*) p + 5, prev2);
		}
		else if (n >= 4) // max 7 bytes
		{
			__m128 prev1 = _mm_loadu_ps((float*) p);
			__m128 prev2 = _mm_loadu_ps((float*) p + 4);
			_mm_storeu_ps((float*) p + 1, prev1);
			_mm_storeu_ps((float*) p + 5, prev2);
		}
		else // max 3 bytes
		{
			_mm_storeu_ps((float*) p + 1, _mm_loadu_ps((float*) p));
		}
	}
	//// @warning This algorithm always overflows by design to increase speed
	////          so it's only useful for shifting the entire array
	static void ptr_unshift(T* ptr, int count)
	{
		T* p = ptr;
		int n = count;
		if (n >= 8)
		{
			__m128 prev1 = _mm_loadu_ps((float*) p);
			__m128 prev2 = _mm_loadu_ps((float*) p + 4);
			do
			{
				__m128 next1 = _mm_loadu_ps((float*) p + 8);
				__m128 next2 = _mm_loadu_ps((float*) p + 12);
				_mm_storeu_ps((float*) p - 1, prev1);
				_mm_storeu_ps((float*) p + 3, prev2);
				prev1 = next1;
				prev2 = next2;
				p += 8;
				n -= 8;
			} while (n >= 8);
			_mm_storeu_ps((float*) p - 1, prev1);
			_mm_storeu_ps((float*) p + 3, prev2);
		}
		else if (n >= 4) // max 7 bytes
		{
			__m128 prev1 = _mm_loadu_ps((float*) p);
			__m128 prev2 = _mm_loadu_ps((float*) p + 4);
			_mm_storeu_ps((float*) p - 1, prev1);
			_mm_storeu_ps((float*) p + 3, prev2);
		}
		else // max 3 bytes
		{
			_mm_storeu_ps((float*) p - 1, _mm_loadu_ps((float*) p));
		}
	}

	void insert(T item)
	{
		int size = Size;
		if (size == Capacity)
		{
			int cap = Capacity;
			cap += 4 + (cap >> 1);  // += 50%
			if (int rem = cap % 8) // align up to 8
				cap += 8 - rem;
			int newBytes = sizeof(T) * (Capacity = cap) + 32; // extra bytes for faster ptr_shift
			Data = (T*) realloc(Data, newBytes);
		}

		const int itemValue = item->FScore;
		T* end = Data + size; // end of vector
		T* rend = Data - 1;    // reverse end
		T* ptr = end - 1;    // current pointer
		for (; ptr > rend; --ptr)
		{
			// if last >= item, then we should insert
			if ((*ptr)->FScore >= itemValue)
			{
				++Size;
				++ptr;
				if (int count = end - ptr)
					ptr_shift(ptr, count);
				*ptr = item;
				return;
			}
		}
		// well, insert to start of vector then :/
		ptr = Data;
		++Size;
		if (int count = end - ptr)
			ptr_shift(ptr, count);
		*ptr = item;
	}
	inline T pop()
	{
		return Data[--Size];
	}
	void erase(T item)
	{
		T* end  = Data + Size; // end of vector
		T* rend = Data - 1;    // reverse end
		T* ptr  = end - 1;     // current pointer
		for (; ptr > rend; --ptr)
		{
			if (*ptr == item)
			{
				--Size;
				++ptr;
				if (int count = end - ptr)
					ptr_unshift(ptr, count);
				return;
			}
		}
	}

	inline int get_reposition_direction(T* ptr, T* rend, T* end, int itemScore)
	{
		T* prev = ptr - 1;
		if (prev != rend)
		{
			// prevScore should always be GREATER or equal to itemScore
			const int prevScore = (*prev)->FScore;
			if (prevScore < itemScore)  return -1; // we have to move backwards
			if (prevScore == itemScore) return 0;  // we can stay put, everything is sorted
			// prevScore > itemScore
			//   the backwards part of vector is sorted, but now we have to check if
			//   nextScore is maybe incorrect:
		}
		T* next = ptr + 1;
		if (next < end)
		{
			// nextScore must be SMALLER or equal to itemScore
			if ((*next)->FScore <= itemScore) return 0; // we can stay put, everything is sorted
			// nextScore > itemScore
			//   the forward part of vector is not sorted
			return +1; // we have to move forward
		}
		return 0; // by default we stay put
	}

	// instead of erase/insert, this function simply repositions
	// this item to a new correct index
	void repos(T repoItem)
	{
		T* end  = Data + Size; // end of vector
		T* rend = Data - 1;    // reverse end
		T* ptr  = end - 1;     // current pointer
		for (; ptr > rend; --ptr)
		{
			if (*ptr == repoItem)
			{
				// now decide which way to shift it
				int itemScore = repoItem->FScore;
				int direction = get_reposition_direction(ptr, rend, end, itemScore);
				if (direction < 0) // move backward until sorted
				{
					// --ptr is guaranteed to be > rend
					--ptr;
					do // shift the item backward until sorted
					{
						T prevItem = *ptr;
						if (prevItem->FScore >= itemScore) break; // it's finally sorted
						*ptr   = repoItem;   // swap the items
						ptr[1] = prevItem;
						--ptr;
					} while (ptr > rend);
				}
				else if (direction > 0) // move forward until sorted
				{
					// ++ptr is guaranteed to be < end
					++ptr;
					do // shift the item forward until sorted
					{
						T nextItem = *ptr;
						if (itemScore >= nextItem->FScore) break; // it's finally sorted
						*ptr    = repoItem;
						ptr[-1] = nextItem;
						++ptr;
					} while (ptr < end);
				}
				return; // nothing to be done our vector is sorted
			}
		}
	}
	void print()
	{
		for (int i = 0; i < Size; ++i) printf("%d ", Data[i]->FScore);
		printf("\n");
	}
};







// a fast POD data vector used for pathfinder results
template<class T> struct PfVector
{
	T*  Data;
	int Size;		// Size
	int Capacity;	// Capacity

	inline PfVector() : Data(0), Size(0), Capacity(0) {}
	inline ~PfVector() { if (Data) free(Data); }

	inline T& operator[](int index) { return Data[index]; }
	inline const T& operator[](int index) const { return Data[index]; }
	inline int size()     const { return Size; }
	inline int capacity() const { return Capacity; }
	inline bool empty()   const { return Size == 0; }

	inline T* begin() const { return Data; }
	inline T* end()   const { return Data + Size; }
	inline T& front() { return *Data; }
	inline T& back()  { return Data[Size - 1]; }


	// deallocates the buffer
	void deallocate()
	{
		if (Data) free(Data), Data = 0, Size = 0, Capacity = 0;
	}

	// clears the vector by zeroing the count; no deallocation
	inline void clear()
	{
		Size = 0;
	}

	// erases item at the given index
	inline void erase(int index)
	{
		erase(Data + index);
	}

	// erases item at the given location
	inline void erase(T* ptr)
	{
		_unshift(ptr);
		--Size;
	}

	// erases the first occurrence of item from this vector
	void erase(const T& item)
	{
		T* ptr = Data;
		for (int count = Size; count; --count, ++ptr) {
			if (*ptr == item) {
				_unshift(ptr);
				--Size;
				return; // we only remove one entry
			}
		}
	}

	// pop the last element and return it
	void pop(T& out)
	{
		out = Data[--Size];
	}

	// push an item to the end of the vector
	void push_back(const T& item)
	{
		if (Size == Capacity) // resize required
			_expand();
		Data[Size++] = item;
	}

	//// inserts an item inside the vector at the current place
	//void insert(T* ptr, const T& item)
	//{
	//	T* src;
	//	if (Size == Capacity) // resize required
	//	{
	//		int offset = int(ptr - Data) / sizeof(T);
	//		_expand();
	//		src = Data + offset;
	//	}
	//	else 
	//		src = ptr;
	//	_shift(src);
	//	*src = item;
	//	++Size;
	//}

	// shifts the vector forward from this point
	inline void _shift(T* src)
	{
		T* dst = src + 1; // shift forward --->
		if (size_t sz = size_t(Data + Size) - size_t(src)) // end - src
		{
			memmove(dst, src, sz);
		}
	}

	// unshifts the vector
	inline void _unshift(T* dst)
	{
		T* src = dst + 1; // shift backward <---
		if (size_t sz = size_t(Data + Size) - size_t(src)) // end - src
		{
			memmove(dst, src, sz);
		}
	}

	// internal resize of buffer
	inline void _expand()
	{
		int cap = Capacity;
		cap += 4 + (cap >> 1);  // += 50%
		if (int rem = cap % 8) // align up to 8
			cap += 8 - rem;
		int newBytes = sizeof(T) * (Capacity = cap) + 32; // extra bytes for faster ptr_shift
		Data = (T*) realloc(Data, newBytes);
	}

	// reserves some bytes 
	void reserve(int newCapacity)
	{
		int cap = newCapacity;
		if (int rem = cap % 8) // align up to 8
			cap += 8 - rem;
		int newBytes = sizeof(T) * (Capacity = cap) + 32; // extra bytes for faster ptr_shift
		Data = (T*)realloc(Data, newBytes);
	}
};