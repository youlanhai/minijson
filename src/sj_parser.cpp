#include "sj_parser.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

NS_SMARTJSON_BEGIN

Parser::Parser(IAllocator *allocator)
    : allocator_(allocator)
{
    if (nullptr == allocator_)
    {
        allocator_ = IAllocator::getDefaultAllocator();
    }
    allocator_->retain();
}

Parser::~Parser()
{
    allocator_->release();
}

bool Parser::parseFromFile(const char *fileName)
{
    std::ifstream stream(fileName);
    if (!stream.is_open())
    {
        return onError(RC_OPEN_FILE_ERROR);
    }

    return parseFromStream(stream);
}

bool Parser::parseFromData(const char *str, size_t length)
{
    std::istringstream ss(std::string(str, length));
    return parseFromStream(ss);
}

bool Parser::parseFromStream(std::istream & stream)
{
    stream_ = &stream;
    root_.setNull();
    line_ = 1;
    column_ = 1;
    nextToken_ = 0;
    errorCode_ = RC_OK;

    int firstChar = nextToken();
    if (firstChar == '{')
    {
        parseDict(root_);
    }
    else if (firstChar == '[')
    {
        parseArray(root_);
    }
    else
    {
        onError(RC_INVALID_JSON);
    }

    if (errorCode_ == RC_OK)
    {
        // 检查末尾是否有多余的符号
        if (aheadToken() != 0)
        {
            onError(RC_INVALID_JSON);
        }
    }

    if (errorCode_ != RC_OK)
    {
#ifdef DEBUG
        std::cerr << "parse error: " << errorCode_
            << " line: " << line_
            << " col: " << column_
            << std::endl;
#endif
    }

    stream_ = nullptr;
    return errorCode_ == RC_OK;
}

char Parser::getChar()
{
    char ch = stream_->get();
    switch (ch)
    {
    case '\n':
        ++line_;
        column_ = 1;
        return ch;
    case EOF:
    case '\0':
        return 0;
    default:
        ++column_;
        return ch;
    }
}

void Parser::ungetChar(char ch)
{
    stream_->unget();
    --column_;
    if (ch == '\n')
    {
        --line_;
    }
}

char Parser::translateChar(char ch)
{
    switch (ch)
    {
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 't': return '\t';
    case 'r': return '\r';
    case '"': return '"';
    case '\\': return '\\';
    case '/': return '/';
    default:
        return ch;
    }
}

int Parser::parseToken()
{
    char ch;
    do
    {
        ch = getChar();
    } while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
    return ch;
}

int Parser::nextToken()
{
    if (nextToken_ != 0)
    {
        int ret = nextToken_;
        nextToken_ = 0;
        return ret;
    }
    else
    {
        return parseToken();
    }
}

int Parser::aheadToken()
{
    if (nextToken_ == 0)
    {
        nextToken_ = parseToken();
    }
    return nextToken_;
}

bool Parser::onError(int code)
{
    errorCode_ = code;
    return false;
}

bool Parser::parseValue(Node &node)
{
    while (errorCode_ == RC_OK)
    {
        int ch = nextToken();
        switch (ch)
        {
            case '\0':
                return onError(RC_END_OF_FILE);
                
            case '{':
                return parseDict(node);
                
            case '[':
                return parseArray(node);
                
            case '"':
                return parseString(node);

            case 'n':
                return parseNull(node);
                
            case 't':
                return parseTrue(node);
                
            case 'f':
                return parseFalse(node);
                
            case '/':
                parseComment();
                break;
                
            default:
                return parseNumber(node, ch);
        }
    }
    return false;
}

bool Parser::parseDict(Node &node)
{
    node.setDict(allocator_);
    
    char ch = aheadToken();
    if (ch == '}')
    {
        nextToken();
        return true;
    }
    if (ch == 0)
    {
        return onError(RC_INVALID_DICT);
    }

    while (true)
    {
        Node key, value;
        if (!parseValue(key))
        {
            return false;
        }
    
        if(!key.isString() && !key.isInt())
        {
            return onError(RC_INVALID_KEY);
        }
        
        int ch = nextToken();
        if(ch != ':')
        {
            return onError(RC_INVALID_DICT);
        }
        
        if(!parseValue(value))
        {
            return false;
        }
    
        node.setMember(key, value);
        
        ch = nextToken();
        if (ch == '}')
        {
            return true;
        }
        else if (ch != ',')
        {
            return onError(RC_INVALID_DICT);
        }
    }
    return false;
}

