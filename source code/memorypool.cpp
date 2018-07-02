#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include "memorypool.h"


//allocate function
template <class T>
typename T *
MemoryPool<T>::allocate(size_type n, const void *hint)
{
	void *ret = 0;
	n = n * sizeof(T); //size transition

	if (n > (size_t)Maxbytes)
	{                    //if size demanded is too large, turn to operating system
		ret = malloc(n); //use malloc for allocation, without initialization
	}
	else
	{                                              //else turn to memory for help
		Obj **cur = freelists + index_freelist(n); //cur point to the optimal freelist in all lists
		Obj *rcur = *cur;                          //get the head of this freelist
		if (rcur == 0)                             //if this freelist is empty
			ret = refill(ROUND_UP(n));             //pad the freelist for larger size
		else
		{
			*cur = rcur->next; //return the first block and modify pointer
			ret = rcur;
		}
	}
	return reinterpret_cast<T *>(ret); //cast pointer and return
}

//return free memory to memory pool rather than to operating system
template <class T>
void MemoryPool<T>::deallocate(T *p, size_type n)
{
	void *ptr = static_cast<void *>(p);
	n = n * sizeof(T); //size transition
	if (n > (size_t)Maxbytes)
		free(ptr); //if the returned size is too large, return to operating system directly
	else
	{                                                     //else return to memory pool
		Obj **myfreelist = freelists + index_freelist(n); //pointer to the optimal freelists
		Obj *cur = (Obj *)ptr;                            //get the head of this freelist
		cur->next = *myfreelist;                          //modify pointer
		*myfreelist = cur;
	}
}

//block allocate function
template <class T>
char *
MemoryPool<T>::blockAlloc(size_t size, size_t &nobj)
{
	char *result;
	size_t bytes_total = size * nobj;            //calculate total bytes required
	size_t bytes_left = endOfFree - startOfFree; //calculate bytes left in the memory pool

	if (bytes_left >= bytes_total)
	{                               //if the bytes left is enough
		result = startOfFree;       //record return address
		startOfFree += bytes_total; //modify start address
		return result;              //return address
	}
	else if (bytes_left >= size)
	{                                       //there is still some bytes left for allocation but not enough
		nobj = (size_t)(bytes_left / size); //calculate num of block which can be allocated
		bytes_total = nobj * size;          //calculate total bytes which can be allocated
		result = startOfFree;               //modified pointer
		startOfFree += bytes_total;
		return result;
	}
	else
	{                                                                     //there is no memory for allocating even one block, turn to operating system for memory
		size_t bytes_to_get = 2 * bytes_total + ROUND_UP(heap_size >> 4); //total bytes to apply for from operating system
		if (bytes_left > 0)
		{ //utilize the memory left
			Obj **myfreelist = freelists + index_freelist(bytes_left);
			((Obj *)startOfFree)->next = *myfreelist; //modify pointers
			*myfreelist = (Obj *)startOfFree;
		}
		//turn to operating system for memory
		startOfFree = (char *)malloc(bytes_to_get);
		if (0 == startOfFree)
		{ //if malloc failed
			size_t i;
			Obj **myfreelist;
			Obj *p;
			for (i = size; i <= (size_t)Maxbytes; i += Align)
			{
				myfreelist = freelists + index_freelist(i);
				p = *myfreelist;
				if (0 != p)
				{
					*myfreelist = p->next;
					endOfFree = startOfFree + i;
					return (blockAlloc(size, nobj));
				}
			}
			endOfFree = 0;
			startOfFree = (char *)malloc(bytes_to_get);
		}
		heap_size += bytes_to_get; //modify size of memory pool
		endOfFree = startOfFree + bytes_to_get;
		return blockAlloc(size, nobj);
	}
}

template <class T>
void *
MemoryPool<T>::refill(size_t size)			//padding block for allocation
{
	size_t num = NumberOfAddedNode;
	char *block = blockAlloc(size, num);
	Obj **myfreelist = 0;
	Obj *cur = 0;
	Obj *nextnode = 0;

	if (num == 1)
	{ //if only add one block
		return block;
	}
	else
	{ //add default number of blocks
		myfreelist = freelists + index_freelist(size);
		*myfreelist = nextnode = reinterpret_cast<Obj *>(block + size); //pointer to next block
		for (int i = 1;; i++)
		{
			cur = nextnode;
			nextnode = reinterpret_cast<Obj *>(reinterpret_cast<char *>(cur) + size);

			if (num - 1 == i)
			{
				cur->next = 0;
				break;
			}
			else
			{
				cur->next = nextnode;
			}
		}
		return block;
	}
}

template <typename T> char* MemoryPool<T>::startOfFree = 0;		//static data member initialization
template <typename T> char* MemoryPool<T>::endOfFree = 0;		//static data member initialization
template <typename T> size_t MemoryPool<T>::heap_size = 0;		//static data member initialization 
template <typename T> typename MemoryPool<T>::Obj* MemoryPool<T>::freelists[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

#endif // !MEMORYPOOL_H