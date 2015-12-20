//
//  array.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef array_hpp
#define array_hpp

#include "object.hpp"

namespace mjson
{
    class Node;
    
    class Array : public Object
    {
        JSON_DISABLE_COPY_ASSIGN(Array);
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
        
        bool equal(const Array *p) const;
        
        virtual Type type() const { return T_ARRAY; }
        virtual Object* clone() const;
        
    private:
        value_type*     begin_;
        value_type*     end_;
        size_t          capacity_;
    };
}

#endif /* array_hpp */
