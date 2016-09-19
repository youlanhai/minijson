#include "sj_allocator.hpp"
#include "sj_string.hpp"
#include "sj_array.hpp"
#include "sj_dict.hpp"

#include <cstdlib>
#include <new>

namespace mjson
{
    static IAllocator* createDefaultAllocator()
    {
        IAllocator *p = new RawAllocator();
        p->retain();
        return p;
    }
    
    IAllocator::IAllocator()
    {
        
    }
    
    IAllocator::~IAllocator()
    {
        
    }
    
    /*static*/ IAllocator* IAllocator::s_pDefault = createDefaultAllocator();
    
    /*static*/ void IAllocator::setDefaultAllocator(IAllocator *p)
    {
        if(p != 0)
        {
            p->retain();
        }
        
        if(s_pDefault)
        {
            s_pDefault->release();
        }
        
        s_pDefault = p;
    }
    
    
    RawAllocator::RawAllocator()
    {
        
    }
    RawAllocator::~RawAllocator()
    {
        
    }
    
    
    void* RawAllocator::malloc(size_t size)
    {
        return std::malloc(size);
    }
    
    void RawAllocator::free(void *p)
    {
        return std::free(p);
    }
    
    String* RawAllocator::createString(const char *str, size_t size, BufferType type)
    {
        void *p = this->malloc(sizeof(String));
        return new (p)String(str, size, type, this);
    }
    
    Array* RawAllocator::createArray()
    {
        void *p = this->malloc(sizeof(Array));
        return new (p)Array(this);
    }
    
    Dict* RawAllocator::createDict()
    {
        void *p = this->malloc(sizeof(Dict));
        return new (p)Dict(this);
    }
    
    void RawAllocator::freeObject(Object *p)
    {
        p->~Object();
        this->free(p);
    }

}
