#ifndef _stable_array_h
#define _stable_array_h
#include "define.h"
#include "G3D/debug.h"
#include <vector>
#include <algorithm>
#ifdef _MSC_VER
#   include <new>

#   pragma warning (push)
// debug information too long
#   pragma warning( disable : 4312)
#   pragma warning( disable : 4786)
#endif


template <class T, size_t MIN_ELEMENTS = 32>
class StableArray
{
private:
	/** Once the array has been allocated, it will never deallocate the underlying
	array unless MIN_ELEMENTS is set to 0, MIN_BYTES is 0, and the array is empty. */
	static const size_t MIN_BYTES = 32;
	T* data;
	std::vector<int> _invalid;
	size_t              num;
	size_t              numAllocated;
	bool dirty;
	/**
	 Allocates a new array of size numAllocated (not a parameter to the method)
	 and then copies at most oldNum elements from the old array to it.  Destructors are
	 called for oldNum elements of the old array.
	 */
	void realloc(size_t oldNum)
	{
		dirty = true;
		T* oldData = data;
		_invalid.clear();
		// The allocation is separate from the constructor invocation because we don't want 
		// to pay for the cost of constructors until the newly allocated
		// elements are actually revealed to the application.  They 
		// will be constructed in the resize() method.

		data = (T*)malloc(sizeof(T) * numAllocated);
		alwaysAssertM(data, "Memory manager returned NULL: out of memory?");

		// Call the copy constructors
		{
			const size_t N = sd_min(oldNum, numAllocated);
			const T* end = data + N;
			T* oldPtr = oldData;
			for (T* ptr = data; ptr < end; ++ptr, ++oldPtr)
			{

				// Use placement new to invoke the constructor at the location
				// that we determined.  Use the copy constructor to make the assignment.
				const T* constructed = new (ptr) T(*oldPtr);

				(void)constructed;
				debugAssertM(constructed == ptr,
					"new returned a different address than the one provided by Array.");
			}
		}

		// Call destructors on the old array (if there is no destructor, this will compile away)
		{
			const T* end = oldData + oldNum;
			for (T* ptr = oldData; ptr < end; ++ptr)
			{
				ptr->~T();
			}
		}

		free(oldData);
	}

	void _copy(const StableArray &other)
	{
		init(other.num);
		for (size_t i = 0; i < num; ++i)
		{
			data[i] = other.data[i];
		}
	}

	void init(size_t n)
	{
		this->num = 0;
		this->numAllocated = 0;
		data = NULL;
		if (n > 0)
		{
			resize(n);
		}
		else
		{
			data = NULL;
		}
	}
	/** Only compiled if you use the sort procedure. */
	static bool __cdecl compareGT(const T& a, const T& b)
	{
		return a > b;
	}
	/**
	 Returns true iff address points to an element of this array.
	 Used by append.
	 */
	inline bool inArray(const T* address)
	{
		return (address >= data) && (address < data + num);
	}

public:
	/**
	 Assignment operator.  Will be private in a future release because this is slow and can be invoked by accident by novice C++ programmers.
	 If you really want to copy an Array, use the explicit copy constructor.
	 */
	StableArray& operator=(const StableArray& other)
	{
		resize(other.num);
		for (int i = 0; i < (int)num; ++i)
		{
			data[i] = other[i];
		}
		return *this;
	}

	StableArray& operator=(const std::vector<T>& other)
	{
		resize(other.size());
		for (size_t i = 0; i < num; ++i)
		{
			data[i] = other[i];
		}
		return *this;
	}

public:

	/**
	 G3D C++ STL style iterator variable.  Call begin() to get
	 the first iterator, pre-increment (++i) the iterator to get to
	 the next value.  Use dereference (*i) to access the element.
	 */
	typedef T* Iterator;
	/** G3D C++ STL style const iterator in same style as Iterator. */
	typedef const T* ConstIterator;

	/** stl porting compatibility helper */
	typedef Iterator iterator;
	/** stl porting compatibility helper */
	typedef ConstIterator const_iterator;
	/** stl porting compatibility helper */
	typedef T value_type;
	/** stl porting compatibility helper */
	typedef int size_type;
	/** stl porting compatibility helper */
	typedef int difference_type;

	/**
	 C++ STL style iterator method.  Returns the first iterator element.
	 Do not change the size of the array while iterating.
	 */
	Iterator begin()
	{
		return data;
	}

	ConstIterator begin() const
	{
		return data;
	}
	/**
	 C++ STL style iterator method.  Returns one after the last valid iterator
	 element.
	 */
	ConstIterator end() const
	{
		return data + num;
	}

	Iterator end()
	{
		return data + num;
	}

	/**
	 The array returned is only valid until the next append() or resize call, or
	 the Array is deallocated.
	 */
	T* getCArray()
	{
		return data;
	}

