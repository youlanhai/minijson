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
    Reference::Reference()
    : counter_(0)
    {}
    
    Reference::~Reference()
    {}
    
    void Reference::retain()
    {
        ++counter_;
    }
    
    void Reference::release()
    {
        if(--counter_ <= 0)
        {
            delete this;
        }
    }
}

#endif /* reference_h */
