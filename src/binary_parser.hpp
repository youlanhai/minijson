#ifndef SMARTJSON_BINARY_PARSER_H
#define SMARTJSON_BINARY_PARSER_H

#include "node.hpp"

namespace mjson
{
    class BinaryReader;

    class BinaryParser
    {
    public:
        explicit BinaryParser(IAllocator *allocator = 0);
        ~BinaryParser();
        
        bool parse(const char *str, size_t length);
        
        Node getRoot() const{ return root_; }
        int getErrorCode() const { return errorCode_; }
        int getErrorOffset() const { return errorOffset_; }

    private:
        bool parseValue(Node &node, BinaryReader &reader);
        bool parseStringTable(BinaryReader &reader);
        
        bool parseString(Node &node, size_t size, BinaryReader &reader);
        bool parseArray(Node &node, size_t size, BinaryReader &reader);
        bool parseDict(Node &node, size_t size, BinaryReader &reader);
        
        Node            root_;
        IAllocator*     allocator_;
        int             errorCode_;
        int             errorOffset_;
        Array*          stringTable_;
    };
}

#endif //SMARTJSON_BINARY_PARSER_H
