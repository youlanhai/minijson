//
//  dict.cpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include "dict.hpp"
#include "allocator.hpp"
#include "node.hpp"
#include "string.hpp"

namespace mjson
{
    size_t maxSize(size_t a, size_t b)
    {
        return a > b ? a : b;
    }
    
    size_t growCapacity(size_t oldSize, size_t newSize)
    {
        return maxSize(maxSize(oldSize << 1, 8), newSize);
    }
    
    Dict::Dict(IAllocator *allocator)
    : Object(allocator)
    , begin_(nullptr)
    , end_(nullptr)
    , capacity_(0)
    {
        
    }
    
    Dict::~Dict()
    {
        clear();
        if(capacity_ > 0)
        {
            allocator_->free(begin_);
        }
    }
    
    void Dict::reserve(size_t capacity)
    {
        if(capacity_ < capacity)
        {
            size_t size = this->size();
            
            size_t bytes = capacity * sizeof(value_type);
            if(capacity_ > 0)
            {
                begin_ = (value_type*)allocator_->realloc(begin_, bytes);
            }
            else
            {
                begin_ = (value_type*)allocator_->malloc(bytes);
            }
            
            end_ = begin_ + size;
            capacity_ = capacity;
        }
    }
    
    void Dict::clear()
    {
        for(iterator it = begin(); it != end(); ++it)
        {
            it->~value_type();
        }
        end_ = begin_;
    }
    
    Dict::iterator Dict::find(const char *key)
    {
        iterator it = begin();
        
        for(; it != end() && it->key.asString()->compare(key) != 0; ++it)
        {}
        
        return it;
    }
    
    Node& Dict::operator[] (const char *key)
    {
        iterator it = find(key);
        if(it != end())
        {
            return it->value;
        }
        else
        {
            size_t newSize = size() + 1;
            if(newSize > capacity_)
            {
                size_t newCapacity = growCapacity(capacity_, newSize);
                reserve(newCapacity);
            }
            
            new (end_) value_type();
            end_->key = allocator_->createString(key, strlen(key));
            return (end_++)->value;
        }
    }
    
    void Dict::remove(const char *key)
    {
        erase(find(key));
    }
    
    void Dict::erase(iterator it)
    {
        if(it >= begin() && it < end())
        {
            it->~value_type();
            
            if(it + 1 < end())
            {
                memmove(it, it + 1, end() - it);
            }
            
            --end_;
        }
    }
    
    Object* Dict::clone() const
    {
        Dict *p = allocator_->createDict();
        p->reserve(this->size());
        
        iterator out = p->begin();
        for(const_iterator it = begin(); it != end(); ++it)
        {
            new (out) value_type(*it);
            ++out;
        }
        
        p->end_ = out;
        return p;
    }
    
    bool Dict::equal(const Dict *p) const
    {
        if(this->size() != p->size())
        {
            return false;
        }
        const_iterator it2 = p->begin();
        for(const_iterator it = this->begin(); it != this->end(); ++it)
        {
            if(it->key != it2->key || it->value != it2->value)
            {
                return false;
            }
        }
        return true;
    }
}