bool Parser::parseArray(Node &node)
{
    node.setArray(allocator_);
    
    char ch = aheadToken();
    if(ch == ']')
    {
        nextToken();
        return true;
    }

    while (true)
    {
        Node child;
        if (!parseValue(child))
        {
            return false;
        }
    
        node.pushBack(child);
        
        char ch = nextToken();
        if(ch == ']')
        {
            return true;
        }
        else if(ch != ',')
        {
            return onError(RC_INVALID_ARRAY);
        }
    }
    return false;
}

static int toHex(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    return -1;
}

static int toOct(char ch)
{
    if (ch >= '0' && ch <= '7')
    {
        return ch - '0';
    }
    return -1;
}

bool Parser::parseNumber(Node &node, char ch)
{
    Integer value = 0;
    double real = 0;
    int exponent = 0;
    int sign = 1;
    bool isFloat = false;

    if (ch == '-')
    {
        sign = -1;
        ch = getChar();
    }
    else if (ch == '+')
    {
        ch = getChar();
    }
    
    if (ch < '0' || ch > '9')
    {
        return onError(RC_INVALID_NUMBER);
    }
    
    if (ch == '0')
    {
        ch = getChar();
        switch (ch)
        {
        case 'x':
        case 'X':
            while (true)
            {
                ch = getChar();
                int v = toHex(ch);
                if (v < 0)
                {
                    break;
                }
                value = (value << 4) | v;
            }
            break;
        case 'o':
        case 'O':
            while (true)
            {
                ch = getChar();
                int v = toOct(ch);
                if (v < 0)
                {
                    break;
                }
                value = (value << 3) | v;
            }
            break;
        case 'b':
        case 'B':
            while (true)
            {
                ch = getChar();
                if (ch == '1')
                {
                    value = (value << 1) | 1;
                }
                else if (ch == '0')
                {
                    value = (value << 1);
                }
                else
                {
                    break;
                }
            }
            break;
        case '.':
            isFloat = true;
            break;
        default:
            break;
        }
    }
    else
    {
        while(ch >= '0' && ch <= '9')
        {
            value = value * 10 + (ch - '0');
            ch = getChar();
        }

        isFloat = ch == '.';
    }
    
    if (isFloat)
    {
        Integer num = 0;
        Integer den = 1;

        ch = getChar();
        while (ch >= '0' && ch <= '9')
        {
            num = num * 10 + (ch - '0');
            den *= 10;
            ch = getChar();
        }
        
        int expSign = 1;
        if(ch == 'e' || ch == 'E')
        {
            ch = getChar();
            if(ch == '-')
            {
                expSign = -1;
                ch = getChar();
            }
            else if(ch == '+')
            {
                ch = getChar();
            }
            else if (ch < '0' || ch > '9')
            {
                return onError(RC_INVALID_NUMBER);
            }
            
            while (ch >= '0' && ch <= '9')
            {
                exponent = exponent * 10 + (ch - '0');
                ch = getChar();
            }
        }

        real = value + (double)num / den;
        if (exponent != 0)
        {
            real *= pow(10.0, exponent * expSign);
        }
    }
    
    if (ch != ',' && ch != ']' && ch != '}')
    {
        return onError(RC_INVALID_NUMBER);
    }
    ungetChar(ch);
    
    if(isFloat)
    {
        real *= sign;
        node = real;
    }
    else
    {
        value *= sign;
        node = value;
    }
    return true;
}

