//
//  dict.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef dict_hpp
#define dict_hpp

#include "object.hpp"

namespace mjson
{
    class Node;
    struct NodePair;
    
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
}

#endif /* dict_hpp */
