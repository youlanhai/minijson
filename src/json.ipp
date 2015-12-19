//
//  json.ipp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

namespace mjson
{
    Reference::Reference()
    : counter_(0)
    {}
    
    Reference::~Reference()
    {}
    
    void Reference::retain()
    {
        ++counter_;
    }
    
    void Reference::release()
    {
        if(--counter_ <= 0)
        {
            delete this;
        }
    }
    
    
    
    Object::Object(IAllocator *allocator)
    : allocator_(allocator)
    {
        allocator_->retain();
    }
    
    Object::~Object()
    {
        allocator_->release();
    }
    
    void Object::release()
    {
        if(--counter_ <= 0)
        {
            allocator_->freeObject(this);
        }
    }
    
    
    RawAllocator::RawAllocator()
    {
        
    }
    RawAllocator::~RawAllocator()
    {
        
    }
    
    void* RawAllocator::malloc(size_t size)
    {
        return std::malloc(size);
    }
    
    void* RawAllocator::realloc(void *p, size_t newSize)
    {
        return std::realloc(p, newSize);
    }
    
    void  RawAllocator::free(void *p)
    {
        return std::free(p);
    }
    
    String* RawAllocator::createString()
    {
        return new String(this);
    }
    
    Array*  RawAllocator::createArray()
    {
        return new Array(this);
    }
    
    Dict*   RawAllocator::createDict()
    {
        return new Dict(this);
    }
    
    void RawAllocator::freeObject(Object *p)
    {
        delete p;
    }
    
    

    Node::Node()
    : type_(T_NULL)
    {
        
    }

    Node::~Node()
    {
        if(isPointer() && value_.p)
        {
            value_.p->release();
        }
    }

    Node::Node(bool value)
    : type_(T_BOOL)
    {
        value_.b = value;
    }

    Node::Node(Integer value)
    : type_(T_NUMBER | T_INT)
    {
        value_.i = value;
    }

    Node::Node(Float value)
    : type_(T_NUMBER | T_FLOAT)
    {
        value_.f = value;
    }

    Node::Node(Object *p)
    : type_(p ? p->type() : T_NULL)
    {
        if(p != 0)
        {
            p->retain();
        }
        value_.p = p;
    }

    Node::Node(const char *str, IAllocator *allocator)
    : type_(T_STRING)
    {
        String *p = allocator->createString();
        p->assign(str, strlen(str));
        
        value_.p = p;
        value_.p->retain();
    }

    Node::Node(const Node &other)
    : type_(other.type_)
    , value_(other.value_)
    {
        if(isPointer() && value_.p)
        {
            value_.p->retain();
        }
    }
    
    bool Node::isPointer() const
    {
        return type_ > T_POINTER;
    }
}