bool Parser::parseString(Node &node)
{
    stringBuffer_.clear();
    while (1)
    {
        char ch = getChar();
        if (ch == 0 || ch == '\n')
        {
            return onError(RC_INVALID_STRING);
        }
        else if (ch == '"')
        {
            break;
        }
        else if (ch == '\\')
        {
            ch = getChar();
            if (ch == 'x')
            {
                int codepoint = 0;
                for (size_t i = 0; i < 2; ++i)
                {
                    int v = toHex(getChar());
                    if (v < 0)
                    {
                        return onError(RC_INVALID_CHAR);
                    }
                    codepoint = (codepoint << 4) | v;
                }
                stringBuffer_.push_back((char)codepoint);
            }
            else if (ch == 'u')
            {
                parseUnicodeChar();
            }
            else
            {
                ch = translateChar(ch);
                stringBuffer_.push_back(ch);
            }
        }
        else
        {
            stringBuffer_.push_back(ch);
        }
    }

    node = allocator_->createString(stringBuffer_.data(), stringBuffer_.size(), BT_MAKE_COPY);
    return true;
}

bool Parser::parseTrue(Node &node)
{
    if (getChar() == 'r' &&
        getChar() == 'u' &&
        getChar() == 'e')
    {
        node = true;
        return true;
    }
    return onError(RC_INVALID_TRUE);
}

bool Parser::parseFalse(Node &node)
{
    if (getChar() == 'a' &&
        getChar() == 'l' &&
        getChar() == 's' &&
        getChar() == 'e')
    {
        node = false;
        return true;
    }
    return onError(RC_INVALID_FALSE);
}

bool Parser::parseNull(Node &node)
{
    if (getChar() == 'u' &&
        getChar() == 'l' &&
        getChar() == 'l')
    {
        node.setNull();
        return true;
    }
    return onError(RC_INVALID_NULL);
}

bool Parser::parseComment()
{
    int nextCh = aheadToken();
    if (nextCh == '/')
    {
        if (!parseLineComment())
        {
            return false;
        }
    }
    else if (nextCh == '*')
    {
        if (!parseLongComment())
        {
            return false;
        }
    }
    else
    {
        return onError(RC_INVALID_COMMENT);
    }
    return true;
}

bool Parser::parseLineComment()
{
    char ch;
    do
    {
        ch = getChar();
    } while (ch != 0 && ch != '\n');
    return true;
}

bool Parser::parseLongComment()
{
    while (true)
    {
        char ch = getChar();
        if (ch == 0)
        {
            return onError(RC_INVALID_COMMENT);
        }

        if (ch == '*')
        {
            ch = getChar();
            if (ch == '/')
            {
                return true;
            }
        }
    }
    return true;
}

static inline void unicodeCharToUTF8(std::vector<char> &buffer, unsigned int cp)
{
    // based on description from http://en.wikipedia.org/wiki/UTF-8
    if (cp <= 0x7f)
    {
        buffer.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7FF)
    {
        buffer.push_back(static_cast<char>(0xC0 | (0x1f & (cp >> 6))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
    else if (cp <= 0xFFFF)
    {
        buffer.push_back(static_cast<char>(0xE0 | (0xf & (cp >> 12))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 6))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
    else if (cp <= 0x10FFFF)
    {
        buffer.push_back(static_cast<char>(0xF0 | (0x7 & (cp >> 18))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 12))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 6))));
        buffer.push_back(static_cast<char>(0x80 | (0x3f & cp)));
    }
}

bool Parser::parseUnicodeChar()
{
    unsigned int code = 0;
    for (size_t index = 0; index < 4; ++index)
    {
        int v = toHex(getChar());
        if (v < 0)
        {
            return onError(RC_INVALID_UNICODE);
        }
        code = (code << 4) | (unsigned int)v;
    }
    unsigned int unicode = code;

    if (code >= 0xD800 && code <= 0xDBFF)
    {
        if (getChar() == '\\' && getChar() == 'u')
        {
            unsigned int code = 0;
            for (size_t index = 0; index < 4; ++index)
            {
                int v = toHex(getChar());
                if (v < 0)
                {
                    return onError(RC_INVALID_UNICODE);
                }
                code = (code << 4) | (unsigned int)v;
            }

            unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (code & 0x3FF);
        }
        else
        {
            return onError(RC_INVALID_UNICODE);
        }
    }

    unicodeCharToUTF8(stringBuffer_, unicode);
    return true;
}

NS_SMARTJSON_END
