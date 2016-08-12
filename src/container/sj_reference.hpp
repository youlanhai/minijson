//
//  reference.hpp
//  smartjson
//
//  Created by youlanhai on 15/12/19.
//  Copyright © 2015年 youlanhai. All rights reserved.
//

#ifndef reference_hpp
#define reference_hpp

#include "../config.hpp"

#define JSON_DISABLE_COPY_ASSIGN(CLASS) \
    CLASS(const CLASS&); \
    const CLASS& operator=(const CLASS&)

namespace mjson
{
    class Reference
    {
    public:
        Reference();
        virtual ~Reference();
        
        virtual void retain();
        virtual void release();
        
        long getRefCount() const;
        
    protected:
        long        counter_;
    };
}


#if JSON_CODE_INLINE
#include "reference.ipp"
#endif

#endif /* reference_hpp */
