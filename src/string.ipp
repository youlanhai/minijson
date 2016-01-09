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
        return compare(str, strlen(str));
    }
    
    JSON_INLINE int String::compare(const char *str, size_t length) const
    {
        size_t minSize = size_ < length ? size_ : length;
        int ret = memcmp(str_, str, minSize);
        if(ret == 0)
        {
            return (int)size_ - (int)length;
        }
        return ret;
    }
    
    JSON_INLINE int String::compare(const String *p) const
    {
        return compare(p->str_, p->size_);
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
