//
//  parser.hpp
//  minijson
//
//  Created by youlanhai on 16/1/1.
//  Copyright © 2016年 youlanhai. All rights reserved.
//

#ifndef parser_hpp
#define parser_hpp

#include "node.hpp"

namespace mjson
{
    class Reader;
    
    class Parser
    {
    public:
        explicit Parser(IAllocator *allocator);
        
        int parse(const char *str, size_t length);
        
        Node getRoot() const;
        
    private:
        
        int parseDict(Node &node, Reader &reader);
        int parseArray(Node &node, Reader &reader);
        int parseNumber(Node &node, Reader &reader);
        int parseString(Node &node, Reader &reader);
        int parseTrue(Node &node, Reader &reader);
        int parseFalse(Node &node, Reader &reader);
        int parseNull(Node &node, Reader &reader);
        int parseValue(Node &node, Reader &reader);
        
        Node            root_;
        IAllocator*     allocator_;
    };
}

#endif /* parser_hpp */
