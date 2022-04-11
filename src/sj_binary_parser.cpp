#include "sj_binary_parser.hpp"

const char MAGIC[5] = "\0\0ab";
const uint32_t VERSION = 0x0001;

namespace mjson
{
    const char* BinaryReader::readBytes(size_t length)
    {
        if(p_ + length <= end_)
        {
            const char *ret = p_;
            p_ += length;
            return ret;
        }
        else
        {
            p_ = end_;
            return 0;
        }
    }

    BinaryParser::BinaryParser(IAllocator *allocator)
    : allocator_(allocator)
    , errorCode_(RC_OK)
    , errorOffset_(0)
    {
        if(allocator_ == NULL)
        {
            allocator_ = IAllocator::getDefaultAllocator();
        }
        allocator_->retain();
    }
    
    BinaryParser::~BinaryParser()
    {
        allocator_->release();
    }

    bool BinaryParser::parseFromFile(const char *fileName)
    {
        FILE *fp = fopen(fileName, "r");
        if(fp == NULL)
        {
            errorCode_ = RC_OPEN_FILE_ERROR;
            return false;
        }
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        std::vector<char> buffer(length);
        fread(buffer.data(), 1, length, fp);
        
        fclose(fp);
        fp = NULL;

        return parseFromData(buffer.data(), length);
    }

    bool BinaryParser::parseFromData(const char *str, size_t length)
    {
        root_.setNull();
        
        errorCode_ = RC_OK;
        errorOffset_ = 0;

        stringTable_.clear();

        BinaryReader reader(str, length);
        
        const char *magic = reader.readBytes(4);
        if(magic == NULL || memcmp(magic, MAGIC, 4) != 0)
        {
            return false;
        }
        
        uint32_t version = reader.readNumber<uint32_t>();
        if(version != VERSION)
        {
            return false;
        }
        
        bool ret = false;
        do
        {
            if(!parseStringTable(reader))
            {
                break;
            }
            if(!parseValue(root_, reader))
            {
                break;
            }
            ret = true;
        }while(0);
        
        Array().swap(stringTable_);
        return ret;
    }

    bool BinaryParser::parseValue(Node &node, BinaryReader &reader)
    {
        uint8_t type = reader.readNumber<uint8_t>();
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
                node = reader.readNumber<int8_t>();
                break;
            case TP_INT16:
                node = reader.readNumber<int16_t>();
                break;
            case TP_INT32:
                node = reader.readNumber<int32_t>();
                break;
            case TP_INT64:
                node = reader.readNumber<int64_t>();
                break;
                
            case TP_FLOAT:
                node = reader.readNumber<float>();
                break;
            case TP_DOUBLE:
                node = reader.readNumber<double>();
                break;
                
            case TP_STR0:
                ret = parseString(node, 0, reader);
                break;
            case TP_STR8:
                ret = parseString(node, reader.readNumber<uint8_t>(), reader);
                break;
            case TP_STR16:
                ret = parseString(node, reader.readNumber<uint16_t>(), reader);
                break;
            case TP_STR32:
                ret = parseString(node, reader.readNumber<uint32_t>(), reader);
                break;
                
            case TP_LIST0:
                node.setArray(allocator_);
                break;
            case TP_LIST8:
                ret = parseArray(node, reader.readNumber<uint8_t>(), reader);
                break;
            case TP_LIST16:
                ret = parseArray(node, reader.readNumber<uint16_t>(), reader);
                break;
            case TP_LIST32:
                ret = parseArray(node, reader.readNumber<uint32_t>(), reader);
                break;
                
            case TP_DICT0:
                node.setDict(allocator_);
                break;
            case TP_DICT8:
                ret = parseDict(node, reader.readNumber<uint8_t>(), reader);
                break;
            case TP_DICT16:
                ret = parseDict(node, reader.readNumber<uint16_t>(), reader);
                break;
            case TP_DICT32:
                ret = parseDict(node, reader.readNumber<uint32_t>(), reader);
                break; 
                
            default:
                errorCode_ = RC_INVALID_TYPE;
                ret = false;
        };
        return ret;
    }

    bool BinaryParser::parseStringTable(BinaryReader &reader)
    {
        Node node;
        parseValue(node, reader);
        
        size_t size = node.as<size_t>();
        stringTable_.reserve(size);
        
        for(size_t i = 0; i < size; ++i)
        {
            size_t length = reader.readNumber<uint16_t>();
            if(length == 0)
            {
                stringTable_.push_back(Node("", 0, allocator_));
            }
            else
            {
                const char *str = reader.readBytes(length);
                if(str == 0)
                {
                    errorCode_ = RC_INVALID_STRING;
                    return false;
                }
                stringTable_.push_back(Node(str, length, allocator_));
            }
        }
        return true;
    }
    
    bool BinaryParser::parseString(Node &node, size_t size, BinaryReader &reader)
    {
        if(size > stringTable_.size())
        {
            errorCode_ = RC_INVALID_STRING;
            return false;
        }
        node = stringTable_[size];
        return true;
    }
    
    bool BinaryParser::parseArray(Node &node, size_t size, BinaryReader &reader)
    {
        Array* arr = node.setArray(allocator_);
        arr->resize(size);
        
        for(size_t i = 0; i < size; ++i)
        {
            if(!parseValue((*arr)[i], reader))
            {
                return false;
            }
        } 
        return true;
    }
    
    bool BinaryParser::parseDict(Node &node, size_t size, BinaryReader &reader)
    {
        Dict* dict = node.setDict(allocator_);
        dict->reserve(size);
        
        Node key, val;
        for(size_t i = 0; i < size; ++i)
        {
            if(!parseValue(key, reader) || !parseValue(val, reader))
            {
                return false;
            }
            (*dict)[key] = val;
        }
        return true;
    }
}
