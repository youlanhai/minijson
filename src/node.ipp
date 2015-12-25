//
//  node.ipp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#include "object.hpp"
#include <cassert>

namespace mjson
{
    
    JSON_INLINE Node::Node()
    : type_(T_NULL)
    {
        
    }
    
    JSON_INLINE Node::~Node()
    {
        setNull();
    }
    
    JSON_INLINE Node::Node(bool value)
    : type_(T_BOOL)
    {
        value_.b = value;
    }
    
    JSON_INLINE Node::Node(Integer value)
    : type_(T_NUMBER | T_INT)
    {
        value_.i = value;
    }
    
    JSON_INLINE Node::Node(Float value)
    : type_(T_NUMBER | T_FLOAT)
    {
        value_.f = value;
    }
    
    JSON_INLINE Node::Node(Object *p)
    : type_(p ? p->type() : T_NULL)
    {
        if(p != 0)
        {
            p->retain();
        }
        value_.p = p;
    }
    
    JSON_INLINE Node::Node(const char *str, size_t size, IAllocator *allocator)
    : type_(T_NULL)
    {
        setString(str, size, allocator);
    }
    
    JSON_INLINE Node::Node(const Node &other)
    : type_(other.type_)
    , value_(other.value_)
    {
        if(isPointer() && value_.p)
        {
            value_.p->retain();
        }
    }
    
    /////////////////////////////////////////////////////////////
    /// types
    /////////////////////////////////////////////////////////////
    
    JSON_INLINE bool Node::isNull() const
    {
        return type_ == T_NULL;
    }
    
    JSON_INLINE bool Node::isBool() const
    {
        return type_ == T_BOOL;
    }
    
    JSON_INLINE bool Node::isInt() const
    {
        return type_ == T_INT;
    }
    
    JSON_INLINE bool Node::isFloat() const
    {
        return type_ == T_FLOAT;
    }
    
    JSON_INLINE bool Node::isString() const
    {
        return type_ == T_STRING;
    }
    
    JSON_INLINE bool Node::isArray() const
    {
        return type_ == T_ARRAY;
    }
    
    JSON_INLINE bool Node::isDict() const
    {
        return type_ == T_DICT;
    }

    JSON_INLINE bool Node::isPointer() const
    {
        return type_ > T_POINTER;
    }
 
    /////////////////////////////////////////////////////////////
    /// conver value to json
    /////////////////////////////////////////////////////////////
    JSON_INLINE void Node::setNull()
    {
        if(isPointer() && value_.p)
        {
            value_.p->release();
        }
        type_ = T_NULL;
    }
    
    JSON_INLINE const Node& Node::operator = (bool value)
    {
        setNull();
        type_ = T_BOOL;
        value_.b = value;
        return *this;
    }
    
    JSON_INLINE const Node& Node::operator = (Integer value)
    {
        setNull();
        type_ = T_INT;
        value_.i = value;
        return *this;
    }
    
    JSON_INLINE const Node& Node::operator = (Float value)
    {
        setNull();
        type_ = T_FLOAT;
        value_.f = value;
        return *this;
    }
    
    JSON_INLINE const Node& Node::operator = (const char *value)
    {
        if(isPointer())
        {
            setString(value, 0, value_.p->getAllocator());
        }
        else
        {
            setString(value);
        }
        return *this;
    }
    
    JSON_INLINE const Node& Node::operator = (const Object *value)
    {
        setNull();
        if(value != 0)
        {
            value_.p = const_cast<Object*>(value);
            value_.p->retain();
        }
        return *this;
    }
    
    JSON_INLINE const Node& Node::operator = (const Node &value)
    {
        setNull();
        
        type_ = value.type_;
        value_ = value.value_;
        if(isPointer())
        {
            value_.p->retain();
        }
        return *this;
    }
    
    /////////////////////////////////////////////////////////////
    /// convert json to value
    /////////////////////////////////////////////////////////////
    
    JSON_INLINE bool Node::asBool() const
    {
        return isBool() ? value_.b : false;
    }
    
    JSON_INLINE Integer Node::asInteger() const
    {
        if(isInt()) return value_.i;
        if(isFloat()) return (Integer)value_.f;
        return 0;
    }
    
    JSON_INLINE Float Node::asFloat() const
    {
        if(isFloat()) return value_.f;
        if(isInt()) return (Float)value_.i;
        return (Float)0;
    }
    
    JSON_INLINE String* Node::asString() const
    {
        return (String*)(isString() ? value_.p : 0);
    }
    
    JSON_INLINE bool Node::operator != (const Node &value) const
    {
        return !(*this == value);
    }
       
    
    JSON_INLINE Node Node::clone() const
    {
        Node ret;
        
        ret.type_ = type_;
        ret.value_ = value_;
        if(isPointer())
        {
            ret.value_.p = value_.p->clone();
            ret.value_.p->retain();
        }
        return ret;
    }
    
#if JSON_SUPPORT_STL_STRING
    JSON_INLINE Node::Node(const std::string &value, IAllocator *allocator)
    : type_(T_NULL)
    {
        setString(value.c_str(), value.size(), allocator);
    }
    
    JSON_INLINE const Node& Node::operator = (const std::string &value)
    {
        setString(value.c_str(), value.size());
        return *this;
    }
    
    JSON_INLINE Node& Node::operator[] (const std::string &key)
    {
        return (*this)[key.c_str()];
    }
    
    JSON_INLINE void Node::setStdString(const std::string &value, IAllocator *allocator)
    {
        return setString(value.c_str(), value.size(), allocator);
    }
   
#endif
    
}
