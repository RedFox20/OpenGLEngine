#pragma once
/** 
 * Copyright (c) 2013 - Jorma Rebane 
 *
 * Smart pointer interface, provides minimal scope 'garbage collection' and allows tedious
 * code to be nice and short.
 * Classes:
 *		
 *			-) scoped_ptr<T>:	A 'dumb' pointer that only lasts during its lifetime. When the pointer goes
 *								out of scope it's deleted (unless it's a NULL).
 *								A scoped_ptr<> takes ownership of a pointer:
 *						// 
 *						scoped_ptr<string*> p1 = new string("p1"); // take ownership of "p1"
 *						scoped_ptr<string*> p2 = new string("p2"); // take ownership of "p2"
 *						p1 = p2; // "p1" is deleted, p2 == NULL
 *
 *			-) smart_ptr<T>:	A smart pointer that actually knows how many other smart_ptr's hold reference
 *								to the pointer. Once all pointers go out of scope and delete themselves, the
 *								underlying pointer is deleted.
 *								
 *								Use smart_pointer::
 *		
 *
 */
#ifndef SMART_PTR_H
#define SMART_PTR_H

template<typename T> struct scoped_ptr
{
	// The encapsulated pointer.
	T* p;

	// Constructs an empty smart pointer.
	inline scoped_ptr() : p(0) {}

	/** 
	 * Assumes ownership of the given instance, if non-null.
	 * @param p An existing pointer, or 0 to create an empty scoped pointer.
	 *	@note The scoped pointer will assume ownership of the given instance. 
	 *	It will delete the object when it goes out of scope (!)
	 */
	inline scoped_ptr(T* p) : p(p) {}

	///**
	// * Assumes ownership of the given instance, if non-null.
	// * @param p An existing pointer, or 0 to create an empty scoped ptr
	// * @note The scoped pointer will assume ownership of the given instance.
	// * It will delete the object when it goes out of scope (!)
	// */
	//template<class U> scoped_ptr(U* p) : p(p) {}

	// Releases the contained instance, but does not reset the point
	inline ~scoped_ptr()
	{
		if(p)
			delete p;
	}

	// Copy-construction. Assumes ownership of this pointer. The other scoped_ptr loses its value.
	inline scoped_ptr(scoped_ptr& ptr)
	{
		if(this == ptr.this_ptr()) // sigh. someone self-assigned during construction.
		{
			p = NULL;
			return;
		}
		p = ptr.p;
		ptr.p = NULL;
	}

	// Gets an object's address, because the addressOf (&x) operator is overriden
	inline const scoped_ptr* this_ptr() const { return this; }

	// Assignment by reference.
	inline scoped_ptr& operator=(scoped_ptr& ptr)
	{
		if(this != ptr.this_ptr())
		{
			if(p) 
				delete p;
			if(p = ptr.p)
				ptr.p = NULL;
		}
		return *this;
	}

	// Assignment by an integer. Always assumes integer is 0, even if not. Same as scoped_ptr::release().
	inline scoped_ptr& operator=(const int i)
	{
		if(p)
		{
			delete p;
			p = NULL;
		}
		return *this;
	}

	// Assignment by a pointer.
	inline scoped_ptr& operator=(T* ptr)
	{
		if(p != ptr)
		{
			if(p)
				delete p;
			p = ptr;
		}
		return *this;
	}

	// Comparison to a pointer
	inline bool operator==(const T* ptr)
	{
		return p == ptr;
	}

	/**
	 * Releases a contained instance, if present.
	 * @note You should never need to call this function unless you wish to
	 * take control a Release an instance before the smart pointer goes out 
	 * of scope.
	 */
	inline void release()
	{
		if(p)
		{
			delete p;
			p = NULL;
		}
	}

	/** 
	 * Gets the address of the pointer.
	 * T *const *
	 */
	inline T*const* operator &() const
	{
		return (T*const*)&p;
	}
	/**
	 * Gets the address of the pointer.
	 * T * *
	 */
	inline T** operator &()
	{
		return &p;
	}

	/**
	 * Gets the address of the const pointer.
	 */
	inline T *const * cref() const
	{
		return (T *const *)&p;
	}

	inline operator T*() const
	{
		return p;
	}

	// Gets the encapsulated pointer.
	inline T* get() const { return p; }

	// Gets the encapsulated pointer.
	inline T* operator->() { return p; }

	// Gets the encapsulated pointer.
	inline const T* operator->() const { return p; }

	// Swaps the encapsulated pointer with that of the argument.
	inline void swap(scoped_ptr& ptr)
	{
		T* temp = p;
		p = ptr.p;
		ptr.p = temp;
	}

	// Returns true if p != NULL
	inline operator bool() const { return p ? true : false; }

}; // scoped_ptr<>



// smart pointer facilities
namespace smart_pointer
{
	// Gets the number of currently 'active' smart pointers
	size_t live_count();

	// Creates a new 'shared' reference handle. The reference is allocated
	// from a dedicated 'lazily' initialized pool. This is very memory efficient,
	// where every handle takes exactly sizeof(unsigned) bytes.
	// Initial *value is '1'
	unsigned* new_handle();
	// Releases a shared handle back to the pool. 
	void delete_handle(unsigned* h);
};



