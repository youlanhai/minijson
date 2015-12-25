//
//  reference.ipp
//  minijson
//
//  Created by youlanhai on 15/12/20.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef reference_h
#define reference_h

namespace mjson
{
    JSON_INLINE Reference::Reference()
    : counter_(0)
    {}
    
    JSON_INLINE Reference::~Reference()
    {}
    
    JSON_INLINE void Reference::retain()
    {
        ++counter_;
    }
    
    JSON_INLINE void Reference::release()
    {
        if(--counter_ <= 0)
        {
            delete this;
        }
    }
}

#endif /* reference_h */
