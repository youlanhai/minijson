//
//  reference.hpp
//  minijson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef reference_hpp
#define reference_hpp

#include <cstdint>

namespace mjson
{
    class Reference
    {
    public:
        Reference();
        virtual ~Reference();
        
        virtual void retain();
        virtual void release();
        
    protected:
        long        counter_;
    };
}

#endif /* reference_hpp */