	/** Exchanges all data between the two arrays, which are required to have a common MemoryManager.
		This is a convenient
		way to avoid large array copies when handing off data without involving reference counting
		or manual memory management. Beware that pointers or references into the arrays will
		access memory in the <i>other</i> array after the swap. */
	static void swap(StableArray<T, MIN_ELEMENTS>& a, StableArray<T, MIN_ELEMENTS>& b)
	{
		std::swap(a.data, b.data);
		std::swap(a.num, b.num);
		std::swap(a.numAllocated, b.numAllocated);
	}

	/**
	 The array returned is only valid until the next append() or resize call, or
	 the Array is deallocated.
	 */
	const T* getCArray() const
	{
		return data;
	}

	/** Creates a zero length array (no heap allocation occurs until resize). */
	StableArray() : num(0)
	{
		init(0);
	}


	/**  Creates an array containing v0. */
	explicit StableArray(const T& v0)
	{
		init(1);
		(*this)[0] = v0;
	}

	/**
	 Copy constructor.  Copying arrays is slow...perhaps you want to pass a reference or a pointer instead?
	 */
	StableArray(const StableArray& other) : num(0)
	{
		_copy(other);
	}
	/**
   Destructor does not delete() the objects if T is a pointer type
   (e.g. T = int*) instead, it deletes the <B>pointers themselves</B> and
   leaves the objects.  Call deleteAll if you want to dealocate
   the objects referenced.  Do not call deleteAll if <CODE>T</CODE> is not a pointer
   type (e.g. do call Array<Foo*>::deleteAll, do <B>not</B> call Array<Foo>::deleteAll).
   */
	~StableArray()
	{
		// Invoke the destructors on the elements
		for (size_t i = 0; i < num; ++i)
		{
			(data + i)->~T();
		}

		// Set to 0 in case this Array is global and gets referenced during app exit
		data = nullptr;
		num = 0;
		numAllocated = 0;
	}

	/**
	 Removes all elements.  Use resize(0, false) or fastClear if you want to
	 remove all elements without deallocating the underlying array
	 so that future append() calls will be faster.
	 */
	void clear(bool shrink = true)
	{
		resize(0, shrink);
	}
	/**
	 Number of elements in the array.
	 */
	inline int size() const
	{
		return static_cast<int>(num - _invalid.size());
	}

	/**
	 Number of elements in the array.  (Same as size; this is just
	 here for convenience).
	 */
	inline int length() const
	{
		return size();
	}

	/** @param shrinkIfNecessary if false, memory will never be
	  reallocated when the array shrinks.  This makes resizing much
	  faster but can waste memory. Default = true.

	  \sa clear, trimToSize
	*/
	void resize(size_t n, bool shrinkIfNecessary = true)
	{
		alwaysAssertM(n < 0xFFFFFFFF, "This implementation does not support arrays with more than 2^32 elements, although the size in memory may be larger.");
		if (num == n)
		{
			return;
		}

		size_t oldNum = num;
		num = n;
		if (oldNum > num)
			dirty = true;
		// Call the destructors on newly hidden elements if there are any
		for (size_t i = num; i < oldNum; ++i)
		{
			(data + i)->~T();
		}

		// Once allocated, always maintain MIN_ELEMENTS elements or 32 bytes, whichever is higher.
		const size_t minSize = sd_max(MIN_ELEMENTS, (size_t)(MIN_BYTES / sizeof(T)));

		if ((MIN_ELEMENTS == 0) && (MIN_BYTES == 0) && (n == 0) && shrinkIfNecessary)
		{
			// Deallocate the array completely
			numAllocated = 0;
			data = nullptr;
			return;
		}

		if (num > numAllocated)
		{
			// Grow the underlying array

			if (numAllocated == 0)
			{
				// First allocation; grow to exactly the size requested to avoid wasting space.
				numAllocated = n;
				debugAssert(oldNum == 0);
				realloc(oldNum);
			}
			else
			{

				if (num < minSize)
				{
					// Grow to at least the minimum size
					numAllocated = minSize;

				}
				else
				{

					// Increase the underlying size of the array.  Grow aggressively
					// up to 64k, less aggressively up to 400k, and then grow relatively
					// slowly (1.5x per resize) to avoid excessive space consumption.
					//
					// These numbers are tweaked according to performance tests.

					double growFactor = 3.0f;

					size_t oldSizeBytes = numAllocated * sizeof(T);
					if (oldSizeBytes > 10000000)
					{
						// Conserve memory more tightly above 10 MB
						growFactor = 1.2f;
					}
					else if (oldSizeBytes > 400000)
					{
						// Avoid bloat above 400k
						growFactor = 1.5f;
					}
					else if (oldSizeBytes > 64000)
					{
						// This is what std:: uses at all times
						growFactor = 2.0f;
					}
					numAllocated = (num - numAllocated) + (size_t)(numAllocated * growFactor);

					if (numAllocated < minSize)
					{
						numAllocated = minSize;
					}
				}

				realloc(oldNum);
			}

		}
		else if ((num <= numAllocated / 3) && shrinkIfNecessary && (num > minSize))
		{
			// Shrink the underlying array

			// Only copy over old elements that still remain after resizing
			// (destructors were called for others if we're shrinking)
			realloc(sd_min(num, oldNum));

		}

		// Call the constructors on newly revealed elements.
		// Do not use parens because we don't want the intializer
		// invoked for POD types.
		for (size_t i = oldNum; i < num; ++i)
		{
			new (data + i) T;
		}
	}
	void flush()
	{
		dirty = false;
	}
	bool IsDirty() const
	{
		return  dirty;
	}
	/**
	 Returns true if the given element is in the array.
	 */
	bool contains(const T& e) const
	{
		for (int i = 0; i < size(); ++i)
		{
			if ((*this)[i] == e)
			{
				return true;
			}
		}

		return false;
	}
	/**
	   "The member function returns the storage currently allocated to hold the controlled
		sequence, a value at least as large as size()"
		For compatibility with std::vector.
	*/
	int capacity() const
	{
		return (int)numAllocated;
	}
	/**
	 Performs bounds checks in debug mode
	 */
	inline T& operator[](int n)
	{
		debugAssertM((n >= 0) && (n < (int)num), "Array index out of bounds");
		debugAssert(data != NULL);
		return data[n];
	}

