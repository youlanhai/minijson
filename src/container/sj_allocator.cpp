#include "sj_allocator.hpp"
#include "sj_string.hpp"
#include "sj_array.hpp"
#include "sj_dict.hpp"

#include <cstdlib>
#include <new>

namespace mjson
{
    IAllocator::IAllocator()
    {
        
    }
    
    IAllocator::~IAllocator()
    {
        
    }
    
    
    RawAllocator::RawAllocator()
    {
        
    }
    RawAllocator::~RawAllocator()
    {
        
    }
    
    /*static*/RawAllocator* RawAllocator::s_pDefault = NULL;
    
    void* RawAllocator::malloc(size_t size)
    {
        return std::malloc(size);
    }
    
    void* RawAllocator::realloc(void *p, size_t newSize)
    {
        return std::realloc(p, newSize);
    }
    
    void  RawAllocator::free(void *p)
    {
        return std::free(p);
    }
    
    String* RawAllocator::createString(const char *str, size_t size)
    {
        void *p = this->malloc(sizeof(String));
        return new (p)String(str, size, this);
    }
    
    Array*  RawAllocator::createArray()
    {
        void *p = this->malloc(sizeof(Array));
        return new (p) Array(this);
    }
    
    Dict*   RawAllocator::createDict()
    {
        void *p = this->malloc(sizeof(Dict));
        return new (p) Dict(this);
    }
    
    
    String* RawAllocator::createRawString(char *str, size_t size)
    {
        void *p = this->malloc(sizeof(String));
        return new (p) String(str, size, true, this);
    }
    
    void RawAllocator::freeObject(Object *p)
    {
        this->retain();
        
        p->~Object();
        this->free(p);
        
        this->release();
    }

}
