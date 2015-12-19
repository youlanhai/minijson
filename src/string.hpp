//
//  string.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include "object.hpp"

namespace mjson
{
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
}

#endif /* string_hpp */
