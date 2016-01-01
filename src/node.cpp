//
//  node.cpp
//  smartjson
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
    
    const char* Node::asCString() const
    {
        return isString() ? rawString()->data() : "";
    }
    
    Array* Node::asArray() const
    {
        return (Array*)(isArray() ? value_.p : 0);
    }
    
    Dict* Node::asDict() const
    {
        return (Dict*)(isDict() ? value_.p : 0);
    }
    
    String* Node::rawString() const
    {
        return const_cast<String*>((String*)value_.p);
    }
    
    Array* Node::rawArray() const
    {
        return const_cast<Array*>((Array*)value_.p);
    }
    
    Dict* Node::rawDict() const
    {
        return const_cast<Dict*>((Dict*)value_.p);
    }
    
    void Node::setString(const char *str, size_t size, IAllocator *allocator)
    {
        if(0 == allocator)
        {
            allocator = new RawAllocator();
        }
        allocator->retain();
        
        if(0 == size)
        {
            size = strlen(str);
        }
        
        setNull();
        type_ = T_STRING;
        
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
        if(this->isNumber() && other.isNumber())
        {
            if(this->isFloat() && other.isFloat())
            {
                return value_.f == other.value_.f;
            }
            else if(this->isFloat())
            {
                return value_.f == (Float)other.value_.i;
            }
            else if(other.isFloat())
            {
                return (Float)value_.i == other.value_.f;
            }
            else
            {
                return value_.i == other.value_.i;
            }
        }
        
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
    
    size_t Node::size() const
    {
        if(isArray())
        {
            return rawArray()->size();
        }
        else if(isDict())
        {
            return rawDict()->size();
        }
        else if(isString())
        {
            return rawString()->size();
        }
        return 0;
    }
    
    
    Node& Node::operator[] (SizeType index)
    {
        if(isArray() && index < rawArray()->size())
        {
            return (*rawArray())[index];
        }
        
        static Node null;
        null.setNull();
        return null;
    }
    
    Node& Node::operator[] (const char *key)
    {
        if(isDict())
        {
            return (*rawDict())[key];
        }
        
        static Node null;
        null.setNull();
        return null;
    }
    
#if JSON_SUPPORT_STL_STRING
    
    void Node::asStdString(std::string &out) const
    {
        if(isString())
        {
            out.assign(rawString()->data(), rawString()->size());
        }
    }
    
    std::string Node::asStdString() const
    {
        if(isString())
        {
            return std::string(rawString()->data(), rawString()->size());
        }
        return "";
    }
    
#endif
}
