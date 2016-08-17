#include "sj_string.hpp"
#include "sj_allocator.hpp"

#if !JSON_CODE_INLINE
#include "sj_string.ipp"
#endif

namespace mjson
{
    static char buffer[1] = {0};

    String::String(const char *str, size_t size, IAllocator *allocator)
    : Object(allocator)
    , str_(buffer)
    , size_(size)
    , managed_(false)
    {
        if(size_ > 0)
        {
            managed_ = true;
            str_ = (char *)allocator_->malloc(size + 1);
            memcpy(str_, str, size);
            str_[size] = 0;
        }
    }
    
    String::String(char *str, size_t size, bool managed, IAllocator *allocator)
    : Object(allocator)
    , str_(str)
    , size_(size)
    , managed_(managed)
    {
    }
    
    String::~String()
    {
        if(managed_)
        {
            allocator_->free(str_);
        }
    }
}
