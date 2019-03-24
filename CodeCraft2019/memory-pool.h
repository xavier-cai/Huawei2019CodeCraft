#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

class MemoryPool
{
private:
    template <typename _T>
    static void Delete(void* &ptr)
    {
        delete (_T*)ptr;
        ptr = 0;
    }
    template <typename _T>
    static void DeleteArray(void* &ptr)
    {
        delete[] (_T*)ptr;
        ptr = 0;
    }
    typedef void (*DeleteFunction)(void*&);

    struct MemoryBlock
    {
        void* ptr;
        DeleteFunction function;
        MemoryBlock* next;
    };//struct MemoryBlock
    
    MemoryBlock* head;
    MemoryBlock* last;
    
    MemoryBlock* CreateMemoryBlock();
    
public:
    MemoryPool();
    ~MemoryPool();
    
    template <typename _T>
    _T* Manage(_T* ptr)
    {
        MemoryBlock* mem = CreateMemoryBlock();
        mem->ptr = ptr;
        mem->function = &Delete<_T>;
        return ptr;
    }

    template <typename _T>
    _T* ManageArray(_T* ptr)
    {
        MemoryBlock* mem = CreateMemoryBlock();
        mem->ptr = ptr;
        mem->function = &DeleteArray<_T>;
        return ptr;
    }
    
    template <typename _T>
    _T* New()
    {
        return Manage(new _T());
    }

    template <typename _T>
    _T* NewArray(int size)
    {
        return ManageArray(new _T[size]);
    }

    
    void Release();
    
    static MemoryPool Instance;
    
};//class MemoryPool

#endif
