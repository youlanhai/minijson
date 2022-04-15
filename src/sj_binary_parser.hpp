#pragma once
#include "sj_node.hpp"
#include <iostream>

NS_SMARTJSON_BEGIN

enum BinaryValueType
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

class BinaryParser
{
    SJ_DISABLE_COPY_ASSIGN(BinaryParser);
public:
    explicit BinaryParser(IAllocator *allocator = nullptr);
    ~BinaryParser();
    
    bool parseFromFile(const char *fileName);
    bool parseFromData(const char *str, size_t length);
    bool parseFromStream(std::istream &stream);
    
    Node getRoot() const{ return root_; }
    int getErrorCode() const { return errorCode_; }
    size_t getErrorOffset() const { return errorOffset_; }

private:
    bool parseValue(Node &node);
    bool parseStringTable();
    
    bool parseString(Node &node, size_t size);
    bool parseArray(Node &node, size_t size);
    bool parseDict(Node &node, size_t size);

    template <typename T>
    inline T readNumber()
    {
        T ret;
        stream_->read(reinterpret_cast<char*>(&ret), sizeof(ret));
        return ret;
    }

    Node            root_;
    IAllocator*     allocator_;
    int             errorCode_;
    size_t          errorOffset_;
    Array           stringTable_;
    std::istream*   stream_;
    size_t          version_;
};


class BinaryWriter
{
public:
    BinaryWriter();

    void write(const Node &node, std::ostream &stream);

private:
    void writeValue(const Node& node);

    void writeInteger(Integer value);
    void writeInt32(int32_t value);
    void writeInt64(int64_t value);
    void writeLength(BinaryValueType type, size_t length);
    void writeStringPool();

    inline void writeType(BinaryValueType type)
    {
        writeNumber((int8_t)type);
    }

    template <typename T>
    inline void writeNumber(T value)
    {
        stream_->write(reinterpret_cast<char*>(&value), sizeof(value));
    }

private:
    std::ostream*   stream_;
    class StringPool* stringPool_;
};

NS_SMARTJSON_END
