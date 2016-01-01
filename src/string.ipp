//
//  string.ipp
//  smartjson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include <cstring>

namespace mjson
{
    JSON_INLINE int String::compare(const char *str) const
    {
        return strncmp(str_, str, size_);
    }
    
    JSON_INLINE int String::compare(const String *p) const
    {
        size_t minSize = size_ < p->size_ ? size_ : p->size_;
        int ret = memcmp(str_, p->str_, minSize);
        if(ret == 0)
        {
            return (int)size_ - (int)p->size_;
        }
        return ret;
    }
    
    JSON_INLINE size_t String::size() const
    {
        return size_;
    }
    
    JSON_INLINE const char* String::data() const
    {
        return str_;
    }
    
    JSON_INLINE Type String::type() const
    {
        return T_STRING;
    }
    
    JSON_INLINE Object* String::deepClone() const
    {
        return clone();
    }
}