	inline T& operator[](uint32 n)
	{
		debugAssertM(n < (uint32)num, "Array index out of bounds");
		return data[n];
	}

	inline T& operator[](uint64 n)
	{
		debugAssertM(n < (uint64)num, "Array index out of bounds");
		return data[n];
	}

	/**
	 Performs bounds checks in debug mode
	 */
	inline const T& operator[](int n) const
	{
		debugAssert((n >= 0) && (n < (int)num));
		debugAssert(data != NULL);
		return data[n];
	}

	inline const T& operator[](uint32 n) const
	{
		debugAssert((n < (uint32)num));
		debugAssert(data != NULL);
		return data[n];
	}

	inline const T& operator[](uint64 n) const
	{
		debugAssert((n < (uint64)num));
		debugAssert(data != NULL);
		return data[n];
	}
	/**
	 Finds an element and returns the iterator to it.  If the element
	 isn't found then returns end().
	 */
	Iterator find(const T& value)
	{
		for (int i = 0; i < num; ++i)
		{
			if (data[i] == value)
			{
				return data + i;
			}
		}
		return end();
	}
	ConstIterator find(const T& value) const
	{
		for (int i = 0; i < num; ++i)
		{
			if (data[i] == value)
			{
				return data + i;
			}
		}
		return end();
	}

	/**
	 Removes count elements from the array
	 referenced either by index or Iterator.
	 */
	void remove(Iterator element, int count = 1)
	{
		debugAssert((element >= begin()) && (element < end()));
		debugAssert((count > 0) && (element + count) <= end());
		const int	pos = element - begin();
		for (int i = 0; i < count; i++)
		{
			if (!element[i].GetInvalid())
			{
				element[i].SetInvalid(true);
				_invalid.emplace_back(pos + i);
			}
		}
	}
	void remove(int index, int count = 1)
	{
		debugAssert((index >= 0) && (index < (int)num));
		debugAssert((count > 0) && (index + count <= (int)num));

		remove(begin() + index, count);
	}
	/** Number of bytes used by the array object and the memory allocated for it's data pointer. Does *not*
	  * include the memory of objects pointed to by objects in the data array */
	size_t sizeInMemory() const {
		return sizeof(StableArray<T>) + (sizeof(T) * numAllocated);
	}
	T* Add(const T& value)
	{
		if (_invalid.size() > 0)
		{
			int	pos = _invalid[_invalid.size() - 1];
			_invalid.resize(_invalid.size() - 1);
			data[pos] = value;
			return data + pos;
		}
		if (num < numAllocated)
		{
			// This is a simple situation; just stick it in the next free slot using
			// the copy constructor.
			new (data + num) T(value);
			++num;
			return  data + num - 1;
		}
		if (inArray(&value))
		{
			// The value was in the original array; resizing
			// is dangerous because it may move the value
			// we have a reference to.
			T tmp = value;
			return 	Add(tmp);
		}
		// Here we run the empty initializer where we don't have to, but
		// this simplifies the computation.
		resize(num + 1, false);
		data[num - 1] = value;
		return data + num - 1;
	}
};

/* StableArray::contains for C-arrays */
template<class T> bool contains(const T* array, int len, const T& e) {
	for (int i = len - 1; i >= 0; --i) {
		if (array[i] == e) {
			return true;
		}
	}
	return false;
}
#endif

