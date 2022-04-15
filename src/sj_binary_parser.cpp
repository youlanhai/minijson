#include "sj_binary_parser.hpp"
#include <algorithm>
#include <unordered_set>
#include <fstream>
#include <sstream>

//const char MAGIC[5] = "\0\0ab";
const uint32_t MAGIC = 0x62610000;
const uint32_t VERSION = 0x0002;

class StringProxy
{
public:
    NS_SMARTJSON StringValue* str_;
    size_t index_ = 0;

    StringProxy(NS_SMARTJSON StringValue * str = nullptr)
        : str_(str)
    {}

    bool operator < (const StringProxy &v)
    {
        return str_->compare(v.str_) < 0;
    }

    bool operator == (const StringProxy &v) const
    {
        return str_->compare(v.str_) == 0;
    }
};

namespace std
{
    template<>
    struct hash<StringProxy>
    {
        typedef StringProxy argument_type;
        typedef size_t result_type;
        size_t operator()(const StringProxy &node) const noexcept
        {
            return node.str_->getHash();
        }
    };
}

NS_SMARTJSON_BEGIN

BinaryParser::BinaryParser(IAllocator *allocator)
    : IParser(allocator)
{
    isBinaryFile_ = true;
}

bool BinaryParser::doParse()
{
    errorOffset_ = 0;

    stringTable_.clear();

    uint32_t magic = readNumber<uint32_t>();
    if (magic != MAGIC)
    {
        return false;
    }

   version_ = readNumber<uint32_t>();
    if (version_ != 1 && version_ != 2)
    {
        return false;
    }

    if (version_ >= 2)
    {
        size_t reserveSize = readNumber<uint16_t>();
        for (size_t i = 0; i < reserveSize; ++i)
        {
            stream_->get();
        }
    }

    bool ret = false;
    do
    {
        if (!parseStringTable())
        {
            break;
        }
        if (!parseValue(root_))
        {
            break;
        }
        ret = true;
    } while (0);

    errorOffset_ = (size_t)stream_->tellg();

    Array().swap(stringTable_);
    return ret;
}

bool BinaryParser::parseValue(Node &node)
{
    uint8_t type = readNumber<uint8_t>();
    bool ret = true;
    switch(type)
    {
        case TP_NONE:
            node.setNull();
            break;
            
        case TP_TRUE:
            node = true;
            break;
        case TP_FALSE:
            node = false;
            break;
            
        case TP_ZERO:
            node = 0;
            break;
        case TP_ONE:
            node = 1;
            break;
        case TP_INT8:
            node = readNumber<int8_t>();
            break;
        case TP_INT16:
            node = readNumber<int16_t>();
            break;
        case TP_INT32:
            node = readNumber<int32_t>();
            break;
        case TP_INT64:
            node = readNumber<int64_t>();
            break;
            
        case TP_FLOAT:
            node = readNumber<float>();
            break;
        case TP_DOUBLE:
            node = readNumber<double>();
            break;
            
        case TP_STR0:
            ret = parseString(node, 0);
            break;
        case TP_STR8:
            ret = parseString(node, readNumber<uint8_t>());
            break;
        case TP_STR16:
            ret = parseString(node, readNumber<uint16_t>());
            break;
        case TP_STR32:
            ret = parseString(node, readNumber<uint32_t>());
            break;
            
        case TP_LIST0:
            node.setArray(allocator_);
            break;
        case TP_LIST8:
            ret = parseArray(node, readNumber<uint8_t>());
            break;
        case TP_LIST16:
            ret = parseArray(node, readNumber<uint16_t>());
            break;
        case TP_LIST32:
            ret = parseArray(node, readNumber<uint32_t>());
            break;
            
        case TP_DICT0:
            node.setDict(allocator_);
            break;
        case TP_DICT8:
            ret = parseDict(node, readNumber<uint8_t>());
            break;
        case TP_DICT16:
            ret = parseDict(node, readNumber<uint16_t>());
            break;
        case TP_DICT32:
            ret = parseDict(node, readNumber<uint32_t>());
            break; 
            
        default:
            return onError(RC_INVALID_TYPE);
    };
    return ret;
}

