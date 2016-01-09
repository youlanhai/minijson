//
//  array.cpp
//  smartjson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include "array.hpp"
#include "node.hpp"
#include "allocator.hpp"

#if !JSON_CODE_INLINE
#include "array.ipp"
#endif

namespace mjson
{
    extern size_t growCapacity(size_t oldSize, size_t newSize);
 
    Array::~Array()
    {
        clear();
        if(capacity_ > 0)
        {
            allocator_->free(begin_);
        }
    }
    
    void Array::reserve(size_t capacity)
    {
        if(capacity > capacity_)
        {
            size_t size = this->size();
            
            size_t bytes = capacity * sizeof(value_type);
            if(capacity_ > 0)
            {
                begin_ = (value_type*) allocator_->realloc(begin_, bytes);
            }
            else
            {
                begin_ = (value_type*) allocator_->malloc(bytes);
            }
            
            end_ = begin_ + size;
            capacity_ = capacity;
        }
    }
    
    void Array::resize(size_t size)
    {
        if(size > capacity_)
        {
            size_t newCapacity = growCapacity(capacity_, size);
            reserve(newCapacity);
        }
        
        iterator newEnd = begin() + size;
        if(newEnd > end())
        {
            for(iterator it = end(); it != newEnd; ++it)
            {
                new (it) value_type;
            }
        }
        else if(newEnd < end())
        {
            for(iterator it = newEnd; it != end(); ++it)
            {
                it->~value_type();
            }
        }
        
        end_ = newEnd;
    }
    
    void Array::insert(iterator it, const value_type &value)
    {
        ptrdiff_t index = it - begin();
        resize(size() + 1);
        it = begin() + index;
        
        for(iterator i = end() - 1; i > it; --i)
        {
            memcpy(i, i - 1, sizeof(value_type));
        }
        new (it) value_type(value);
    }
    
    void Array::erase(iterator it)
    {
        if(it != end_)
        {
            it->~value_type();
            memmove(it, it + 1, (end_ - it) * sizeof(value_type));
            --end_;
        }
    }
    
    void Array::pop()
    {
        assert(!empty());
        erase(end_ - 1);
    }
    
    Object* Array::clone() const
    {
        Array *p = allocator_->createArray();
        p->reserve(this->size());
        
        for(const_iterator it = begin(); it != end(); ++it)
        {
            new (p->end_++) value_type(*it);
        }
        return p;
    }
    
    Object* Array::deepClone() const
    {
        Array *p = allocator_->createArray();
        p->reserve(this->size());
        
        for(const_iterator it = begin(); it != end(); ++it)
        {
            Node tmp = it->deepClone();
            new (p->end_++) value_type(tmp);
        }
        return p;
    }
    
    bool Array::equal(const Array *p) const
    {
        if(this->size() != p->size())
        {
            return false;
        }
        const_iterator it2 = p->begin();
        for(const_iterator it = begin(); it != end(); ++it)
        {
            if(*it != *it2)
            {
                return false;
            }
        }
        return true;
    }
    
    Array::value_type& Array::at(size_t index)
    {
        assert(index < size());
        return  begin_[index];
    }
    
    size_t Array::size() const
    {
        return end_ - begin_;
    }
    
    Array::value_type& Array::front()
    {
        assert(!empty());
        return *begin_;
    }
    
    Array::value_type& Array::back()
    {
        assert(!empty());
        return *(end_ - 1);
    }
    
}
