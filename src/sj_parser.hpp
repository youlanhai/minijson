#pragma once
#include "sj_node.hpp"
#include <istream>

NS_SMARTJSON_BEGIN

/** json parser.
 *  bnf: http://www.json.org/json-zh.html
 *  NOTICE: The format '\uxxxx' doesn't supported.
 */
class Parser
{
public:
    explicit Parser(IAllocator *allocator = 0);
    ~Parser();
    
    // return one of `ResultCode`.
    bool parseFromFile(const char *fileName);
    bool parseFromData(const char *str, size_t length);
    bool parseFromStream(std::istream &stream);
    
    Node getRoot() const{ return root_; }
    int getErrorCode() const { return errorCode_; }
    int getLine() const { return line_; }
    int getColumn() const { return column_; }

private:
    char getChar();
    void ungetChar(char ch);
    char translateChar(char ch);

    int parseToken();
    int nextToken();
    int aheadToken();
    
    bool parseDict(Node &node);
    bool parseArray(Node &node);
    bool parseNumber(Node &node, char ch);
    bool parseString(Node &node);
    bool parseTrue(Node &node);
    bool parseFalse(Node &node);
    bool parseNull(Node &node);
    bool parseValue(Node &node);

    bool parseComment();
    bool parseLineComment();
    bool parseLongComment();
    
    bool parseUnicodeChar();

    bool onError(int code);
    
    Node            root_;
    IAllocator*     allocator_;
    std::istream*   stream_;
    std::vector<char> stringBuffer_;
    int 			errorCode_;
    int             line_;
    int             column_;
    int             nextToken_;
};

NS_SMARTJSON_END
