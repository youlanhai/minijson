#ifndef SMARTJSON_ALLOCATOR_HPP
#define SMARTJSON_ALLOCATOR_HPP

#include "sj_reference.hpp"
#include <cstdlib>

namespace mjson
{
    class Object;
    class String;
    class Array;
    class Dict;
    
    class IAllocator : public Reference
    {
    public:
        IAllocator();
        ~IAllocator();
        
        virtual void*   malloc(size_t size) = 0;
        virtual void*   realloc(void *p, size_t newSize) = 0;
        virtual void    free(void *p) = 0;
        
        virtual String* createString(const char *str, size_t size) = 0;
        virtual Array*  createArray() = 0;
        virtual Dict*   createDict() = 0;
        
        virtual String* createRawString(char *str, size_t size, bool managed) = 0;
        
        virtual void    freeObject(Object *p) = 0;
    };
    
    class RawAllocator : public IAllocator
    {
    public:
        RawAllocator();
        ~RawAllocator();
        
        virtual void*   malloc(size_t size);
        virtual void*   realloc(void *p, size_t newSize);
        virtual void    free(void *p);
        
        virtual String* createString(const char *str, size_t size);
        virtual Array*  createArray();
        virtual Dict*   createDict();
        
        virtual String* createRawString(char *str, size_t size, bool managed);
        
        virtual void    freeObject(Object *p);
        
        static RawAllocator* defaultAllocator()
        {
            if(s_pDefault == NULL)
            {
                s_pDefault = new RawAllocator();
                s_pDefault->retain();
            }
            return s_pDefault;
        }
    private:
        static RawAllocator *s_pDefault;
    };
}


#endif /* SMARTJSON_ALLOCATOR_HPP */
