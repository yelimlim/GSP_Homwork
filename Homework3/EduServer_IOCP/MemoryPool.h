#pragma once



/// Ŀ�����ϰ� ������ �Ҵ� �޴� �ֵ��� ���� �޸� ���� �ٿ��ֱ�
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
struct MemAllocInfo : SLIST_ENTRY
{
	MemAllocInfo(int size) : mAllocSize(size), mExtraInfo(-1)
	{}
	
	long mAllocSize; ///< MemAllocInfo�� ���Ե� ũ��
	long mExtraInfo; ///< ��Ÿ �߰� ���� (��: Ÿ�� ���� ���� ��)

}; ///< total 16 ����Ʈ

inline void* AttachMemAllocInfo(MemAllocInfo* header, int size)
{
	//TODO: header�� MemAllocInfo�� ��ģ ������ ���� �ۿ��� ����� �޸� �ּҸ� void*�� ����... ���� ���Ǵ� �� �� DetachMemAllocInfo ����.
    //header->mAllocSize = size;
    //++header;
    //return header;

	/// �̷���...replacement new�� �����ڸ� �ҷ�����Ѵ�.  
	new (header)MemAllocInfo(size);
	return reinterpret_cast<void*>(++header);
}

inline MemAllocInfo* DetachMemAllocInfo(void* ptr)
{
	MemAllocInfo* header = reinterpret_cast<MemAllocInfo*>(ptr);
	--header;
	return header;
}

__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
class SmallSizeMemoryPool
{
public:
	SmallSizeMemoryPool(DWORD allocSize);

	MemAllocInfo* Pop();
	void Push(MemAllocInfo* ptr);
	

private:
	SLIST_HEADER mFreeList; ///< �ݵ�� ù��° ��ġ

	const DWORD mAllocSize;
	volatile long mAllocCount = 0;
};

class MemoryPool
{
public:
	MemoryPool();

	void* Allocate(int size);
	void Deallocate(void* ptr, long extraInfo);

private:
	enum Config
	{
		/// �Ժη� �ٲٸ� �ȵ�. ö���� ����� �ٲ� ��
		MAX_SMALL_POOL_COUNT = 1024/32 + 1024/128 + 2048/256, ///< ~1024���� 32����, ~2048���� 128����, ~4096���� 256����
		MAX_ALLOC_SIZE = 4096
	};

	/// ���ϴ� ũ���� �޸𸮸� ������ �ִ� Ǯ�� O(1) access�� ���� ���̺�
	SmallSizeMemoryPool* mSmallSizeMemoryPoolTable[MAX_ALLOC_SIZE+1];

};

extern MemoryPool* GMemoryPool;


/// ����� ��� �޾ƾ߸� xnew/xdelete ����� �� �ְ�...
//template <class T, class... Args>
struct PooledAllocatable
{
public:

    template <class T, class... Args>
    T* xnew(Args... arg)
    {
        //TŸ���� PooledAllocatable���� ��ȯ�� �������� Ȯ�� -> ��� �޾Ҵ��� Ȯ��
        static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

        //TODO: T* obj = xnew<T>(...); ó�� ����� ���ֵ��� �޸�Ǯ���� �Ҵ��ϰ� ������ �ҷ��ְ� ����.

        //void* alloc = nullptr;

        //�Ҵ��ϰ�
        //alloc = GMemoryPool->Allocate(sizeof(T));

        //������ �ҷ��ֱ� (.... �³�?)
        //alloc = new T(arg...);

        //TODO: ... ...

		///# �̷��� Ǯ���� �Ҵ� ���� ���� replacement new�� �����ڸ� �ҷ���� �Ѵ�.
		void* alloc = GMemoryPool->Allocate(sizeof(T));
		new (alloc)T(arg...);


        return reinterpret_cast<T*>(alloc);
    }

    template <class T>
    void xdelete(T* object)
    {
        static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

        //TODO: object�� �Ҹ��� �ҷ��ְ� �޸�Ǯ�� �ݳ�.

        //�Ҹ��� �θ���
        object->~T();
        //�ݳ�?
        GMemoryPool->Deallocate(object, sizeof(T));

        return;
    }

};