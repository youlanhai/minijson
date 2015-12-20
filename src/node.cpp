//
//  node.cpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include "node.hpp"
#include "string.hpp"
#include "array.hpp"
#include "dict.hpp"
#include "allocator.hpp"

#if !JSON_CODE_INLINE
#include "node.ipp"
#endif

namespace mjson
{
    
    void Node::setString(const char *str, size_t size, IAllocator *allocator)
    {
        setNull();
        type_ = T_STRING;
        
        if(0 == size)
        {
            size = strlen(str);
        }
        
        if(0 == allocator)
        {
            allocator = new RawAllocator();
        }
        allocator->retain();
        
        value_.p = allocator->createString(str, size);
        value_.p->retain();
        
        allocator->release();
    }
    
    void Node::setArray(IAllocator *allocator)
    {
        setNull();
        type_ = T_ARRAY;
        
        if(0 == allocator)
        {
            allocator = new RawAllocator();
        }
        allocator->retain();
        
        value_.p = allocator->createArray();
        value_.p->retain();
        
        allocator->release();
    }
    
    void Node::setDict(IAllocator *allocator)
    {
        setNull();
        type_ = T_DICT;
        
        if(0 == allocator)
        {
            allocator = new RawAllocator();
        }
        allocator->retain();
        
        value_.p = allocator->createDict();
        value_.p->retain();
        
        allocator->release();
    }
    
    bool Node::operator == (const Node &other) const
    {
        if(type_ != other.type_)
        {
            return false;
        }
        if(type_ == T_NULL)
        {
            return true;
        }
        else if(type_ == T_BOOL)
        {
            return value_.b == other.value_.b;
        }
        else if(type_ == T_INT)
        {
            return value_.i == other.value_.i;
        }
        else if(type_ == T_FLOAT)
        {
            return value_.f == other.value_.f;
        }
        else if(type_ == T_STRING)
        {
            return rawString()->compare(other.rawString()) == 0;
        }
        else if(type_ == T_ARRAY)
        {
            return rawArray()->equal(other.rawArray());
        }
        else if(type_ == T_DICT)
        {
            return rawDict()->equal(other.rawDict());
        }
        else
        {
            assert(false && "shouldn't reach here.");
            return false;
        }
    }
}
