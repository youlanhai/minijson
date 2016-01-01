//
//  dict.ipp
//  minijson
//
//  Created by youlanhai on 15/12/20.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

namespace mjson
{
    JSON_INLINE Dict::Dict(IAllocator *allocator)
    : Object(allocator)
    , begin_(nullptr)
    , end_(nullptr)
    , capacity_(0)
    {
        
    }
    
    JSON_INLINE Dict::iterator Dict::begin()
    {
        return begin_;
    }
    
    JSON_INLINE Dict::iterator Dict::end()
    {
        return end_;
    }
    
    JSON_INLINE Dict::const_iterator Dict::begin() const
    {
        return begin_;
    }
    
    JSON_INLINE Dict::const_iterator Dict::end() const
    {
        return end_;
    }
    
    JSON_INLINE const Node& Dict::at(const char *key) const
    {
        return const_cast<Dict*>(this)->at(key);
    }
    
    JSON_INLINE Node& Dict::operator[] (const char *key)
    {
        return at(key);
    }
    
    JSON_INLINE const Node& Dict::operator[] (const char *key) const
    {
        return at(key);
    }
    
    JSON_INLINE void Dict::remove(const char *key)
    {
        erase(find(key));
    }
    
    JSON_INLINE bool Dict::empty() const
    {
        return this->size() == 0;
    }
    
    JSON_INLINE size_t Dict::capacity() const
    {
        return capacity_;
    }
    
    JSON_INLINE Type Dict::type() const
    {
        return T_DICT;
    }
}
