//
//  parser.cpp
//  minijson
//
//  Created by youlanhai on 16/1/1.
//  Copyright © 2016年 youlanhai. All rights reserved.
//

#include "parser.hpp"

#include "string.hpp"
#include "array.hpp"
#include "dict.hpp"
#include "allocator.hpp"

namespace mjson
{
    class Reader
    {
    public:
        Reader(const char *begin, const char *end)
        : p_(begin)
        , end_(end)
        {}
        
        char read()
        {
            if(p_ < end_)
            {
                return *p_++;
            }
            return 0;
        }
        
        void unget()
        {
            p_ -= 1;
        }
        
        bool empty() const
        {
            return p_ >= end_;
        }
        
        const char* current() const
        {
            return p_;
        }
        
    private:
        const char*     p_;
        const char*     begin_;
        const char*     end_;
    };
    
    enum ResultCode
    {
        RC_OK,
        RC_END_OF_FILE,
        RC_INVALID_JSON,
        RC_INVALID_DICT,
        RC_INVALID_KEY,
        RC_INVALID_ARRAY,
        RC_INVALID_STRING,
        RC_INVALID_NUMBER,
        RC_INVALID_NULL,
        RC_INVALID_TRUE,
        RC_INVALID_FALSE,
    };

    char skipWhiteSpace(Reader &reader)
    {
        char ch;
        do
        {
            ch = reader.read();
        }while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
        return ch;
    }
    
#define ZERO16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    static const char ESCAPE[256] = {
        ZERO16, ZERO16, 0, 0,'\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'/',
        ZERO16, ZERO16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,'\\', 0, 0, 0,
        0, 0,'\b', 0, 0, 0,'\f', 0, 0, 0, 0, 0, 0, 0,'\n', 0,
        0, 0,'\r', 0,'\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        ZERO16, ZERO16, ZERO16, ZERO16, ZERO16, ZERO16, ZERO16, ZERO16
    };
#undef ZERO16
    
    Parser::Parser(IAllocator *allocator)
    : allocator_(allocator)
    {
        allocator_->retain();
    }
    
    int Parser::parse(const char *str, size_t length)
    {
        root_.setNull();
        
        Reader reader(str, str + length);
        
        char firstChar = skipWhiteSpace(reader);
        if(firstChar != '{' && firstChar != '[')
        {
            return RC_INVALID_JSON;
        }
        
        int ret = parseValue(root_, reader);
        if(ret != RC_OK)
        {
            return ret;
        }
        
        char endChar = skipWhiteSpace(reader);
        if(endChar != 0)
        {
            return RC_INVALID_JSON;
        }
        return RC_OK;
    }
    
    int Parser::parseValue(Node &node, Reader &reader)
    {
        int ret = RC_OK;
        do
        {
            char ch = skipWhiteSpace(reader);
            switch (ch)
            {
                case '\0':
                    ret = RC_END_OF_FILE;
                    break;
                    
                case '{':
                    ret = parseDict(node, reader);
                    break;
                    
                case '[':
                    ret = parseArray(node, reader);
                    break;
                    
                case '\"':
                    ret = parseString(node, reader);
                    break;
                    
                case 'n':
                    ret = parseNull(node, reader);
                    break;
                    
                case 't':
                    ret = parseTrue(node, reader);
                    break;
                    
                case 'f':
                    ret = parseFalse(node, reader);
                    break;
                    
                default:
                    reader.unget();
                    ret = parseNumber(node, reader);
                    break;
            }
        }while(ret == 0);
        return ret;
    }
    
    int Parser::parseDict(Node &node, Reader &reader)
    {
        node.setDict(allocator_);
        
        char ch = skipWhiteSpace(reader);
        if(ch == '}')
        {
            return RC_OK;
        }
        reader.unget();
        
        int ret;
        do
        {
            Node key, value;
            ret = parseValue(key, reader);
            if(ret != RC_OK)
            {
                break;
            }
        
            if(!key.isString())
            {
                ret = RC_INVALID_KEY;
                break;
            }
            
            char ch = skipWhiteSpace(reader);
            if(ch != ':')
            {
                ret = RC_INVALID_DICT;
                break;
            }
            
            ret = parseValue(value, reader);
            if(ret != RC_OK)
            {
                break;
            }
        
            node.rawDict()->insert(key, value);
            
            ch = skipWhiteSpace(reader);
            if(ch == '}')
            {
                break;
            }
            else if(ch != ',')
            {
                ret = RC_INVALID_DICT;
            }
        }while(ret == RC_OK);
        
        return ret;
    }
    
    int Parser::parseArray(Node &node, Reader &reader)
    {
        node.setArray(allocator_);
        
        char ch = skipWhiteSpace(reader);
        if(ch == ']')
        {
            return RC_OK;
        }
        reader.unget();
        
        int ret;
        do
        {
            Node child;
            ret = parseValue(child, reader);
            if(ret != RC_OK)
            {
                break;
            }
        
            node.rawArray()->append(child);
            
            char ch = skipWhiteSpace(reader);
            if(ch == '}')
            {
                break;
            }
            else if(ch != ',')
            {
                ret = RC_INVALID_ARRAY;
            }
        }while(ret == RC_OK);
        
        return ret;
    }
    
    int Parser::parseNumber(Node &node, Reader &reader)
    {
        return false;
    }
    
    int Parser::parseString(Node &node, Reader &reader)
    {
        const char *begin = reader.current();
        char ch;
        do
        {
            ch = reader.read();
        }while(ch != 0 && ch != '\"' && ch != '\n');
        
        if(ch != '\"')
        {
            return RC_INVALID_STRING;
        }
    
        int ret = RC_OK;
        char* buffer = (char*)allocator_->malloc(reader.current() - begin + 1);
        char *p = buffer;
        for(; begin != reader.current() && ret == RC_OK; ++p, ++begin)
        {
            if(*begin == '\\')
            {
                ++begin;
                *p = ESCAPE[*begin];
            }
            else
            {
                *p = *begin;
            }
        }
        *p = '\0';
        node = allocator_->createString(buffer, p - buffer);
        return RC_OK;
    }
    
    int Parser::parseTrue(Node &node, Reader &reader)
    {
        if(reader.read() == 'r' &&
           reader.read() == 'u' &&
           reader.read() == 'e')
        {
            node = true;
            return RC_OK;
        }
        return RC_INVALID_TRUE;
    }
    
    int Parser::parseFalse(Node &node, Reader &reader)
    {
        if(reader.read() == 'a' &&
           reader.read() == 'l' &&
           reader.read() == 's' &&
           reader.read() == 'e')
        {
            node = false;
            return RC_OK;
        }
        return RC_INVALID_FALSE;
    }
    
    int Parser::parseNull(Node &node, Reader &reader)
    {
        if(reader.read() == 'u' &&
           reader.read() == 'l' &&
           reader.read() == 'l')
        {
            node.setNull();
            return RC_OK;
        }
        return RC_INVALID_NULL;
    }
   
   
}