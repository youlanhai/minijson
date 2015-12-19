//
//  types.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef types_h
#define types_h

#include <cstdint>

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
}

#endif /* types_h */
