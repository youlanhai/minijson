#ifndef SMARTJSON_BINARY_PARSER_HPP
#define SMARTJSON_BINARY_PARSER_HPP

#include "sj_node.hpp"

namespace mjson
{
    enum ValueType
    {
        TP_EOF       = 0, //end of file
        TP_NONE      = 1,
        TP_TRUE      = 2,
        TP_FALSE     = 3,
        TP_ZERO      = 4,
        TP_ONE       = 5,

        TP_FLOAT     = 6,
        TP_DOUBLE    = 7,

        TP_INT8      = 8,
        TP_INT16     = 9,
        TP_INT32     = 10,
        TP_INT64     = 11,

        TP_STR0      = 12,
        TP_STR8      = 13,
        TP_STR16     = 14,
        TP_STR32     = 15,

        TP_LIST0     = 16,
        TP_LIST8     = 17,
        TP_LIST16    = 18,
        TP_LIST32    = 19,

        TP_DICT0     = 20,
        TP_DICT8     = 21,
        TP_DICT16    = 22,
        TP_DICT32    = 23,

        TP_MAX       = 24,
    };

    class BinaryReader
    {
    public:
        BinaryReader(const char *data, size_t length)
            : data_(data)
            , p_(data)
            , end_(data + length)
        {}
        
        template<typename T>
        T readNumber()
        {
            T v = T(0);
            if(p_ + sizeof(T) <= end_)
            {
                memcpy(&v, p_, sizeof(T));
            }
            p_ += sizeof(T);
            return v;
        }
        
        const char* readBytes(size_t length);
        
        bool empty() const { return p_ >= end_; }
        
        const char* data() const{ return data_; }
        
    private:
        const char*     data_;
        const char*     p_;
        const char*     end_;
    };

    class BinaryParser
    {
    public:
        explicit BinaryParser(IAllocator *allocator = 0);
        ~BinaryParser();
        
        bool parseFromFile(const char *fileName);
        bool parseFromData(const char *str, size_t length);
        
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
        Array           stringTable_;
    };
}

#endif //SMARTJSON_BINARY_PARSER_HPP
