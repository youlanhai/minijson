#include "sj_allocator_imp.hpp"

#include "sj_string.hpp"
#include "sj_array.hpp"
#include "sj_dict.hpp"

#include <string.h>

namespace mjson
{
    IAllocator* createDefaultAllocator()
    {
        IAllocator *p = new RawAllocator();
        p->retain();
        return p;
    }

    RawAllocator::RawAllocator()
    {
        
    }

    RawAllocator::~RawAllocator()
    {
        
    }
    
    void* RawAllocator::malloc(size_t size)
    {
        return ::malloc(size);
    }
    
    void RawAllocator::free(void *p)
    {
        return ::free(p);
    }
    

    MemoryPoolAllocator::MemoryPoolAllocator(size_t pageSize)
    : slot_(0)
    , pageSize_(pageSize)
    {
        minAllocSize_ = maxSize(sizeof(String), maxSize(sizeof(Array), sizeof(Dict)));

        const size_t align = sizeof(void*);
        minAllocSize_ = ((minAllocSize_ + align - 1) / align) * align;
    }
    
    MemoryPoolAllocator::~MemoryPoolAllocator()
    {
        while(slot_ != NULL)
        {
            MemSlot *p = slot_;
            slot_ = slot_->next;

            ::free(p->addr);
            ::free(p);
        }
    }
    
    MemoryPoolAllocator::MemSlot* MemoryPoolAllocator::newSlot()
    {
        MemSlot *slot = (MemSlot*)::malloc(sizeof(MemSlot));
        slot->addr = (char*)::malloc(pageSize_);
        slot->freeList = NULL;
        slot->allocCount = 0;

        slot->next = slot_;
        slot_ = slot;

        int n = pageSize_ / minAllocSize_;
        for(int i = 0; i < n; ++i)
        {
            MemNode *mp = (MemNode*)(slot->addr + i * minAllocSize_);
            mp->next = slot->freeList;
            slot->freeList = mp;
        }
        return slot;
    }

    void MemoryPoolAllocator::tryFreeSlot(MemSlot * s)
    {
        const int pageCount = pageSize_ / minAllocSize_;
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
            JSON_ASSERT(prev);
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
                
                JSON_ASSERT(s->allocCount >= 0);
                
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
}
