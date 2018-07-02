#pragma once
#include <new>
#include <iostream>
#include <cstddef> //size_t declaration
using namespace std;

template <class T>
class MemoryPool
{
public:
	//some typedef
	typedef T value_type;					//type value
	typedef T *pointer;						//type pointer
	typedef const T *const_pointer;			//const type pointer
	typedef T &reference;					//type reference
	typedef const T &const_reference;		//const type reference
	typedef size_t size_type;				//size_t alias
	typedef ptrdiff_t difference_type;		//difference of poitner value

	//rebind allocator
	template <class U>
	struct rebind
	{
		typedef MemoryPool<U> other;
	};

	//Memory Pool constructors and destructors
	MemoryPool() {}			//constructor
	MemoryPool(const MemoryPool &) {}		//copy constructor
	template <typename U>		//template copy constructor
	MemoryPool(const MemoryPool<U> &) throw() {}
	~MemoryPool() {}		//destructor

	//allocate memory
	static T *allocate(size_type n = 1, const void *hint = 0);

	//return memory to memory pool or operating system
	static void deallocate(T *p, size_type n = 1);

	//object constructor, copy initialization
	static inline void construct(pointer p, const T &value)
	{
		*p = value;
	}

	//object destructor, call destructor
	static inline void destroy(pointer p)
	{
		p->~T();
	}

	//reload destructor
	static inline void destroy(T *first, T *last)
	{
		for (; first != last; ++first)
		{
			first->~T();
		}
	}

	//get address
	static inline pointer address(reference x)
	{
		return (pointer)&x;
	}

	//get const address
	static inline const_pointer address(const_reference x)
	{
		return (const_pointer)&x;
	}

	//calculate maxsize of memory pool
	inline size_type max_size() const
	{
		return size_type(-1) / sizeof(T);
	}

private:
	static const int Align = 8;                        //bytes for alignment
	static const int Maxbytes = 128;                   //maximum bytes of freelists
	static const int NumberOfLists = Maxbytes / Align; //size of freelists
	static const int NumberOfAddedNode = 20;           //default add 20 nodes to the lists

	static inline size_t ROUND_UP(size_t bytes) { return (((bytes)+(size_t)Align - 1) & ~((size_t)Align - 1)); } //calculate round up quantity by 8

	//union to manage freelists, linked lists
	union Obj {
		union Obj *next;
		char data[1];
	};

	static Obj *freelists[NumberOfLists]; //freelists declaration
	static inline size_t index_freelist(size_t bytes)	//calculate index of freelists according to size specified
	{
		return (((bytes)+(size_t)Align - 1) / (size_t)Align - 1);
	}
	static void *refill(size_t n);                      //padding freelists
	static char *blockAlloc(size_t size, size_t &nobj); //refer to memory block for allocation

	static char *startOfFree; //start address for allocation of memory pool
	static char *endOfFree;   //end address for allocation of memory pool
	static size_t heap_size;  //size of heap for allocation
};

#include "memorypool.cpp"