//
//  string.ipp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include <cstring>

namespace mjson
{
    int String::compare(const char *str) const
    {
        return strncmp(str_, str, size_);
    }
    
    int String::compare(const String *p) const
    {
        size_t minSize = size_ < p->size_ ? size_ : p->size_;
        int ret = memcmp(str_, p->str_, minSize);
        if(ret == 0)
        {
            return (int)size_ - (int)p->size_;
        }
        return ret;
    }
    
    size_t String::size() const
    {
        return size_;
    }
    
    const char* String::data() const
    {
        return str_;
    }
    
    Type String::type() const
    {
        return T_STRING;
    }
}
