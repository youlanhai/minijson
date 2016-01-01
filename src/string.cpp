//
//  string.cpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include "string.hpp"
#include "allocator.hpp"

#if !JSON_CODE_INLINE
#include "string.ipp"
#endif

namespace mjson
{
    static char buffer[1] = {0};

    String::String(const char *str, size_t size, IAllocator *allocator)
    : Object(allocator)
    , str_(buffer)
    , size_(size)
    {
        if(size_ > 0)
        {
            str_ = (char *)allocator_->malloc(size + 1);
            memcpy(str_, str, size);
            str_[size] = 0;
        }
    }
    
    String::String(char *str, size_t size, bool placeHolder, IAllocator *allocator)
    : Object(allocator)
    , str_(str)
    , size_(size)
    {
    }
    
    String::~String()
    {
        if(size_ > 0)
        {
            allocator_->free(str_);
        }
    }

    Object* String::clone() const
    {
        return allocator_->createString(str_, size_);
    }
}