bool BinaryParser::parseStringTable()
{
    size_t size = 0;
    size_t maxStringLength = 1024;

    if (version_ >= 2)
    {
        size = readNumber<uint32_t>();
        maxStringLength = readNumber<uint32_t>();
    }
    else
    {
        Node node;
        if (!parseValue(node))
        {
            return false;
        }
        size = node.as<uint32_t>();
    }
    stringTable_.reserve(size);

    std::vector<char> buffer;
    buffer.reserve(maxStringLength);
    
    for(size_t i = 0; i < size; ++i)
    {
        size_t length = readNumber<uint16_t>();
        if(length == 0)
        {
            stringTable_.push_back(Node("", 0, allocator_));
        }
        else
        {
            buffer.resize(length);
            stream_->read(buffer.data(), length);
            if(!stream_->good())
            {
                return onError(RC_INVALID_STRING);
            }
            stringTable_.push_back(Node(buffer.data(), length, allocator_));
        }
    }
    return true;
}

bool BinaryParser::parseString(Node &node, size_t size)
{
    if(size > stringTable_.size())
    {
        return onError(RC_INVALID_STRING);
    }
    node = stringTable_[size];
    return true;
}

bool BinaryParser::parseArray(Node &node, size_t size)
{
    Array* arr = node.setArray(allocator_);
    arr->resize(size);
    
    for(size_t i = 0; i < size; ++i)
    {
        if(!parseValue((*arr)[i]))
        {
            return false;
        }
    } 
    return true;
}

