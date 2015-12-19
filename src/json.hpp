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
 
}

#if JSON_CODE_INLINE
#include "json.ipp"
#endif

#endif /* json_hpp */
