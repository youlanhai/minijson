//
//  types.hpp
//  smartjson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef types_h
#define types_h

#include "config.hpp"

#include <cstdint>
#include <cstdlib>

namespace mjson
{
    typedef unsigned int SizeType;
    
#if JSON_USE_LARGE_NUMBER
    typedef int64_t     Integer;
    typedef double      Float;
#else
    typedef int         Integer;
    typedef float       Float;
#endif
    
    enum Type
    {
        T_NULL,
        T_BOOL,
        T_INT,
        T_FLOAT,
        
        T_POINTER,
        T_STRING,
        T_ARRAY,
        T_DICT,
    };

    const Float DefaultEpsilon = 0.0000001;
}

#endif /* types_h */
