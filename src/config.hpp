//
//  config.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef config_h
#define config_h

#define JSON_USE_LARGE_NUMBER 1

#define JSON_CODE_INLINE 1

#define JSON_SUPPORT_STL_STRING 1

#if JSON_CODE_INLINE
#define JSON_INLINE inline
#else
#define JSON_INLINE
#endif

#endif /* config_h */
