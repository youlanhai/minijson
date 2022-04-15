#include "sj_allocator_imp.hpp"

#include <cstring>
#include <algorithm>

NS_SMARTJSON_BEGIN

MemoryPoolAllocator::MemoryPoolAllocator(size_t pageSize)
: slot_(0)
, pageSize_(pageSize)
{
    minAllocSize_ = std::max(sizeof(StringValue), std::max(sizeof(Array), sizeof(Dict)));

    const size_t align = sizeof(void*);
    minAllocSize_ = ((minAllocSize_ + align - 1) / align) * align;
}

MemoryPoolAllocator::~MemoryPoolAllocator()
{
    while(slot_ != nullptr)
    {
        MemSlot *p = slot_;
        slot_ = slot_->next;

        ::free(p->addr);
        ::free(p);
    }
}

static char s_emptyStrBuffer[1] = { 0 };

StringValue* MemoryPoolAllocator::createString(const char *str, size_t size, BufferType type)
{
    if (nullptr == str || 0 == size)
    {
        str = s_emptyStrBuffer;
        size = 0;
        type = BT_NOT_CARE;
    }

    if (type == BT_MAKE_COPY)
    {
        char *buffer = (char*)this->malloc(sizeof(StringValue) + size + 1);
        char *s = buffer + sizeof(StringValue);
        memcpy(s, str, size);
        s[size] = 0;
        return new(buffer)StringValue(s, size, this);
    }
    else
    {
        void *p = this->malloc(sizeof(StringValue));
        return new (p)StringValue(str, size, this);
    }
}

ArrayValue* MemoryPoolAllocator::createArray(size_t capacity)
{
    void *p = this->malloc(sizeof(ArrayValue));
    ArrayValue *ret = new (p)ArrayValue(this);
    ret->imp.reserve(capacity);
    return ret;
}

DictValue* MemoryPoolAllocator::createDict(size_t capacity)
{
    void *p = this->malloc(sizeof(DictValue));
    DictValue * ret = new (p)DictValue(this);
    ret->imp.reserve(capacity);
    return ret;
}

void MemoryPoolAllocator::freeObject(IObjectValue *p)
{
    this->retain();
    
    p->~IObjectValue();
    this->free(p);
    
    this->release();
}

MemoryPoolAllocator::MemSlot* MemoryPoolAllocator::newSlot()
{
    MemSlot *slot = (MemSlot*)::malloc(sizeof(MemSlot));
    slot->addr = (char*)::malloc(pageSize_);
    slot->freeList = NULL;
    slot->allocCount = 0;

    slot->next = slot_;
    slot_ = slot;

    size_t n = pageSize_ / minAllocSize_;
    for(size_t i = 0; i < n; ++i)
    {
        MemNode *mp = (MemNode*)(slot->addr + i * minAllocSize_);
        mp->next = slot->freeList;
        slot->freeList = mp;
    }
    return slot;
}

void MemoryPoolAllocator::tryFreeSlot(MemSlot * s)
{
    const int pageCount = (int)(pageSize_ / minAllocSize_);
    int freeNodeCount = 0;

    {
        MemSlot *p = slot_;
        while (p)
        {
            freeNodeCount += pageCount - p->allocCount;
            p = p->next;
        }
    }

    if (freeNodeCount < pageCount + pageCount / 2)
    {
        return;
    }

    // remove s from list
    if (s == slot_)
    {
        slot_ = slot_->next;
    }
    else
    {
        MemSlot *prev = slot_;
        while (prev && s != prev->next)
        {
            prev = prev->next;
        }
        SJ_ASSERT(prev);
        prev->next = s->next;
    }

    ::free(s->addr);
    ::free(s);
}

void* MemoryPoolAllocator::malloc(size_t size)
{
    if(size > minAllocSize_)
    {
        return ::malloc(size);
    }
    
    MemSlot *s = slot_;
    while (s != NULL && NULL == s->freeList)
    {
        s = s->next;
    }
    if (NULL == s)
    {
        s = newSlot();
    }
    
    MemNode *p = s->freeList;
    s->freeList = p->next;
    ++s->allocCount;

    p->next = NULL;
    return p;
}

void MemoryPoolAllocator::free(void *p)
{
    MemSlot *s = slot_;
    while(s)
    {
        if(p >= s->addr && p < s->addr + pageSize_)
        {
            MemNode *mp = (MemNode*)p;
            mp->next = s->freeList;
            s->freeList = mp;
            --s->allocCount;
            
            SJ_ASSERT(s->allocCount >= 0);
            
            if (s->allocCount == 0)
            {
                tryFreeSlot(s);
            }
            return;
        }
        s = s->next;
    }
    
    // not allocated by this allocator
    ::free(p);
}
NS_SMARTJSON_END