template<typename T> struct smart_ptr
{
private:
	// The encapsulated pointer.
	T* p;
	// The current reference count (if any)
	unsigned* nrefs;
public:
	// Constructs an empty smart pointer.
	inline smart_ptr() : p(NULL), nrefs(NULL) {}

	/** 
	 * Assumes ownership of the given instance, if non-null.
	 * @param p An existing pointer, or 0 to create an empty smart pointer.
	 * @note The smart pointer will assume ownership of the given instance. 
	 * It will delete the object when it goes out of scope (!)
	 */
	inline smart_ptr(T* p) : p(p) 
	{
		nrefs = p ? smart_pointer::new_handle() : NULL;
	}

	///** 
	// * Assumes ownership of the given instance, if non-null.
	// * @param p An existing pointer, or 0 to create an empty smart pointer.
	// * @note The smart pointer will assume ownership of the given instance. 
	// * It will delete the object when it goes out of scope (!)
	// */
	//template<class U> inline smart_ptr(U* p) : p(p)
	//{
	//	nrefs = p ? smart_pointer::new_handle() : NULL;
	//}

	// Releases the contained instance, but does not reset the point
	inline ~smart_ptr()
	{
		if(nrefs && !--*nrefs)
		{
			smart_pointer::delete_handle(nrefs);
			delete p;
		}
	}

	// returns the number of references this smart_ptr shares (including itself)
	// if the reference is a NULL, then refcount() is also 0
	inline size_t refcount() const
	{
		return nrefs ? *nrefs : 0;
	}

	/**
	 * Decreases the reference count of this instance and releases the object if it's at null.
	 * @note You should never need to call this function unless you wish to
	 * take control a Release an instance before the smart pointer goes out 
	 * of scope.
	 */
	inline void release()
	{
		if(nrefs && !--*nrefs)
		{
			smart_pointer::delete_handle(nrefs);
			delete p;
			nrefs = 0, p = 0; // null-ing is mandatory in this case...
		}
	}

	// Copy-construction. Assumes ownership of this pointer. The other scoped_ptr loses its value.
	inline smart_ptr(const smart_ptr& ptr)
	{
		if(this == ptr.this_ptr()) // sigh. someone self-assigned during construction.
		{
			p = NULL;
			refcount = NULL;
			return;
		}
		p = ptr.p;
		nrefs = ptr.nrefs;
		addref(); // increase refcount
	}

	// Construction from a scoped_ptr. Assumes ownership of this pointer. The scoped_ptr loses its value.
	inline smart_ptr(scoped_ptr<T>& ptr)
	{
		p = ptr.p;
		nrefs = smart_pointer::new_handle();
		ptr.p = NULL;
	}

	// Gets an object's address, because the addressOf (&x) operator is overriden
	inline const smart_ptr* this_ptr() const { return this; }

	// Assignment by reference.
	inline smart_ptr& operator=(const smart_ptr& ptr)
	{
		if(this != ptr.this_ptr())
		{
			if(nrefs && !--*nrefs) // manually inlined .release()
			{
				smart_pointer::delete_handle(nrefs);
				delete p;
			}
			p = ptr.p;
			if(nrefs = ptr.nrefs)
				++*nrefs; // increase refcount
		}
		return *this;
	}

	// Assignment by an integer. Always assumes integer is 0, even if not. Same as smart_ptr::release().
	inline smart_ptr& operator=(const int i)
	{
		if(nrefs && !--*nrefs) // manually inlined .release()
		{
			smart_pointer::delete_handle(nrefs);
			delete p;
			nrefs = 0, p = 0;
		}
		return *this;
	}

	// Assignment by a pointer.
	inline smart_ptr& operator=(T* ptr)
	{
		if(p != ptr)
		{
			if(nrefs && !--*nrefs) // manually inline .release();
			{
				smart_pointer::delete_handle(nrefs);
				delete p;
			}
			nrefs = (p = ptr) ? smart_pointer::new_handle() : NULL; // yes. (p = ptr) is intentional
		}
		return *this;
	}

	// Comparison to a pointer
	inline bool operator==(const T* ptr)
	{
		return p == ptr;
	}

	/** 
	 * Gets the address of the pointer.
	 * T *const *
	 */
	inline T*const* operator &() const
	{
		return (T*const*)&p;
	}
	/**
	 * Gets the address of the pointer.
	 * T * *
	 */
	inline T** operator &()
	{
		return &p;
	}

	/**
	 * Gets the address of the const pointer.
	 */
	inline T *const * cref() const
	{
		return (T *const *)&p;
	}

	inline operator T*() const
	{
		return p;
	}


	// Gets the encapsulated pointer.
	inline T* get() const { return p; }

	// Gets the encapsulated pointer.
	inline T* operator->() { return p; }

	// Gets the encapsulated pointer.
	inline const T* operator->() const { return p; }

	// Returns true if p != NULL
	inline operator bool() const { return p ? true : false; }


}; // scoped_ptr<>



template<class T> inline void swap_ptr(smart_ptr<T>& a, smart_ptr<T>& b)
{
	a.swap(b);
}

#endif // SCOPED_PTR_H