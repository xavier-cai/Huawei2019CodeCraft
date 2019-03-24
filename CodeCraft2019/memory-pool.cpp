#include "memory-pool.h"

MemoryPool::MemoryPool()
    : head(0), last(0)
{ }

MemoryPool::~MemoryPool()
{
    Release();
}

MemoryPool::MemoryBlock* MemoryPool::CreateMemoryBlock()
{
    MemoryBlock* ret = new MemoryBlock();
    if (head == 0)
    {
        head = ret;
        last = ret;
    }
    else
    {
        last->next = ret;
        last = ret;
    }
    ret->ptr = 0;
    ret->next = 0;
    return ret;
}

void MemoryPool::Release()
{
    while(head != 0)
    {
        MemoryBlock* peek = head;
        head = head->next;
        peek->function(peek->ptr);
        delete peek;
    }
    last = head;
}

MemoryPool MemoryPool::Instance;
