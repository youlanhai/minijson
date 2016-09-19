﻿#ifndef SMARTJSON_ARRAY_HPP
#define SMARTJSON_ARRAY_HPP

#include "sj_object.hpp"
#include "sj_iterator.hpp"

namespace mjson
{
    class Node;
    
    class Array : public Object
    {
        JSON_DISABLE_COPY_ASSIGN(Array);
    public:
        typedef Node                    value_type;
        typedef Node*                   pointer;
        typedef Iterator<Array, Node>         iterator;
        typedef Iterator<const Array, const Node>   const_iterator;
        
        explicit Array(IAllocator *allocator);
        ~Array();
        
        void reserve(size_t capacity);
        void alignedReserve(size_t capacity);
        void resize(size_t size);
        
        void insert(iterator it, const value_type &value);
        void erase(iterator it);
        
        iterator find(const value_type &value);
        const_iterator find(const value_type &value) const;
        
        void remove(const value_type &value);

        void push(const value_type &value);
        void pop();
        
        iterator begin();
        iterator end();
        
        const_iterator begin() const;
        const_iterator end() const;
        
        value_type& front();
        const value_type& front() const;
        
        value_type& back();
        const value_type& back() const;
        
        value_type& operator[](size_t index);
        const value_type& operator[](size_t index) const;
        
        value_type& at(size_t index);
        const value_type& at(size_t index) const;
        
        bool empty() const;
        size_t size() const;
        size_t capacity() const;
        
        void clear();
        
        bool equal(const Array *p) const;
        
        virtual Type type() const;
        virtual Object* clone() const;
        virtual Object* deepClone() const;
        
    private:
        value_type*     begin_;
        value_type*     end_;
        size_t          capacity_;
    };
}

#if JSON_CODE_INLINE
#include "sj_array.ipp"
#endif

#endif /* SMARTJSON_ARRAY_HPP */
