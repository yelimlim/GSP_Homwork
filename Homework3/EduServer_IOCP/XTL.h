#pragma once
#include "MemoryPool.h"
#include <list>
#include <vector>
#include <deque>
#include <set>
#include <hash_set>
#include <hash_map>
#include <map>
#include <queue>

template <class T>
class STLAllocator
{
public:
	STLAllocator() = default;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <class U>
	STLAllocator(const STLAllocator<U>&)
	{}

	template <class U>
	struct rebind
	{
		typedef STLAllocator<U> other;
	};

	void construct(pointer p, const T& t)
	{
		new(p)T(t);
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	T* allocate(size_t n)
	{
		//TODO: 메모리풀에서 할당해서 리턴
        return static_cast<T*>(GMemoryPool->Allocate(sizeof(T) * n));
	}

	void deallocate(T* ptr, size_t n)
	{
		//TODO: 메모리풀에 반납
        GMemoryPool->Deallocate(ptr, n);
	}
};


template <class T>
struct xvector
{
	typedef std::vector<T, STLAllocator<T>> type;
};

template <class T>
struct xdeque
{
	//TODO: STL 할당자를 사용하는 deque를 type으로 선언
	///# 이게 되나? 테스트 안해본것 같은데, typedef std::deque<STLAllocator<T>> type;

	typedef std::deque<T, STLAllocator<T>> type;
};

template <class T>
struct xlist : public PooledAllocatable
{
	//TODO: STL 할당자 사용
	typedef std::list<T, STLAllocator<T>> type;
};

template <class K, class T, class C = std::less<K> >
struct xmap
{
	//TODO: STL 할당자 사용하는 map을  type으로 선언
    //std::less<K>로 인해 오름차순으로 정렬된 map이 형성 됨
	///# 이건 틀림.. typedef std::map<K, STLAllocator<T>, C> type;

	typedef std::map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::less<T> >
struct xset
{
	//TODO: STL 할당자 사용하는 set을  type으로 선언
    //위와 같이 오름차순 set
	///# typedef std::set<STLAllocator<T>, C> type;

	typedef std::set<T, C, STLAllocator<T> > type;
};

template <class K, class T, class C = std::hash_compare<K, std::less<K>> >
struct xhash_map
{
	typedef std::hash_map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::hash_compare<T, std::less<T>> >
struct xhash_set
{
	typedef std::hash_set<T, C, STLAllocator<T> > type;
};

template <class T, class C = std::less<std::vector<T>::value_type> >
struct xpriority_queue
{
	//TODO: STL 할당자 사용하는 priority_queue을  type으로 선언
	///# 이렇게 하면 STLAllocator를 쓰지 않게 된다. typedef std::priority_queue<T, std::vector<T>, C> type;

	typedef std::priority_queue<T, std::vector<T, STLAllocator<T>>, C> type;
};

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>> xstring;

