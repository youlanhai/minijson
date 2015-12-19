//
//  json.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef json_hpp
#define json_hpp

#include <cstdlib>
#include <cstring>

#include "config.hpp"

#if JSON_SUPPORT_STL_STRING
#include <string>
#endif

namespace mjson
{
    typedef unsigned int SizeType;
    
#if JSON_USE_LARGE_NUMBER
    typedef long long   Integer;
    typedef double      Float;
#else
    typedef int         Integer;
    typedef float       Float;
#endif
    
    enum Type
    {
        T_NULL      = 0,
        T_BOOL      = 1 << 0,
        T_INT       = 1 << 1,
        T_FLOAT     = 1 << 2,
        
        T_NUMBER    = 1 << 3,
        T_POINTER   = 1 << 4,
        
        T_STRING,
        T_ARRAY,
        T_DICT,
    };
    
    class Node;
    class Object;
    class String;
    class Array;
    class Dict;
    
    struct NodePair;
    
    class Reference
    {
    public:
        Reference();
        virtual ~Reference();
        
        virtual void retain();
        virtual void release();
        
    protected:
        long        counter_;
    };
    
    class IAllocator : public Reference
    {
    public:
        IAllocator();
        ~IAllocator();
        
        virtual void*   malloc(size_t size) = 0;
        virtual void*   realloc(void *p, size_t newSize) = 0;
        virtual void    free(void *p) = 0;
        
        virtual String* createString() = 0;
        virtual Array*  createArray() = 0;
        virtual Dict*   createDict() = 0;
        
        virtual void    freeObject(Object *p) = 0;
    };
    
    class RawAllocator : public IAllocator
    {
    public:
        RawAllocator();
        ~RawAllocator();
        
        virtual void*   malloc(size_t size);
        virtual void*   realloc(void *p, size_t newSize);
        virtual void    free(void *p);
        
        virtual String* createString();
        virtual Array*  createArray();
        virtual Dict*   createDict();
        
        virtual void    freeObject(Object *p);
    };
    
    class Object : public Reference
    {
    public:
        Object(IAllocator *allocator);
        ~Object();
        
        virtual void release();
        virtual Type type() const = 0;

    protected:
        IAllocator*     allocator_;
    };
    
    class String : public Object
    {
    public:
        String(IAllocator *allocator);
        String(const char *str, size_t length, IAllocator *allocator);
        String(const String &str);
        ~String();
        
        void assign(const char *str, size_t length);
        
        const String& operator = (const char *str);
        const String& operator = (const String &str);
        
        bool operator == (const char *str) const;
        bool operator == (const String &str) const;
        
        size_t size() const { return size_; }
        const char* data() const{ return str_; }
        
        virtual Type type() const { return T_STRING; }
        
    private:
        const char*     str_;
        size_t          size_;
    };
    
    class Array : public Object
    {
    public:
        typedef Node            value_type;
        typedef Node*           iterator;
        typedef const Node*     const_iterator;
        
        Array(IAllocator *allocator);
        ~Array();
        
        void reserve(size_t capacity);
        void resize(size_t size);
        
        void insert(iterator it, const value_type &value);
        void erase(iterator it);
        
        void append(const value_type &value);
        void pop();
        
        iterator begin();
        iterator end();
        
        const_iterator begin() const;
        const_iterator end() const;
        
        Node& operator[](size_t index);
        const Node& operator[](size_t index) const;
        
        bool empty() const;
        size_t size() const;
        size_t capacity() const { return capacity_; }
        
        void clear();
        
        virtual Type type() const { return T_ARRAY; }
        
    private:
        value_type*     begin_;
        value_type*     end_;
        size_t          capacity_;
    };
    
    class Dict : public Object
    {
    public:
        typedef NodePair            value_type;
        typedef value_type*         iterator;
        typedef const value_type    const_iterator;
        
        Dict(IAllocator *allocator);
        ~Dict();
        
        iterator begin();
        iterator end();
        
        const_iterator begin() const;
        const_iterator end() const;
        
        void remove(const char *key);
        void erase(iterator it);
        
        iterator find(const char *key);
        const_iterator find(const char *key) const;
        
        Node& operator[] (const char *key);
        const Node& operator[] (const char *key) const;
        
        bool empty() const;
        size_t size() const;
        size_t capacity() const;
        
        void reserve(size_t size);
        
        virtual Type type() const { return T_DICT; }
        
    private:
        value_type*     begin_;
        value_type*     end_;
        size_t          capacity_;
    };
    
    class Node
    {
    public:
        
        Node();
        Node(bool value);
        Node(Integer value);
        Node(Float value);
        Node(Object *p);
        Node(const char *str, IAllocator *allocator);
        Node(const Node &other);
        ~Node();
        
        bool isNull() const;
        bool isBool() const;
        bool isInt() const;
        bool isFloat() const;
        bool isString() const;
        bool isArray() const;
        bool isDict() const;
        bool isPointer() const;
        
        bool        asBool()    const;
        Integer     asInteger() const;
        Float       asFloat()   const;
        String*     asString()  const;
        const char* asCString() const;
        Array*      asArray()   const;
        Dict*       asDict()    const;
        
        void setNull();
        void setBool(bool value);
        void setInteger(Integer value);
        void setFloat(Float value);
        void setString(const char *str, IAllocator *allocator = 0);
        void setString(const char *str, size_t length, IAllocator *allocator = 0);
        void setArray();
        void setDict();
        
        const Node& operator = (bool value);
        const Node& operator = (Integer value);
        const Node& operator = (Float value);
        const Node& operator = (const char *value);
        const Node& operator = (const Object *value);
        const Node& operator = (const Node &value);
        
        bool operator == (const Node &value);
        
        size_t size() const;
        Node clone() const;
        
        Node& operator[] (SizeType index);
        Node& operator[] (const char *key);
        
        
#if JSON_SUPPORT_STL_STRING
        Node(const std::string &value, IAllocator *allocator = 0);
        const Node& operator = (const std::string &value);
        Node& operator[] (const std::string &key);
        
        void setStdString(const std::string &value, IAllocator *allocator = 0);
        void asStdString(std::string &out) const;
        std::string asStdString() const;
#endif

    private:
        struct Value
        {
            union
            {
                bool        b;
                Integer     i;
                Float       f;
                Object*     p;
            };
        };
        
        Value       value_;
        int         type_;
    };
    
    struct NodePair
    {
        String*     key;
        Node        value;
    };
}

#if JSON_CODE_INLINE
#include "json.ipp"
#endif

#endif /* json_hpp */