bool BinaryParser::parseDict(Node &node, size_t size)
{
    Dict* dict = node.setDict(allocator_);
    dict->reserve(size);
    
    Node key, val;
    for(size_t i = 0; i < size; ++i)
    {
        if(!parseValue(key) || !parseValue(val))
        {
            return false;
        }
        (*dict)[key] = val;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// BinaryWriter
//////////////////////////////////////////////////////////////////////

class StringPool
{
public:
    StringPool() = default;

    size_t size() const { return pool_.size(); }

    size_t getStringIndex(StringValue *str)
    {
        StringProxy key(str);
        auto it = pool_.find(key);
        if (it != pool_.end())
        {
            return it->index_;
        }
        assert(false && "shouldn't reach here!");
        return 0;
    }

    void collectStrings(const Node &node)
    {
        collectStringsRecursively(node);
    }

    void getAndSortStrings(std::vector<const StringProxy*> &ret)
    {
        ret.clear();
        for (const StringProxy &v : pool_)
        {
            ret.push_back(&v);
        }

        std::sort(ret.begin(), ret.end(), [](const StringProxy* a, const StringProxy* b) {
            return a->str_->compare(b->str_) < 0;
            });

        for (size_t i = 0; i < ret.size(); ++i)
        {
            const_cast<StringProxy*>(ret[i])->index_ = i;
        }
    }

    size_t getMaxStringLength() const
    {
        size_t maxLength = 0;
        for (auto & v : pool_)
        {
            maxLength = std::max(maxLength, v.str_->size());
        }
        return maxLength;
    }

private:
    inline void addString(StringValue *str)
    {
        pool_.insert(StringProxy(str));
    }

    void collectStringsRecursively(const Node &node)
    {
        switch (node.getType())
        {
        case T_STRING:
            addString(node.rawString());
            break;
        case T_ARRAY:
            for (const Node & n : node)
            {
                collectStrings(n);
            }
            break;
        case T_DICT:
            for (const auto &pair : node.refDict())
            {
                collectStrings(pair.first);
                collectStrings(pair.second);
            }
            break;
        default:
            break;
        }
    }

    std::unordered_set<StringProxy> pool_;
};

void BinaryWriter::writeInteger(Integer value)
{
#if SJ_USE_LARGE_NUMBER
    static_assert(sizeof(value) == sizeof(int64_t), "invalid Integer size");
    writeInt64(value);
#else
    static_assert(sizeof(value) == sizeof(int32_t), "invalid Integer size");
    writeInt32(value);
#endif
}

void BinaryWriter::writeInt32(int32_t value)
{
    if (value == 0)
    {
        writeType(TP_ZERO);
    }
    else if (value == 1)
    {
        writeType(TP_ONE);
    }
    else if (value < 0)
    {
        if (value >= std::numeric_limits<int8_t>::min())
        {
            writeType(TP_INT8);
            writeNumber((int8_t)value);
        }
        else if (value >= std::numeric_limits<int16_t>::min())
        {
            writeType(TP_INT16);
            writeNumber((int16_t)value);
        }
        else
        {
            writeType(TP_INT32);
            writeNumber((int32_t)value);
        }
    }
    else
    {
        if (value <= std::numeric_limits<int8_t>::max())
        {
            writeType(TP_INT8);
            writeNumber((int8_t)value);
        }
        else if (value <= std::numeric_limits<int16_t>::max())
        {
            writeType(TP_INT16);
            writeNumber((int16_t)value);
        }
        else
        {
            writeType(TP_INT32);
            writeNumber((int32_t)value);
        }
    }
}

void BinaryWriter::writeInt64(int64_t value)
{
    if (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max())
    {
        writeInt32(static_cast<int32_t>(value));
    }
    else
    {
        writeType(TP_INT64);
        writeNumber(value);
    }
}

void BinaryWriter::writeLength(BinaryValueType type0, size_t length)
{
    if(length == 0)
    {
        writeType(type0);
    }
    else if(length <= std::numeric_limits<uint8_t>::max())
    {
        writeType(BinaryValueType(type0 + 1));
        writeNumber((uint8_t)length);
    }
    else if(length <= std::numeric_limits<uint16_t>::max())
    {
        writeType(BinaryValueType(type0 + 2));
        writeNumber((uint16_t)length);
    }
    else
    {
        writeType(BinaryValueType(type0 + 3));
        writeNumber((uint32_t)length);
    }
}

void BinaryWriter::writeValue(const Node &node)
{
    switch (node.getType())
    {
        case T_NULL:
        {
            writeType(TP_NONE);
            break;
        }
        case T_BOOL:
        {
            if (node.rawBool())
            {
                writeType(TP_TRUE);
            }
            else
            {
                writeType(TP_FALSE);
            }
            break;
        }
        case T_INT:
        {
            writeInteger(node.rawInteger());
            break;
        }
        case T_FLOAT:
        {
            Float v = node.rawFloat();
            Float nearV = std::round(v);
            
            if (v == nearV)
            {
                writeInteger(static_cast<Integer>(nearV));
            }
            else
            {
#if SJ_USE_LARGE_NUMBER
                writeType(TP_DOUBLE);
#else
                writeType(TP_FLOAT);
#endif
                writeNumber(v);
            }
            break;
        }
        case T_STRING:
        {
            size_t strIndex = stringPool_->getStringIndex(node.rawString());
            writeLength(TP_STR0, strIndex);
            break;
        }
        case T_ARRAY:
        {
            const Array &arr = node.refArray();
            writeLength(TP_LIST0, arr.size());
            for (const Node & v : arr)
            {
                writeValue(v);
            }
            break;
        }
        case T_DICT:
        {
            std::vector<NodePair> members;
            node.getMembers(members, true);

            writeLength(TP_DICT0, members.size());
            for (const auto & pair : members)
            {
                writeValue(pair.first);
                writeValue(pair.second);
            }
            break;
        }
        default:
            break;
    }
}

void BinaryWriter::writeStringPool()
{
    std::vector<const StringProxy*> strings;
    stringPool_->getAndSortStrings(strings);

    writeNumber((uint32_t)strings.size());
    for (const auto & v : strings)
    {
        const StringValue *str = v->str_;
        writeNumber((uint16_t)str->size());
        stream_->write(str->data(), str->size());
    }
}

BinaryWriter::BinaryWriter()
{
    isBinaryFile_ = true;
}

void BinaryWriter::onWrite(const Node &node)
{
    StringPool stringPool;
    stringPool_ = &stringPool;

    stringPool.collectStrings(node);

    std::vector<const StringProxy*> strings;
    stringPool.getAndSortStrings(strings);

    writeNumber(MAGIC);
    writeNumber(VERSION);

    // 增加一个预留大小，方便前向兼容
    size_t reserveSize = 0;
    writeNumber((uint16_t)reserveSize);
    for (size_t i = 0; i < reserveSize; ++i)
    {
        stream_->write("\0", 1);
    }

    writeNumber((uint32_t)strings.size());
    writeNumber((uint32_t)stringPool.getMaxStringLength());

    for (const auto & v : strings)
    {
        const StringValue *str = v->str_;
        writeNumber((uint16_t)str->size());
        stream_->write(str->data(), str->size());
    }

    writeValue(node);

    stringPool_ = nullptr;
}

NS_SMARTJSON_END
